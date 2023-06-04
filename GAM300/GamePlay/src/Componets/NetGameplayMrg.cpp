#include "NetGameplayMrg.h"
#include "Player\PlayerCamera.h"
#include "Player\Player.h"
#include "Health\Health.h"
#include "WaveSystem\WaveSystem.h"
#include "TaskSystem\TaskSystem.h"
#include "Engine.h"
#include "Weapon\Weapon.h"
#include "Networking\Common\Common.h"

#include "Doors/Doors.h"
#include "VendingMachine/VendingMachine.h"
#include "TaskSystem\TaskInfo.h"
#include "UtilityComponents/FadeInOut.h"

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR

NetGameplayMrg::NetGameplayMrg()
{
}

NetGameplayMrg::~NetGameplayMrg()
{
}

void NetGameplayMrg::Initialize()
{
	alive_player.fill(false);
	update_y = true;
	serverItIsI = NetworkingMrg.AmIServer();
	not_singleplayer = NetworkingMrg.multiplay();
	my_id = NetworkingMrg.get_id();

	MenuMgr.SetInLobby(false);

	auto spawnPointsTag = mOwner->GetSpace()->FindObjects(Tags::SpawnPoint);

	auto fader = Scene.FindObject("GameOverFader");
	if (fader)
	{
		fader_comp = fader->GetComponentType<FadeInOut>();
		fader_comp->trigger = false;
	}


	for (unsigned i = 0; i < 4 && i < spawnPointsTag.size(); ++i)
		spawnPoints[i] = spawnPointsTag[i];

	for (int id = 0; id < max_clients; id++)
	{
		CreatePlayer(id);
	}

	if (serverItIsI)
		CreatePlayer(0);

	gameOver = false;

	auto TagDoors = mOwner->GetSpace()->FindObjects(Tags::Door);
	int door_id = 0;
	for (auto door_go : TagDoors) {
		Door* d = door_go->GetComponentType<Door>();
		if (d) {
			d->setID(door_id);
			mDoors.insert({ door_id++, d });
		}
	}

	auto TagMachine = mOwner->GetSpace()->FindObjects(Tags::VendingMachine);
	int vm_id = 0;
	for (auto vm : TagMachine) {
		VendingMachine* m = vm->GetComponentType<VendingMachine>();
		if (m) {
			m->setID(vm_id);
			mVendingMachines.insert({ vm_id++, m });
		}
	}
}

