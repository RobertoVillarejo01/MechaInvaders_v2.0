#include "WeaponMachine.h"
#include "Player/Player.h"
#include "Weapon.h"
#include "Interaction/InteractionComp.h"
#include "../Engine/src/AudioManager/Audio.h"
#include "Serializer/Factory.h"
#include "Networking\Networking.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR


#ifdef EDITOR
bool WeaponMachine::Edit()
{
	bool changed = false;
	ImGui::DragInt("Cost", &cost, 5.0f, 0, 10000);

	char name_temp[50];
	strcpy_s(name_temp, weapon_name.c_str());

	ImGui::PushItemWidth(160);
	ImGui::InputText("Weapon Name", name_temp, 50);
	weapon_name = name_temp;
	return changed;
}
#endif

void WeaponMachine::Initialize()
{
	interaction = mOwner->GetComponentType<InteractionComp>();
	emitter = mOwner->GetComponentType<SoundEmitter>();
	
#ifdef EDITOR
#else
	weapon_visual = mOwner->GetSpace()->CreateObject();
	std::string weapon_empty_name = weapon_name + "_empty";
	serializer.LoadArchetype(weapon_empty_name.c_str(), weapon_visual);
	weapon_visual->mTransform.mScale *= glm::vec3(2.0f);
	weapon_visual->mTransform.mPosition = mOwner->mTransform.mPosition + glm::vec3(0, 5.0f, 0);
#endif
}

void WeaponMachine::Update()
{
	if (weapon_visual)
		weapon_visual->mTransform.RotateAround(glm::vec3(0, 1, 0), 5.0f);
		
	if (interaction && interaction->mbInteracting && interaction->mPlayer)
	{
		//check if the player already has the weapon
		Player* player = interaction->mPlayer;

		//set a new weapon
		if (player->money >= (unsigned)cost)
		{
			//player->buy_weapon(weapon_name, cost);

			if (emitter)
				emitter->PlayCue("./../Resources/Audio/chiclin.mp3", 1, false, false, false);

			buy_weapon w{};
			std::memcpy(w.name.data(), weapon_name.c_str(), weapon_name.size());
			w.weapon_cost = cost;

			if (NetworkingMrg.AmIServer())
			{
				network_event e = NetworkingMrg.CreateNetEvent(player->getId(), event_type::weapon_new);
				std::memcpy(e.payload.data(), reinterpret_cast<char*>(&w), sizeof(w));
				NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
			}
			else
			{
				network_event e = NetworkingMrg.CreateNetEvent(player->getId(), event_type::weapon_new_request);
				std::memcpy(e.payload.data(), reinterpret_cast<char*>(&w), sizeof(w));
				NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
			}
		}
		interaction->StopInteracting(true);
	}
}

void WeaponMachine::Shutdown()
{
	mOwner->GetSpace()->DestroyObject(weapon_visual);
	weapon_visual = nullptr;
}

IComp* WeaponMachine::Clone()
{
	return Scene.CreateComp<WeaponMachine>(mOwner->GetSpace(), this);
}

void WeaponMachine::ToJson(nlohmann::json& j) const
{
	j["cost"] << cost;
	j["weapon_name"] << weapon_name;
}

void WeaponMachine::FromJson(nlohmann::json& j)
{
	if (j.find("cost") != j.end())
		j["cost"] >> cost;
	if (j.find("weapon_name") != j.end())
		j["weapon_name"] >> weapon_name;
}