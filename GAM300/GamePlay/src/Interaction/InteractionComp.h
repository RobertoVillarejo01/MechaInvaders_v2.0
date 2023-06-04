#pragma once
#include "LogicSystem/Logic.h"

class Player;
class HUDBar;
class InteractionText;
class Door;
class VendingMachine;
class WeaponMachine;
class TaskInfo;

class InteractionComp : public ILogic
{
public:
	enum class Type { Door, Revive, Task, VendingMachine , WeaponMachine ,  type_size };

	void Initialize();
	void Update();
	void Shutdown();

	IComp* Clone();

#ifdef EDITOR
	bool Edit();
#endif
	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);

	void SetBar(float percentage);
	void StopInteracting(bool keepactive = false);
	void Activate(bool _active) { active = _active; }
	bool CheckInteract();
	void SetCustomMsg(bool use_custom, std::string msg = {});

	bool mbInteracting = false;
	bool mbCustomMsg = false;
	bool mbTriggerAction = true;
	Player* mPlayer = nullptr;
	Type mType = Type::Door;
private:
	float range = 10.0f;
	
	std::string message{};
	std::string cost{};

	bool attached = false;
	bool active = true;

	HUDBar* hud_bar = nullptr;
	Door* mDoor = nullptr;
	VendingMachine* mVM = nullptr;
	InteractionText* interaction_text = nullptr;
	TaskInfo* task = nullptr;
	WeaponMachine* weapon_machine = nullptr;
};
