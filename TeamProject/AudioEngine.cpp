#include "AudioEngine.h"

//Link of tutorial followed https://codyclaborn.me/tutorials/making-a-basic-fmod-audio-engine-in-c/#implementation-source

CAudioEngine audioEngine;

//Initialize FMod Studio and Core
Implementation::Implementation() {
    mpStudioSystem = NULL;
    CAudioEngine::ErrorCheck(FMOD::Studio::System::create(&mpStudioSystem));
    CAudioEngine::ErrorCheck(mpStudioSystem->initialize(32, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_PROFILE_ENABLE, NULL));

    mpSystem = NULL;
    CAudioEngine::ErrorCheck(mpStudioSystem->getCoreSystem(&mpSystem));
}

//Destructor
Implementation::~Implementation() {
    CAudioEngine::ErrorCheck(mpStudioSystem->unloadAll());
    CAudioEngine::ErrorCheck(mpStudioSystem->release());

    CAudioEngine::ErrorCheck(mpSystem->release()); //This line Ameya added
}

//Updates FMod system and removes stopped channels from tracking.
void Implementation::Update() {
    vector<ChannelMap::iterator> pStoppedChannels;
    for (auto it = mChannels.begin(), itEnd = mChannels.end(); it != itEnd; ++it) {
        bool bIsPlaying = false;
        it->second->isPlaying(&bIsPlaying);
        if (!bIsPlaying) 
        {
            pStoppedChannels.push_back(it);
        }
    }
    for (auto& it : pStoppedChannels)
    {
        mChannels.erase(it);
    }
    CAudioEngine::ErrorCheck(mpStudioSystem->update());
}

Implementation* sgpImplementation = nullptr;

//Initializing a global FMod implementation instance
void CAudioEngine::Init() {
    sgpImplementation = new Implementation;
}

//Updates FMod system every frame to process audio events
void CAudioEngine::Update() {
    sgpImplementation->Update();
}

//Loads a sound file into FMod with it's properties
void CAudioEngine::LoadSound(const std::string& strSoundName, bool b3d, bool bLooping, bool bStream) {
    auto tFoundIt = sgpImplementation->mSounds.find(strSoundName);
    if (tFoundIt != sgpImplementation->mSounds.end()) {
        return;
    }
    FMOD_MODE eMode = FMOD_DEFAULT;
    eMode |= b3d ? FMOD_3D : FMOD_2D;
    eMode |= bLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
    eMode |= bStream ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;

    FMOD::Sound* pSound = nullptr;
    CAudioEngine::ErrorCheck(sgpImplementation->mpSystem->createSound(strSoundName.c_str(), eMode, nullptr, &pSound));
    if (pSound) {
        sgpImplementation->mSounds[strSoundName] = pSound;
    }
}

//Unloads and releases a sound from memory
void CAudioEngine::UnLoadSound(const std::string& strSoundName) {
    auto tFoundIt = sgpImplementation->mSounds.find(strSoundName);
    if (tFoundIt == sgpImplementation->mSounds.end()) {
        return;
    }
    CAudioEngine::ErrorCheck(tFoundIt->second->release());
    sgpImplementation->mSounds.erase(tFoundIt);
}

//Plays a sound and assigns it to an available channel
int CAudioEngine::PlaySounds(const string& strSoundName, const NCL::Maths::Vector3& vPosition, float fVolumedB) {
    int nChannelId = sgpImplementation->mnNextChannelId++;
    auto tFoundIt = sgpImplementation->mSounds.find(strSoundName);

    if (tFoundIt == sgpImplementation->mSounds.end()) {
        LoadSound(strSoundName);
        tFoundIt = sgpImplementation->mSounds.find(strSoundName);
        if (tFoundIt == sgpImplementation->mSounds.end()) {
            return nChannelId;
        }
    }
    
    FMOD::Channel* pChannel = nullptr;
    CAudioEngine::ErrorCheck(sgpImplementation->mpSystem->playSound(tFoundIt->second, nullptr, true, &pChannel));
    if (pChannel) {
        FMOD_MODE currMode;
        tFoundIt->second->getMode(&currMode);
        if (currMode & FMOD_3D) {
            FMOD_VECTOR position = VectorToFmod(vPosition);
            CAudioEngine::ErrorCheck(pChannel->set3DAttributes(&position, nullptr));
        }
        CAudioEngine::ErrorCheck(pChannel->setVolume(dbToVolume(fVolumedB)));
        CAudioEngine::ErrorCheck(pChannel->setPaused(false));
        sgpImplementation->mChannels[nChannelId] = pChannel;
    }
    return nChannelId;
}

//Sets the 3D position of an FMod channel
void CAudioEngine::SetChannel3dPosition(int nChannelId, const NCL::Maths::Vector3& vPosition) {
    auto tFoundIt = sgpImplementation->mChannels.find(nChannelId);
    if (tFoundIt == sgpImplementation->mChannels.end()) {
        return;
    }

    FMOD_VECTOR position = VectorToFmod(vPosition);
    CAudioEngine::ErrorCheck(tFoundIt->second->set3DAttributes(&position, NULL));
}

