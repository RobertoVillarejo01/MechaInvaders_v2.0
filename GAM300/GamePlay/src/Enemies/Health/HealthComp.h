#pragma once
#include "LogicSystem/Logic.h"

class EnemyHealth : public ILogic
{
public:
	void   Initialize();
	void   Update();
#ifdef EDITOR
	bool Edit();
#endif
	//process where was it shot
	void ProcessHit(const glm::vec3& shot);
	IComp* Clone();
	glm::vec3 direction;
	int MaxHealth = 100;
	int CurrentHealth = 100;
	int BodyDamage = 50;
	int HeadDamage = 100;
	
private:
	
};