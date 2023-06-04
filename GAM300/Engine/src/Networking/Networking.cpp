#include "Networking.h"
#include "../GamePlay/src/Componets/NetGameplayMrg.h"
#include "GameStateManager\GameStateManager.h"
#include "GameStateManager\MenuManager\MenuManager.h"

Networking::~Networking() { ShutDown(); }

void Networking::Initialize(bool custom)
{
	connected_clients.fill(false);

	WSAData data{};

	//Initialize Winsock
	auto result = WSAStartup(MAKEWORD(2, 2), &data);
	if (result != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		exit(-1);
	}

	random_seed = static_cast<unsigned int>(time(NULL) % 1000);

	if (custom)
	{
		//change when we have a menu
		// {
		// 	std::string data;
		// 	printf("Write 'server' or 's' to be a SERVER: \n");
		// 	printf("Write 'client' or 'c' to be a CLIENT: \n");
		// 	printf("Write anything to play SINGLEPLAYER:  \n");
		// 	std::getline(std::cin, data);
		// 
		// 	if (data == std::string("server") || data == std::string("s"))
		// 		InitializeAsServer();
		// 	else if (data == std::string("client") || data == std::string("c"))
		// 		InitializeAsClient();
		// 	else
		// 		InitializeAsSinglePlayer();
		// }

		what = SetUp::SINGLEPLAYER;
	}
	else
	{
		InitializeAsSinglePlayer();
	}
}

void Networking::InitializeAsServer()
{
	what = SetUp::SERVER;

	server.setUp(GSM.mConfig.mbPort, GSM.mConfig.mbIP);
	server.initialize();

	//server id
	my_id = 0;
	connected_clients[0] = true;
	//initialize the random
	srand(random_seed);

	//GSM.SetNextLevel("LobbyLevel");
	playin_multiplayer = true;
}

void Networking::InitializeAsClient()
{
	what = SetUp::CLIENT;

	std::string ADDRESS = GSM.mConfig.mbIP;

	if (!GSM.mConfig.mbLocalHost) {
		std::cin >> ADDRESS;
	}

	client.setUp(GSM.mConfig.mbPort, ADDRESS);
	client.initialize();

	my_id = -1;

	//GSM.SetNextLevel("LobbyLevel");
	playin_multiplayer = true;
}

void Networking::InitializeAsSinglePlayer()
{
	what = SetUp::SINGLEPLAYER;
	//server id
	my_id = 0;
	connected_clients[0] = true;
	//initialize the random
	srand(random_seed);
	playin_multiplayer = false;
}

void Networking::Update()
{
	switch (what)
	{
	case SetUp::SERVER:
		update_server();
		break;
	case SetUp::CLIENT:
		update_client();
		break;
	case SetUp::SINGLEPLAYER:
		break;
	default:
		break;
	}

}

void Networking::update_server()
{
	if (!thread_created) {
		std::thread my_thread(&process_server);
		my_thread.detach();
		thread_created = true;
	}
}

void Networking::update_client()
{
	if (!client.connected) //check if connected
	{
		bool connection_failed = client.connect();
		if (!connection_failed)
		{
			printf("Client error: connection failed \n");
			NetworkingMrg.GoBackMenu();
			NetworkingMrg.ShutDownMenu();
			MenuMgr.Reset();
		}

		client.connected = true;
		client.the_client.in_use = true;
		client.the_client.id = max_clients;
	}
	else {
		if (!thread_created) {
			std::thread my_thread(&process_client);
			my_thread.detach();
			thread_created = true;
		}
	}
}

void Networking::startFrame()
{

}

void Networking::ShutDown()
{
	ShutDownMenu();
	// cleanup
	WSACleanup();
}

void Networking::ShutDownMenu()
{
	cleanPackets();

	switch (what)
	{
	case SetUp::SERVER:
		server.disconnect();
		server.free();
		break;
	case SetUp::CLIENT:
		client.disconnect();
		client.free();
		break;
	case SetUp::SINGLEPLAYER:
		break;
	default:
		break;
	}

	MenuMgr.SetInLobby(true);

	connected_clients.fill(false);

	what = SetUp::SINGLEPLAYER;
}

int Networking::process_client()
{
	while (client.the_client.in_use)
	{
		//receive a reply and print it
		Flag flag(Flag::NONE);

		Host_Safe::receive_packet(client.recvbuf, BUFF_SIZE, flag, client.the_client.socket, client.the_client.client_addr);

		if (flag == Flag::FIN) {
			printf("> disconnecting from server... \n");
			client.disconnect();
			NetworkingMrg.GoBackMenu();
			NetworkingMrg.ShutDownMenu();
			MenuMgr.Reset();
		}
		else if (flag != Flag::SYN && flag != Flag::FIN && flag != Flag::NONE)
		{
			std::vector<char>received;
			for (int i = 0; i < BUFF_SIZE; i++)
			{
				received.push_back(client.recvbuf[i]);
			}
			NetworkingMrg.AddPacket(flag, received);
		}
	}

	NetworkingMrg.thread_created = false;
	return 0;
}

