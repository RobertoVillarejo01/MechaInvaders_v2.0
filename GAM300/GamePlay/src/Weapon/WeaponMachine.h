#pragma once
#include "LogicSystem/Logic.h"

class Player;
class Weapon;
class SoundEmitter;
class InteractionComp;

class WeaponMachine : public ILogic
{
public:
	void Initialize();
	void Update();
	void Shutdown();

	IComp* Clone();

#ifdef EDITOR
	bool Edit();
#endif
	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);

	int cost = 100;
	std::string weapon_name = "";
private:

	GameObject* weapon_visual = nullptr;
	Weapon* mWeapon = nullptr;
	SoundEmitter* emitter = nullptr;
	InteractionComp* interaction = nullptr;
	Player* mPlayer = nullptr;
	
};