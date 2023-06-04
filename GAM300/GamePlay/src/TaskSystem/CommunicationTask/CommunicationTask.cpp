#include "CommunicationTask.h"
#include "TaskSystem/TaskSystem.h"
#include "TaskSystem/TaskInfo.h"
#include "../Engine/src/Objects/GameObject.h"
#include "../Engine/src/Utilities/FrameRateController/FrameRateController.h"
#include "../Engine/src/Graphics/ParticleSystem/Particle.h"
#include "../Engine/src/Graphics/Lighting/Light.h"
#include "../Engine/src/Serializer/Factory.h"
#include "AudioManager/Audio.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR

void CommunicationTask::StateMachineInitialize()
{
	SetBrainSize(SMTypes::TOTAL_SM);

	AddState(&CommunicationTask::IdleInit, &CommunicationTask::IdleUpdate, nullptr, "Idle", SMTypes::INTERACTION_SM);
	SetMainState("Idle", SMTypes::INTERACTION_SM);
	AddState(&CommunicationTask::ActivatedInit, &CommunicationTask::ActivatedUpdate, &CommunicationTask::ActivatedShutDown, "Activated", SMTypes::INTERACTION_SM);
	
	AddState(nullptr, &CommunicationTask::ColorUpdate, nullptr, "Idle", SMTypes::COLOR_SM);
	SetMainState("Idle", SMTypes::COLOR_SM);

	info = mOwner->GetComponentType<TaskInfo>();
}

void CommunicationTask::ToJson(nlohmann::json& j) const
{
	j["num_rot90"] << num_rot90;
	j["num_rot180"] << num_rot180;
}
void CommunicationTask::FromJson(nlohmann::json& j)
{
	if (j.find("num_rot90") != j.end())
		j["num_rot90"] >> num_rot90;
	if (j.find("num_rot180") != j.end())
		j["num_rot180"] >> num_rot180;
}

#ifdef EDITOR
bool CommunicationTask::Edit()
{
	ImGui::Checkbox("Rotate the number 90 degrees", &num_rot90);
	ImGui::Checkbox("Rotate the number 180 degrees", &num_rot180);
	return false;
}
#endif // EDITOR

void CommunicationTask::ResetTask()
{
	if (info)
	{
		info->ResetInfo();
		info->correct = false;
		info->interacted = false;

		if (info->warning)
			info->warning->GetComponentType<IRenderable>()->SetVisible(false);
		if (info->mParticles)
		{
			info->mParticles->SetActive(false);
			info->mParticles->Reset();
		}
	}
	TaskSys.ClearActiveTasks();
}

//---------IDLE-----------
void CommunicationTask::IdleInit()
{
}

void CommunicationTask::IdleUpdate()
{
	if (info)
	{
		if (info->IsActivated())
			ChangeState("Activated", SMTypes::INTERACTION_SM);
	}
}

//-------ACTIVATED-------
void CommunicationTask::ActivatedInit()
{
	if (emitter && info->position == 1)
		emitter->PlayCue("./../Resources/Audio/Scifi_AlarmLoop2.wav", 0.4f, false, true, true);
	if (info->mParticles)
		info->mParticles->SetActive(true);
	if (info->warning)
		info->warning->GetComponentType<IRenderable>()->SetVisible(true);
	switch (info->position)
	{
		case 1:
			number = Scene.CreateObject();
			serializer.LoadArchetype("Number1", number);
			number->mTransform.mPosition = mOwner->mTransform.mPosition;
			number_rend = number->GetComponentType<IRenderable>();
			break;
		case 2:
			number = Scene.CreateObject();
			serializer.LoadArchetype("Number2", number);
			number->mTransform.mPosition = mOwner->mTransform.mPosition;
			number_rend = number->GetComponentType<IRenderable>();
			break;
		case 3:
			number = Scene.CreateObject();
			serializer.LoadArchetype("Number3", number);
			number->mTransform.mPosition = mOwner->mTransform.mPosition;
			number_rend = number->GetComponentType<IRenderable>();
			break;
	}
	if (number && num_rot90)
		number->mTransform.RotateAround(number->mTransform.mUpVect, 90);
	if (number && num_rot180)
		number->mTransform.RotateAround(number->mTransform.mUpVect, 180);

	TaskSys.electricity = false;
}

void CommunicationTask::ActivatedUpdate()
{
	AllCorrect();
	//logic for first position
	if (info->position == 1)
	{
		if (info->interacted)
		{
			info->correct = true;
			info->interacted = false;
		}
	}
	//logic for second position
	if (info->position == 2)
	{
		if (info->interacted)
		{
			if (TaskSys.activetasks[0]->correct)
				info->correct = true;
			else
				ResetAll();
			info->interacted = false;
		}
	}
	//logic for third position
	if (info->position == 3)
	{
		if (info->interacted)
		{
			if (TaskSys.activetasks[0]->correct && TaskSys.activetasks[1]->correct)
				info->correct = true;
			else
				ResetAll();
			info->interacted = false;
		}
	}

	if (info->IsFinished())
		ChangeState("Idle", SMTypes::INTERACTION_SM);
}

void CommunicationTask::ActivatedShutDown()
{
	if (emitter && info->position == 1)
		emitter->Stop();
	if (number)
	{
		Scene.DestroyObject(number);
		number = nullptr;
		number_rend = nullptr;
	}
	ResetTask();
	TaskSys.electricity = true;
}

void CommunicationTask::ColorUpdate()
{
	if (number_rend)
	{
		if (info->correct)
			number_rend->SetColor(glm::vec4(0, 1, 0, 1));
		else
			number_rend->SetColor(glm::vec4(1, 0, 0, 1));
	}
		
}

void CommunicationTask::ResetAll()
{
	for (auto task : TaskSys.activetasks)
	{
		task->correct = false;
	}
}

void CommunicationTask::AllCorrect()
{
	for (auto task : TaskSys.activetasks)
	{
		if (!task->correct)
			return;
	}
	for (auto task : TaskSys.activetasks)
		task->EndTask();
}