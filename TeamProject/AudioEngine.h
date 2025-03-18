#pragma once

//Link of tutorial followed https://codyclaborn.me/tutorials/making-a-basic-fmod-audio-engine-in-c/#implementation-source

#ifndef _AUDIO_ENGINE_H_
#define _AUDIO_ENGINE_H_

#include "Camera.h"

#include "fmod_studio.hpp"
#include <fmod.hpp>
#include <string>
#include <map>
#include <vector>
#include <math.h>
#include <iostream>

struct Implementation {
	Implementation();
	~Implementation();

	void Update(NCL::Camera* camera);

	FMOD::Studio::System* mpStudioSystem;
	FMOD::System* mpSystem;

	int mnNextChannelId;

	typedef std::map<std::string, FMOD::Sound*> SoundMap;
	typedef std::map<int, FMOD::Channel*> ChannelMap;
	typedef std::map<std::string, FMOD::Studio::EventInstance*> EventMap;
	typedef std::map<std::string, FMOD::Studio::Bank*> BankMap;

	BankMap mBanks;
	EventMap mEvents;
	SoundMap mSounds;
	ChannelMap mChannels;
};

/*class CAudioEngine {
public:
    static void Init();
    static void Update();
    static void Shutdown();
    static int ErrorCheck(FMOD_RESULT result);

    void LoadBank(const string& strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags);
    void LoadEvent(const string& strEventName);
    void Loadsound(const string& strSoundName, bool b3d = true, bool bLooping = false, bool bStream = false);
    void UnLoadSound(const string& strSoundName);
    void Set3dListenerAndOrientation(const Vector3& vPos = Vector3{ 0, 0, 0 }, float fVolumedB = 0.0f);
    void PlaySound(const string& strSoundName, const Vector3& vPos = Vector3{ 0, 0, 0 }, float fVolumedB = 0.0f);
    void PlayEvent(const string& strEventName);
    void StopChannel(int nChannelId);
    void StopEvent(const string& strEventName, bool bImmediate = false);
    void GeteventParameter(const string& strEventName, const string& strEventParameter, float* parameter);
    void SetEventParameter(const string& strEventName, const string& strParameterName, float fValue);
    void StopAllChannels();
    void SetChannel3dPosition(int nChannelId, const Vector3& vPosition);
    void SetChannelvolume(int nChannelId, float fVolumedB);
    bool IsPlaying(int nChannelId) const;
    bool IsEventPlaying(const string& strEventName) const;
    float dbToVolume(float db);
    float VolumeTodb(float volume);
    FMOD_VECTOR VectorToFmod(const Vector3& vPosition);
};*/

class CAudioEngine {
public:
	static void Init();
	static void Update(NCL::Camera* camera);
	static void Shutdown();
	static int ErrorCheck(FMOD_RESULT result);

	void LoadBank(const std::string& strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags);
	void LoadEvent(const std::string& strEventName);
	void LoadSound(const std::string& strSoundName, bool b3d = true, bool bLooping = false, bool bStream = false);
	void UnLoadSound(const std::string& strSoundName);
	void Set3dListenerAndOrientation(const NCL::Maths::Vector3& vPosition, const NCL::Maths::Vector3& vLook, const NCL::Maths::Vector3& vUp);
	int PlaySounds(const std::string& strSoundName, const NCL::Maths::Vector3& vPos = NCL::Maths::Vector3{ 0, 0, 0 }, float fVolumedB = 0.0f);
	void PlayEvent(const std::string& strEventName);
	void StopChannel(int nChannelId);
	void StopEvent(const std::string& strEventName, bool bImmediate = false);
	void GetEventParameter(const std::string& strEventName, const std::string& strEventParameter, float* parameter);
	void SetEventParameter(const std::string& strEventName, const std::string& strParameterName, float fValue);
	void StopAllChannels();
	void SetChannel3dPosition(int nChannelId, const NCL::Maths::Vector3& vPosition);
	void SetChannelPlaybackPosition(int channelId, unsigned int positionMs);
	void SetChannelVolume(int nChannelId, float fVolumedB);
	bool IsPlaying(int nChannelId) const;
	bool IsEventPlaying(const std::string& strEventName) const;
	float dbToVolume(float dB);
	float VolumeTodB(float volume);
	static FMOD_VECTOR VectorToFmod(const NCL::Maths::Vector3& vPosition);
};

//Global instance of the audio engine
extern CAudioEngine audioEngine;


#endif















