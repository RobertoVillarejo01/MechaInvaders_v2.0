#pragma once
#include "LogicSystem/Logic.h"

class InteractionComp;
class SoundEmitter;
class Player;

class VendingMachine : public ILogic
{
public:
	enum class VM_type { AMMO, HP, AGILITY, DOUBLE_SHOT, AUTOREVIVE };

	void Initialize();
	void Update();

	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);
	IComp* Clone() override;

#ifdef EDITOR
	bool Edit();
#endif

	void Interact(Player* player);

	SoundEmitter* emitter = nullptr;
	GameObject* mPlayer = nullptr;
	InteractionComp* interaction = nullptr;
	int cost = 0;

	VM_type mType = VM_type::AMMO;

	bool serverItIsI = false;

	int getID() { return id; }
	void setID(int _id) { id = _id; }
	int id = 0;
};

class IconLogic : public ILogic
{
public:
	void Initialize();
	void Update();
	void Shutdown();

	IComp* Clone();

	void ChangeIcon(VendingMachine::VM_type icon) { actual_icon = icon; }

#ifdef EDITOR
	bool Edit();
#endif
	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);

private:
	renderable* model = nullptr;
	VendingMachine::VM_type actual_icon = VendingMachine::VM_type::AMMO;
	VendingMachine::VM_type prev_icon = VendingMachine::VM_type::AMMO;
	int position = 0;
};