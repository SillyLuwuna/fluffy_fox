#include "events/event.hpp"
#include "networking/tcp_client.hpp"
#include "networking/tcp_server.hpp"
#include <iostream>
#include <cstdint>

using namespace fluffy_fox;
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

#if defined(__unix__)
	#if defined(__has_include) && __has_include(<wayland-client.h>)
		#define GLFW_EXPOSE_NATIVE_WAYLAND
	#else
		#define GLFW_EXPOSE_NATIVE_X11
	#endif
#elif defined(_WIN32)
	#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__APPLE__)
	#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <bgfx/bgfx.h>
#include <wayland-client.h>

int main()
{
	if (!glfwInit())
	{
		return -1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	int width = 640;
	int height = 480;
	GLFWwindow* window = glfwCreateWindow(width, height, "test", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	bgfx::Init init;
	init.type = bgfx::RendererType::Vulkan;

	// platform specific
	bool valid_platform = false;
	switch (glfwGetPlatform())
	{
#if defined(__unix__)
	#if defined(__has_include) && __has_include(<wayland-client.h>)
		case GLFW_PLATFORM_WAYLAND:
			init.platformData.nwh = glfwGetWaylandWindow(window);
			init.platformData.ndt = glfwGetWaylandDisplay();
			init.platformData.type = bgfx::NativeWindowHandleType::Wayland;
			valid_platform = true;
			break;
	#else
		case GLFW_PLATFORM_X11:
			init.platformData.nwh = glfwGetX11Window(window);
			init.platformData.ndt = glfwGetX11Display();
			init.platformData.type = bgfx::NativeWindowHandleType::Default;
			valid_platform = true;
			break;
	#endif
#elif defined(__APPLE__)
		case GLFW_PLATFORM_COCOA:
			init.platformData.nwh = glfwGetCocoaWindow(window);
			valid_platform = true;
			break;
#elif defined(_WIN32)
		case GLFW_PLATFORM_WIN32:
			init.platformData.nwh = glfwGetWin32Window(window);
			valid_platform = true;
			break;
#endif
	}

	if (!valid_platform)
	{
		return -1;
	}

	init.resolution.width = width;
	init.resolution.height = height;
	init.resolution.reset = BGFX_RESET_VSYNC;

	if (!bgfx::init(init))
	{
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		bgfx::setViewRect(0, 0, 0, width, height);
		bgfx::touch(0);
		bgfx::frame();
	}

	bgfx::shutdown();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
