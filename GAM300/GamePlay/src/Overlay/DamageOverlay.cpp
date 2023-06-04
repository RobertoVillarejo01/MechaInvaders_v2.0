#include "DamageOverlay.h"
#include "Networking\Networking.h"
#include "Window\Window.h"
#include "Health\Health.h"
#include "../Engine/src/GameStateManager/GameStateManager.h"
#include "../Engine/src/Utilities\Utils.h"
#include "Serializer\Factory.h"

DamageOverlay::DamageOverlay()
{
	
}

DamageOverlay::~DamageOverlay()
{
}

void DamageOverlay::Initialize()
{
	damageOverlay = mOwner->GetComponentType<renderable>();
	damageColor = damageOverlay->GetColor();

	// (Expected format : Player#)
	int id = NetworkingMrg.get_id();
	GameObject* main_player = Scene.GetSpace("MainArea")->FindObject("Player" + std::to_string(id));
	if (main_player)
	{
		player_health = main_player->GetComponentType<Health>();
		max_health = player_health->getMaxHealth();

		auto size = WindowMgr.GetResolution();
		mOwner->mTransform.mScale = glm::vec3(size, 1.0f);
	}
	else
		mOwner->mTransform.mScale = glm::vec3(0.01f);
		
	damageColor.a = prev_alpha = curr_alpha = 0.0f;
	damageOverlay->SetColor(damageColor);

	gameOver = nullptr;
}

void DamageOverlay::Update()
{
	//damage overlay alpha
	if (damageOverlay && player_health)
	{
		float current_health = player_health->getCurrentHealth();
		curr_alpha = 1 - current_health / max_health;

		if (curr_alpha != prev_alpha)
		{
			//get alpha value from our range
			///float lerp_alpha = lerp(0.0f, max_alpha, curr_alpha);
			float range_alpha = curr_alpha * max_alpha;
			damageColor.a = range_alpha;
			damageOverlay->SetColor(damageColor);

			prev_alpha = curr_alpha;
		}
	}
}

void DamageOverlay::Shutdown()
{
}

#ifdef EDITOR

bool DamageOverlay::Edit()
{
	bool changed = false;

	ImGui::DragFloat("max_alpha", &max_alpha, 0.001f, 0.0f, 1.0f);

	return changed;
}

#endif

IComp* DamageOverlay::Clone()
{
	return Scene.CreateComp<DamageOverlay>(mOwner->GetSpace(), this);
}

void DamageOverlay::ToJson(nlohmann::json& j) const
{
	j["max_alpha"] << max_alpha;
}

void DamageOverlay::FromJson(nlohmann::json& j)
{
	if (j.find("max_alpha") != j.end())
		j["max_alpha"] >> max_alpha;
}
