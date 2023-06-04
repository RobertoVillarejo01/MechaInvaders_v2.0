#include "Rooms.h"
#include "Spawner/Spawner.h"
#include "TaskSystem/TaskInfo.h"
#include "Player/Player.h"
#include "Collisions/CollisionSystem.h"


#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#include "Graphics/DebugDraw/DebugDrawing.h"
#endif // EDITOR

#include "Utilities/Utils.h"

void RoomSystem::Initialize()
{
	auto player = Scene.get_base_player();
	if (player)
		mPlayer = player->GetComponentType<Player>();

	players = mOwner->GetSpace()->FindObjects(Tags::Player);

	auto childs = mOwner->GetChilds();
	auto spawners = mOwner->GetSpace()->FindObjects(Tags::Spawner);
	auto tasks = mOwner->GetSpace()->FindObjects(Tags::Task);

	for (auto new_room : childs)
	{
		Room room = { new_room };

		//add the spawners that belong to the room
		for (auto spawner : spawners)
		{
			geometry::sphere sph = { spawner->mTransform.mPosition, spawner->mTransform.mScale.x /2.0f};
			geometry::obb zone = { new_room->mTransform.mPosition, new_room->mTransform.mScale / 2.0f,
						   new_room->mTransform.GetRotMtx() };
			if (CollisionManager.SpherevsOBB(sph, zone))
				room.mSpawners.push_back(spawner->GetComponentType<Spawner>());
		}

		//add the task to the room if needed
		for (auto task : tasks)
		{
			geometry::sphere sph = { task->mTransform.mPosition, task->mTransform.mScale.x };
			geometry::obb zone = { new_room->mTransform.mPosition, new_room->mTransform.mScale / 2.0f,
						   new_room->mTransform.GetRotMtx() };
			if (CollisionManager.SpherevsOBB(sph, zone))
				room.mTasks.push_back(task->GetComponentType<TaskInfo>());
		}
		rooms[new_room->GetName()] = room;
	}
}

void RoomSystem::Update()
{
	if(players.empty())
		players = mOwner->GetSpace()->FindObjects(Tags::Player);

	for (auto& it : rooms)
	{
		GameObject* room = it.second.mRoom;
		geometry::obb zone = { room->mTransform.mPosition, room->mTransform.mScale / 2.0f,
						   room->mTransform.GetRotMtx() };

		bool success = false;
		for (GameObject* player : players)
		{
			geometry::sphere sph = { player->mTransform.mPosition, player->mTransform.mScale.x / 2.0f };
			if (CollisionManager.SpherevsOBB(sph, zone))
			{
				ActivateRoom(it.second);
				success = true;
				break;
			}
		}
	}
}

void RoomSystem::ActivateRoom(Room room)
{
	room.active = true;
	for (auto& spawner : room.mSpawners)
		spawner->Activate();
	for (auto& task : room.mTasks)
		task->MakeAvailable();
}

void RoomSystem::DeactivateRoom(Room room)
{
	room.active = false;
	for (auto& spawner : room.mSpawners)
		spawner->Deactivate();
}

void RoomSystem::Shutdown()
{

}

#ifdef EDITOR
bool RoomSystem::Edit()
{
	bool changed = false;

	if (ImGui::Selectable("Create Room"))
	{
		std::string name("Room");
		char buffer[100];
		_itoa_s(static_cast<unsigned>(rooms.size()), buffer, sizeof(buffer), 10);
		name += buffer;

		auto new_room = mOwner->CreateChild();
		new_room->SetName(name.c_str());
		new_room->mbUseParentPosition = false;
		rooms[name] = Room(new_room);
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	
	if (ImGui::TreeNode("Edit Rooms"))
	{
		for (auto& it : rooms)
		{
			ImGui::Bullet();
			ImGui::SameLine();
			if (ImGui::TreeNode(it.first.c_str()))
			{
				GameObject* room = it.second.mRoom;
				geometry::obb zone = { room->mTransform.mPosition, room->mTransform.mScale / 2.0f,
						   room->mTransform.GetRotMtx() };
				room->EditTransform();
				Debug::DrawOBB(zone, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
				ImGui::TreePop();
			}
			ImGui::Separator();
		}
		ImGui::TreePop();
	}
	return changed;
}
#endif

void RoomSystem::ToJson(nlohmann::json& j) const
{
}

void RoomSystem::FromJson(nlohmann::json& j)
{

}

IComp* RoomSystem::Clone()
{
	return Scene.CreateComp<RoomSystem>(mOwner->GetSpace(), this);
}