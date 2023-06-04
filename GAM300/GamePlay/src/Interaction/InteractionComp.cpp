#include "InteractionComp.h"
#include "InteractionText.h"
#include "Player/Player.h"
#include "HUD/HUDBar.h"
#include "Doors/Doors.h"
#include "VendingMachine/VendingMachine.h"
#include "Weapon/WeaponMachine.h"
#include "TaskSystem/TaskInfo.h"
#include "TaskSystem/TaskSystem.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR

#include "Utilities/Utils.h"

void InteractionComp::Initialize()
{
	auto player = Scene.get_base_player();
	if (player)
		mPlayer = player->GetComponentType<Player>();
	auto text = mOwner->GetSpace()->FindObject("InteractionText");
	if (text)
		interaction_text = text->GetComponentType<InteractionText>();

	GameObject* BarHUD = Scene.FindObject("HUDBar");
	if (BarHUD)
		hud_bar = BarHUD->GetComponentType<HUDBar>();

	mDoor = mOwner->GetComponentType<Door>();
	if (mDoor)
	{
		char buffer[100];
		_itoa_s(static_cast<unsigned>(mDoor->cost), buffer, sizeof(buffer), 10);
		cost = buffer;
	}

	mVM = mOwner->GetComponentType<VendingMachine>();
	if (mVM)
	{
		char buffer[100];
		_itoa_s(static_cast<unsigned>(mVM->cost), buffer, sizeof(buffer), 10);
		cost = buffer;
	}
	weapon_machine = mOwner->GetComponentType<WeaponMachine>();
	if (weapon_machine)
	{
		char buffer[100];
		_itoa_s(static_cast<unsigned>(weapon_machine->cost), buffer, sizeof(buffer), 10);
		cost = buffer;
	}

	task = mOwner->GetComponentType<TaskInfo>();
}

void InteractionComp::Update()
{
	if (!active)
		return;

	if (!mPlayer)
		mPlayer = Scene.get_base_player()->GetComponentType<Player>();

	glm::vec3 direction = mPlayer->mOwner->mTransform.mPosition - mOwner->mTransform.mPosition;
	float distance = glm::length(direction);

	if (distance <= range && !attached)
	{
		if (!mbCustomMsg)
		{
			message = {};
			if (mType == Type::Door)
			{
				message = "Press E to Open (";
				message += cost;
				message += " $ )";
			}
			else if (mType == Type::Revive)
				message = "Press E to Revive";

			else if (mType == Type::Task)
			{
				if (!task->IsActivated())
					message = "";
				else
				{
					if(task->tag == TaskTag::Fixing)
						message = "Hold E to fix electricity";
					if(task->tag == TaskTag::Communication)
						message = "Press E to fix communication";
				}
			}
			else if (mType == Type::VendingMachine)
			{
				if (TaskSys.electricity)
				{
					if (mPlayer->total_pu < mPlayer->icons.size())
					{
						if (mVM)
						{
							if (mVM->mType == VendingMachine::VM_type::AMMO)
								message = "Press E to refill your ammo (";
							if (mVM->mType == VendingMachine::VM_type::HP)
								message = "Press E to increase your max HP (";
							if (mVM->mType == VendingMachine::VM_type::AGILITY)
								message = "Press E to reload and run faster (";
							if (mVM->mType == VendingMachine::VM_type::DOUBLE_SHOT)
								message = "Press E to duplicate your damage (";
							if (mVM->mType == VendingMachine::VM_type::AUTOREVIVE)
								message = "Press E to be able revive automatically when you die (";
						}
						message += cost;
						message += " $ )";
					}
					else
						message = "You cannot buy more";
				}
				else
				{
					if (mVM)
						message = "Electricity not working";
				}
			}
			else if (mType == Type::WeaponMachine)
			{
				message = "Press E to buy weapon: " + weapon_machine->weapon_name;
				message += " (";
				message += cost;
				message += " $ )";
			}
		}
		interaction_text->AttachObj(this, message);
		attached = true;
	}
	else if (distance > range && attached && !mbInteracting)
	{
		interaction_text->DeatachObj(this);
		attached = false;
	}
}

void InteractionComp::StopInteracting(bool keepactive)
{
	active = keepactive;
	mbInteracting = false;
	interaction_text->DeatachObj(this);
	attached = false;
	hud_bar->Reset();
}

void InteractionComp::SetBar(float percentage)
{
	if (hud_bar)
		hud_bar->SetBar(percentage);
}

bool InteractionComp::CheckInteract()
{
	if (!mbInteracting)
	{
		if (mType == Type::Door)
		{
			Door* mDoor = mOwner->GetComponentType<Door>();
			if (mDoor)
				return mDoor->ChecInteract();
			return false;
		}
		return true;
	}
	return false;
}

void InteractionComp::SetCustomMsg(bool use_custom, std::string msg)
{
	mbCustomMsg = use_custom;
	message = msg;
}

void InteractionComp::Shutdown()
{
}

#ifdef EDITOR
bool InteractionComp::Edit()
{
	bool changed = false;

	int type = static_cast<int>(mType);
	const char* dyn[5] = { "Door","Revive", "Task", "VendingMachine", "WeaponMachine"};
	if (ImGui::Combo("Shape", &type, dyn, 5, 14)) {
		switch (type)
		{
		case 0:
			mType = Type::Door;
			break;
		case 1:
			mType = Type::Revive;
			break;
		case 2:
			mType = Type::Task;
			break;
		case 3:
			mType = Type::VendingMachine;
			break;
		case 4:
			mType = Type::WeaponMachine;
			break;
		}
		changed = true;
	}

	ImGui::DragFloat("Range to Interact", &range, 0.1f);
	ImGui::Checkbox("Key Trigger To Activate", &mbTriggerAction);
	ImGui::Checkbox("Active", &active);

	return changed;
}
#endif

void InteractionComp::ToJson(nlohmann::json& j) const
{
	j["mType"] << static_cast<int>(mType);
	j["range"] << range;
	j["active"] << active;
	j["mbTriggerAction"] << mbTriggerAction;
}

void InteractionComp::FromJson(nlohmann::json& j)
{
	int type = 0;
	if (j.find("mType") != j.end())
		j["mType"] >> type;
	mType = static_cast<Type>(type);

	if (j.find("range") != j.end())
		j["range"] >> range;
	if (j.find("active") != j.end())
		j["active"] >> active;
	if (j.find("mbTriggerAction") != j.end())
		j["mbTriggerAction"] >> mbTriggerAction;
}

IComp* InteractionComp::Clone()
{
	return Scene.CreateComp<InteractionComp>(mOwner->GetSpace(), this);
}