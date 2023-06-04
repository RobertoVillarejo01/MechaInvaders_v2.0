#include "Audio.h"
#include "Objects/GameObject.h"
#include "Graphics/RenderManager/RenderManager.h"
#include "../Engine/src/Utilities/Input/Input.h"

AudioManager::~AudioManager()
{
	Shutdown();
}

void AudioManager::Initialize()
{
	if (FMOD::System_Create(&mpSystem) != FMOD_OK)
		return;

	if (ErrorCheck(mpSystem->init(MAX_VOICES, FMOD_INIT_NORMAL, 0)) == 1)
	{
		mpSystem->release();
		mpSystem = nullptr;
		return;
	}
}

void AudioManager::Update()
{
	if (!mpSystem)
		return;

	//if (KeyTriggered(Key::K))
	//	PauseAll(false);
	//if (KeyTriggered(Key::L))
	//	PauseAll(true);

	FMOD_VECTOR forward;
	forward.x = -1; forward.y = 0; forward.z = 0;
	FMOD_VECTOR up;
	up.x = 0; up.y = 1; up.z = 0;

	auto& spaces = Scene.GetSpaces();
	std::for_each(spaces.begin(), spaces.end(), [&](auto& it)
		{
			auto& sound_listeners = it->GetComponentsType<SoundListener>();
			std::for_each(sound_listeners.begin(), sound_listeners.end(), [&](IComp* sl)
				{
					const auto& listener = static_cast<SoundListener*>(sl);
					//listener pos
					FMOD_VECTOR lispos;
					lispos.x = listener->mOwner->mTransform.mPosition.x; lispos.y = listener->mOwner->mTransform.mPosition.y; lispos.z = listener->mOwner->mTransform.mPosition.z;
					//listener vel
					Rigidbody* rb = listener->mOwner->GetComponentType<Rigidbody>();
					if(!rb)
						mpSystem->set3DListenerAttributes(listener->listenerID, &lispos, nullptr, &forward, &up);
					else
					{
						FMOD_VECTOR lisvel;
						lisvel.x = rb->mVelocity.x; lisvel.y = rb->mVelocity.y; lisvel.z = rb->mVelocity.z;
						mpSystem->set3DListenerAttributes(listener->listenerID, &lispos, &lisvel, &forward, &up);
					}

					auto& sound_emmiters = it->GetComponentsType<SoundEmitter>();
					std::for_each(sound_emmiters.begin(), sound_emmiters.end(), [&](IComp* se)
						{
							const auto& emmiter = static_cast<SoundEmitter*>(se);
							//emiter pos
							FMOD_VECTOR empos;
							empos.x = emmiter->mOwner->mTransform.mPosition.x; empos.y = emmiter->mOwner->mTransform.mPosition.y; empos.z = emmiter->mOwner->mTransform.mPosition.z;
							//emmiter vel
							FMOD_VECTOR emvel;
							Rigidbody* rb = emmiter->mOwner->GetComponentType<Rigidbody>();
							if (rb)
							{
								emvel.x = rb->mVelocity.x; emvel.y = rb->mVelocity.y; emvel.z = rb->mVelocity.z;
							}

							float distance = glm::distance(emmiter->mOwner->mTransform.mPosition, listener->mOwner->mTransform.mPosition);

							for (auto voice : emmiter->voices)
							{
								if (!voice)
									continue;
								if (voice->global)
									voice->channel->setVolume(voice->volume * MusicVolume);
								else
								{
									float value = glm::clamp(distance, listener->min_distance, listener->max_distance);
									voice->channel->setVolume((1 - (value - listener->min_distance) / (listener->max_distance - (listener->min_distance))) * voice->volume * SFXVolume);
								}
							}
						});
				});
		});

	//Update channels, if it is no longer valid send it to free channels
	bool playing = true;
	for (auto it = inUseVoices.begin(); it != inUseVoices.end(); ++it)
	{
		(*it)->channel->isPlaying(&playing);
		if (!playing)
		{
			freeVoices.push_back(*it);
			it = inUseVoices.erase(it);
			if (it == inUseVoices.end())
				break;
		}
	}
	mpSystem->update();
}

void AudioManager::Shutdown()
{
	if (!mpSystem)
		return;
	FreeVoices();
	soundnames.clear();
	mpSystem->release();
}

int AudioManager::ErrorCheck(FMOD_RESULT result)
{
	if (result != FMOD_OK) 
	{
		return 1;
	}
	return 0;
}

FMOD::Sound* AudioManager::LoadSound(const std::string& strSoundName)
{
	if (!mpSystem)
		return nullptr;

	auto it = soundmap.find(strSoundName);
	if (it != soundmap.end())
		return nullptr;

	FMOD::Sound* pSound = nullptr;
	ErrorCheck(mpSystem->createSound(strSoundName.c_str(), FMOD_LOOP_NORMAL | FMOD_3D, nullptr, &pSound));

	if (pSound) {
		soundmap[strSoundName] = pSound;
		soundnames.push_back(strSoundName);
		return pSound;
	}

	return nullptr;
}

void AudioManager::UnLoadSound(const std::string& strSoundName)
{
	if (!mpSystem)
		return;

	auto it = soundmap.find(strSoundName);
	if (it == soundmap.end())
		return;

	it->second->release();
	soundmap.erase(it);
}

Voice* AudioManager::Play(const std::string& strSoundName, float _vol, bool _pause, bool _loop, bool _global)
{
	if (!mpSystem)
		return nullptr;

	auto it = soundmap.find(strSoundName);
	if (it == soundmap.end())
	{
		LoadSound(strSoundName);
		it = soundmap.find(strSoundName);
		if (it == soundmap.end())
			return nullptr;
	}

	Voice* voice = GetVoices(_vol, _pause, _loop, _global);
	mpSystem->playSound(it->second, nullptr, _pause, &voice->channel);
	if (!_loop && voice && voice->channel)
	/*	voice->channel->setMode(FMOD_LOOP_NORMAL);
	else*/
		voice->channel->setMode(FMOD_LOOP_OFF);
	inUseVoices.push_back(voice);
	return voice;
}

void AudioManager::StopAll()
{
	if (!mpSystem)
		return;
	while (inUseVoices.size())
	{
		inUseVoices.back()->channel->stop();
		freeVoices.push_back(inUseVoices.back());
		inUseVoices.pop_back();
	}
	inUseVoices.clear();
}

void AudioManager::PauseAll(bool set)
{
	for (auto it = inUseVoices.begin(); it != inUseVoices.end(); ++it)
	{
		(*it)->channel->setPaused(set);
		(*it)->pause = set;
	}
}

void AudioManager::FreeVoices()
{
	StopAll();
	while (freeVoices.size())
	{
		delete freeVoices.back();
		freeVoices.pop_back();
	}
}

Voice* AudioManager::GetVoices(float _vol, bool _pause,	bool _loop, bool _global)
{
	Voice* voice = nullptr;
	if (freeVoices.size() == 0)
	{
		voice = new Voice(_vol, _loop, _pause, _global);
		return voice;
	}
	else
	{
		voice = freeVoices.back();
		freeVoices.pop_back();
		voice->volume = _vol;
		voice->loop = _loop;
		voice->pause = _pause;
		voice->global = _global;
		return voice;
	}
}