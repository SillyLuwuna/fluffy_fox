#pragma once
#include <cstdint>
#include <string>
#include "event2/event.h"
#include "event2/listener.h"
#include "event2/bufferevent.h"
#include "event2/buffer.h"
#include "events/event.hpp"

namespace fluffy_fox::networking
{
	// TODO timeout / heartbeat
	class TcpServer
	{
	private:
		uint16_t max_connections_;
		uint8_t port_;

		Event<struct bufferevent*> on_connect;
		Event<struct bufferevent*> on_disconnect;
		Event<struct bufferevent*, const std::string&> on_data_received;

		event_base* base_;
		evconnlistener* listener_;
		std::unordered_set<bufferevent*> connections_;

		static void on_accept(evconnlistener* listener, evutil_socket_t fd, sockaddr* address, int socklen, void* ctx);
		static void on_read(bufferevent* buffer, void* ctx);
		static void on_event(bufferevent* buffer, short events, void* ctx);

	public:
		TcpServer(const TcpServer&) = delete;
		TcpServer(TcpServer&&) = delete;
		TcpServer& operator=(const TcpServer&) = delete;
		TcpServer& operator=(TcpServer&&) = delete;

		TcpServer(uint16_t max_connections, uint8_t port);
		~TcpServer();

		bool start();
		void stop();


		uint8_t port(int i) const;
		// bool is_listening() const;
		// bool is_receiving_connections() const;

		// void start_listening();
		// void stop_listening();
		// void start_receiving_connections();
		// void stop_receiving_connections();
		// void send_data(const std::string& ip, const std::string& data); // FIXME should be ip class
		// void disconnect(const std::string& ip); // FIXME should be ip class
		// void connect(const std::string& ip, u_int16_t attempts); // FIXME should be ip class

		// void subscribe_data_received(const MemberFunction<void(const std::string&, const std::string&)>&& callback); // FIXME should be ip class
		// void subscribe_data_sent(const MemberFunction<void(const std::string&, const std::string&)>&& callback); // FIXME should be ip class
		// void subscribe_connection(const MemberFunction<void(const std::string&)>&& callback); // FIXME should be ip class
		// void subscribe_disconnection(const MemberFunction<void(const std::string&)>&& callback); // FIXME should be ip class
	};
}
