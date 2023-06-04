#include "TaskInfo.h"
#include "TaskSystem.h"
#include "FixingTask/FixingTask.h"
#include "../Engine/src/Objects/GameObject.h"
#include "../Engine/src/Utilities/FrameRateController/FrameRateController.h"
#include "GameStateManager/GameStateManager.h"
#include "../Engine/src/Utilities/Input/Input.h"
#include "UtilityComponents/FadeInOut.h"
#include "Networking\Networking.h"

void TaskSystem::Initialize()
{
	//for si las flies
	tasks.clear();
	fixingtasks.clear();
	communicationtasks.clear();
	activetasks.clear();

	timer = 0.0f;
	std::vector<GameObject*> ob = Scene.FindObjects(Tags::Task);
	int task_id = 0;
	for (auto task : ob)
	{
		TaskInfo* info = task->GetComponentType<TaskInfo>();
		if (!info)
		{
			continue;
		}

		//set the id of the task for networking to know which one is active or interacted
		info->setID(task_id);

		if (info->tag == TaskTag::Fixing && info->IsAvailable())			
			fixingtasks.push_back(task->GetComponentType<TaskInfo>());
		if (info->tag == TaskTag::Communication && info->IsAvailable())
			communicationtasks.push_back(task->GetComponentType<TaskInfo>());
	}

	if (Scene.FindObject("FixText") && Scene.FindObject("FixText")->GetComponentType<FadeInOut>())
		letterElec = Scene.FindObject("FixText")->GetComponentType<FadeInOut>();

	if (Scene.FindObject("FixCom") && Scene.FindObject("FixCom")->GetComponentType<FadeInOut>())
		letterCom = Scene.FindObject("FixCom")->GetComponentType<FadeInOut>();

	serverItIsI = NetworkingMrg.AmIServer();
}
void TaskSystem::Update()
{
	if (serverItIsI)
	{
		if (!tasks.empty() && activetasks.empty())
				timer += FRC.GetFrameTime();
		if (timer >= timetotrigger)
		{
			TaskTag tag = PickRandomTaskTag();
			TriggerTask(tag);
			timer = 0.0f;
		}
		if (GSM.mConfig.mbCheats)
			CheatCodes();
	}
}
void TaskSystem::Shutdown()
{
	std::vector<GameObject*> ob = Scene.FindObjects(Tags::Task);
	//look for which task it is and store the component in the apropiate vector
	for (auto& task : ob)
		task->Shutdown();
}

TaskTag TaskSystem::PickRandomTaskTag()
{
	int random = rand() % static_cast<int>(TaskTag::Max) + 1;
	return (TaskTag)random;
}

void TaskSystem::TriggerTask(TaskTag task)
{
	if (activetasks.size() != 0)
		return;
	if (task == TaskTag::Communication && communicationtasks.size() < 3)
		task = TaskTag::Fixing;
	if (task == TaskTag::Fixing)
	{
		if (fixingtasks.size() == 0)
		{
			std::cerr << "TaskSystem ERROR: there are not enough fixing tasks on the level" << std::endl;
			return;
		}

		int random = rand() % fixingtasks.size();
		activetasks.push_back(fixingtasks[random]);
		fixingtasks[random]->Activate(true);
		if (letterCom)
			letterCom->trigger = true;

		task_activation task_{ (unsigned)task, random };
		
		SendStarTaskEvent(task_);
	}
	else if(task == TaskTag::Communication)
	{
		task_activation task_{ (unsigned)task };
		for (TaskInfo* task : communicationtasks)
			task->correct = false;

		std::vector<TaskInfo*> temp = communicationtasks;
		for (int i = 0; i < 3; i++)
		{
			int index = rand() % temp.size();
			activetasks.push_back(temp[index]);
			temp[index]->Activate(true);
			temp[index]->position = i + 1;
			temp.erase(temp.begin() + index);

			task_.task_id[i] = index;
		}

		SendStarTaskEvent(task_);
		if (letterElec)
			letterElec->trigger = true;
	}
}

void TaskSystem::MakeAllAvailable()
{
	for (auto task : tasks)
		task.second->MakeAvailable();
}


void TaskSystem::EndActiveTasks()
{
	for (auto t : activetasks)
		t->EndTask();
	ClearActiveTasks();
}


void TaskSystem::EndActiveTasksEvent()
{
	EndActiveTasks();

	network_event e = NetworkingMrg.CreateNetEvent(0, event_type::end_task);
	NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
}

bool TaskSystem::playersInPlace()
{
	bool inPlace = true;

	for (TaskInfo* info : activetasks)
		inPlace = info->player_in_place;

	return inPlace;
}

void TaskSystem::SendStarTaskEvent(task_activation t)
{
	network_event e = NetworkingMrg.CreateNetEvent(0, event_type::start_task);
	std::memcpy(e.payload.data(), reinterpret_cast<char*>(&t), sizeof(t));
	NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
}

void TaskSystem::StartTaskEvent(task_activation task)
{
	if ((TaskTag)task.task_tag == TaskTag::Fixing)
	{
		activetasks.push_back(fixingtasks[task.task_id[0]]);
		fixingtasks[task.task_id[0]]->Activate(true);
		if (letterCom)
			letterCom->trigger = true;
	}
	else if ((TaskTag)task.task_tag == TaskTag::Communication)
	{
		std::vector<TaskInfo*> temp = communicationtasks;
		for (int i = 0; i < 3; i++)
		{
			int index = task.task_id[i];
			activetasks.push_back(temp[index]);
			temp[index]->Activate(true);
			temp[index]->position = i + 1;
			temp.erase(temp.begin() + index);
		}
	}
}

void TaskSystem::CheatCodes()
{
	if (KeyDown(Key::T) && KeyTriggered(Key::Num1))
	{
		EndActiveTasksEvent();
	}
	if (KeyDown(Key::T) && KeyTriggered(Key::Num2))
	{
		EndActiveTasksEvent();
		TriggerTask(TaskTag::Fixing);
	}
	if (KeyDown(Key::T) && KeyTriggered(Key::Num3))
	{
		EndActiveTasksEvent();
		TriggerTask(TaskTag::Communication);
	}
}