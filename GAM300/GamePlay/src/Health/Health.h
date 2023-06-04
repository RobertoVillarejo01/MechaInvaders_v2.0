#pragma once
#include "LogicSystem/Logic.h"
#include "../Enemies/Robot/Robot.h"

class Health : public ILogic
{
public:
	void Initialize();
	void Update();

	void SetMaxHealth(float health);
	void GetDamaged(float damage);
	void Die();
	bool Heal(float heal_quantity);

	void RestoreHealthMax();
	void IncreaseMaxHealth(float increasing);
	void DecreaseMaxHealth(float decreasing);

	IComp* Clone();

	float getMaxHealth();
	float getCurrentHealth();
	void setCurrentHealth(float health);

private:
	float max_health;
	float current_health;

	bool invincible;

	Robot* mRob;
};