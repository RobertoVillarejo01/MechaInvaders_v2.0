#include "Audio.h"
#include "Objects/GameObject.h"
#include "System/Scene/SceneSystem.h"
#include "Graphics/RenderManager/RenderManager.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#include "../Editor/src/Editor.h"
#endif // EDITOR


void SoundEmitter::Initialize()
{
	if (!default_sound.empty() && !once)
	{
#ifdef EDITOR
		if (!EditorMgr.mbInEditor)
		{
#endif // EDITOR

			PlayCue(default_sound, default_volume, default_pause, default_loop, default_global);
			once = true;

#ifdef EDITOR
		}
#endif // EDITOR

	}
}

void SoundEmitter::Update()
{
	bool playing = true;
	for (auto it = voices.begin(); it != voices.end(); ++it)
	{
		(*it)->channel->isPlaying(&playing);
		if (!playing)
			it = voices.erase(it);
	}
}

void SoundEmitter::PlayCue(const std::string& srtSoundName, float _vol, bool _pause, bool _loop, bool _global)
{
	Voice* voice = nullptr;
	voice = gAudioMgr.Play(srtSoundName, _vol, _pause, _loop, _global);
	auto modifer = _global ? gAudioMgr.MusicVolume : gAudioMgr.SFXVolume;
	if(voice && voice->channel)
	voice->channel->setVolume(_vol * modifer);
	voices.push_back(voice);
}

void SoundEmitter::Stop()
{
	for (auto v : voices)
	{
		if(v) v->channel->stop();
	}
	voices.clear();
}

IComp * SoundEmitter::Clone() { return Scene.CreateComp<SoundEmitter>(mOwner->GetSpace() ,this); }

#ifdef EDITOR
bool SoundEmitter::Edit()
{
	bool changed = false;
	std::string current_name = default_sound.empty() ? "None" : default_sound;

	if (ImGui::BeginCombo(" Default Sound", current_name.c_str()))
	{
		if (ImGui::Selectable(" - None - "))
		{
			default_sound.clear();
			changed = true;
		}

		for (auto& sounds_names : gAudioMgr.soundnames)
		{
			if (ImGui::Selectable(sounds_names.c_str()))
			{
				default_sound = sounds_names;
				changed = true;
			}
		}
		ImGui::EndCombo();
	}

	ImGui::DragFloat("Volume", &default_volume, 0.1f, 0.f, 1.f);
	ImGui::Checkbox("Looping", &default_loop);
	ImGui::Checkbox("Pause", &default_pause);
	ImGui::Checkbox("Global", &default_global);

	return changed;
}
#endif


void SoundEmitter::FromJson(nlohmann::json& j)
{
	j["default_sound"] >> default_sound;
	j["default_volume"] >> default_volume;
	j["default_loop"] >> default_loop;
	j["default_pause"] >> default_pause;
	j["default_global"] >> default_global;
}
void SoundEmitter::ToJson(nlohmann::json& j) const
{
	j["default_sound"] << default_sound;
	j["default_volume"] << default_volume;
	j["default_loop"] << default_loop;
	j["default_pause"] << default_pause;
	j["default_global"] << default_global;
}

int SoundListener::listenercntr = 1;

IComp * SoundListener::Clone() { return Scene.CreateComp<SoundListener>(mOwner->GetSpace(), this); }

#ifdef EDITOR
bool SoundListener::Edit()
{
	ImGui::DragFloat("Maximum distance : ", &max_distance);
	ImGui::DragFloat("Minimum distance : ", &min_distance);

	return true;
}
#endif

void SoundListener::FromJson(nlohmann::json& j)
{
	j["listenerID"] >> listenerID;
	j["max_distance"] >> max_distance;
	j["min_distance"] >> min_distance;
}
void SoundListener::ToJson(nlohmann::json& j) const
{
	j["listenerID"] << listenerID;
	j["max_distance"] << max_distance;
	j["min_distance"] << min_distance;
}
