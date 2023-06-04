#pragma once

#include "../Engine/src/LogicSystem/StateMachine.h"
#include <vector>

class GameObject;
class Spawner;
class TaskInfo;
class ParticleSystem;
class LightComponent;
class Door;

class FixingTask : public SMComponent<FixingTask>
{
public:
	enum SMTypes { INTERACTION_SM, TOTAL_SM };

	void StateMachineInitialize() override;

	void ChangeBrainSize(size_t _size) { SetBrainSize(_size); }

	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);

	void ResetTask();

#ifdef EDITOR
	bool Edit();
#endif

	int spawner_amount = 0;
	int order_per_iteration = 0;

private:
	std::vector<LightComponent*> lights;
	TaskInfo* info = nullptr;
	std::vector<GameObject*> players;
	std::vector<Spawner*> spawners;
	std::vector<Door*> doors;

	float timer = 0.0f;
	float order_time = 5.0f;

	//---------IDLE-----------
	void IdleInit();
	void IdleUpdate();

	//-------ACTIVATED-------
	void ActivatedInit();
	void ActivatedUpdate();
	void ActivatedShutDown();

	void TurnOnLights();
	void TurnOffLights();
};