void NetGameplayMrg::Update()
{
	//============================ process events =========================================
	while (!NetworkingMrg.eventQueue.isEmpty())
	{
		// get the front message
		network_event net_event = NetworkingMrg.eventQueue.dequeue();

		switch (net_event.type)
		{
		case event_type::level:
		{
			change_level l = *(reinterpret_cast<change_level*>(net_event.payload.data()));
			GSM.SetNextLevel(GSM.GetLevel(l.level));
			break;
		}
		case event_type::shoot:
		{
			PlayerComps[net_event.id]->shoot();
			break;
		}
		case event_type::door_movement:
		{
			door_movement info = *(reinterpret_cast<door_movement*>(net_event.payload.data()));
			if (info.state)
			{
				if(!info.temp)
					mDoors[info.door_id]->Open();
				else					
					mDoors[info.door_id]->OpenTemp();

			}
			//else mDoors[info.door_id]->TaskActivate();
			break;
		}
		case event_type::door_movement_request:
		{
			door_movement d = *(reinterpret_cast<door_movement*>(net_event.payload.data()));
			
			if (net_event.id != my_id) {
				door_movement d = *(reinterpret_cast<door_movement*>(net_event.payload.data()));
				network_event e = NetworkingMrg.CreateNetEvent(0, event_type::door_movement);
				std::memcpy(e.payload.data(), reinterpret_cast<char*>(&d), sizeof(d));
				NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
			}
			break;
		}
		case event_type::vending:
		{
			vending_machine info = *(reinterpret_cast<vending_machine*>(net_event.payload.data()));
			mVendingMachines[info.machine_id]->Interact(PlayerComps[net_event.id]);
			break;
		}
		case event_type::vending_reqest:
		{
			if (net_event.id != my_id) {
				vending_machine v = *(reinterpret_cast<vending_machine*>(net_event.payload.data()));
				network_event e = NetworkingMrg.CreateNetEvent(0, event_type::vending);
				std::memcpy(e.payload.data(), reinterpret_cast<char*>(&v), sizeof(v));
				NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
			}
			break;
		}
		case event_type::task_interact:
		{
			task_interaction info = *(reinterpret_cast<task_interaction*>(net_event.payload.data()));
			TaskSys.tasks[info.task_id]->EndTask();
			break;
		}
		case event_type::task_interact_request:
		{
			if (net_event.id != my_id) {
				task_interaction v = *(reinterpret_cast<task_interaction*>(net_event.payload.data()));
				network_event e = NetworkingMrg.CreateNetEvent(0, event_type::task_interact);
				std::memcpy(e.payload.data(), reinterpret_cast<char*>(&v), sizeof(v));
				NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
			}
			break;
		}
		case event_type::weapon_new:
		{
			buy_weapon weapon = *(reinterpret_cast<buy_weapon*>(net_event.payload.data()));
			PlayerComps[net_event.id]->buy_weapon(weapon.name.data(), weapon.weapon_cost);
			break;
		}
		case event_type::weapon_new_request:
		{
			if (net_event.id != my_id) {
				buy_weapon weapon = *(reinterpret_cast<buy_weapon*>(net_event.payload.data()));
				network_event e = NetworkingMrg.CreateNetEvent(net_event.id, event_type::weapon_new);
				std::memcpy(e.payload.data(), reinterpret_cast<char*>(&weapon), sizeof(weapon));
				NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
			}
			break;
		}
		case event_type::weapon_change:
		{
			PlayerComps[net_event.id]->ChangeWeapon();
			break;
		}
		case event_type::weapon_change_request:
		{
			if (net_event.id != my_id) {
				network_event e = NetworkingMrg.CreateNetEvent(net_event.id, event_type::weapon_change);
				NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
			}
			break;
		}
		case event_type::start_wave:
		{
			WaveSys.StartWave();
			break;
		}
		case event_type::end_wave:
		{
			WaveSys.EndWave();
			break;
		}
		case event_type::spawner_orde:
		{
			spawn_info info = *(reinterpret_cast<spawn_info*>(net_event.payload.data()));
			WaveSys.SendOrdePacket(info);
			break;
		}
		case event_type::interact:
		{
			PlayerComps[net_event.id]->interactEvent();
			break;
		}
		case event_type::start_task:
		{
			task_activation task = *(reinterpret_cast<task_activation*>(net_event.payload.data()));
			TaskSys.StartTaskEvent(task);
			break;
		}
		case event_type::end_task:
		{
			if(!NetworkingMrg.AmIServer())
				TaskSys.EndActiveTasks();
			break;
		}
		case event_type::player_dead:
		{
			alive_player[net_event.id] = false;
			if(!PlayerComps[net_event.id]->mbDead)
				PlayerComps[net_event.id]->ChangeState("Die");
			break;
		}
		case event_type::respawn:
		{
			respawnPlayer(net_event.id);
			break;
		}
		case event_type::game_over:
		{
			gameOver = true;
			if (my_id < PlayerComps.size() && PlayerComps[my_id])
				PlayerComps[my_id]->GameOverEvent();

			//NetworkingMrg.GoBackMenu();
			fader_comp->trigger = true;
			//NetworkingMrg.ShutDownMenu();
			MenuMgr.Reset();

			break;
		}
		case event_type::pause:
		{
			pause_menu pause = *(reinterpret_cast<pause_menu*>(net_event.payload.data()));

			MenuMgr.SetInMenu(pause.enable);

			break;
		}
		default:
			break;
		}

		NetworkingMrg.send_event.erase(net_event.num);
	}

	//============================ player state reports =========================================

	//The server will simulate the remote players' input and notify back with the rest of the players' kinematic state
	if (serverItIsI)
	{
		//Update Clients from packets;
		UpdateClients();

		if (not_singleplayer) {
			//fill game status report and broadcast
			status_report report{};
			report.id = 0; //host id

			for (int id = 0; id < max_clients; id++) {
				if (players[id]) {
					report.state[id].position = players[id]->mTransform.mPosition;
					report.state[id].aim_angle = PlayerCameraComps[id]->angle;

					///maybe bitset info
					report.state[id].jumping = PlayerComps[id]->mbJumping;
					report.state[id].sprinting = PlayerComps[id]->mbSprinting;

					report.state[id].current_health = PlayerHealthComps[id]->getCurrentHealth();

					//report.state[id].money = PlayerComps[id]->money;
					// report.state[id].bullet_count = WeaponComps[id]->bullet_count;
					// report.state[id].total_bullets = WeaponComps[id]->total_bullets;
				}
			}

			NetworkingMrg.broadcast_packet(Flag::e_status_report, report);
		}
	}
	else {

		//update from game status report
		while (!NetworkingMrg.status_reports.isEmpty())
		{
			// get the front message
			status_report report = NetworkingMrg.status_reports.dequeue();

			for (int id = 0; id < max_clients; id++)
			{
				if (id != my_id)
				{
					if (players[id]) {
						players[id]->mTransform.mPosition = report.state[id].position;
						//players[id]->mTransform.mPosition = lerp(players[id]->mTransform.mPosition, report.state[id].position, 0.6f);
						PlayerCameraComps[id]->angle = report.state[id].aim_angle;
						PlayerCameraComps[id]->UpdateVectors();

						PlayerComps[id]->mbJumping = report.state[id].jumping;
						PlayerComps[id]->mbSprinting = report.state[id].sprinting;
						PlayerHealthComps[id]->setCurrentHealth(report.state[id].current_health);

						//PlayerComps[id]->money = report.state[id].money;
						// WeaponComps[id]->bullet_count = report.state[id].bullet_count;
						// WeaponComps[id]->total_bullets = report.state[id].total_bullets;
					}
				}
				else
				{
					// if(KeyDown(Key::M))
					// 		__debugbreak();

					bool nan = glm::any(glm::isnan(report.state[id].position));
					bool inf = glm::any(glm::isinf(report.state[id].position));

					if (!nan && !inf)
					{
						if (players[id]->mTransform.mPosition.x != report.state[id].position.x)
							players[id]->mTransform.mPosition.x = lerp(players[id]->mTransform.mPosition.x, report.state[id].position.x, 0.6f);

						if (players[id]->mTransform.mPosition.z != report.state[id].position.z)
							players[id]->mTransform.mPosition.z = lerp(players[id]->mTransform.mPosition.z, report.state[id].position.z, 0.6f);

						if (update_y) {
							players[id]->mTransform.mPosition = report.state[id].position;
							update_y = false;
						}

						if (PlayerHealthComps[id]->getCurrentHealth() != report.state[id].current_health)
							PlayerHealthComps[id]->setCurrentHealth(report.state[id].current_health);
					}

					//PlayerComps[id]->money = report.state[id].money;
					// if (WeaponComps[id]->total_bullets != report.state[id].total_bullets) {
					// 	WeaponComps[id]->bullet_count = report.state[id].bullet_count;
					// 	WeaponComps[id]->total_bullets = report.state[id].total_bullets;
					// }
				}
			}
		}
	}

	//============================ player state reports =========================================

	// The server will simulate the remote enemy behavior authoritatively and perform correction on client-side
	if (serverItIsI)
	{
		if (not_singleplayer) {
			enemy_report report{};
			report.id = 0; //host id

			auto enemies = mOwner->GetSpace()->FindObjects(Tags::Enemy);
			report.enemy_count = (unsigned int)enemies.size();

			for (unsigned i = 0; i < report.enemy_count; i++)
			{
				GameObject* obj = enemies[i];
				Robot* enemy = obj->GetComponentType<Robot>();
				unsigned int enemy_id = enemy->getEnemyID();
				//int state;
				unsigned int following_player_id = enemy->getFollowingPlayerId();
				glm::vec3 pos = obj->mTransform.mPosition;
				float health = obj->GetComponentType<Health>()->getCurrentHealth();

				report.state[i] = { enemy_id, following_player_id, pos, health };
			}

			if (report.enemy_count)
				NetworkingMrg.broadcast_packet(Flag::e_enemy_report, report);
		}
	}
	else
	{
		while (!NetworkingMrg.enemy_reports.isEmpty())
		{
			// get the front message
			enemy_report report = NetworkingMrg.enemy_reports.dequeue();

			unsigned enemy_count = report.enemy_count;
			for (unsigned i = 0; i < enemy_count; i++)
			{
				auto enemy = WaveSys.enemies.find(report.state[i].enemy_id);
				if (enemy != WaveSys.enemies.end() && enemy->second) {
					if (enemy->second->mTransform.mPosition != report.state[i].pos)
						enemy->second->mTransform.mPosition = lerp(enemy->second->mTransform.mPosition, report.state[i].pos, 0.6f);

					auto health = enemy->second->GetComponentType<Health>();
					if (health)
						health->setCurrentHealth(report.state[i].health);

					auto robot = enemy->second->GetComponentType<Robot>();
					if (robot)
						robot->setFollowingPlayerId(report.state[i].following_player_id);
				}
			}
		}
	}

	//============================ check if game has ended =========================================

	IsGameOverYet();

	if (gameOver) 
	{
		if (!NetworkingMrg.multiplay())
		{
			//NetworkingMrg.GoBackMenu();
			fader_comp->trigger = true;
			//NetworkingMrg.ShutDownMenu();
			MenuMgr.Reset();
		}
		// else 
		// {
		// 	change_level cl{ 3 };
		// 	network_event e = NetworkingMrg.CreateNetEvent(0, event_type::level);
		// 	std::memcpy(e.payload.data(), reinterpret_cast<char*>(&cl), sizeof(cl));
		// 
		// 	NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
		// }
	}

	//============================ re-send event =========================================

	// re send not acked events
	for (auto& it : NetworkingMrg.send_event)
	{
		NetworkingMrg.sendPacketToServer(Flag::e_network_event, it.second);
	}
}

