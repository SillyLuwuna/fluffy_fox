#include "networking/tcp_server.hpp"
#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>

using fluffy_fox::networking::TcpServer;

TcpServer::TcpServer(uint16_t max_connections, uint8_t port) :
	max_connections_(max_connections),
	port_(port),
	base_(nullptr)
{
	base_ = event_base_new();
	if (!base_)
	{
		throw std::runtime_error("failed to create event base");
	}
}

TcpServer::~TcpServer()
{
	stop();
	if (base_)
	{
		event_base_free(base_);
	}
}

bool TcpServer::start()
{
	sockaddr_in sin;
	std::memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(port_);

	uint32_t flags = LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE;
	listener_ = evconnlistener_new_bind(base_, on_accept, this, flags, -1, (sockaddr*)&sin, sizeof(sin));
	if (!listener_)
	{
		std::cerr << "failed to create listener\n";
		return false;
	}

	std::cout << "server listening on port " << port_ << "\n";

	// TODO blocks
	bool success = event_base_dispatch(base_);
	if (!success) return false;

	return true;
}

void TcpServer::stop()
{
	event_base_loopbreak(base_);

	if (listener_)
	{
		evconnlistener_free(listener_);
		listener_ = nullptr;
	}

	for (bufferevent* buffer : connections_)
	{
		bufferevent_free(buffer);
	}
	connections_.clear();
}

void TcpServer::on_accept(evconnlistener* listener, evutil_socket_t fd, sockaddr* address, int socklen, void* ctx)
{
	TcpServer* server = static_cast<TcpServer*>(ctx);
	event_base* base = evconnlistener_get_base(listener);

	bufferevent* buffer = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE); // TODO "do not rely on bufferevent"?
	if (!buffer)
	{
		std::cerr << "error creating buffer\n";
		evutil_closesocket(fd);
		return;
	}

	server->connections_.insert(buffer);

	bufferevent_setcb(buffer, on_read, nullptr, on_event, server);
	bufferevent_enable(buffer, EV_READ | EV_WRITE);

	char ip[INET_ADDRSTRLEN];
	sockaddr_in* sin = (sockaddr_in*)address;
	inet_ntop(AF_INET, &sin->sin_addr, ip, sizeof(ip));
	std::cout << "client connected " << ip << ":" << ntohs(sin->sin_port) << "\n";

	server->on_connect.invoke(buffer);
}

void TcpServer::on_read(bufferevent* buffer, void* ctx)
{
	TcpServer* server = static_cast<TcpServer*>(ctx);
	evbuffer* input = bufferevent_get_input(buffer);

	char *line;
	size_t n;
	while ((line = evbuffer_readln(input, &n, EVBUFFER_EOL_ANY)))
	{
		std::string message(line);
		free(line);

		server->on_data_received.invoke(buffer, message);
	}
}

void TcpServer::on_event(bufferevent* buffer, short events, void* ctx)
{
	TcpServer* server = static_cast<TcpServer*>(ctx);

	if (events & BEV_EVENT_ERROR)
	{
		std::cerr << "buffer event error: " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()) << "\n";
	}

	if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR))
	{
		server->on_disconnect.invoke(buffer);
		server->connections_.erase(buffer);
		std::cout << "client disconnected\n";
	}
}
