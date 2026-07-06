#include "networking/tcp_server.hpp"
#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>

using namespace fluffy_fox;
using namespace fluffy_fox::networking;

TcpServer::TcpServer(uint16_t max_connections, uint16_t port) :
	max_connections_(max_connections),
	port_(port),
	connection_event_(),
	disconnection_event_(),
	data_received_event_(),
	base_(nullptr),
	connections_(),
	running_(false)
{
	connections_.reserve(max_connections);
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
	running_ = true;
	int success = event_base_dispatch(base_);
	if (success < 0)
	{
		running_ = false;
		evconnlistener_free(listener_);
		std::cerr << "error dispatching\n";
		return false;
	}

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

	for (auto& [fd, client] : connections_)
	{
		bufferevent_free(client.buffer_event);
		client.buffer_event = nullptr;
	}
	connections_.clear();

	running_ = false;
}

void TcpServer::on_accept(evconnlistener* listener, evutil_socket_t fd, sockaddr* address, int , void* ctx)
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

	char ip[INET_ADDRSTRLEN];
	sockaddr_in* sin = (sockaddr_in*)address;
	inet_ntop(AF_INET, &sin->sin_addr, ip, sizeof(ip));
	uint16_t port = ntohs(sin->sin_port);

	ClientData client(ip, port, buffer);
	server->connections_.emplace(client.fd, client);

	bufferevent_setcb(buffer, on_read, nullptr, on_event, server);
	bufferevent_enable(buffer, EV_READ | EV_WRITE);

	std::cout << "client connected " << ip << ":" << port << "\n";

	server->connection_event_.invoke(client);
}

void TcpServer::on_read(bufferevent* buffer_event, void* ctx)
{
	TcpServer* server = static_cast<TcpServer*>(ctx);
	evbuffer* input = bufferevent_get_input(buffer_event);
	int fd = bufferevent_getfd(buffer_event);

	size_t len = evbuffer_get_length(input);
	if (len == 0) return;

	std::vector<uint8_t> data(len);
	evbuffer_remove(input, data.data(), len);

	const ClientData& client = server->connections_.at(fd);
	server->data_received_event_.invoke(client, data);
}

void TcpServer::on_event(bufferevent* buffer_event, short events, void* ctx)
{
	TcpServer* server = static_cast<TcpServer*>(ctx);
	int fd = bufferevent_getfd(buffer_event);

	if (events & BEV_EVENT_ERROR)
	{
		std::cerr << "buffer event error: " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()) << "\n";
	}

	if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR))
	{
		const ClientData& client = server->connections_.at(fd);
		server->disconnection_event_.invoke(client);
		server->connections_.erase(fd);
		std::cout << "client disconnected\n";
	}
}

Event<const ClientData&>& TcpServer::connection_event()
{
	return connection_event_;
}

Event<const ClientData&>& TcpServer::disconnection_event()
{
	return disconnection_event_;
}

Event<const ClientData&, const std::vector<uint8_t>&>& TcpServer::data_received_event()
{
	return data_received_event_;
}
