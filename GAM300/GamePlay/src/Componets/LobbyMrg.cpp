#include "LobbyMrg.h"
#include "Graphics\Camera\Camera.h"
#include "Player\Player.h"
#include "Health\Health.h"
#include "Player\PlayerCamera.h"
#include "Engine.h"
#include "GameStateManager/GameStateManager.h"

#include <iostream>

#ifdef EDITOR
#include "../Editor/ImGui/imgui.h"
#endif // EDITOR

LobbyMrg::LobbyMrg()
{
}

LobbyMrg::~LobbyMrg()
{
}

void LobbyMrg::Initialize()
{
	serverItIsI = NetworkingMrg.AmIServer();
	my_id = NetworkingMrg.get_id();
	not_singleplayer = NetworkingMrg.multiplay();

	auto spawnPointsTag = mOwner->GetSpace()->FindObjects(Tags::SpawnPoint);

	for (unsigned i = 0; i < 4 && i < spawnPointsTag.size(); ++i)
		spawnPoints[i] = spawnPointsTag[i];

	if (serverItIsI)
	  CreatePlayer(0);

	MenuMgr.SetInLobby(true);
}

void LobbyMrg::Update()
{
	if (!serverItIsI) {
		my_id = NetworkingMrg.client.the_client.id;
	}

	// re send not acked events
	for (auto& it : NetworkingMrg.send_event)
	{
		NetworkingMrg.sendPacketToServer(Flag::e_network_event, it.second);
	}

	while (!NetworkingMrg.clients_conected.isEmpty())
	{
		// get the front message
		new_client report = NetworkingMrg.clients_conected.dequeue();

		for (int id = 0; id <= report.new_id; ++id)
		{
			CreatePlayer(id);
		}
	}

	while (!NetworkingMrg.eventQueue.isEmpty())
	{
		// get the front message
		network_event net_event = NetworkingMrg.eventQueue.dequeue();

		switch (net_event.type)
		{
		case event_type::level: {
			change_level l = *(reinterpret_cast<change_level*>(net_event.payload.data()));
			std::string name = GSM.GetLevel(l.level);
			GSM.SetNextLevel(name);
			break; }
		case event_type::shoot:
			PlayerComps[net_event.id]->shoot();
			break;
		case event_type::pause:
		{
			pause_menu pause = *(reinterpret_cast<pause_menu*>(net_event.payload.data()));

			if (!serverItIsI)
				MenuMgr.SetInMenu(pause.enable);

			break;
		}
		case event_type::interact:
			//PlayerComps[net_event.id]->taskEvent();
			break;
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
		while (!NetworkingMrg.input.isEmpty())
		{
			// get the front message
			move_input p = NetworkingMrg.input.dequeue();

			if (p.id != my_id && players[p.id])
				PlayerComps[p.id]->moveUpdate(p);
		}

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
					if (players[id]) 
					{
						players[id]->mTransform.mPosition = lerp(players[id]->mTransform.mPosition, report.state[id].position, 0.6f);
						PlayerCameraComps[id]->angle = report.state[id].aim_angle;
						PlayerCameraComps[id]->UpdateVectors();

						PlayerComps[id]->mbJumping = report.state[id].jumping;
						PlayerComps[id]->mbSprinting = report.state[id].sprinting;
					}
				}
				else
				{
					players[id]->mTransform.mPosition = lerp(players[id]->mTransform.mPosition, report.state[id].position, 0.6f);
				}
			}
		}
	}

	if (serverItIsI)
	{
		if (KeyDown(Key::O) && !MenuMgr.InMenu()) {

			// Get the index of the level we will be using for the game (and send an event with it)
			auto& levels = GSM.GetLevels();
			int idx = 0;
			for (auto& l : levels) {
				if (GSM.mConfig.mGameLevel == l) {
					break;
				}
				else idx++;
			}

			if (idx >= levels.size()) {
				std::cerr << "Could notfind the level : " << GSM.mConfig.mGameLevel << std::endl;
				return;
			}

			change_level cl{ idx };

			network_event e = NetworkingMrg.CreateNetEvent(0, event_type::level);

			std::memcpy(e.payload.data(), reinterpret_cast<char*>(&cl), sizeof(cl));

			NetworkingMrg.broadcast_packet(Flag::e_network_event, e);
		}
	}

	
}

void LobbyMrg::Shutdown()
{
}

#ifdef EDITOR

bool LobbyMrg::Edit()
{
	bool changed = false;

	// changed = ImGui::ColorEdit3("P1_Color", &P1_Color[0]);
	// changed = ImGui::ColorEdit3("P2_Color", &P2_Color[0]);
	// changed = ImGui::ColorEdit3("P3_Color", &P3_Color[0]);
	// changed = ImGui::ColorEdit3("P4_Color", &P4_Color[0]);

	return changed;
}

#endif

IComp* LobbyMrg::Clone()
{
	return Scene.CreateComp<LobbyMrg>(mOwner->GetSpace(), this);
}

void LobbyMrg::ToJson(nlohmann::json& j) const
{
}

void LobbyMrg::FromJson(nlohmann::json& j)
{
}

void LobbyMrg::CreatePlayer(int id)
{
	std::string player_name = "Player" + std::to_string(id);

	if (NetworkingMrg.connected_clients[id])
	{
		//sanity check in case we didnt start the run from zero
		if (players[id] = Scene.GetSpace("MainArea")->FindObject(player_name)) {
			getComponents(id);
			PlayerCameraComps[id]->angle = init_aim[id];
			if (spawnPoints[id])
				players[id]->mTransform.mPosition = spawnPoints[id]->mTransform.mPosition;

			PlayerComps[id]->InLobby = true;
		}
		else
		{
			players[id] = Scene.CreateObject(players[id], Scene.GetSpace("MainArea"));
			if (id == my_id)
				serializer.LoadArchetype("Player", players[id]);
			else
				serializer.LoadArchetype("PlayerOther", players[id]);

			players[id]->SetName(player_name.c_str());
		}

		getComponents(id);

		PlayerComps[id]->setId(id);
		PlayerComps[id]->InLobby = true;

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
	else
		if (players[id] = Scene.GetSpace("MainArea")->FindObject(player_name))
			players[id]->GetSpace()->DestroyObject(players[id]);
}

void LobbyMrg::getComponents(int id)
{
	PlayerComps[id] = players[id]->GetComponentType<Player>();
	PlayerCameraComps[id] = players[id]->GetComponentType<PlayerCamera>();
	PlayerHealthComps[id] = players[id]->GetComponentType<Health>();
}