void NetGameplayMrg::Shutdown()
{

}

#ifdef EDITOR

bool NetGameplayMrg::Edit()
{
	return false;
}

#endif

IComp* NetGameplayMrg::Clone()
{
	return Scene.CreateComp<NetGameplayMrg>(mOwner->GetSpace(), this);
}

void NetGameplayMrg::ToJson(nlohmann::json& j) const
{
}

void NetGameplayMrg::FromJson(nlohmann::json& j)
{
}

void NetGameplayMrg::CreatePlayer(int id)
{
	std::string player_name = "Player" + std::to_string(id);

	if (NetworkingMrg.connected_clients[id])
	{
		alive_player[id] = true;

		//sanity check in case we didnt start the run from zero
		if (players[id] = Scene.GetSpace("MainArea")->FindObject(player_name)) {
			getComponents(id);
			PlayerCameraComps[id]->angle = init_aim[id];
			if (spawnPoints[id])
				players[id]->mTransform.mPosition = spawnPoints[id]->mTransform.mPosition;

			PlayerComps[id]->InLobby = false;
		}
		else
		{
			players[id] = Scene.CreateObject(players[id], Scene.GetSpace("MainArea"));
			if(id == my_id)
				serializer.LoadArchetype("Player", players[id]);
			else
				serializer.LoadArchetype("PlayerOther", players[id]);

			players[id]->SetName(player_name.c_str());
		}

		getComponents(id);

		PlayerComps[id]->setId(id);
		PlayerComps[id]->InLobby = false;

		if (id == my_id)
		{
			PlayerComps[id]->base_player = true;
			players[id]->GetComponentType<CameraComp>()->render = true;
			PlayerCameraComps[id]->base_player = true;
		}
		else
		{
			PlayerComps[id]->base_player = false;
			players[id]->GetComponentType<CameraComp>()->render = false;
			PlayerCameraComps[id]->base_player = false;
		}
		
		//color for visualy distinguis
		renderable* render = players[id]->GetComponentType<renderable>();

		if (render) 
			render->SetColor(glm::normalize(color[id]));

		PlayerCameraComps[id]->angle = init_aim[id];
		if (spawnPoints[id])
			players[id]->mTransform.mPosition = spawnPoints[id]->mTransform.mPosition;
	}
	else {
		alive_player[id] = false;

		if (players[id] = Scene.GetSpace("MainArea")->FindObject(player_name))
			players[id]->GetSpace()->DestroyObject(players[id]);
	}
}