//Sets the volume of an FMod channel
void CAudioEngine::SetChannelVolume(int nChannelId, float fVolumedB) {
    auto tFoundIt = sgpImplementation->mChannels.find(nChannelId);
    if (tFoundIt == sgpImplementation->mChannels.end()) {
        return;
    }

    CAudioEngine::ErrorCheck(tFoundIt->second->setVolume(dbToVolume(fVolumedB)));
}

//Loads an FMod bank file for event-based sounds
void CAudioEngine::LoadBank(const std::string& strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags) {
    auto tFoundIt = sgpImplementation->mBanks.find(strBankName);
    if (tFoundIt != sgpImplementation->mBanks.end()) {
        return;
    }

    FMOD::Studio::Bank* pBank;
    CAudioEngine::ErrorCheck(sgpImplementation->mpStudioSystem->loadBankFile(strBankName.c_str(), flags, &pBank));
    if (pBank) {
        sgpImplementation->mBanks[strBankName] = pBank;
    }
}

//Loads an FMod event for playback
void CAudioEngine::LoadEvent(const std::string& strEventName) {
    auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
    if (tFoundIt != sgpImplementation->mEvents.end()) {
        return;
    }

    FMOD::Studio::EventDescription* pEventDescription = NULL;
    CAudioEngine::ErrorCheck(sgpImplementation->mpStudioSystem->getEvent(strEventName.c_str(), &pEventDescription));
    if (pEventDescription) {
        FMOD::Studio::EventInstance* pEventInstance = NULL;
        CAudioEngine::ErrorCheck(pEventDescription->createInstance(&pEventInstance));
        if (pEventInstance) {
            sgpImplementation->mEvents[strEventName] = pEventInstance;
        }
    }
}

//Plays an FMod event
void CAudioEngine::PlayEvent(const std::string& strEventName) {
    auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
    if (tFoundIt == sgpImplementation->mEvents.end()) {
        LoadEvent(strEventName);
        tFoundIt = sgpImplementation->mEvents.find(strEventName);
        if (tFoundIt == sgpImplementation->mEvents.end()) {
            return;
        }
    }
    tFoundIt->second->start();
}

//Stops an FMod event instance.
void CAudioEngine::StopEvent(const string& strEventName, bool bImmediate) {
    auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
    if (tFoundIt == sgpImplementation->mEvents.end()) {
        return;
    }

    FMOD_STUDIO_STOP_MODE eMode;
    eMode = bImmediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
    CAudioEngine::ErrorCheck(tFoundIt->second->stop(eMode));
}

//Checks if an FMod event is currently playing
bool CAudioEngine::IsEventPlaying(const string& strEventName) const {
    auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
    if (tFoundIt == sgpImplementation->mEvents.end()) {
        return false;
    }

    FMOD_STUDIO_PLAYBACK_STATE* state = NULL;
    if (tFoundIt->second->getPlaybackState(state) == FMOD_STUDIO_PLAYBACK_PLAYING) {
        return true;
    }
    return false;
}

//Retrieves the current value of an FMod event parameter
void CAudioEngine::GetEventParameter(const string& strEventName, const string& strParameterName, float* parameter) {
    auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
    if (tFoundIt == sgpImplementation->mEvents.end()) {
        return;
    }

    //ParameterInstance is deprecated, the following line is what I think should be the direct replacement
    CAudioEngine::ErrorCheck(tFoundIt->second->getParameterByName(strParameterName.c_str(), parameter));
}

//Sets the value of an FMod event parameter
void CAudioEngine::SetEventParameter(const string& strEventName, const string& strParameterName, float fValue) {
    auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
    if (tFoundIt == sgpImplementation->mEvents.end()) {
        return;
    }

    //ParameterInstance is deprecated, the following line is what I think should be the direct replacement
    CAudioEngine::ErrorCheck(tFoundIt->second->setParameterByName(strParameterName.c_str(), fValue));
}

//Converts a custom Vector3 structure to FMOD_VECTOR format
FMOD_VECTOR CAudioEngine::VectorToFmod(const NCL::Maths::Vector3& vPosition) {
    FMOD_VECTOR fVec;
    fVec.x = vPosition.x;
    fVec.y = vPosition.y;
    fVec.z = vPosition.z;

    return fVec;
}

//Converts decibels to linear volume
float CAudioEngine::dbToVolume(float dB) {
    return powf(10.0f, 0.05f * dB);
}

//Converts linear volume to decibels
float CAudioEngine::VolumeTodB(float volume) {
    return 20.0f * log10f(volume);
}

//Checks FMod function return values and logs errors if found
int CAudioEngine::ErrorCheck(FMOD_RESULT result) {
    if (result != FMOD_OK) {
        cout << "FMOD ERROR: " << result << endl;
        return 1;
    }
    //cout << "FMOD all good" << endl;

    return 0;
}

//Shuts down the audio engine and releases resources.
void CAudioEngine::Shutdown() {
    delete sgpImplementation;
}