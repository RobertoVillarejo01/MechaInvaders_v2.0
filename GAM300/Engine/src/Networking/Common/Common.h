#pragma once
#include <winsock2.h>
#include <WS2tcpip.h>
#include <vector>
#include <array>
#include "Utilities/coloring.h"
#include "Packets.h"

const int BUFF_SIZE = 1024;

#define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)

namespace Host {
	//struc with each client data 
	struct CLIENTINFO
	{
		int id = 0;
		bool in_use = false;
		sockaddr_in client_addr = sockaddr_in();
		SOCKET socket = INVALID_SOCKET;
	};

	bool receive_data(const CLIENTINFO& client_data,  std::vector<char>& received, Flag& type);
	bool send_data(const CLIENTINFO& client_data, const std::vector<char>& send, const Flag& type);
}

namespace Host_Safe {

	static unsigned short seq_next = 0;
	static unsigned short seq_ = 0;

	int send_packet(const char* msg, const size_t msg_size, const Flag flag, const SOCKET socket, const sockaddr_in& si_other);
	int receive_packet(char* msg, const size_t msg_size, Flag& flag, const SOCKET socket, const sockaddr_in &si_other);

	int send_to(const SOCKET socket, Packet packet, const sockaddr_in& remote_endpoint);
	int recv_from(const SOCKET socket, Packet& packet, const sockaddr_in& remote_endpoint);
};
