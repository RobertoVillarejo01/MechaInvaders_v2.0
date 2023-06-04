#pragma once
#include "Utilities/Singleton.h"
#include "../Common/Common.h"

#include <string>
#include <vector>
#include <thread>
#include <unordered_map>
#include <array>

class Server
{
	//MAKE_SINGLETON(Server)
public:

	void setUp(unsigned short port_s, const std::string IP_s);

	void initialize();
	void update();
	void free();

	void set_socketOpt();

	static bool clientCompare(struct sockaddr_in client1, struct sockaddr_in client2);
	bool isConnected(struct sockaddr_in newClient);
	bool connectClient(struct sockaddr_in newClient);

	bool connect(sockaddr_in newClient);
	void connect_client(int newId, sockaddr_in newAddr, SOCKET newSocket);
	
	// Networking gameplay interface
	bool new_connection(sockaddr_in& remote_endpoint, int id);
	void connect_client(sockaddr_in& remote_endpoint, int id);
	void disconnect_client(int id);
	void disconnect_client(sockaddr_in& remote_endpoint);

	bool disconnect();

//private:
	SOCKET my_socket;
	struct sockaddr_in server;
	struct sockaddr_in other;

	char recvbuf[BUFF_SIZE] = {};
	std::array<char, INET6_ADDRSTRLEN> remote_ip{};

	unsigned short port;
	std::string IP;

	std::string local_ip;

	std::array<Host::CLIENTINFO, 4> clients;

	bool connected = false;

	// key for the map
	int player_count;

	unsigned connection_drop_timer = 200;  // [s]
};