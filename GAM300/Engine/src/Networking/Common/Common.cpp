#include "Common.h"

bool Host::receive_data(const CLIENTINFO& client_data, std::vector<char>& received, Flag& type)
{
	std::array< char, sizeof(Packet) > msg;
	bool err = Host_Safe::receive_packet(msg.data(), msg.size(), type, client_data.socket, client_data.client_addr);
	//received = msg.data();

	received.clear();
	for (int i = 0; i < msg.size(); i++)
	{
		received.push_back(msg[i]);
	}

	return err;
}

bool Host::send_data(const CLIENTINFO& client_data, const std::vector<char>& send, const Flag& type)
{
	if (!client_data.in_use) return false;
	size_t bytes_left = send.size();
	return Host_Safe::send_packet(send.data(), bytes_left, type,
		client_data.socket, client_data.client_addr);

}

int Host_Safe::send_packet(const char* msg, const size_t msg_size, const Flag flag, const SOCKET socket, const sockaddr_in &si_other)
{
	Packet packet;
	packet.flag = flag;
	packet.seq = seq_next++;
	packet.ack = seq_;
	//packet.type = type;

	if (msg != nullptr && msg_size) {
		size_t size = msg_size < sizeof(packet.msg) ? msg_size : sizeof(packet.msg);
		memcpy(packet.msg, msg, size);
	}

	return send_to(socket, packet, si_other);
}

int Host_Safe::receive_packet(char* msg, const size_t msg_size, Flag& flag, const SOCKET socket, const sockaddr_in& si_other)
{
	Packet packet;
	int recv_return = recv_from(socket, packet, si_other);

	if (recv_return > 0) {
		if (msg != nullptr)
			memcpy(msg, packet.msg, sizeof(packet.msg));

		flag = packet.flag;
		//type = packet.type;

		//unsigned short* wp = const_cast <unsigned short*> (&seq_);
		//*wp = packet.seq++;

		seq_ = packet.seq++;

		return static_cast<int>(packet.ack);
	}

	return recv_return;
}

int Host_Safe::send_to(const SOCKET socket, Packet packet, const sockaddr_in& remote_endpoint)
{
	int bytes_send = sendto(socket, reinterpret_cast<char*>(&packet), sizeof(Packet), 0, (struct sockaddr*)&remote_endpoint, sizeof(remote_endpoint));

	if (bytes_send <= 0)
	{
		int error_code = WSAGetLastError();
		// if (error_code == WSAETIMEDOUT)
		// {
		// 	//printf("Connected party did not properly respond after a period of time...\n");
		// 	shutdown(socket, SD_SEND);
		// 	closesocket(socket);
		// 	exit(-1);
		// }
		if (error_code == WSAEWOULDBLOCK || error_code == WSAETIMEDOUT)
			return bytes_send;

		//int error_code = WSAGetLastError();
		printf("sendto() failed with error code : %d  \n", error_code);
		return bytes_send;
	}

	return bytes_send;
}

int Host_Safe::recv_from(const SOCKET socket, Packet& packet, const sockaddr_in& remote_endpoint)
{
	int remote_endpoint_size = sizeof(remote_endpoint);
	int bytes_recv = recvfrom(socket, reinterpret_cast<char*>(&packet), sizeof(Packet), 0, (struct sockaddr*)&remote_endpoint, &remote_endpoint_size);

	//try to receive some data, this is a blocking call
	if (bytes_recv <= 0)
	{
		const int error_code = WSAGetLastError();
		if (error_code == WSAEWOULDBLOCK || error_code == WSAETIMEDOUT)
			return bytes_recv;

		printf("recvfrom() failed with error code : %d  \n", error_code);
		return bytes_recv;
	}

	return bytes_recv;
}
