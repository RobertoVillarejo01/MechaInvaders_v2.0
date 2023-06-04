#include "VendingMachine/VendingMachine.h"
#include "Interaction/InteractionComp.h"
#include "Player/Player.h"
#include "Weapon/Weapon.h"
#include "Health/Health.h"
#include "../Engine/src/AudioManager/Audio.h"
#include "Networking\Networking.h"
#include "../Engine/src/resourcemanager/Resourcemanager.h"
#include "TaskSystem/TaskSystem.h"


void VendingMachine::Initialize()
{
	interaction = mOwner->GetComponentType<InteractionComp>();
	emitter = mOwner->GetComponentType<SoundEmitter>();

	serverItIsI = NetworkingMrg.AmIServer();
}

void VendingMachine::Update() 
{
	if (interaction && interaction->mbInteracting && interaction->mPlayer)
	{
		//Interact(interaction->mPlayer);
		
		if (serverItIsI)
		{
			vending_machine d{ id, interaction->mPlayer->getId() };
			network_event e = NetworkingMrg.CreateNetEvent(0, event_type::vending);
			std::memcpy(e.payload.data(), reinterpret_cast<char*>(&d), sizeof(d));
			NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
		}
		else // (!serverItIsI)
		{
			vending_machine d{ id, interaction->mPlayer->getId() };
			network_event e = NetworkingMrg.CreateNetEvent(interaction->mPlayer->getId(), event_type::vending_reqest);
			std::memcpy(e.payload.data(), reinterpret_cast<char*>(&d), sizeof(d));
			NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
		}

		interaction->StopInteracting(true);
	}
}
	 
void VendingMachine::ToJson(nlohmann::json& j) const
{
	j["cost"] << cost;
	j["mType"] << static_cast<int>(mType);
}

void VendingMachine::FromJson(nlohmann::json& j)
{
	if (j.find("cost") != j.end())
		j["cost"] >> cost;
	int type = 0;
	if (j.find("mType") != j.end())
		j["mType"] >> type;
	mType = static_cast<VM_type>(type);
}

#ifdef EDITOR
bool VendingMachine::Edit()
{
	bool changed = false;

	const char* Type[] = { "AMMO", "HP", "AGILITY", "DOUBLE SHOT", "AUTO REVIVE" };
	if (ImGui::BeginCombo(" Type", Type[static_cast<int>(mType)]))
	{
		if (ImGui::Selectable(" - AMMO - "))
			mType = VM_type::AMMO;
		if (ImGui::Selectable(" - HP - "))
			mType = VM_type::HP;
		if (ImGui::Selectable(" - AGILITY - "))
			mType = VM_type::AGILITY;
		if (ImGui::Selectable(" - DOUBLE SHOT - "))
			mType = VM_type::DOUBLE_SHOT;
		if (ImGui::Selectable(" - AUTO REVIVE - "))
			mType = VM_type::AUTOREVIVE;

		ImGui::EndCombo();
	}
	ImGui::DragInt("COST: ", &cost);

	return changed;
}
#endif

IComp* VendingMachine::Clone() { return Scene.CreateComp<VendingMachine>(mOwner->GetSpace(), this); }

