#include "FixingTask.h"
#include "TaskSystem/TaskSystem.h"
#include "TaskSystem/TaskInfo.h"
#include "WaveSystem/WaveSystem.h"
#include "../Engine/src/Objects/GameObject.h"
#include "../Engine/src/Utilities/FrameRateController/FrameRateController.h"
#include "Spawner/Spawner.h"
#include "../Engine/src/Graphics/ParticleSystem/Particle.h"
#include "Doors/Doors.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR

void FixingTask::StateMachineInitialize()
{
	SetBrainSize(SMTypes::TOTAL_SM);

	AddState(&FixingTask::IdleInit, &FixingTask::IdleUpdate, nullptr, "Idle", SMTypes::INTERACTION_SM);
	SetMainState("Idle", SMTypes::INTERACTION_SM);
	AddState(&FixingTask::ActivatedInit, &FixingTask::ActivatedUpdate, &FixingTask::ActivatedShutDown, "Activated", SMTypes::INTERACTION_SM);

	players = Scene.FindObjects(Tags::Player);
	info = mOwner->GetComponentType<TaskInfo>();

	auto doors_obj = mOwner->GetSpace()->FindObjects(Tags::Door);
	for (GameObject* obj : doors_obj)
	{
		auto door = obj->GetComponentType<Door>();
		if (door)
			doors.push_back(door);
	}
}

void FixingTask::ToJson(nlohmann::json& j) const
{
	j["spawner_amount"] << spawner_amount;
	j["order_per_iteration"] << order_per_iteration;
}
void FixingTask::FromJson(nlohmann::json& j)
{
	if (j.find("spawner_amount") != j.end())
		j["spawner_amount"] >> spawner_amount;
	if (j.find("order_per_iteration") != j.end())
		j["order_per_iteration"] >> order_per_iteration;
}

#ifdef EDITOR
bool FixingTask::Edit()
{
	ImGui::DragInt("Spawners to spawn by the task: ", &spawner_amount);
	ImGui::DragInt("Orders sent to spawner per iteration ", &order_per_iteration);

	return false;
}
#endif // EDITOR

void FixingTask::ResetTask()
{
	if (info)
	{
		info->ResetInfo();

		if (info->warning)
			info->warning->GetComponentType<IRenderable>()->SetVisible(false);
		if (info->mParticles)
		{
			info->mParticles->SetActive(false);
			info->mParticles->Reset();
		}
	}
	for (auto sp : spawners)
	{
		sp->ClearTQueue();
		sp->onTask = false;
	}
	TaskSys.ClearActiveTasks();
}

//---------IDLE-----------
void FixingTask::IdleInit()
{
}

void FixingTask::IdleUpdate()
{
	if (info)
	{
		if (info->IsActivated())
			ChangeState("Activated", SMTypes::INTERACTION_SM);
	}
}

//-------ACTIVATED-------
void FixingTask::ActivatedInit()
{
	if (spawners.size() == 0)
		spawners = WaveSys.ChooseNearSpawners(spawner_amount, mOwner->mTransform.mPosition);
	for (auto sp : spawners)
		sp->onTask = true; 
	if (info->mParticles)
		info->mParticles->SetActive(true);
	if (info->warning)
		info->warning->GetComponentType<IRenderable>()->SetVisible(true);
	TurnOffLights();

	//close all the doors
	for (Door* door : doors)
		if (door->mbOpen)
			door->TaskActivate();
}

void FixingTask::ActivatedUpdate()
{
	timer += FRC.GetFrameTime();
	if (timer >= order_time)
	{
		std::vector<Spawner*> temp = WaveSys.ChooseSomeSpawners(spawners, order_per_iteration);
		WaveSys.SendOrder(temp);
		timer = 0.0f;
	}
	if(info->IsFinished())
		ChangeState("Idle", SMTypes::INTERACTION_SM);
}

void FixingTask::ActivatedShutDown()
{
	TurnOnLights();
	ResetTask();

	//deactivate the doors
	for (Door* door : doors)
		door->TaskEnd();
}

void FixingTask::TurnOnLights()
{

}

void FixingTask::TurnOffLights()
{

}