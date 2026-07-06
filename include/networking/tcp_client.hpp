#pragma once
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>
#include "event2/event.h"
#include "event2/bufferevent.h"
#include "event2/buffer.h"
#include "events/event.hpp"
#include "events/thread_safe_event.hpp"

namespace fluffy_fox::networking
{
	// TODO timeout / heartbeat
	class TcpClient
	{
	private:
		ThreadSafeEvent<> connect_event_;
		ThreadSafeEvent<> disconnect_event_;
		ThreadSafeEvent<const std::vector<uint8_t>&> on_data_received_;

		event_base* base_;
		bufferevent* event_buffer_;
		std::mutex event_buffer_mutex_;
		std::mutex events_mutex_;

		std::atomic<bool> connected_;
		std::mutex is_connecting_or_disconnecting_mutex_;
		std::jthread event_thread_;

		void run();

		static void on_read(bufferevent* buffer, void* ctx);
		static void on_event(bufferevent* buffer, short events, void* ctx);

	public:
		TcpClient(const TcpClient&) = delete;
		TcpClient(TcpClient&&) = delete;
		TcpClient& operator=(const TcpClient&) = delete;
		TcpClient& operator=(TcpClient&&) = delete;

		TcpClient();
		~TcpClient();

		bool connect(const std::string& ip, uint16_t port);
		void disconnect();

		bool is_connected() const;

		void send_data(const std::vector<uint8_t>& data);
		void send_data(const uint8_t* data, size_t size);
	};
}
