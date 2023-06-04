#include "Doors.h"
#include "Player/Player.h"
#include "Interaction/InteractionComp.h"
#include "Networking\Networking.h"
#include "AudioManager/Audio.h"
#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR

#include "Utilities/Utils.h"

void Door::Initialize()
{
	auto player = Scene.get_base_player();
	if (player)
		mPlayer = player->GetComponentType<Player>();

	InteractComp = mOwner->GetComponentType<InteractionComp>();
	init_pos = mOwner->mTransform.mPosition;
	
	emitter = mOwner->GetComponentType<SoundEmitter>();

	auto childs = mOwner->GetChilds();
	auto parent = mOwner->GetParent();


	for (auto child : childs)
	{
		Door* child_door = child->GetComponentType<Door>();
		if (child_door)
			linked_doors.push_back(child_door);
	}

	if(parent)
	{
		Door* parent_door = parent->GetComponentType<Door>();
		if (parent_door)
			linked_doors.push_back(parent_door);
	}

	serverItIsI = NetworkingMrg.AmIServer();
}

void Door::Update()
{
	if (!mbTaskActive || temporal_open)
	{
		if (temporal_open)
		{
			timer += FRC.GetFrameTime();
			if (timer >= timer_opened)
			{
				InteractComp->Activate(true);
				temporal_open = false;
			}
		}

		if (InteractComp->mbInteracting)
		{
			if (mPlayer->money >= static_cast<unsigned>(cost))
			{
				mPlayer->money -= static_cast<unsigned>(cost);
				//Open();

				if (serverItIsI)
				{
					door_movement d{ id, true, false };
					network_event e = NetworkingMrg.CreateNetEvent(0, event_type::door_movement);
					std::memcpy(e.payload.data(), reinterpret_cast<char*>(&d), sizeof(d));
					NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
				}
				else // (!serverItIsI)
				{
					door_movement d{ id, true, false };
					network_event e = NetworkingMrg.CreateNetEvent(mPlayer->getId(), event_type::door_movement_request);
					std::memcpy(e.payload.data(), reinterpret_cast<char*>(&d), sizeof(d));
					NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
				}

			}
		}

		if (mbOpen)
			mOwner->mTransform.mPosition = lerp(mOwner->mTransform.mPosition, end_pos, door_speed);
		else
			mOwner->mTransform.mPosition = lerp(mOwner->mTransform.mPosition, init_pos, door_speed);
	}
	else
	{
		if (InteractComp->mbInteracting)
		{
			lerp_dt += FRC.GetFrameTime() * 0.5f;
			mOwner->mTransform.mPosition = lerp(init_pos, end_pos, lerp_dt);
			InteractComp->SetBar(lerp_dt);

			if (lerp_dt >= 1.0f)
			{
				//OpenTemp();

				if (serverItIsI)
				{
					door_movement d{ id, true, true };
					network_event e = NetworkingMrg.CreateNetEvent(0, event_type::door_movement);
					std::memcpy(e.payload.data(), reinterpret_cast<char*>(&d), sizeof(d));
					NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
				}
				else // (!serverItIsI)
				{
					door_movement d{ id, true, true };
					network_event e = NetworkingMrg.CreateNetEvent(mPlayer->getId(), event_type::door_movement_request);
					std::memcpy(e.payload.data(), reinterpret_cast<char*>(&d), sizeof(d));
					NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
				}
			}
		}
		else
		{
			mOwner->mTransform.mPosition = lerp(mOwner->mTransform.mPosition, init_pos, door_speed);
			lerp_dt = glm::distance(mOwner->mTransform.mPosition, init_pos) / glm::distance(init_pos, end_pos);
		}
	}
}

void Door::Open()
{

	mbOpen = !mbOpen;
	for (Door* door : linked_doors)
		if(!door->mbOpen)
			door->Open();
	InteractComp->StopInteracting(false);

	if (emitter)
		emitter->PlayCue("./../Resources/Audio/Scifi_DoorOpen.wav", 0.45f, false, false, false);
}

void Door::OpenTemp()
{
	mbOpen = true;
	temporal_open = true;
	InteractComp->StopInteracting(false);
	lerp_dt = 0.0f;
	timer = 0.0f;

	if (emitter)
		emitter->PlayCue("./../Resources/Audio/Scifi_DoorOpen.wav", 0.45f, false, false, false);
}

void Door::TaskActivate()
{
	mbPrevState = mbOpen;
	mbTaskActive = true;
	InteractComp->SetCustomMsg(true, "No Power, press E to open");
	InteractComp->Activate(true);
}

void Door::TaskEnd()
{	
	mbOpen = mbPrevState;
	mbTaskActive = false;
	temporal_open = false;
	InteractComp->SetCustomMsg(false);
	InteractComp->Activate(!mbOpen);
}

bool Door::ChecInteract()
{
	if (!mbTaskActive)
		return mPlayer->money >= static_cast<unsigned>(cost);
	return true;
}

void Door::Shutdown()
{
	mOwner->mTransform.mPosition = init_pos;
}

#ifdef EDITOR
bool Door::Edit()
{
	bool changed = false;

	ImGui::DragInt("Money Cost", &cost);
	ImGui::DragFloat("Door Speed", &door_speed, 0.001f);
	ImGui::DragFloat3("End Position", &end_pos[0], 0.1f);
	if (ImGui::Checkbox("check new position", &check_new_pos))
	{
		if (!check_new_pos)
			mOwner->mTransform.mPosition = init_pos;
	}
	if (ImGui::Button("Reset Position"))
		end_pos = init_pos;

	if (!check_new_pos)
		init_pos = mOwner->mTransform.mPosition;
		
	else
		mOwner->mTransform.mPosition = end_pos;

	return changed;
}
#endif

void Door::ToJson(nlohmann::json& j) const
{
	j["end_pos"] << end_pos;
	j["door_speed"] << door_speed;
	j["cost"] << cost;
}

void Door::FromJson(nlohmann::json& j)
{
	if (j.find("end_pos") != j.end())
		j["end_pos"] >> end_pos;
	if (j.find("door_speed") != j.end())
		j["door_speed"] >> door_speed;
	if (j.find("cost") != j.end())
		j["cost"] >> cost;
}

IComp* Door::Clone()
{
	return Scene.CreateComp<Door>(mOwner->GetSpace(), this);
}