void NetGameplayMrg::UpdateClients()
{
	while (!NetworkingMrg.input.isEmpty())
	{
		// get the front message
		move_input p = NetworkingMrg.input.dequeue();

		if (p.id != my_id)
			PlayerComps[p.id]->moveUpdate(p);
	}
}


void NetGameplayMrg::IsGameOverYet()
{
	if (serverItIsI) {
		bool everyone_dead = true;

		for (bool alive : alive_player)
			if (alive)
				everyone_dead = false;

		if (everyone_dead) {
			network_event e = NetworkingMrg.CreateNetEvent(0, event_type::game_over);
			NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
		}
	}

}

void NetGameplayMrg::respawnPlayer(int dead_player_id)
{
	PlayerComps[dead_player_id]->ChangeState("Revive");
	alive_player[dead_player_id] = true;

	update_y = true;
}

void NetGameplayMrg::getComponents(int id)
{
	PlayerComps[id] = players[id]->GetComponentType<Player>();
	PlayerCameraComps[id] = players[id]->GetComponentType<PlayerCamera>();
	PlayerHealthComps[id] = players[id]->GetComponentType<Health>();

	auto weapon = players[id]->FindChild("Pistol");
	if (weapon) WeaponComps[id] = weapon->GetComponentType<Weapon>();
}
