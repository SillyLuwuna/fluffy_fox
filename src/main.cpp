#include "events/event.hpp"
#include "networking/tcp_client.hpp"
#include "networking/tcp_server.hpp"
#include <iostream>
#include <thread>

using namespace fluffy_fox;
using namespace fluffy_fox::networking;

void on_message_received(const ClientData& client, const std::vector<uint8_t>& data)
{
	std::string_view str((const char*)data.data(), data.size());
	std::cout << "[" << client.ip << ":" << client.port << "] " << str << std::endl;
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		std::cerr << "usage: " << argv[0] << " [server|client]" << std::endl;
		return 1;
	}

	std::string_view type = argv[1];
	if (type == "server")
	{
		TcpServer server(1, 13774);
		server.data_received_event() += on_message_received;
		server.start();
	}
	else if (type == "client")
	{
		TcpClient client;
		std::jthread t(&TcpClient::connect, &client, "localhost", 13774);

		std::string message;
		std::getline(std::cin, message);
		while (message != "")
		{
			client.send_data((uint8_t*)message.data(), message.length());
			std::getline(std::cin, message);
		}

		client.disconnect();
	}
	else
	{
		std::cerr << "unknown type \"" << type << "\"\n";
		std::cerr << "usage: " << argv[0] << " [server|client]" << std::endl;
		return 1;
	}


	return 0;
}
