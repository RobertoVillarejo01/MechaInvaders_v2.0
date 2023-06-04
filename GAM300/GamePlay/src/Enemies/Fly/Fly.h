#pragma once

#include "LogicSystem/Logic.h"

class Fly : public SMComponent<Fly>
{
public:
	enum SMTypes { MOVEMENT_SM, WINGBEAT_SM, ROTATION_SM, TOTAL_SM };

	void StateMachineInitialize() override;

	// Public Wrapper aroud protected method for testing purposes
	void ChangeBrainSize(size_t _size) { SetBrainSize(_size); }

	void ToJson(nlohmann::json& j) const;
	void FromJson(nlohmann::json& j);

#ifdef EDITOR
	bool Edit();
#endif

private:
	GameObject* Player;
	Rigidbody* rb;
	float movement_velocity;
	float wingbeat_velocity;
	float bullet_velocity;
	float height;
	float range;
	float cooldown;
	float timer;

	// Movement States
	void Chase();
	void ChaseUpdate();
	void Shoot();
	void ShootUpdate();

	// Wingbeat
	void Move();
	void MoveUpdate();

	//Rotation
	void UpdateRotationInitialize();
	void UpdateRotation();
};

class Projectile : public ILogic
{
public:
	void   Initialize();
	void   Update();

	IComp* Clone();
	glm::vec3 direction = {};
	float velocity = 0.0f;
private:
	Rigidbody* rb = nullptr;
	float timer = 0.0f;
	float time_destroy = 0.0f;
};