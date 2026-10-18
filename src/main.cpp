#include "rendering/renderer.hpp"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <iostream>
#include "rendering/window.hpp"
#include "rendering/vulkan_tutorial.hpp"


using namespace fluffy_fox;
using namespace fluffy_fox::rendering;
// using namespace fluffy_fox::networking;

// TcpServer* srvr = nullptr;
//
// void server_on_message_received(const Connection& client, const std::vector<uint8_t>& data)
// {
// 	std::string_view str((const char*)data.data(), data.size());
// 	std::cout << "[" << client.ip << ":" << client.port << "] " << str << std::endl;
//
// 	std::string message = "[" + client.endpoint + "] " + std::string(str);
//
// 	std::vector<const Connection*> connections = srvr->connections();
// 	for (const Connection* conn : connections)
// 	{
// 		if (*conn == client) continue;
// 		srvr->send_data((uint8_t *)message.data(), message.length(), *conn);
// 	}
// }
//
// void server_connection(const Connection& client)
// {
// 	std::string message = "> [" + client.endpoint + "] connected";
// 	std::cout << message << "\n";
//
// 	std::vector<const Connection*> connections = srvr->connections();
// 	for (const Connection* conn : connections)
// 	{
// 		if (*conn == client) continue;
// 		srvr->send_data((uint8_t *)message.data(), message.length(), *conn);
// 	}
// }
//
// void server_disconnection(const Connection& client)
// {
// 	std::string message = "> [" + client.endpoint + "] disconnected";
// 	std::cout << message << "\n";
//
// 	std::vector<const Connection*> connections = srvr->connections();
// 	for (const Connection* conn : connections)
// 	{
// 		if (*conn == client) continue;
// 		srvr->send_data((uint8_t *)message.data(), message.length(), *conn);
// 	}
// }
//
// void server_start()
// {
// 	std::cout << "server started on port " << srvr->port() << " (max " << srvr->max_connections() << " users)\n";
// }
//
// void server_stop()
// {
// 	std::cout << "server stopped\n";
// }
//
// void client_on_message_received(const std::vector<uint8_t>& data)
// {
// 	std::string_view str((const char*)data.data(), data.size());
// 	std::cout << str << std::endl;
// }
//
// void client_connection()
// {
// 	std::cout << "connected to server\n";
// }
//
// void client_disconnection()
// {
// 	std::cout << "disconnected from server\n";
// }
//
// int main(int argc, char **argv)
// {
// 	if (argc < 2)
// 	{
// 		std::cerr << "usage: " << argv[0] << " [server|client]" << std::endl;
// 		return 1;
// 	}
//
// 	std::string_view type = argv[1];
// 	if (type == "server")
// 	{
// 		TcpServer server(2, 13774);
// 		srvr = &server;
// 		server.start_event() += server_start;
// 		server.stop_event() += server_stop;
// 		server.data_received_event() += server_on_message_received;
// 		server.connection_event() += server_connection;
// 		server.disconnection_event() += server_disconnection;
// 		bool success = server.start();
// 		if (!success)
// 		{
// 			return 1;
// 		}
//
// 		std::string message;
// 		std::getline(std::cin, message);
//
// 		server.stop();
// 	}
// 	else if (type == "client")
// 	{
// 		TcpClient client;
// 		client.data_received_event() += client_on_message_received;
// 		client.connection_event() += client_connection;
// 		client.disconnection_event() += client_disconnection;
//
// 		bool success = false;
// 		while (!(success = client.connect("localhost", 13774)))
// 		{
// 			std::cout << "couldn't connect. Trying again...\n";
// 			std::this_thread::sleep_for(std::chrono::seconds(1));
// 		}
//
// 		std::string message;
// 		std::getline(std::cin, message);
// 		while (client.is_connected() && message != "")
// 		{
// 			client.send_data((uint8_t*)message.data(), message.length());
// 			std::getline(std::cin, message);
// 		}
//
// 		client.disconnect();
// 	}
// 	else
// 	{
// 		std::cerr << "unknown type \"" << type << "\"\n";
// 		std::cerr << "usage: " << argv[0] << " [server|client]" << std::endl;
// 		return 1;
// 	}
//
//
// 	return 0;
// }

void setup_logger()
{
	spdlog::enable_backtrace(32);

	auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	stdout_sink->set_level(spdlog::level::debug);
	stdout_sink->set_pattern("[%^%=9l%$] %v");

	auto file_rotating_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/log.txt", 1048576 * 10, 5);
	file_rotating_sink->set_level(spdlog::level::trace);
	file_rotating_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%t] [%^%l%$] %v");

	std::vector<spdlog::sink_ptr> sinks { stdout_sink, file_rotating_sink };
	auto logger = std::make_shared<spdlog::logger>("default", sinks.begin(), sinks.end());
	logger->set_level(spdlog::level::debug);
	spdlog::set_default_logger(logger);
}

int main()
{
	setup_logger();

	Window window("owo", 840, 630);
	// Renderer::instance().start(window, RendererType::Vulkan, false, 360);

	try
	{
		VulkanTutorial app(window);
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}
