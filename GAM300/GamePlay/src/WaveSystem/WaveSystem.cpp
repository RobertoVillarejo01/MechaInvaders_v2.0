#include "../Engine/src/Objects/GameObject.h"
#include "../Engine/src/Utilities/FrameRateController/FrameRateController.h"
#include "../Engine/src/AudioManager/Audio.h"
#include "UtilityComponents/FadeInOut.h"
#include "Spawner/Spawner.h"
#include "WaveSystem.h"
#include "TaskSystem/TaskSystem.h"
#include <iterator>
#include "Enemies\Robot\Robot.h"
#include "GameStateManager/GameStateManager.h"
#include "../Engine/src/Utilities/Input/Input.h"
#include "AudioManager/Audio.h"
#include "Networking\Networking.h"

WaveSystem::~WaveSystem()
{

}

void WaveSystem::Initialize() 
{
	std::ifstream file("./../Resources/Score.json");
	if (file.good() && file.is_open())
	{
		nlohmann::json j;
		file >> j;

		if (j.find("Highest_Round") != j.end())
			j["Highest_Round"] >> highest_round;
		file.close();
	}

	onWave = false;
	actual_round = 0;
	round_num = 0;

	//Spawners
	oSpawners = Scene.FindObjects(Tags::Spawner);
	cSpawners.clear();

	int spawner_id = 0;
	for (auto spawner : oSpawners) {
		Spawner* s = spawner->GetComponentType<Spawner>();
		s->setID(spawner_id);
		cSpawners.insert({ spawner_id++, s });
	}
	for (auto spawner : cSpawners)
	{
		if (spawner.second->activated)
			active_spawners.push_back(spawner.second);
	}

	//Scene.GetComponentsType<Spawner>();
	ClearSpawns();

	//Players  
	players = Scene.FindObjects(Tags::Player);

	//SoundEmmiter
	if (Scene.FindObject("Emitter") && Scene.FindObject("Emitter")->GetComponentType<SoundEmitter>())
		emitter = Scene.FindObject("Emitter")->GetComponentType<SoundEmitter>();

	if(Scene.FindObject("Letters") && Scene.FindObject("Letters")->GetComponentType<FadeInOut>())
		letter = Scene.FindObject("Letters")->GetComponentType<FadeInOut>();

	spawned_enemies = 0;
	defeated_enemies = 0;
	enemies_this_round = 10;
	max_enemies_ingame = 20;

	spawn_amount = 1;
	range = 200;
	max_velocity = 50.0f;
	min_velocity = 1.0f;
	max_time_spawn = 8.0f;
	min_time_spawn = 3.0f;
	health = 4;

	timer_rounds = 0.0f;
	time_between_rounds = 5.0f;
	timer_spawning = 0.0f;
	spawning_deltat = 5.0f;

	order_per_iteration = 1;

	enemies.clear();

	serverItIsI = NetworkingMrg.AmIServer();
}

void WaveSystem::Update() 
{
	if (serverItIsI)
	{
		if (!onWave)
			timer_rounds += FRC.GetFrameTime();
		if (timer_rounds >= time_between_rounds)
		{
			//StartWave();
			WaveStartEvent(); //send event through network
			timer_rounds = 0.0f;
		}
		if (onWave)
		{
			UpdateWave();
			if (defeated_enemies == enemies_this_round /*&& TaskSys.actualtasktag == TaskSys.None*/) {
				//EndWave();
				WaveEndEvent(); //send event through network
			}
		}
		if (GSM.mConfig.mbCheats)
			CheatCodes();
	}
}

void WaveSystem::Shutdown() 
{
	//change the highest round
	if (highest_round <= round_num)
	{
		highest_round = round_num;
		std::ofstream file("./../Resources/Score.json");
		if (file.good() && file.is_open()) 
		{
			nlohmann::json j;
			j["Highest_Round"] = highest_round;
			file << std::setw(4) << j;
			file.close();
		}
	}

	round_num = 0;
	letter = nullptr;
	emitter = nullptr;
	enemies.clear();
	cSpawners.clear();
	active_spawners.clear();
}

void WaveSystem::StartWave() 
{
	onWave = true;
	round_num += 1;
	if(emitter)
		emitter->PlayCue("./../Resources/Audio/RoundBegin.wav", 0.75f, false, false, true);
	if (letter)
		letter->trigger = true;

	spawned_enemies = 0;
	defeated_enemies = 0;
}

void WaveSystem::UpdateWave()
{
	if (spawned_enemies < enemies_this_round)
	{
		timer_spawning += FRC.GetFrameTime();
		if (timer_spawning >= spawning_deltat)
		{
			SendOrder(ChooseSomeSpawners(active_spawners, order_per_iteration));
			timer_spawning = 0.0f;
		}
	}
}

void WaveSystem::EndWave() 
{
	if(emitter)
		emitter->PlayCue("./../Resources/Audio/RoundEnd.wav", 0.75f, false, false, true);
	enemies.clear();
	for (auto spawner : cSpawners)
		spawner.second->ClearQueues();
	enemies.clear();
	UpdateStats();
	onWave = false;
}
//ADJUST
void WaveSystem::UpdateStats() 
{
	//ENEMIES PER ROUND
	enemies_this_round += 4;
	//MAX ENEMIES ON SCREEN
	if(max_enemies_ingame < 25)
		max_enemies_ingame += 1;
	//SPAWN AMOUNT
	if(round_num == 10)
		spawn_amount += 1;
	else if(round_num == 20)
		spawn_amount += 1;
	//SPAWNING TIMERS
	max_time_spawn /= 1.05f;
	min_time_spawn /= 1.05f;
	//ENEMY STATS
	health += 2;
	//SPAWNING POINTS PER ITERATION
	if (round_num == 2)
		order_per_iteration += 1;
	else if (round_num == 4)
		order_per_iteration += 1;
	else if (round_num == 8)
		order_per_iteration += 1;
	else if (round_num == 12)
		order_per_iteration += 1;
	else if (round_num == 17)
		order_per_iteration += 1;
	else if (round_num == 22)
		order_per_iteration += 1;
}

