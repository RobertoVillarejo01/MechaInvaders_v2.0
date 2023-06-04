#pragma once
#include "LogicSystem/Logic.h"
#include <vector>
class SoundEmitter;
class Player;
class InteractionComp;

class Door : public ILogic
{
public:
	void Initialize();
	void Update();
	void Shutdown();

	IComp* Clone();

#ifdef EDITOR
	bool Edit();
	bool check_new_pos = false;
#endif
	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);
	void Open();
	void OpenTemp();
	void TaskActivate();
	void TaskEnd();
	bool ChecInteract();

	bool mbOpen = false;
	bool mbTaskActive = false;
	int cost = 750;

	int getID() { return id; }
	void setID(int _id) { id = _id; }

private:
	bool mbPrevState = false;
	bool temporal_open = false;
	float timer = 0.0f;
	float timer_opened = 5.0f;

	float door_speed = 0.02f;
	float lerp_dt = 0.0f;
	
	glm::vec3 init_pos = {};
	glm::vec3 end_pos = {};

	Player* mPlayer = nullptr;
	InteractionComp* InteractComp = nullptr;
	std::string spawn_behind_name = {};
	std::string spawn_infront_name = {};
	std::vector<Door*> linked_doors;
	SoundEmitter* emitter = nullptr;

	bool serverItIsI = false;
	int id = 0;
};