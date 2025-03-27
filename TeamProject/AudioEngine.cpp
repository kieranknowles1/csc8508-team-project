#include "AudioEngine.h"
#include "Assets.h"

#ifdef __PROSPERO__
#include <kernel.h>
#endif // __PROSPERO__


//Link of tutorial followed https://codyclaborn.me/tutorials/making-a-basic-fmod-audio-engine-in-c/#implementation-source
//With some custom functions written by Ameya

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
void Implementation::Update(NCL::Camera* camera) {
    std::erase_if(mChannels, [](const auto& channel) {
        bool playing = false;
        channel.second->isPlaying(&playing);
        return !playing;
    });

    auto position = CAudioEngine::VectorToFmod(camera->GetPosition());
    // TODO: Pass other fields
    mpSystem->set3DListenerAttributes(0, &position, nullptr, nullptr, nullptr);

    CAudioEngine::ErrorCheck(mpSystem->update());
    CAudioEngine::ErrorCheck(mpStudioSystem->update());
}

Implementation* sgpImplementation = nullptr;

//Initializing a global FMod implementation instance
void CAudioEngine::Init() {
    if (sgpImplementation != nullptr) {
        std::cerr << "Duplicate CAudioEngine::Init, ignoring" << std::endl;
        return;
    }
#if __PROSPERO__
    auto libFmod = sceKernelLoadStartModule("/app0/libfmod.prx", 0, nullptr, 0, nullptr, nullptr);
    auto libFmodStudio = sceKernelLoadStartModule("/app0/libfmodstudio.prx", 0, nullptr, 0, nullptr, nullptr);

    if (libFmod < 0 || libFmodStudio < 0) {
        std::cerr << "Failed to load fmod runtime" << std::endl;
        return;
    }

#endif

    sgpImplementation = new Implementation;
}

//Updates FMod system every frame to process audio events
void CAudioEngine::Update(NCL::Camera* camera) {
    sgpImplementation->Update(camera);
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
    std::string assetPath = NCL::Assets::AUDIODIR + strSoundName;
    CAudioEngine::ErrorCheck(sgpImplementation->mpSystem->createSound(assetPath.c_str(), eMode, nullptr, &pSound));
    if (pSound) {
        sgpImplementation->mSounds[strSoundName] = pSound;
    }
    if (b3d && pSound) {
        pSound->set3DMinMaxDistance(10.0f, 50.0f);
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
int CAudioEngine::PlaySounds(const std::string& strSoundName, const NCL::Maths::Vector3& vPosition, float fVolumedB) {
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

            CAudioEngine::ErrorCheck(pChannel->setMode(FMOD_3D | FMOD_3D_LINEARROLLOFF));
        }
        CAudioEngine::ErrorCheck(pChannel->setVolume(dbToVolume(fVolumedB)));
        CAudioEngine::ErrorCheck(pChannel->setPaused(false));
        sgpImplementation->mChannels[nChannelId] = pChannel;
    }
    return nChannelId;
}

void CAudioEngine::StopChannel(int nChannelId) {
    auto tFoundIt = sgpImplementation->mChannels.find(nChannelId);
    if (tFoundIt != sgpImplementation->mChannels.end()) {
        if (tFoundIt->second) {
            tFoundIt->second->stop(); // Stop the sound
        }
        sgpImplementation->mChannels.erase(tFoundIt); // Remove from active channels
    }
}

void CAudioEngine::StopAllNonUISounds() {
    for (auto& [channelId, channel] : sgpImplementation->mChannels) {
        if (channel) {
            std::string playingSound = GetSoundNameByChannel(channelId);
            if (playingSound != "MenuScroll.wav" && playingSound != "MenuSelect.wav") {
                channel->stop();
            }
        }
    }
}

std::string CAudioEngine::GetSoundNameByChannel(int nChannelId) {
    auto found = sgpImplementation->mChannels.find(nChannelId);
    if (found == sgpImplementation->mChannels.end()) {
        return "";  // Channel ID not found
    }

    FMOD::Sound* sound = nullptr;
    if (found->second->getCurrentSound(&sound) != FMOD_OK || !sound) {
        return "";  // No sound associated with this channel
    }

    char soundName[256];
    if (sound->getName(soundName, sizeof(soundName)) == FMOD_OK) {
        return std::string(soundName);
    }
    return "";
}