std::vector<Spawner*> WaveSystem::ChooseSomeSpawners(std::vector<Spawner*> spawners, int amount)
{
	if (amount >= spawners.size())
		return spawners;
	std::vector<Spawner*> output;
	for (int i = 0; i < amount; ++i)
	{
		int random = rand() % spawners.size();
		output.push_back(spawners[random]);
		spawners.erase(spawners.begin() + random);
	}
	return output;
}

void WaveSystem::SendOrder(std::vector<Spawner*> spawners, float time, unsigned int amount)
{
	for (auto spawner : spawners)
	{
		//Calculate the random velocity between an interval
		float vel = static_cast<float>(rand()) / (RAND_MAX / (max_velocity - min_velocity));
		EnemyStats stats = EnemyStats(health, vel);
		//Calculate the amount of enemies to spawn in an interval
		if (amount == 0)
			amount = rand() % spawn_amount + 1;
		//Calculate the random time that will last the spawning 
		if (time == 0)
			time = static_cast<float>(rand()) / (RAND_MAX / (max_time_spawn - min_velocity));
		SpawnInfo sInfo = SpawnInfo("Robot", stats, amount, time);
		//spawner->CreateOrder(sInfo);
		SendOrdeEvent(spawner, sInfo);
	}

	//spawn info
}

void WaveSystem::ClearSpawns()
{
	for (auto spawner : cSpawners)
	{
		spawner.second->ClearQueues();
	}
}


//--------------------------SEARCHING METHODS-------------------------------

std::vector<Spawner*> WaveSystem::ChooseNearSpawners(int amount, glm::vec3 position)
{
	std::vector<Spawner*> nearests;
	std::unordered_map<unsigned int, Spawner*> temp = cSpawners;
	Spawner* temp_sp = nullptr; 
	float length;
	float distance;

	for (int i = 0; i < amount; i++)
	{
		length = INFINITY;
		for (auto sp : temp)
		{
			distance = glm::length(position - sp.second->mOwner->mTransform.mPosition);
			if (distance < length && std::find(nearests.begin(), nearests.end(), sp.second) == nearests.end())
			{
				length = distance;
				temp_sp = sp.second;
			}
		}
		nearests.push_back(temp_sp);
	}
	return nearests;
}

std::vector<Spawner*> WaveSystem::GetSpawnersFromRoom(int room)
{
	std::vector<Spawner*> spawners;
	for(auto spawner : cSpawners)
	{
			if (spawner.second->GetRoom() == room)
				spawners.push_back(spawner.second);
	}
	return spawners;
}

void WaveSystem::WaveStartEvent()
{
	//wave_info wave(round_num);
	actual_round += 1;

	network_event e = NetworkingMrg.CreateNetEvent(0, event_type::start_wave);

	//std::memcpy(e.payload.data(), reinterpret_cast<char*>(&wave), sizeof(wave));

	NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
}

void WaveSystem::WaveEndEvent()
{
	network_event e = NetworkingMrg.CreateNetEvent(0, event_type::end_wave);

	NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
}

void WaveSystem::SendOrdeEvent(Spawner* spawner, SpawnInfo& info)
{
	int spawner_id = spawner->getID();
	int enemy_type = info.name == "Robot" ? 0 : 1;

	spawn_info s{ spawner_id, enemy_type, info.stats.health, info.stats.velocity, info.spawn_num, info.time };

	network_event e = NetworkingMrg.CreateNetEvent(0, event_type::spawner_orde);

	std::memcpy(e.payload.data(), reinterpret_cast<char*>(&s), sizeof(s));

	NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
}

void WaveSystem::SendOrdePacket(spawn_info& info)
{
	if (active_spawners.empty()) return;

	std::string name = info.enemy_type == 0 ? "Robot" : "Fly";

	EnemyStats stats{ info.health, info.velocity };
	SpawnInfo sInfo{ name, stats, info.amount, info.time };

	if (cSpawners[info.spawner_id])
		cSpawners[info.spawner_id]->CreateOrder(sInfo);
}

void WaveSystem::addEnemy(GameObject* enemy)
{
	enemy->GetComponentType<Robot>()->setEnemyID(++base_id);
	enemies.insert({ base_id, enemy });
	//enemies.push_back(enemy);
}

void WaveSystem::CheatCodes()
{
	//END WAVE
	if (KeyDown(Key::Y) && KeyTriggered(Key::Num1))
		WaveEndEvent();
	//SKIP WAVE
	if (KeyDown(Key::Y) && KeyTriggered(Key::Num2))
	{
		WaveEndEvent();
		WaveStartEvent();
	}
	//CREATE ONE ENEMY IN EACH SPAWNER
	if (KeyDown(Key::Y) && KeyTriggered(Key::Num3))
	{
		SpawnInfo info;
		info.stats.health = health;
		for (auto sp : cSpawners)
			if(sp.second->activated)
				sp.second->CreateEnemies(info);
	}
	//CLEAR ALL ENEMIES
	if (KeyDown(Key::Y) && KeyTriggered(Key::Num4))
	{
		std::vector<GameObject*> enemies;
		enemies = Scene.FindObjects(Tags::Enemy);
		for (auto enemy : enemies)
			Scene.DestroyObject(enemy);
	}
}
