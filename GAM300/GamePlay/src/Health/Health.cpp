#include "Health.h"
#include "WaveSystem/WaveSystem.h"
#include "Graphics/Renderable/Renderable.h"
#include "../Engine/src/Utilities\Utils.h"

void Health::Initialize()
{
	mRob = mOwner->GetComponentType<Robot>();

	if (mOwner->mTag == Tags::Player)
	{
		SetMaxHealth(100.0f);
		
	}
	if (mOwner->mTag == Tags::Enemy)
		SetMaxHealth(INFINITY);

	invincible = false;
}

void Health::Update()
{
	if (!mRob) mOwner->GetComponentType<Robot>();

	//ENEMIES
	if (mOwner->mTag == Tags::Enemy)
	{

		if (current_health <= 0.0f)
		{
			if (!mRob)
				Die();
			else
				mRob->ChangeState("Die");
		}
	}
}

bool Health::Heal(float heal_quantity)
{
	current_health += heal_quantity;

	if (current_health >= max_health)
	{
		current_health = max_health;
		return true;
	}
	return false;
}

void Health::SetMaxHealth(float health)
{
	max_health = health;
	current_health = health;
}

void Health::GetDamaged(float damage)
{
	if (mOwner->mTag == Tags::Player)
	{
		if (invincible)
			return;
	}
	
	current_health -= damage;
	
	if (current_health < 0.0f) current_health = 0.0f;
	else if (mRob)
		mRob->StunEnemy();
}

void Health::Die()
{
	Scene.DestroyObject(mOwner);
	WaveSys.defeated_enemies += 1;
}

void Health::RestoreHealthMax()
{
	current_health = max_health;
}

IComp* Health::Clone() { return Scene.CreateComp<Health>(mOwner->GetSpace(), this); }

float Health::getMaxHealth()
{
	return max_health;
}

float Health::getCurrentHealth()
{
	return current_health;
}

void Health::setCurrentHealth(float health)
{
	current_health = health;
}

void Health::IncreaseMaxHealth(float increasing)
{
	max_health += increasing;
	current_health += increasing;
}

void Health::DecreaseMaxHealth(float decreasing)
{
	max_health -= decreasing;
	current_health = max_health;
}

