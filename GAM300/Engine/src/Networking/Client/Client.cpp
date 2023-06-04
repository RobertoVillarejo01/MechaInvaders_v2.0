#include "Client.h"

#include <iostream>
#include <array>
#include <sstream> 
#include <string>
#include <chrono>

void Client::setUp(unsigned short port_c, const std::string IP_c)
{
	port = port_c;
	IP = IP_c;
}

void Client::initialize()
{
	the_client = Host::CLIENTINFO{};

	//create socket
	if ((the_client.socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		printf("socket() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	set_socketOpt();

	//setup address structure
	memset((char*)&the_client.client_addr, 0, sizeof(the_client.client_addr));
	the_client.client_addr.sin_family = AF_INET;
	the_client.client_addr.sin_port = htons(port);
	if(inet_pton(AF_INET, IP.c_str(), &the_client.client_addr.sin_addr) == SOCKET_ERROR)
	{
		printf("inet_pton() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	the_client.in_use = false;

	connected = false;
}

void Client::update() {}

void Client::free()
{
	shutdown(the_client.socket, SD_SEND);
	closesocket(the_client.socket);
	printf("close client \n");
}

void Client::set_socketOpt()
{
	timeval OPTION_VALUE;
	OPTION_VALUE.tv_sec = 12;

	int err = setsockopt(the_client.socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&OPTION_VALUE, sizeof(timeval));
	if (err == SOCKET_ERROR)
	{
		printf("setsockopt() SO_RCVTIMEO failed with error code %d\n", WSAGetLastError());
		shutdown(the_client.socket, SD_SEND);
		closesocket(the_client.socket);
	}
#ifdef EDITOR 
	printf("SO_RCVTIMEO timeout was set...\n");
#endif

	err = setsockopt(the_client.socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&OPTION_VALUE, sizeof(timeval));

	if (err == SOCKET_ERROR)
	{
		printf("setsockopt() SO_SNDTIMEO failed with error code %d\n", WSAGetLastError());
		shutdown(the_client.socket, SD_SEND);
		closesocket(the_client.socket);
	}

#ifdef EDITOR 
	printf("SO_SNDTIMEO timeout was set...\n");
#endif
}

bool Client::connect()
{
	printf("> connecting... \n");

	//send syn request
	int err = Host_Safe::send_packet(nullptr, 0u, Flag::SYN, the_client.socket, the_client.client_addr);
	if (err == SOCKET_ERROR)
		return false;

	Flag flag(Flag::NONE);
	std::array< char, sizeof(Packet) > msg;
	int received_ack = Host_Safe::receive_packet(msg.data(), msg.size(), flag, the_client.socket, the_client.client_addr);
	bool incorrect_SYN_ACK = (flag != Flag::SYN || received_ack < 0);
	
	int count = 0;
	int recv_fail_count = 0;
	std::chrono::time_point<std::chrono::system_clock>  m_start_time = std::chrono::system_clock::now();
	//Resend syn until ack received
	while (incorrect_SYN_ACK && count++ <= 5)
	{
		//time out wihtout response
		std::chrono::time_point<std::chrono::system_clock> m_end_time = std::chrono::system_clock::now();
		if (((double)std::chrono::duration_cast<std::chrono::milliseconds>(m_end_time - m_start_time).count() / 1000.0f) > connection_drop_timer)
			return false;

		//send syn request
		err = Host_Safe::send_packet(nullptr, 0u, Flag::SYN, the_client.socket, the_client.client_addr);
		if (err == SOCKET_ERROR)
			return false;

		//receive message
		received_ack = Host_Safe::receive_packet(msg.data(), msg.size(), flag, the_client.socket, the_client.client_addr);
		if (received_ack < 0)
		{
			recv_fail_count++;
			m_start_time = std::chrono::system_clock::now();
		}
		incorrect_SYN_ACK = (flag != Flag::SYN || received_ack < 0);

		if (recv_fail_count >= 5) return false;
	}
	
	received_ack = 1;
	count = 0;
	m_start_time = std::chrono::system_clock::now();
	recv_fail_count = 0;

	///SEND ACK UNTIL THE SERVER SEND AN ACK
	while (flag == Flag::SYN || received_ack < 0 && count++ <= 5)
	{
		std::chrono::time_point<std::chrono::system_clock> m_end_time = std::chrono::system_clock::now();
		if (((double)std::chrono::duration_cast<std::chrono::milliseconds>(m_end_time - m_start_time).count() / 1000.0f) > connection_drop_timer)
			return false;

		//bool send_error = send_ack();
		err = Host_Safe::send_packet(nullptr, 0u, Flag::NONE, the_client.socket, the_client.client_addr);
		if (err == SOCKET_ERROR)
			return false;

		received_ack = Host_Safe::receive_packet(nullptr, 0u, flag, the_client.socket, the_client.client_addr);
		if (received_ack < 0)
		{
			recv_fail_count++;
			m_start_time = std::chrono::system_clock::now();
		}

		if (recv_fail_count >= 5) return false;
	}

	printf("> client connected \n");

	err = Host_Safe::send_packet(nullptr, 0u, Flag::NONE, the_client.socket, the_client.client_addr);
	if (err == SOCKET_ERROR)
		return false;

	BOOL bNewBehavior = FALSE;
	DWORD dwBytesReturned = 0;
	WSAIoctl(the_client.socket, SIO_UDP_CONNRESET, &bNewBehavior, sizeof bNewBehavior, NULL, 0, &dwBytesReturned, NULL, NULL);

	return true;
}

bool Client::disconnect()
{
	if (connected)
	{
		if (the_client.in_use)
		{
			printf("> disconnecting... \n");

			//send fin request
			int err = Host_Safe::send_packet(nullptr, 0u, Flag::FIN, the_client.socket, the_client.client_addr);
			if (err == SOCKET_ERROR)
				return false;

			Flag flag(Flag::NONE);
			std::array< char, sizeof(Packet) > msg;
			int received_ack = Host_Safe::receive_packet(msg.data(), msg.size(), flag, the_client.socket, the_client.client_addr);
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
				int err = Host_Safe::send_packet(nullptr, 0u, Flag::FIN, the_client.socket, the_client.client_addr);
				if (err == SOCKET_ERROR)
					return false;

				//receive message
				received_ack = Host_Safe::receive_packet(msg.data(), msg.size(), flag, the_client.socket, the_client.client_addr);
				if (received_ack < 0) count++;
				incorrect_SYN_ACK = (flag != Flag::FIN || received_ack < 0);
			}

			printf("> client disconnected \n");

			the_client.in_use = false;
			connected = false;
		}
	}

	return true;
}

