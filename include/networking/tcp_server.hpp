#pragma once
#include <cstdint>
#include "event2/event.h"
#include "event2/listener.h"
#include "event2/bufferevent.h"
#include "event2/buffer.h"
#include "events/event.hpp"
#include "networking/client_data.hpp"

namespace fluffy_fox::networking
{
	// TODO timeout / heartbeat
	class TcpServer
	{
	private:
		uint16_t max_connections_;
		uint16_t port_;

		Event<const ClientData&> connection_event_;
		Event<const ClientData&> disconnection_event_;
		Event<const ClientData&, const std::vector<uint8_t>&> data_received_event_;

		event_base* base_;
		evconnlistener* listener_;
		std::unordered_map<int, ClientData> connections_;
		// std::unordered_set<ClientData> connections_;
		// std::unordered_set<bufferevent*> connections_;
		bool running_;

		static void on_accept(evconnlistener* listener, evutil_socket_t fd, sockaddr* address, int socklen, void* ctx);
		static void on_read(bufferevent* buffer, void* ctx);
		static void on_event(bufferevent* buffer, short events, void* ctx);

	public:
		TcpServer(const TcpServer&) = delete;
		TcpServer(TcpServer&&) = delete;
		TcpServer& operator=(const TcpServer&) = delete;
		TcpServer& operator=(TcpServer&&) = delete;

		TcpServer(uint16_t max_connections, uint16_t port); // TODO max_connections actually works
		~TcpServer();

		bool start();
		void stop();

		Event<const ClientData&>& connection_event();
		Event<const ClientData&>& disconnection_event();
		Event<const ClientData&, const std::vector<uint8_t>&>& data_received_event();
	};
}
