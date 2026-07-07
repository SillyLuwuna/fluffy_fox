#include "networking/tcp_server.hpp"
#include "event2/thread.h"
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
	base_(nullptr),
	listener_(nullptr),
	running_(false),
	event_invoker_(1) // MUST BE 1
{
	evthread_use_pthreads();
	base_ = event_base_new();
	if (!base_)
	{
		throw std::runtime_error("failed to create event base");
	}

	connections_.reserve(max_connections);
	connected_endpoints_.reserve(max_connections);
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
	std::lock_guard lock(is_connecting_or_disconnecting_mutex_);

	if (running_.load()) return true;

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

	running_.store(true);
	if (event_thread_.joinable())
	{
		event_thread_.join();
	}
	event_thread_ = std::jthread(&TcpServer::run, this);

	return true;
}

void TcpServer::stop()
{
	std::lock_guard lock(is_connecting_or_disconnecting_mutex_);

	if (!running_.exchange(false)) return;

	event_base_loopbreak(base_);

	if (event_thread_.joinable())
	{
		event_thread_.join();
	}

	if (listener_)
	{
		evconnlistener_free(listener_);
		listener_ = nullptr;
	}

	std::lock_guard connection_lock(connections_mutex_);
	for (auto& [fd, connection] : connections_)
	{
		bufferevent_free(connection.buffer_event);
		connection.buffer_event = nullptr;
	}
	connections_.clear();
	connected_endpoints_.clear();
}

void TcpServer::run()
{
	int success = event_base_dispatch(base_);
	if (success < 0)
	{
		std::cerr << "error on stopping event loop\n";
	}
	else if (success > 0)
	{
		std::cerr << "out of events in loop\n";
	}
}

void TcpServer::on_accept(evconnlistener* listener, evutil_socket_t fd, sockaddr* address, int , void* ctx)
{
	TcpServer* server = static_cast<TcpServer*>(ctx);
	event_base* base = evconnlistener_get_base(listener);

	if (!server->running_.load()) return;

	std::lock_guard connection_lock(server->connections_mutex_);
	if (server->connections_.size() >= server->max_connections_)
	{
		std::cout << "max connections reached! denying connection\n";
		evutil_closesocket(fd);
		return;
	}

	bufferevent* buffer = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
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

	Connection connection(ip, port, buffer);
	server->connections_.emplace(connection.fd, connection);
	server->connected_endpoints_.emplace(connection);

	bufferevent_setcb(buffer, on_read, nullptr, on_event, server);
	bufferevent_enable(buffer, EV_READ | EV_WRITE);

	std::cout << "client connected " << ip << ":" << port << "\n";

	server->event_invoker_.execute(Function<void()>(
		[server, c = connection]() { server->connection_event_.invoke(c); }
	));
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

	std::lock_guard connection_lock(server->connections_mutex_);
	const Connection& connection = server->connections_.at(fd);

	server->event_invoker_.execute(Function<void()>(
		[server, c = connection, d = std::move(data)]() { server->data_received_event_.invoke(c, d); }
	));
}

void TcpServer::on_event(bufferevent* buffer_event, short events, void* ctx)
{
	TcpServer* server = static_cast<TcpServer*>(ctx);
	int fd = bufferevent_getfd(buffer_event);

	if (events & BEV_EVENT_ERROR)
	{
		std::cerr << "socket error: " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()) << "\n";
	}

	if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR))
	{
		server->disconnect(fd);

		std::cout << "client disconnected\n";
	}
}

bool TcpServer::is_connected(const Connection& connection) const
{
	if (!running_.load()) return false;

	std::lock_guard connection_lock(connections_mutex_);
	return connected_endpoints_.contains(connection);
}

void TcpServer::disconnect(int fd)
{
	if (!running_.load()) return;

	std::unique_lock connection_lock(connections_mutex_);
	Connection connection = connections_.at(fd);
	connection_lock.unlock();

	disconnect(connection);
}

void TcpServer::disconnect(Connection& connection)
{
	if (!running_.load()) return;

	std::lock_guard connection_lock(connections_mutex_);

	connections_.erase(connection.fd);
	connected_endpoints_.erase(connection);

	bufferevent_free(connection.buffer_event);
	connection.buffer_event = nullptr;

	event_invoker_.execute(Function<void()>(
		[this, connection]() { this->disconnection_event_.invoke(connection); }
	));
}

Event<const Connection&>& TcpServer::connection_event()
{
	return connection_event_;
}

Event<const Connection&>& TcpServer::disconnection_event()
{
	return disconnection_event_;
}

Event<const Connection&, const std::vector<uint8_t>&>& TcpServer::data_received_event()
{
	return data_received_event_;
}
