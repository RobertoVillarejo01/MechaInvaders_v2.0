#pragma once

//#include <string>
#include <list>
#include "Objects/Components.h"
#include "Objects/GameObject.h"
#include "Utilities/Singleton.h"
#include "System/Scene/SceneSystem.h"
#include <Fmod/fmod.hpp>

namespace FMOD
{
	class Sound;
	class Channel;
	class System;
}

class Voice
{
public:
	Voice(float _vol = 1.0f, bool _loop = false, bool _pause = false, bool _global = true) 
		: channel(nullptr), volume(_vol), loop(_loop), pause(_pause), global(_global) {}
	~Voice() {}

	FMOD::Channel*	channel;
	float			volume;
	bool			loop;	
	bool			pause;
	bool			global;

};

class SoundEmitter : public IComp
{
public:
	SoundEmitter() {}
	~SoundEmitter() {}

	void Initialize();
	void Update();
	void ShutDown() { voices.clear(); }

	void PlayCue(const std::string& srtSoundName, float _vol, bool _pause, bool _loop, bool _global);
	void Stop();

	IComp * Clone();

#ifdef EDITOR
	bool Edit();
#endif

	std::list<Voice*> voices;

	//defaut sound
	std::string		default_sound;
	float			default_volume = 1.0f;
	bool			default_loop = false;
	bool			default_pause = false;
	bool			default_global = true;

	bool			once = false;

protected:
	void FromJson(nlohmann::json& j);
	void ToJson(nlohmann::json& j) const;
};

class SoundListener : public IComp
{
public:
	SoundListener(float _max = 0.0f, float _min = 0.0f) : max_distance(_max), min_distance(_min), rate(max_distance - min_distance)
		{ listenerID = listenercntr; listenercntr += 1; }

	IComp * Clone();
	bool Edit();

	int	listenerID;
	float max_distance;
	float min_distance;
	float rate;

protected:
	void FromJson(nlohmann::json& j);
	void ToJson(nlohmann::json& j) const;

private:
	static int listenercntr;
};

class AudioManager
{
	MAKE_SINGLETON(AudioManager)

	friend class SoundEmitter;

public:
	~AudioManager();

	void Initialize();
	void Update();
	void Shutdown();

	int ErrorCheck(FMOD_RESULT result);

	FMOD::Sound* LoadSound(const std::string& strSoundName);
	void UnLoadSound(const std::string& strSoundName);
	Voice* Play(const std::string& strSoundName, float _vol = 1.0f, bool _pause = false, 
		bool _loop = false, bool _global = true);
	void StopAll();
	void PauseAll(bool set = true);
	void FreeVoices();

	FMOD::System * GetFMOD() { return mpSystem; }

	float SFXVolume = 0.5f;
	float MusicVolume = 0.5f;

	std::list<std::string>					soundnames;
private:
	Voice* GetVoices(float _vol = 1.0f, bool _pause = false,
		bool _loop = false, bool _global = true);

	FMOD::System*							mpSystem = nullptr;
	std::map<std::string, FMOD::Sound*>		soundmap;
	std::list<Voice*>						inUseVoices;
	std::list<Voice*>						freeVoices;
	int										MAX_VOICES = 100;
};

// GLOBAL Audio Manager
#define gAudioMgr (AudioManager::Instance())