void VendingMachine::Interact(Player* p)
{
	if (!TaskSys.electricity)
		return;
	if (p->money >= (unsigned)cost && p->total_pu < p->icons.size())
	{
		switch (mType)
		{
			case VM_type::AMMO:
				if (p->mWeaponComp)
				{
					if(p->mWeaponComp->total_bullets + p->mWeaponComp->bullet_count < p->mWeaponComp->max_bullets + p->mWeaponComp->charger_size)
					{
						p->mWeaponComp->FillAmmo();
						if(emitter)
							emitter->PlayCue("./../Resources/Audio/chiclin.mp3", 1, false, false, false);
						p->money -= (unsigned)cost;
					}
					break;
				}
				else
					std::cerr << "AMMO VENDING MACHINE: Cannot find the weapon component of the player" << std::endl;
				break;
			case VM_type::HP:
				if (p->mHealth)
				{
					p->mHealth->IncreaseMaxHealth(float(p->hp_increase));
					if (emitter)
						emitter->PlayCue("./../Resources/Audio/chiclin.mp3", 1, false, false, false);
					p->money -= cost;
					//ICON CHANGING
					p->health_pu += 1;
					p->total_pu += 1;
					dynamic_cast<IconLogic*>(p->icons[p->total_pu - 1])->ChangeIcon(VM_type::HP);
					break;
				}
				else
					std::cerr << "HEALTH VENDING MACHINE: Cannot find the health component of the player" << std::endl;
				break;
			case VM_type::AGILITY:
				p->reload_vel += 1;
				if (emitter)
					emitter->PlayCue("./../Resources/Audio/chiclin.mp3", 1, false, false, false);
				p->money -= cost;
				//ICON CHANGING
				p->agility_pu += 1;
				p->total_pu += 1;
				p->sprintLimit += 0.5f;
				dynamic_cast<IconLogic*>(p->icons[p->total_pu - 1])->ChangeIcon(VM_type::AGILITY);
				break;
			case VM_type::DOUBLE_SHOT:
				p->damage_bonification += 1;
				if (emitter)
					emitter->PlayCue("./../Resources/Audio/chiclin.mp3", 1, false, false, false);
				p->money -= cost;
				//ICON CHANGING
				p->doubleshot_pu += 1;
				p->total_pu += 1;
				dynamic_cast<IconLogic*>(p->icons[p->total_pu - 1])->ChangeIcon(VM_type::DOUBLE_SHOT);
				break;
			case VM_type::AUTOREVIVE:
				if (!p->fast_revive)
				{
					p->fast_revive = true;
					if (emitter)
						emitter->PlayCue("./../Resources/Audio/chiclin.mp3", 1, false, false, false);
					p->money -= cost;
					//ICON CHANGING
					p->total_pu += 1;
					dynamic_cast<IconLogic*>(p->icons[p->total_pu - 1])->ChangeIcon(VM_type::AUTOREVIVE);
				}
				break;
		}
	}
}


void IconLogic::Initialize()
{
	model = mOwner->GetComponentType<renderable>();
}

void IconLogic::Update()
{
	if (!model)
		return;
	if (actual_icon != prev_icon)
	{
		switch (actual_icon)
		{

			case VendingMachine::VM_type::AMMO:
				model->SetModel(ResourceMgr.GetResource<GFX::Model>("./../Resources/Model/PowerUps/None.fbx"));
				break;
			case VendingMachine::VM_type::HP:
				model->SetModel(ResourceMgr.GetResource<GFX::Model>("./../Resources/Model/PowerUps/HP_UP.fbx"));
				break;
			case VendingMachine::VM_type::AGILITY:
				model->SetModel(ResourceMgr.GetResource<GFX::Model>("./../Resources/Model/PowerUps/reload.fbx"));
				break;
			case VendingMachine::VM_type::DOUBLE_SHOT:
				model->SetModel(ResourceMgr.GetResource<GFX::Model>("./../Resources/Model/PowerUps/x2Shot.fbx"));
				break;
			case VendingMachine::VM_type::AUTOREVIVE:
				break;
		}
		prev_icon = actual_icon;
	}
}

void IconLogic::Shutdown()
{

}

IComp* IconLogic::Clone()
{
	return Scene.CreateComp<IconLogic>(mOwner->GetSpace(), this);
}

#ifdef EDITOR
bool IconLogic::Edit()
{
	ImGui::DragInt("Position in the grid", &position);
	return false;
}
#endif

void IconLogic::ToJson(nlohmann::json& j) const
{
	j["position"] << position;
}

void IconLogic::FromJson(nlohmann::json& j)
{
	if (j.find("position") != j.end())
		j["position"] >> position;
}