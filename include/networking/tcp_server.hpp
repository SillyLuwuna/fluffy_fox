#pragma once
#include <cstdint>
#include <mutex>
#include <thread>
#include "event2/event.h"
#include "event2/listener.h"
#include "event2/bufferevent.h"
#include "event2/buffer.h"
#include "events/event.hpp"
#include "events/thread_safe_event.hpp"
#include "networking/connection.hpp"
#include "threading/threadpool.hpp"

namespace fluffy_fox::networking
{
	class TcpServer
	{
	private:
		uint16_t max_connections_;
		uint16_t port_;

		ThreadSafeEvent<const Connection&> connection_event_;
		ThreadSafeEvent<const Connection&> disconnection_event_;
		ThreadSafeEvent<const Connection&, const std::vector<uint8_t>&> data_received_event_;

		event_base* base_;
		evconnlistener* listener_;
		std::unordered_map<int, Connection> connections_;
		std::unordered_set<Connection> connected_endpoints_;
		mutable std::mutex connections_mutex_;

		std::mutex is_connecting_or_disconnecting_mutex_;
		std::atomic<bool> running_;

		std::jthread event_thread_;
		threading::ThreadPool event_invoker_;

		void run();

		static void on_accept(evconnlistener* listener, evutil_socket_t fd, sockaddr* address, int socklen, void* ctx);
		static void on_read(bufferevent* buffer, void* ctx);
		static void on_event(bufferevent* buffer, short events, void* ctx);

		void disconnect(int fd);

	public:
		TcpServer(const TcpServer&) = delete;
		TcpServer(TcpServer&&) = delete;
		TcpServer& operator=(const TcpServer&) = delete;
		TcpServer& operator=(TcpServer&&) = delete;

		TcpServer(uint16_t max_connections, uint16_t port);
		~TcpServer();

		bool start();
		void stop();

		bool is_connected(const Connection& connection) const;
		void disconnect(Connection& connection);

		Event<const Connection&>& connection_event();
		Event<const Connection&>& disconnection_event();
		Event<const Connection&, const std::vector<uint8_t>&>& data_received_event();
	};
}
