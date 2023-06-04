#include "Fly.h"
#include "Engine.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR

void Fly::StateMachineInitialize()
{
	SetBrainSize(SMTypes::TOTAL_SM);

	//Movement
	AddState(&Fly::Chase, &Fly::ChaseUpdate, nullptr, "Chase", SMTypes::MOVEMENT_SM);
	AddState(&Fly::Shoot, &Fly::ShootUpdate, nullptr, "Shoot", SMTypes::MOVEMENT_SM);
	SetMainState("Chase", SMTypes::MOVEMENT_SM);
	//Wingbeat
	AddState(&Fly::Move, &Fly::MoveUpdate, nullptr, "Move", SMTypes::WINGBEAT_SM);
	SetMainState("Move", SMTypes::WINGBEAT_SM);
	//Rotation
	AddState(&Fly::UpdateRotationInitialize, &Fly::UpdateRotation, nullptr, "Rotation", SMTypes::ROTATION_SM);
	SetMainState("Rotation", SMTypes::ROTATION_SM);

	mOwner->mTag = Tags::Enemy;
}

void Fly::ToJson(nlohmann::json& j) const
{
	j["movement_velocity"] << movement_velocity;
	j["wingbeat_velocity"] << wingbeat_velocity;
	j["bullet_velocity"] << bullet_velocity;
	j["height"] << height;
	j["range"] << range;
	j["cooldown"] << cooldown;
}

void Fly::FromJson(nlohmann::json& j)
{
	j["movement_velocity"] >> movement_velocity;
	j["wingbeat_velocity"] >> wingbeat_velocity;
	j["bullet_velocity"] >> bullet_velocity;
	j["height"] >> height;
	j["range"] >> range;
	j["cooldown"] >> cooldown;
}

#ifdef EDITOR
bool Fly::Edit()
{
	ImGui::DragFloat("Movement Velocity", &movement_velocity, 1.f, 0.f);
	ImGui::DragFloat("Wingbeat Velocity", &wingbeat_velocity, 1.f, 0.f);
	ImGui::DragFloat("Bullet Velocity", &bullet_velocity, 1.f, 0.f);
	ImGui::DragFloat("Height", &height, 1.f, 0.f);
	ImGui::DragFloat("Range", &range, 1.f, 0.f);
	ImGui::DragFloat("Cool Down", &cooldown, 1.f, 0.f);
	return true;
}
#endif // EDITOR

// MOVEMENT STATE MACHINE ///////////////////////////////////////////////////////////////////////////

void Fly::Chase()
{
	Player = Scene.FindObject("Player0");
	rb = mOwner->GetComponentType<Rigidbody>();
}

void Fly::ChaseUpdate()
{
	glm::vec3 direction = (Player->mTransform.mPosition - mOwner->mTransform.mPosition);
	direction.y = 0;
	float distance = glm::length(direction);
	direction = glm::normalize(direction);
	if (distance <= range)
	{
		rb->mVelocity.x = 0;
		rb->mVelocity.z = 0;
		ChangeState("Shoot", SMTypes::MOVEMENT_SM);
	}
	else
	{
		rb->mVelocity.x = movement_velocity * direction.x;
		rb->mVelocity.z = movement_velocity * direction.z;
	}
}

void Fly::Shoot()
{
	Player = Scene.FindObject("Player0");
	timer = 0.0f;
	//Create projectile
	GameObject* projectile = Scene.CreateObject();
	serializer.LoadArchetype("Projectile", projectile);
	projectile->mTag = Tags::EnemyBullet;
	projectile->mTransform.mPosition = mOwner->mTransform.mPosition;
	glm::vec3 direction = glm::normalize(Player->mTransform.mPosition - mOwner->mTransform.mPosition);
	Projectile* mproj = projectile->GetComponentType<Projectile>();
	mproj->direction = direction;
	mproj->velocity = bullet_velocity;
}

void Fly::ShootUpdate()
{
	timer += FRC.GetFrameTime();
	if(timer >= cooldown)
		ChangeState("Chase", SMTypes::MOVEMENT_SM);
}

// WINGBEAT STATE MACHINE ///////////////////////////////////////////////////////////////////////////

void Fly::Move()
{

}

void Fly::MoveUpdate()
{

}

// ROTATION STATE MACHINE ///////////////////////////////////////////////////////////////////////////

void Fly::UpdateRotationInitialize()
{
	Player = Scene.FindObject("Player0");
}

void Fly::UpdateRotation()
{
	glm::vec3 direction = (Player->mTransform.mPosition - mOwner->mTransform.mPosition);
	mOwner->mTransform.LookAt(Player->mTransform.mPosition);
}

// PROJECTILE LOGIC ///////////////////////////////////////////////////////////////////////////

void   Projectile::Initialize()
{
	velocity = 0;
	time_destroy = 10;
	rb = mOwner->GetComponentType<Rigidbody>();
}

void   Projectile::Update()
{
	timer += FRC.GetFrameTime();
	rb->mVelocity = direction * velocity;
	if (timer >= time_destroy)
		Scene.DestroyObject(mOwner);
}

IComp* Projectile::Clone() { return Scene.CreateComp<Projectile>(mOwner->GetSpace(), this); }