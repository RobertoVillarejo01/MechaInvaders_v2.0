#include "Engine.h"
#include "WaveSystem/WaveSystem.h"
#include "TaskSystem/TaskSystem.h"
#include "TaskSystem/TaskInfo.h"
#include "Spawner.h"
#include "Health/Health.h"
#include <algorithm>

#include "Networking\Networking.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR

void Spawner::Initialize()
{
	velocityIncrease = 15.0f;
	mOwner->mTag = Tags::Spawner;
	creating = false;
	ClearQueues();
}

void Spawner::Update()
{
	if (!creating)
	{
		if (onTask && TaskSys.activetasks[0]->tag == TaskTag::Fixing && !T_Queue.empty())
		{
			actual_info = T_Queue.front();
			T_Queue.pop();
			timer = 0.0f;
			creating = true;
		}
		else if (!WS_Queue.empty())
		{
			actual_info = WS_Queue.front();
			WS_Queue.pop();
			timer = 0.0f;
			creating = true;
		}
	}
	if (creating)
	{
		timer += FRC.GetFrameTime();
		if (timer >= actual_info.time)
		{
			//if (TaskSys.activetasks.empty() || onTask)
				PrepareOrders();
			creating = false;
		}
	}
	if (enemies_to_move)
		MoveEnemies(spawned_enemies);
}

void Spawner::CreateOrder(SpawnInfo info) 
{ 
	if (!onTask)
		WS_Queue.push(info);
	else
		T_Queue.push(info);
}

void Spawner::PrepareOrders()
{
	if (WaveSys.enemies_this_round - WaveSys.spawned_enemies == 0 || WaveSys.spawned_enemies - WaveSys.defeated_enemies == WaveSys.max_enemies_ingame)
		return;
	if (WaveSys.enemies_this_round - WaveSys.spawned_enemies < actual_info.spawn_num)
	{
		actual_info.spawn_num = WaveSys.enemies_this_round - WaveSys.spawned_enemies;
		WaveSys.spawned_enemies += actual_info.spawn_num;
		CreateEnemies(actual_info);
	}
	else if (WaveSys.spawned_enemies - WaveSys.defeated_enemies < WaveSys.max_enemies_ingame &&
		WaveSys.max_enemies_ingame - (WaveSys.spawned_enemies - WaveSys.defeated_enemies) < actual_info.spawn_num)
	{
		actual_info.spawn_num = WaveSys.max_enemies_ingame - (WaveSys.spawned_enemies - WaveSys.defeated_enemies);
		WaveSys.spawned_enemies += actual_info.spawn_num;
		CreateEnemies(actual_info);
	}
	else
	{
		CreateEnemies(actual_info);
		WaveSys.spawned_enemies += actual_info.spawn_num;
	}
}

void Spawner::CreateEnemies(SpawnInfo info)
{
	int randRound = rand() % (WaveSys.round_num - 1 + 1) + 1; //random velocity depending on round
	float roundF = static_cast<float>(randRound); //for choosing velocity 
	
	roundF = 20.0f + roundF * velocityIncrease; //increase velocity each round
	if(roundF > 70) roundF = 70.0f;

	if (info.spawn_num >= 1)
	{
		GameObject* enemy1 = Scene.CreateObject();
		serializer.LoadArchetype(info.name.data(), enemy1);
		enemy1->mTransform.mPosition = mOwner->mTransform.mPosition + robot_begin1;
		enemy1->GetComponentType<Health>()->SetMaxHealth((float)info.stats.health);
		//update velocity
		enemy1->GetComponentType<Robot>()->SetVelocity(roundF);
		WaveSys.addEnemy(enemy1);
		spawned_enemies.push_back(enemy1);
	}
	if (info.spawn_num >= 2)
	{
		GameObject* enemy2 = Scene.CreateObject();
		serializer.LoadArchetype(info.name.data(), enemy2);
		enemy2->mTransform.mPosition = mOwner->mTransform.mPosition + robot_begin2;
		enemy2->GetComponentType<Health>()->SetMaxHealth((float)info.stats.health);
		//update velocity
		enemy2->GetComponentType<Robot>()->SetVelocity(roundF);
		WaveSys.addEnemy(enemy2);
		spawned_enemies.push_back(enemy2);
		
	}
	if (info.spawn_num >= 3)
	{
		GameObject* enemy3 = Scene.CreateObject();
		serializer.LoadArchetype(info.name.data(), enemy3);
		enemy3->mTransform.mPosition = mOwner->mTransform.mPosition + robot_begin3;
		enemy3->GetComponentType<Health>()->SetMaxHealth((float)info.stats.health);
		//update velocity
		enemy3->GetComponentType<Robot>()->SetVelocity(roundF);
		WaveSys.addEnemy(enemy3);
		spawned_enemies.push_back(enemy3);
	}
	enemies_to_move = true;
}

