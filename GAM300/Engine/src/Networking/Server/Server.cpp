#include "Server.h"
#include <assert.h>
#include <chrono>
#include <fstream>
#include "GameStateManager\GameStateManager.h"

void Server::setUp(unsigned short port_s, const std::string IP_s) 
{
	port = port_s;
	IP = IP_s;
}

void Server::initialize()
{
	WSAData data{};

	//Initialize Winsock
	auto result = WSAStartup(MAKEWORD(2, 2), &data);
	if (result != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		exit(-1);
	}

	//Create a socket
	if ((my_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
#ifdef EDITOR 
	printf("Socket created.\n");
#endif
	set_socketOpt();

	//Prepare the sockaddr_in structure
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(port);

	//Bind server socket
	if (bind(my_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
#ifdef EDITOR 
	printf("Bind done \n");
#endif

	connected = true;

	BOOL bNewBehavior = FALSE;
	DWORD dwBytesReturned = 0;
	WSAIoctl(my_socket, SIO_UDP_CONNRESET, &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);

	player_count = 0;

	// Get the local hostname
	const char* search0 = "IPv4"; // search pattern
	size_t offset;
	std::ifstream IPFile;


	if (GSM.mConfig.mbLocalHost) return;

	system("ipconfig > ip.txt");

	IPFile.open("ip.txt");
	if (IPFile.is_open())
	{
		while (!IPFile.eof())
		{
			std::getline(IPFile, local_ip);
			if ((offset = local_ip.find(search0, 0)) != std::string::npos)
			{
				//   IPv4 Address. . . . . . . . . . . : 1
				//so we find the ": " and keep the numbers
				if ((offset = local_ip.find(": ", 0)) != std::string::npos)  
				local_ip.erase(0, offset + 2);

				print("local ip address: " + local_ip + "\n", colors::green);

				break;
			}
		}
	}

	//system("del ip.txt");
	remove("ip.txt");
}

void Server::update()
{

}

void Server::free()
{
	shutdown(my_socket, SD_SEND);
	closesocket(my_socket);
	printf("close server \n");
}

void Server::set_socketOpt()
{
	timeval OPTION_VALUE;
	OPTION_VALUE.tv_sec = 12;

	int err = setsockopt(my_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&OPTION_VALUE, sizeof(timeval));
	if (err == SOCKET_ERROR)
	{
		printf("setsockopt() SO_RCVTIMEO failed with error code %d\n", WSAGetLastError());
		shutdown(my_socket, SD_SEND);
		closesocket(my_socket);
	}
#ifdef EDITOR 
	printf("SO_RCVTIMEO timeout was set...\n");
#endif

	err = setsockopt(my_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&OPTION_VALUE, sizeof(timeval));

	if (err == SOCKET_ERROR)
	{
		printf("setsockopt() SO_SNDTIMEO failed with error code %d\n", WSAGetLastError());
		shutdown(my_socket, SD_SEND);
		closesocket(my_socket);
	}

#ifdef EDITOR 
	printf("SO_SNDTIMEO timeout was set...\n");
#endif
}

bool Server::clientCompare(sockaddr_in client1, sockaddr_in client2)
{
	if (strncmp ((char*)&client1.sin_addr.s_addr, (char*)&client2.sin_addr.s_addr, sizeof(unsigned long)) == 0)
		if (strncmp((char*)&client1.sin_port, (char*)&client2.sin_port, sizeof(unsigned short)) == 0)
			if (strncmp ((char*)&client1.sin_family, (char*)&client2.sin_family,
				sizeof(unsigned short)) == 0)
				return TRUE;

	return FALSE;
}

bool Server::isConnected(sockaddr_in newClient)
{
	for(auto& client : clients) {
		if (client.in_use && clientCompare(client.client_addr, newClient)) {
			//strncpy(sender_name, element->username, USERNAME_LEN);
#ifdef EDITOR 
			printf("Client is already connected\n");
#endif
			return TRUE;
		}
	}
#ifdef EDITOR 
	printf("Client is not already connected\n");
#endif
	return FALSE;
}

bool Server::connectClient(sockaddr_in newClient)
{
	int count = 0;
	// check if is a connected player
	for (int i = 0; i < max_clients; i++)
	{
		if (clients[i].in_use)
		{
			count++;
		}
	}

	if (count == max_clients)
		return false;

	// the incoming message comes from a not connected client
	connect(newClient);
	return true;
}

bool Server::connect(sockaddr_in newClient)
{
	printf("> server connecting \n");
	int ack = -1;
	Flag flag = Flag::NONE;

	bool incorrect_ACK_received = true;
	int recv_fail_count = 0;

	//start countdown
	std::chrono::time_point<std::chrono::system_clock>  m_start_time = std::chrono::system_clock::now();
	while (incorrect_ACK_received)
	{
		//time out wihtout response
		std::chrono::time_point<std::chrono::system_clock> m_end_time = std::chrono::system_clock::now();
		if (((double)std::chrono::duration_cast<std::chrono::milliseconds>(m_end_time - m_start_time).count() / 1000.0f) > connection_drop_timer)
			return false;

		//send SYN+ACK
		int err = Host_Safe::send_packet(nullptr, 0u, Flag::SYN, my_socket, newClient);
		if (err == SOCKET_ERROR)
			return false;

		//receive ACK
		ack = Host_Safe::receive_packet(nullptr, 0u, flag, my_socket, newClient);

		if (ack < 0)
		{
			recv_fail_count++;
			m_start_time = std::chrono::system_clock::now();
		}

		incorrect_ACK_received = flag != Flag::NONE || ack < 0;
	}

	//flag = Flag::SYN;
	ack = -1;
	m_start_time = std::chrono::system_clock::now();

	//SEND ACK UNTIL CLIENT SEND SOMETHING ELSE
	while (flag == Flag::SYN || ack < 0)
	{
		//time out wihtout response
		std::chrono::time_point<std::chrono::system_clock> m_end_time = std::chrono::system_clock::now();
		if (((double)std::chrono::duration_cast<std::chrono::milliseconds>(m_end_time - m_start_time).count() / 1000.0f) > connection_drop_timer)
			return false;

		//send SYN+ACK
		int err = Host_Safe::send_packet(nullptr, 0u, Flag::NONE, my_socket, newClient);
		if (err == SOCKET_ERROR)
			return false;

		//receive ACK
		ack = Host_Safe::receive_packet(nullptr, 0u, flag, my_socket, newClient);
		if (ack < 0)
		{
			recv_fail_count++;
			m_start_time = std::chrono::system_clock::now();
		}
	}

	printf("> server connected \n");

	return false;
}

void Server::connect_client(int newId, sockaddr_in newAddr, SOCKET newSocket)
{
	clients[newId].id = newId;
	clients[newId].in_use = true;
	clients[newId].client_addr = newAddr;
	clients[newId].socket = newSocket;
}

bool Server::new_connection(sockaddr_in& newClient, int id)
{
	for (auto& client : clients) {
		if (clientCompare(client.client_addr, newClient)) {
			//strncpy(sender_name, element->username, USERNAME_LEN);
			printf("Client is already connected\n");
			return false;
		}
	}

	// the incoming message comes from a not connected client
	connect_client(newClient, id);

	return true;
}

void Server::connect_client(sockaddr_in& newClient, int id)
{
	if (clients[id].in_use == false) {
		// 3-way hand shake
		connect(newClient);

		//save client 
		clients[id].id = id;
		clients[id].in_use = true;
		clients[id].client_addr = newClient;
		clients[id].socket = INVALID_SOCKET;

		player_count++;

		printf(">> Client id: %i in %s:%d connected to server \n", id, 
						inet_ntop(AF_INET, &newClient.sin_addr, remote_ip.data(), remote_ip.size() - 1),
						ntohs(newClient.sin_port));

		return;
	}
	printf(">> couldn't connect the are alreaddy %i clients \n", max_clients);
}

void Server::disconnect_client(int id) {
	// check if is a connected player and disconnect it
	if (clients[id].in_use) {
		clients[id].in_use = false;
		player_count--;
		return;
	}
}

void Server::disconnect_client(sockaddr_in& client_addr)
{
	for (auto& client : clients) {
		if (clientCompare(client.client_addr, client_addr)) {
			Host_Safe::send_packet(nullptr, 0u, Flag::FIN, my_socket, client_addr);
			disconnect_client(client.id);
			printf("Client disconnected\n");
		}
	}
}

bool Server::disconnect()
{
	printf("> disconnecting... \n");

	for (int i = 0; i < 4; i++)
	{
		if (clients[i].in_use)
		{
			//send fin request
			int err = Host_Safe::send_packet(nullptr, 0u, Flag::FIN, my_socket, clients[i].client_addr);
			if (err == SOCKET_ERROR)
				return false;

			Flag flag(Flag::NONE);
			std::array< char, sizeof(Packet) > msg;
			int received_ack = Host_Safe::receive_packet(msg.data(), msg.size(), flag, my_socket, clients[i].client_addr);
			bool incorrect_SYN_ACK = (flag != Flag::SYN || received_ack < 0);

			std::chrono::time_point<std::chrono::system_clock>  m_start_time = std::chrono::system_clock::now();
			int count = 0;
			//Resend fin until ack received
			while (incorrect_SYN_ACK && count <= 5)
			{
				std::chrono::time_point<std::chrono::system_clock> m_end_time = std::chrono::system_clock::now();
				if (((double)std::chrono::duration_cast<std::chrono::milliseconds>(m_end_time - m_start_time).count() / 1000.0f) > connection_drop_timer)
				{
					m_start_time = std::chrono::system_clock::now();
					count++;
				}

				//send syn request
				int err = Host_Safe::send_packet(nullptr, 0u, Flag::FIN, my_socket, clients[i].client_addr);
				if (err == SOCKET_ERROR)
					return false;

				//receive message
				received_ack = Host_Safe::receive_packet(msg.data(), msg.size(), flag, my_socket, clients[i].client_addr);
				if (received_ack < 0) count++;
				incorrect_SYN_ACK = (flag != Flag::FIN || received_ack < 0);
			}


			disconnect_client(i);
		}
	}

	connected = false;

	printf("> server disconnected \n");

	return true;
}