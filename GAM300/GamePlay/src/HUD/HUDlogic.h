#pragma once
#include "LogicSystem/Logic.h"

class TextComponent;
class Player;
class Weapon;

class HUDlogic : public ILogic
{
public:
	void Initialize();
	void Update();

	IComp* Clone();

#ifdef EDITOR
	bool Edit();
#endif
	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);

	Weapon* mWeapon = nullptr;
private:

	TextComponent* CurrentPoints;
	TextComponent* BulletsCount;
	TextComponent* RoundsCount;

	Player* mPlayer = nullptr;
};