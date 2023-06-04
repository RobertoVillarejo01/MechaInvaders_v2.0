#include "TaskInfo.h"
#include "Serializer/Factory.h"
#include "Graphics/ParticleSystem/Particle.h"
#include "AudioManager/Audio.h"
#include "Interaction/InteractionComp.h"
#include "Utilities/Utils.h"
#include "TaskSystem/TaskSystem.h"
#include "Networking\Networking.h"


void TaskInfo::Initialize()
{
	if(mOwner->GetComponentType<ParticleSystem>())
		mParticles = mOwner->GetComponentType<ParticleSystem>();
	letter = Scene.FindObject("InteractText");
	InteractComp = mOwner->GetComponentType<InteractionComp>();
}

void TaskInfo::Update()
{
	if (!warning_created && !warning)
	{
		warning = Scene.CreateObject();
		serializer.LoadArchetype("Warning", warning);
		warning->mTransform.mPosition = mOwner->mTransform.mPosition;
		warning->mTransform.mPosition.y = mOwner->mTransform.mPosition.y + offset_warning;
		warning_created = true;
	}

	if (tag == TaskTag::Fixing)
	{
		if (!finished)
		{
			if (InteractComp->mbInteracting)
			{
				interaction_timer += FRC.GetFrameTime();

				if (interaction_timer >= time_to_score)
					IncreaseProgress();

				InteractComp->SetBar(GetProgressPercentage());
			}
			if (progress >= maxprogress)
			{
				interaction_timer = 0.0f;
				InteractComp->StopInteracting();
				finished = true;

				if(NetworkingMrg.AmIServer())
					gAudioMgr.Play("../Resources/Audio/Vocoder_SystemOnline03.wav", 1.0F, false, false, false);

				if (NetworkingMrg.AmIServer())
				{
					task_interaction d{ id, false };
					network_event e = NetworkingMrg.CreateNetEvent(0, event_type::task_interact);
					std::memcpy(e.payload.data(), reinterpret_cast<char*>(&d), sizeof(d));
					NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
				}
				else // (!serverItIsI)
				{
					task_interaction d{ id, false };
					network_event e = NetworkingMrg.CreateNetEvent(NetworkingMrg.get_id(), event_type::task_interact_request);
					std::memcpy(e.payload.data(), reinterpret_cast<char*>(&d), sizeof(d));
					NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
				}
			}
		}
	}
	if (InteractComp->mbInteracting)
		interacted = true;
	/*else if (tag == TaskTag::Communication)
	{
		if (!finished)
		{
			
		}
	}*/

}

void TaskInfo::Shutdown()
{
	if (warning)
	{
		Scene.DestroyObject(warning);
		warning = nullptr;
		warning_created = false;
	}
}

IComp* TaskInfo::Clone() { return Scene.CreateComp<TaskInfo>(mOwner->GetSpace(), this); }

void TaskInfo::ToJson(nlohmann::json& j) const
{
	j["tag"] << static_cast<int>(tag);
	j["available"] << available;

//------------INTERACTIBLE TASKS--------------
	j["interactible"] << interactible;
	j["interaction_range"] << interaction_range;
	j["maxprogress"] << maxprogress;
	j["time_to_score"] << time_to_score;
	j["score_per_iteration"] << score_per_iteration;
//--------------------------------------------

	j["offset_warning"] << offset_warning;

	j["room"] << room;
}

void TaskInfo::FromJson(nlohmann::json& j)
{
	int temp = 0;
	if (j.find("tag") != j.end())
		j["tag"] >> temp;
	tag = static_cast<TaskTag>(temp);

	if (j.find("available") != j.end())
		j["available"] >> available;

//------------INTERACTIBLE TASKS--------------
	if (j.find("interactible") != j.end())
		j["interactible"] >> interactible;
	if (j.find("interaction_range") != j.end())
		j["interaction_range"] >> interaction_range;
	if (j.find("maxprogress") != j.end())
		j["maxprogress"] >> maxprogress;
	if (j.find("time_to_score") != j.end())
		j["time_to_score"] >> time_to_score;
	if (j.find("score_per_iteration") != j.end())
		j["score_per_iteration"] >> score_per_iteration;
//--------------------------------------------

	if (j.find("offset_warning") != j.end())
		j["offset_warning"] >> offset_warning;

	if (j.find("room") != j.end())
		j["room"] >> room;
}

#ifdef EDITOR
bool TaskInfo::Edit()
{
	const char* Tag[] = { "None", "Fixing", "Communication"};
	if (ImGui::BeginCombo(" Tags", Tag[static_cast<int>(tag)]))
	{
		if (ImGui::Selectable(" - None - "))
			tag = TaskTag::None;
		if (ImGui::Selectable(" - Fixing Task - "))
			tag = TaskTag::Fixing;
		if (ImGui::Selectable(" - Communication Task - "))
			tag = TaskTag::Communication;

		ImGui::EndCombo();
	}

	ImGui::Checkbox("Available ", &available);
	ImGui::DragFloat("Offset of the warning:", &offset_warning);


//------------INTERACTIBLE TASKS--------------
	ImGui::Checkbox("Interactible task? ", &interactible);
	ImGui::DragFloat("Interaction Range:", &interaction_range);
	ImGui::DragFloat("Maximum progress:", &maxprogress);
	ImGui::DragFloat("Time to update progress interaction:", &time_to_score);
	ImGui::DragFloat("Scoring points per interaction update:", &score_per_iteration);
//--------------------------------------------

	ImGui::DragInt("Room: ", &room);

	return false;
}
#endif

void TaskInfo::ResetInfo()
{
	active = false;
	finished = false;
	progress = 0.0f;
	player_in_place = false;
}

void TaskInfo::Activate(bool onoff)
{ 
	active = onoff; 
	InteractComp->Activate(onoff);
}

void TaskInfo::MakeAvailable()
{
	if (available)
		return;
	available = true;
	if (tag == TaskTag::Fixing)
		TaskSys.fixingtasks.push_back(this);
	if (tag == TaskTag::Communication)
		TaskSys.communicationtasks.push_back(this);
	TaskSys.tasks.insert({ id, this });
}
