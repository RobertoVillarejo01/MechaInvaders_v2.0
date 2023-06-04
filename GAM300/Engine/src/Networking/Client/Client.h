#pragma once
#include "../Common/Common.h"

#include <thread>
#include <mutex>
#include <string>

class Client
{
public:
	void setUp(unsigned short port_c, const std::string IP_c);

	void initialize();
	void update();
	void free();

	void set_socketOpt();

	bool connect();

	bool disconnect();

	unsigned short get_id() { return the_client.id; }

	Host::CLIENTINFO the_client;

	char recvbuf[BUFF_SIZE] = {};
	bool connected = false;

private:

	unsigned short port = 8008;
	std::string IP = "127.0.0.1";

	unsigned connection_drop_timer = 200;  // [s]
};