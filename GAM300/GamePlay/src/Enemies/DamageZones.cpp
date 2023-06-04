#include "DamageZones.h"
#include "Health/Health.h"
#include "Player/Player.h"
#ifdef EDITOR
#include "Graphics/DebugDraw/DebugDrawing.h"
#include "../Editor/ImGui/imgui.h"
#endif


#ifdef EDITOR
bool DamageZonesHandler::Edit()
{
	if (ImGui::TreeNode("Add damage zone"))
	{
		if (ImGui::Selectable("Head damage"))
		{
			if (mDamageZones[zones::HEAD]) return false;
			auto head = mOwner->CreateChild();
			mDamageZones[zones::HEAD] = head;
		}
		if (ImGui::Selectable("Body damage"))
		{
			if (mDamageZones[zones::BODY]) return false;
			auto body = mOwner->CreateChild();
			mDamageZones[zones::BODY] = body;
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Edit zones"))
	{
		for (auto& zone : mDamageZones)
		{
			ImGui::Bullet();
			ImGui::SameLine();
			std::string type;
			switch (zone.first)
			{
			case zones::HEAD:
				type = "Head";
				break;
			case zones::BODY:
				type = "Body";
				break;
			}
			if (ImGui::TreeNode(type.c_str()))
			{
				EditZones(mDamageZones[zone.first]);
				ImGui::TreePop();
			}
			ImGui::Separator();
		}
		ImGui::TreePop();
	}

	return false;
}

void DamageZonesHandler::EditZones(GameObject* child)
{
	geometry::obb zone = { child->mTransform.mPosition, child->mTransform.mScale / 2.0f,
						   child->mTransform.GetRotMtx() };
	child->EditTransform();
	Debug::DrawOBB(zone, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
}

#endif

float DamageZonesHandler::CheckHit(const geometry::ray& _ray, float smallest_t)
{
	for (auto& zone : mDamageZones)
	{
		geometry::obb hitbox = { mOwner->mTransform.mPosition,
								total_aabb.max,
								mOwner->mTransform.GetRotMtx() };
		float t = geometry::intersection_ray_obb(_ray, hitbox);
		if (t <= smallest_t && t != -1.0f)
			return t;
	}
	return -1.0f;
}

float DamageZonesHandler::ProcessHit(const geometry::ray& _ray, float damage, Player* player)
{
	auto health = mOwner->GetComponentType<Health>();
	auto logic = mOwner->GetComponentType<Robot>();

	for (auto& zone : mDamageZones)
	{
		geometry::obb hitbox = {zone.second->mTransform.mPosition,
							    zone.second->mTransform.mScale / 2.0f ,
								zone.second->mTransform.GetRotMtx() };
		float t = geometry::intersection_ray_obb(_ray, hitbox);
		if (t != -1.0f)
		{
			switch (zone.first)
			{
				//here is where the money gained by the player should be updated
			case zones::HEAD:
				health->GetDamaged(damage * 2.0f);
				if (logic && logic->farmeable)
				{
					if (health->getCurrentHealth() <= 0)
					{
						player->money += 80;
						logic->farmeable = false;
					}
					else
						player->money += 20;
				}
				break;
			case zones::BODY:
				health->GetDamaged(damage);
				if (logic && logic->farmeable)
				{
					if (health->getCurrentHealth() <= 0)
					{
						player->money += 60;
						logic->farmeable = false;
					}
					else
						player->money += 10;
				}
				break;
			}
			//there should be here some kind of enemy->GetDamaged() that will 
			// be virtual and will be defined by the robot or the fly enemy etc
			//instead of checking in the health what to call
			return t;
		}
	}

	return -1.0f;
}

void DamageZonesHandler::Initialize()
{
	mDamageZones[zones::HEAD] = mOwner->FindChild("Head");
	mDamageZones[zones::BODY] = mOwner->FindChild("Body");


	//initialize the resultan bounding volume
	total_aabb = { { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()},
				 { -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()} };
	//iterate through all of the objects and get the min and max point of all of them
	for (auto& zone : mDamageZones)
	{
		total_aabb.min = glm::min(total_aabb.min, zone.second->mTransform.mOffset - zone.second->mTransform.mScale / 2.0f);
		total_aabb.max = glm::max(total_aabb.max, zone.second->mTransform.mOffset + zone.second->mTransform.mScale / 2.0f);
	}
}

void DamageZonesHandler::Update()
{
	
}

void DamageZonesHandler::Shutdown()
{
}

void DamageZonesHandler::ToJson(nlohmann::json& j) const
{
	std::string z;
	for (auto zone : mDamageZones)
	{
		if (zone.first == zones::HEAD) z = "Head";
		else						   z = "Body";
		j[z.c_str()] << *zone.second;
	}
}

void DamageZonesHandler::FromJson(nlohmann::json& j)
{
	
}



IComp* DamageZonesHandler::Clone()
{
	return Scene.CreateComp<DamageZonesHandler>(mOwner->GetSpace(), this);
}