void Spawner::MoveEnemies(std::vector<GameObject*> e)
{
	if (lerp_dt < 1)
	{
		lerp_dt += FRC.GetFrameTime() * WaveSys.spawn_move_vel;
		if (lerp_dt > 1)
			lerp_dt = 1.0f;
		if (e.size() >= 1)
			e[0]->mTransform.mPosition = lerp(mOwner->mTransform.mPosition + robot_begin1, mOwner->mTransform.mPosition + robot_end1, lerp_dt);
		if (e.size() >= 2)
			e[1]->mTransform.mPosition = lerp(mOwner->mTransform.mPosition + robot_begin2, mOwner->mTransform.mPosition + robot_end2, lerp_dt);
		if (e.size() >= 3)
			e[2]->mTransform.mPosition = lerp(mOwner->mTransform.mPosition + robot_begin3, mOwner->mTransform.mPosition + robot_end3, lerp_dt);
	}
	else
	{
		if (e.size() >= 1)
		{
			auto brain1 = e[0]->GetComponentType<Robot>();
			if(brain1)
				brain1->spawned = true;
		}
		if (e.size() >= 2)
		{
			auto brain2 = e[1]->GetComponentType<Robot>();
			if (brain2)
				brain2->spawned = true;
		}
		if (e.size() >= 3)
		{
			auto brain3 = e[2]->GetComponentType<Robot>();
			if (brain3)
				brain3->spawned = true;
		}
		lerp_dt = 0;
		enemies_to_move = false;
		spawned_enemies.clear();
	}
}

void Spawner::ClearQueues()
{
	ClearWSQueue();
	ClearTQueue();
}

void Spawner::ClearWSQueue()
{
	while (!WS_Queue.empty())
	{
		WS_Queue.pop();
	}
}

void Spawner::ClearTQueue()
{
	while (!T_Queue.empty())
	{
		T_Queue.pop();
	}
}

void Spawner::Activate()
{
	if (activated)
		return;
	activated = true;
	WaveSys.active_spawners.push_back(this);
}

void Spawner::Deactivate()
{
	if (!activated)
		return;
	activated = false;
	auto it = std::find(WaveSys.active_spawners.begin(), WaveSys.active_spawners.end(), this);
	if (it == WaveSys.active_spawners.end())
		return;
	else
		WaveSys.active_spawners.erase(it);
}

IComp* Spawner::Clone() { return Scene.CreateComp<Spawner>(mOwner->GetSpace(), this); }

#ifdef EDITOR
bool Spawner::Edit()
{
	bool changed = false;

	ImGui::Checkbox("Spawner activated", &activated);

	ImGui::DragFloat3("Point where Robot1 spawns: ", &robot_begin1[0], 2);
	ImGui::DragFloat3("Point where Robot2 spawns: ", &robot_begin2[0], 2);
	ImGui::DragFloat3("Point where Robot3 spawns: ", &robot_begin3[0], 2);

	ImGui::DragFloat3("Point where Robot1 gets released: ", &robot_end1[0], 2);
	ImGui::DragFloat3("Point where Robot2 gets released: ", &robot_end2[0], 2);
	ImGui::DragFloat3("Point where Robot3 gets released: ", &robot_end3[0], 2);

	ImGui::DragInt("Room: ", &room);

	//Draw position offset robot for debug
	geometry::sphere sp;
	sp.mRadius = 1;

	glm::vec4 red(1.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 blue(0.0f, 0.0f, 1.0f, 1.0f);
	glm::vec4 green(0.0f, 1.0f, 0.0f, 1.0f);

	sp.mCenter = mOwner->mTransform.mPosition + robot_begin1;
	Debug::DrawSphere(sp, red);
	sp.mCenter = mOwner->mTransform.mPosition + robot_end1;
	Debug::DrawSphere(sp, red);

	sp.mCenter = mOwner->mTransform.mPosition + robot_begin2;
	Debug::DrawSphere(sp, green);
	sp.mCenter = mOwner->mTransform.mPosition + robot_end2;
	Debug::DrawSphere(sp, green);

	sp.mCenter = mOwner->mTransform.mPosition + robot_begin3;
	Debug::DrawSphere(sp, blue);
	sp.mCenter = mOwner->mTransform.mPosition + robot_end3;
	Debug::DrawSphere(sp, blue);

	return changed;
}
#endif // EDITOR


void Spawner::ToJson(nlohmann::json& j) const
{
	j["activated"] << activated;

	j["robot_begin1"] << robot_begin1;
	j["robot_begin2"] << robot_begin2;
	j["robot_begin3"] << robot_begin3;

	j["robot_end1"] << robot_end1;
	j["robot_end2"] << robot_end2;
	j["robot_end3"] << robot_end3;

	j["room"] << room;
}

void Spawner::FromJson(nlohmann::json& j)
{
	if(j.find("activated") != j.end())
		j["activated"] >> activated;

	if(j.find("robot_begin1") != j.end())
		j["robot_begin1"] >> robot_begin1;
	if(j.find("robot_begin2") != j.end())
		j["robot_begin2"] >> robot_begin2;
	if(j.find("robot_begin3") != j.end())
		j["robot_begin3"] >> robot_begin3;

	if(j.find("robot_end1") != j.end())
		j["robot_end1"] >> robot_end1;
	if(j.find("robot_end2") != j.end())
		j["robot_end2"] >> robot_end2;
	if(j.find("robot_end3") != j.end())
		j["robot_end3"] >> robot_end3;

	if(j.find("room") != j.end())
		j["room"] >> room;
}