#pragma once
#include "../Engine/src/Utilities/Singleton.h"
#include <vector>

class GameObject;
class FixingTask;
class ReactorTask;
class TaskInfo;
enum class TaskTag;
class FadeInOut;
struct task_activation;

class TaskSystem
{
	MAKE_SINGLETON(TaskSystem)
public:

	void Initialize();
	void Update();
	void Shutdown();

	TaskTag PickRandomTaskTag();
	void TriggerTask(TaskTag task);
	void CheatCodes();
	void MakeAllAvailable();
	void ClearActiveTasks() { activetasks.clear(); }
	void EndActiveTasks();
	void EndActiveTasksEvent();

	bool playersInPlace();

	void SendStarTaskEvent(task_activation task);
	void StartTaskEvent(task_activation task);

	//all available tasks
	//std::vector<TaskInfo*> tasks;
	std::unordered_map<unsigned int, TaskInfo*> tasks;
	//all fixing available tasks
	std::vector<TaskInfo*> fixingtasks;
	//all ccommunication available tasks
	std::vector<TaskInfo*> communicationtasks;
	//active task(s)
	std::vector<TaskInfo*> activetasks;

	bool electricity = true;

private:
	FadeInOut* letterElec = nullptr;
	FadeInOut* letterCom = nullptr;
	float timer = 0.0f;
	float timetotrigger = 45.0f;

	bool serverItIsI = false;
};
#define TaskSys (TaskSystem::Instance())