int Networking::process_server()
{
	bool exit = false;
	while (server.connected)
	{
		for (Host::CLIENTINFO& client : server.clients) {
			//Accept new client
			sockaddr_in client_addr{};
			int len = sizeof(client_addr);

			Flag flag = Flag::NONE;
			Host_Safe::receive_packet(server.recvbuf, BUFF_SIZE, flag, server.my_socket, client_addr);

			if (flag == Flag::SYN && !server.isConnected(client_addr)) {

				const bool connection_failed = server.connectClient(client_addr);
				if (!connection_failed)
				{
					printf("Server error: connection failed \n");
					//exit(EXIT_FAILURE);
				}

				int id = ++server.player_count;

				server.clients[id].id = id;
				server.clients[id].in_use = true;
				server.clients[id].client_addr = client_addr;
				server.clients[id].socket = server.my_socket;

				//Send the id to that client
				print("Client#" + std::to_string(server.player_count) + " Accepted \n", colors::blue);

				new_client packet{};
				packet.id = 0;
				packet.new_id = id;
				packet.rand_seed = NetworkingMrg.random_seed;
				packet.connected_clients[0] = true;
				packet.connected_clients[1] = server.clients[1].in_use;
				packet.connected_clients[2] = server.clients[2].in_use;
				packet.connected_clients[3] = server.clients[3].in_use;

				NetworkingMrg.broadcast_packet(Flag::e_new_client, packet);
			}
			else if (flag == Flag::FIN && server.isConnected(client_addr)) {
				server.disconnect_client(client_addr);
				NetworkingMrg.ShutDownMenu();
				NetworkingMrg.GoBackMenu();
				MenuMgr.Reset();
				break;
			}
			else if (flag != Flag::SYN && flag != Flag::FIN && flag != Flag::NONE)
			{
				//print details of the client/peer and the data received
				// printf("Received packet from %s:%d \n", inet_ntop(AF_INET, &client_addr.sin_addr, server.remote_ip.data(), server.remote_ip.size() - 1),
				// 	ntohs(client_addr.sin_port));

				std::vector<char>received;
				for (int i = 0; i < BUFF_SIZE; i++)
				{
					received.push_back(server.recvbuf[i]);
				}

				NetworkingMrg.AddPacket(flag, received);
				NetworkingMrg.broadcast(flag, received);
			}
		}

	}

	NetworkingMrg.thread_created = false;
	return 0;
}

void Networking::AddPacket(Flag flag, std::vector<char> received)
{
	switch (flag)
	{
	case Flag::NONE:
		break;
	case Flag::SYN:
		break;
	case Flag::FIN:
		break;
	case Flag::e_move_input:
		AddPacket(char_to_packet<move_input>(received));
		break;
	case Flag::e_new_client:
		AddPacket(char_to_packet<new_client>(received));
		break;
	case Flag::e_network_event:
		AddPacket(char_to_packet<network_event>(received));
		break;
	case Flag::e_status_report:
		AddPacket(char_to_packet<status_report>(received));
		break;
	case Flag::e_enemy_report:
		AddPacket(char_to_packet<enemy_report>(received));
		break;
	default:
		break;
	}
}

void Networking::AddPacket(move_input packet) 
{ 
	input.enqueue(packet);
}

void Networking::AddPacket(status_report packet)
{ 
	status_reports.enqueue(packet);
}

void Networking::AddPacket(enemy_report packet)
{
	enemy_reports.enqueue(packet);
}

void Networking::AddPacket(new_client packet)
{
	clients_conected.enqueue(packet);

	connected_clients = packet.connected_clients;

	if (my_id == -1) {
		my_id = packet.new_id;
		if (what == SetUp::CLIENT)
			client.the_client.id = my_id;

		random_seed = packet.rand_seed;
		srand(random_seed);
	}
}

void Networking::AddPacket(network_event packet)
{
	eventQueue.enqueue(packet);
}

void Networking::GoBackMenu()
{
	// Get the index of the level we will be using for the game (and send an event with it)
	auto& levels = GSM.GetLevels();
	int idx = 0;
	for (auto& l : levels) {
		if (GSM.mConfig.mStartingLevel == l) {
			break;
		}
		else idx++;
	}

	if (idx >= levels.size()) {
		std::cerr << "Could notfind the level : " << GSM.mConfig.mGameLevel << std::endl;
		return;
	}

	GSM.SetNextLevel(GSM.GetLevel(idx));
}

network_event& Networking::CreateNetEvent(int id, event_type type)
{
	network_event e{};
	e.num = num++;
	e.id = id;
	e.type = type;

	send_event[e.num] = e;

	return send_event[e.num];
}

bool Networking::AmIServer()
{
	if (what == SetUp::CLIENT)
		return false;

	return true;
}

void Networking::cleanPackets()
{
	while (!input.isEmpty())
		input.dequeue();

	while (!status_reports.isEmpty())
		status_reports.dequeue();

	while (!enemy_reports.isEmpty())
		enemy_reports.dequeue();

	while (!eventQueue.isEmpty())
	{
		network_event net_event = eventQueue.dequeue();
		send_event.erase(net_event.num);
	}
}
