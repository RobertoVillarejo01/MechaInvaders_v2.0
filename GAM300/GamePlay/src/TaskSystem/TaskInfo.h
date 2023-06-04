#pragma once
#include "LogicSystem/Logic.h"

class ParticleSystem;
class InteractionComp;

enum class TaskTag { None, Fixing , Communication, Max};

class TaskInfo : public ILogic
{
public:

	void Initialize();
	void Update();
	void Shutdown();

	IComp* Clone();

	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);
#ifdef EDITOR
	bool Edit();
#endif
	//Methods for doors
	bool IsAvailable() { return available; }
	void MakeAvailable();

	void Activate(bool onoff);
	bool IsActivated() { return active; }
	void EndTask() { finished = true; }
	bool IsFinished() { return finished; }
	void ResetInfo(); //call when finishing task to be reused later

//------------INTERACTIBLE TASKS--------------
	bool IsInteractible() { return interactible; }
	void IncreaseProgress() { progress += score_per_iteration; }
	float GetProgressPercentage() { return progress / maxprogress; }

	float time_to_score = 0.0f;
	float interaction_range = 0.0f;
	float progress = 0.0f;
	bool is_interacting = false;
	bool interacted = false;
//--------------------------------------------	

	GameObject* letter = nullptr;
	ParticleSystem* mParticles = nullptr;
	GameObject* warning;
	TaskTag tag = TaskTag::None;

	//for communication tasks
	unsigned int position = 0;
	bool correct = false;

	bool player_in_place = false;

	int getID() { return id; }
	void setID(int _id) { id = _id; }

private:
	GameObject* mFeedbackParticlesActive = nullptr;
	GameObject* mFeedbackParticlesCompleted = nullptr;

	bool available = false;
	bool active = false;
	bool finished = false;

	int room = 0;

//------------INTERACTIBLE TASKS--------------
	bool interactible = false;
	float interaction_timer = 0.0f;
	float maxprogress = 0.0f;
	float score_per_iteration = 0.0f;
	float offset_warning = 0.0f;
	InteractionComp* InteractComp = nullptr;
//--------------------------------------------

	bool warning_created = false;
	int id = 0;
};

