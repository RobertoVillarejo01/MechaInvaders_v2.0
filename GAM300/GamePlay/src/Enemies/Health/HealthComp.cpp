#include "HealthComp.h"
#include "../Robot/Robot.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif

void EnemyHealth::ProcessHit(const glm::vec3& shot){

	DynamicCollider* Head = mOwner->FindChild("Head")->GetComponentType<DynamicCollider>();
	DynamicCollider* Body = mOwner->FindChild("Body")->GetComponentType<DynamicCollider>();

	glm::vec3 HeadPos = mOwner->mTransform.mPosition + Head->mPosition;
	geometry::aabb HeadBox({ HeadPos - Head->mScale, HeadPos + Head->mScale });

	glm::vec3 BodyPos = mOwner->mTransform.mPosition + Body->mPosition;
	geometry::aabb BodyBox({ BodyPos - Body->mScale, BodyPos + Body->mScale });

	//shot in the head
	if (geometry::intersection_point_aabb(shot, HeadBox)) CurrentHealth -= HeadDamage;
	else if (mOwner->GetComponentType<Robot>() &&  geometry::intersection_point_aabb(shot, BodyBox)) CurrentHealth -= BodyDamage;
}

void EnemyHealth::Initialize(){

	CurrentHealth = MaxHealth;
}

void EnemyHealth::Update(){

	int i = 0;
}

#ifdef EDITOR
bool EnemyHealth::Edit()
{
	ImGui::DragInt("MaxHealth: ", &MaxHealth);
	ImGui::DragInt("Body Damage", &BodyDamage);
	ImGui::DragInt("Head Damage", &HeadDamage);

	return false;
}
#endif


IComp* EnemyHealth::Clone() { return Scene.CreateComp<EnemyHealth>(mOwner->GetSpace(), this); }