bool CAudioEngine::IsPlaying(int nChannelId) const {
    auto it = sgpImplementation->mChannels.find(nChannelId);
    if (it == sgpImplementation->mChannels.end()) return false;

    bool isPlaying = false;
    FMOD_RESULT result = it->second->isPlaying(&isPlaying);
    ErrorCheck(result);
    return isPlaying;
}

bool CAudioEngine::isPlayingByString(const std::string& soundName) {
    for (auto& [channelId, channel] : sgpImplementation->mChannels) {
        if (channel) {
            std::string playingSound = GetSoundNameByChannel(channelId);
            if (playingSound == soundName) {
                return true;  // The sound is currently playing
            }
        }
    }
    return false;
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

void CAudioEngine::SetChannel3dMinMaxDistance(int channelId, float minDist, float maxDist) {
    auto it = sgpImplementation->mChannels.find(channelId);
    if (it == sgpImplementation->mChannels.end()) return;

    CAudioEngine::ErrorCheck(it->second->set3DMinMaxDistance(minDist, maxDist));
}

void CAudioEngine::SetChannelPlaybackPosition(int channelId, unsigned int positionMs) {
    auto tFoundIt = sgpImplementation->mChannels.find(channelId);
    if (tFoundIt == sgpImplementation->mChannels.end()) {
        return;
    }

    FMOD_RESULT result = tFoundIt->second->setPosition(positionMs, FMOD_TIMEUNIT_MS);
    ErrorCheck(result);
}

void CAudioEngine::Set3dListenerAndOrientation(const NCL::Maths::Vector3& vPosition, const NCL::Maths::Vector3& vLook, const NCL::Maths::Vector3& vUp) {
    FMOD_VECTOR fmodPosition = VectorToFmod(vPosition);
    //FMOD_VECTOR fmodLook = VectorToFmod(vLook);
    //FMOD_VECTOR fmodUp = VectorToFmod(vUp);

    FMOD_VECTOR fmodForward = { vLook.x, vLook.y, vLook.z };
    FMOD_VECTOR fmodUp = { vUp.x, vUp.y, -vUp.z };

    FMOD_VECTOR fmodVelocity = { 0.0f, 0.0f, 0.0f }; // Velocity is usually needed for Doppler effect, but we don't have it here

    if (sgpImplementation && sgpImplementation->mpSystem) {
        sgpImplementation->mpSystem->set3DListenerAttributes(0, &fmodPosition, &fmodVelocity, &fmodForward, &fmodUp);
    }
}

//Sets the volume of an FMod channel
void CAudioEngine::SetChannelVolume(int nChannelId, float fVolumedB) {
    auto tFoundIt = sgpImplementation->mChannels.find(nChannelId);
    if (tFoundIt == sgpImplementation->mChannels.end()) {
        return;
    }

    CAudioEngine::ErrorCheck(tFoundIt->second->setVolume(dbToVolume(fVolumedB)));
}

//Sets the frequency of an FMod channel in Hz
void CAudioEngine::SetChannelFrequencyHz(int channelId, float frequency) {
    auto it = sgpImplementation->mChannels.find(channelId);
    if (it == sgpImplementation->mChannels.end()) return;

    FMOD_RESULT result = it->second->setFrequency(frequency);
    ErrorCheck(result);
}

//Sets the pitch of a channel using a relative multiplier (e.g., 1.1 = +10% pitch)
void CAudioEngine::SetChannelPitchMultiplier(int channelId, float multiplier) {
    auto it = sgpImplementation->mChannels.find(channelId);
    if (it == sgpImplementation->mChannels.end() || multiplier <= 0.0f) return;

    float currentFrequency = 0.0f;
    if (it->second->getFrequency(&currentFrequency) == FMOD_OK) {
        float newFrequency = currentFrequency * multiplier;
        ErrorCheck(it->second->setFrequency(newFrequency));
    }
}

void CAudioEngine::SetChannelPitch(int channelId, float pitch) {
    auto it = sgpImplementation->mChannels.find(channelId);
    if (it != sgpImplementation->mChannels.end() && it->second) {
        FMOD_RESULT result = it->second->setPitch(pitch);
        ErrorCheck(result);
    }
}

//Reduce volume of all non-essential sounds temporarily
void CAudioEngine::DuckVolume(float duckDb, float durationSec) {
    for (auto& [id, channel] : sgpImplementation->mChannels) {
        if (channel) {
            float currentVolume;
            channel->getVolume(&currentVolume);
            float ducked = dbToVolume(VolumeTodB(currentVolume) + duckDb); // duckDb is negative
            channel->setVolume(ducked);

            // Optional: store timer to restore after `durationSec` (implement async if needed)
        }
    }
}

void CAudioEngine::StopAllChannels() {
    for (auto& [channelId, channel] : sgpImplementation->mChannels) {
        channel->stop();
    }
    sgpImplementation->mChannels.clear();
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
void CAudioEngine::StopEvent(const std::string& strEventName, bool bImmediate) {
    auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
    if (tFoundIt == sgpImplementation->mEvents.end()) {
        return;
    }

    FMOD_STUDIO_STOP_MODE eMode;
    eMode = bImmediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
    CAudioEngine::ErrorCheck(tFoundIt->second->stop(eMode));
}

//Checks if an FMod event is currently playing
bool CAudioEngine::IsEventPlaying(const std::string& strEventName) const {
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

bool CAudioEngine::IsAnySoundPlaying() const {
    for (const auto & channel : sgpImplementation->mChannels) {
        bool isPlaying = false;
        if (channel.second->isPlaying(&isPlaying) == FMOD_OK && isPlaying) {
            return true; // At least one sound is playing
        }
    }
    return false; // No sounds are playing
}

//Retrieves the current value of an FMod event parameter
void CAudioEngine::GetEventParameter(const std::string& strEventName, const std::string& strParameterName, float* parameter) {
    auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
    if (tFoundIt == sgpImplementation->mEvents.end()) {
        return;
    }

    //ParameterInstance is deprecated, the following line is what I think should be the direct replacement
    CAudioEngine::ErrorCheck(tFoundIt->second->getParameterByName(strParameterName.c_str(), parameter));
}

//Sets the value of an FMod event parameter
void CAudioEngine::SetEventParameter(const std::string& strEventName, const std::string& strParameterName, float fValue) {
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
    fVec.z = -vPosition.z;

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

void CAudioEngine::SetChannelPaused(int channelId, bool paused) {
    auto tFoundIt = sgpImplementation->mChannels.find(channelId);
    if (tFoundIt == sgpImplementation->mChannels.end()) {
        return;
    }

    FMOD_RESULT result = tFoundIt->second->setPaused(paused);
    ErrorCheck(result);
}

void CAudioEngine::SetChannelVolumeRamp(int channelId, bool enable) {
    auto tFoundIt = sgpImplementation->mChannels.find(channelId);
    if (tFoundIt == sgpImplementation->mChannels.end()) {
        return;
    }

    FMOD_RESULT result = tFoundIt->second->setVolumeRamp(enable);
    ErrorCheck(result);
}

void CAudioEngine::SetChannelPitchRandom(int channelId, float minPitchScale, float maxPitchScale) {
    auto tFoundIt = sgpImplementation->mChannels.find(channelId);
    if (tFoundIt == sgpImplementation->mChannels.end()) {
        return;
    }

    float baseFrequency = 0.0f;
    if (tFoundIt->second->getFrequency(&baseFrequency) != FMOD_OK) {
        return;
    }

    // Generate a random pitch scale within the range
    float pitchScale = minPitchScale + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (maxPitchScale - minPitchScale)));

    float newFrequency = baseFrequency * pitchScale;
    tFoundIt->second->setFrequency(newFrequency);
}

void CAudioEngine::SetChannelMute(int channelId, bool mute) {
    auto tFoundIt = sgpImplementation->mChannels.find(channelId);
    if (tFoundIt == sgpImplementation->mChannels.end()) {
        return;
    }

    FMOD_RESULT result = tFoundIt->second->setMute(mute);
    ErrorCheck(result);
}

//Checks FMod function return values and logs errors if found
int CAudioEngine::ErrorCheck(FMOD_RESULT result) {
    if (result != FMOD_OK) {
        std::cout << "FMOD ERROR: " << result << std::endl;
        return 1;
    }
    //cout << "FMOD all good" << endl;

    return 0;
}

//Shuts down the audio engine and releases resources.
void CAudioEngine::Shutdown() {
    delete sgpImplementation;
    sgpImplementation = nullptr;
}