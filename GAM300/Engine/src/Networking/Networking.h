#pragma once
#include "Utilities/Singleton.h"

#include "System\Scene\SceneSystem.h"
#include "Utilities\ThreadSafeQueue.h"


#include <winsock2.h>
#include <WS2tcpip.h>
#include <fstream>
#include <string>
#include <map>
#include <thread>
#include <mutex>

#include "Networking/Client/Client.h"
#include "Networking/Server/Server.h"


class Networking
{
	MAKE_SINGLETON(Networking)

public:
	~Networking();
	void Initialize(bool custom);
	void InitializeAsServer();
	void InitializeAsClient();
	void InitializeAsSinglePlayer();

	void Update();
	void ShutDown();
	void ShutDownMenu();

	void update_server();
	void update_client();
	bool thread_created = false;

	void startFrame();

	unsigned short get_id() { return my_id; }

	bool multiplay() { return playin_multiplayer; };
	bool AmIServer();

	void cleanPackets();


	inline static Server server = Server();
	inline static Client client = Client();

private:
	unsigned int random_seed = 0;
	bool playin_multiplayer = false;

	enum class SetUp
	{
		SERVER, CLIENT, SINGLEPLAYER
	};

	SetUp what = SetUp::SINGLEPLAYER;

	//================================================================
		//helper function to broadcast to all the clients
	void broadcast(Flag flag, const std::vector<char>& packet)
	{
		for (unsigned int i = 0; i < max_clients; i++)
		{
			if (server.clients[i].in_use /*&& server.clients[i].socket != INVALID_SOCKET*/)
				send_data(server.clients[i], packet, flag);
		}
	}

	template<typename T>
	std::vector<char> packet_to_char(T& packet) {
		const char* c = reinterpret_cast<char*>(&packet);
		std::vector<char> buffer(c, c + sizeof(T));
		return buffer;
	}

	template<typename T>
	T char_to_packet(std::vector<char>& buffer) {
		return *(reinterpret_cast<T*>(buffer.data()));
	}


	//================================================================

	//thread function in which we receive the Client data
	static int process_client();
	static int process_server();

	//=======================PACKET STORAGE===================================

	void AddPacket(Flag flag, std::vector<char> received);

	void AddPacket(new_client packet);
	void AddPacket(move_input packet);
	void AddPacket(status_report packet);
	void AddPacket(enemy_report packet);
	void AddPacket(network_event packet);

public:
	//std::vector<new_client>			clients_conected;
	ThreadSafeQueue<new_client>		clients_conected;

	ThreadSafeQueue<move_input>		input;
	ThreadSafeQueue<enemy_report>	enemy_reports;
	ThreadSafeQueue<status_report>	status_reports;

	ThreadSafeQueue<network_event>	eventQueue;


	network_event& CreateNetEvent(int id, event_type type);
	std::unordered_map<int, network_event> send_event;
	int num = 0;

	void GoBackMenu();

	//===========================================================
	template<typename T>
	void broadcast_packet(Flag flag, T& packet)
	{
		switch (what)
		{
		case Networking::SetUp::SERVER:
			AddPacket(packet);
			for (unsigned int i = 1; i < max_clients; i++)
			{
				if (connected_clients[i])
					send_data(server.clients[i], packet_to_char(packet), flag);
			}
			break;
		case Networking::SetUp::CLIENT:
			send_data(client.the_client, packet_to_char(packet), flag);
			break;
		case Networking::SetUp::SINGLEPLAYER:
			AddPacket(packet);
			break;
		default:
			AddPacket(packet);
			break;
		}
	}

	template <typename T>
	void sendPacketToServer(Flag flag, T& packet)
	{
		if (what != SetUp::CLIENT)
			AddPacket(packet);
		else {
			send_data(client.the_client, packet_to_char(packet), flag);
		}
	}

	template <typename T>
	void sendPacket(Flag flag, T& packet)
	{
		if (what == SetUp::SINGLEPLAYER)
			AddPacket(packet);
		else {
			send_data(client.the_client, packet_to_char(packet), flag);
		}
	}

	//========================= usufeul data for gameplay
	int my_id = -1;
	std::array<bool, max_clients> connected_clients{ false };
};

#define NetworkingMrg (Networking::Instance())

