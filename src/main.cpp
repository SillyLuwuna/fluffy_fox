#include "events/event.hpp"
#include "networking/tcp_client.hpp"
#include "networking/tcp_server.hpp"
#include <iostream>

using namespace fluffy_fox;
using namespace fluffy_fox::networking;

void server_on_message_received(const ClientData& client, const std::vector<uint8_t>& data)
{
	std::string_view str((const char*)data.data(), data.size());
	std::cout << "[" << client.ip << ":" << client.port << "] " << str << std::endl;
}

void client_on_message_received(const std::vector<uint8_t>& data)
{
	std::string_view str((const char*)data.data(), data.size());
	std::cout << "[server] " << str << std::endl;
}

void client_connection()
{

}

void client_disconnection()
{

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
		server.data_received_event() += server_on_message_received;
		bool success = server.start();
		if (!success)
		{
			return 1;
		}

		std::string message;
		std::getline(std::cin, message);

		server.stop();
	}
	else if (type == "client")
	{
		TcpClient client;
		client.data_received_event() += client_on_message_received;
		client.connection_event() += client_connection;
		client.disconnection_event() += client_disconnection;

		bool success = false;
		while (!(success = client.connect("localhost", 13774)))
		{
			std::cout << "couldn't connect. Trying again...\n";
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		// bool success = client.connect("localhost", 13774);
		if (!success)
		{
			return 1;
		}

		std::string message;
		std::getline(std::cin, message);
		while (client.is_connected() && message != "")
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
