#pragma once
#include <string>
#include "util/event.hpp"

namespace networking
{
	class NetworkEngine
	{
	public:
		NetworkEngine(u_int64_t max_connections, u_int32_t timeout_ms, u_int8_t protocol, u_int8_t port); // FIXME protocol should be enum
		NetworkEngine(u_int64_t max_connections, u_int32_t timeout_ms, u_int8_t protocol); // FIXME protocol should be enum

		unsigned short port() const;
		bool is_listening() const;
		bool is_receiving_connections() const;

		void start_listening();
		void stop_listening();
		void start_receiving_connections();
		void stop_receiving_connections();
		void send_data(const std::string& ip, const std::string& data); // FIXME should be ip class
		void disconnect(const std::string& ip); // FIXME should be ip class
		void connect(const std::string& ip, u_int16_t attempts); // FIXME should be ip class

		void subscribe_data_received(const NamedFunction<void(const std::string&, const std::string&)>&& callback); // FIXME should be ip class
		void subscribe_data_sent(const NamedFunction<void(const std::string&, const std::string&)>&& callback); // FIXME should be ip class
		void subscribe_connection(const NamedFunction<void(const std::string&)>&& callback); // FIXME should be ip class
		void subscribe_disconnection(const NamedFunction<void(const std::string&)>&& callback); // FIXME should be ip class

	private:
		NetworkEngine(const NetworkEngine&) = delete;
		NetworkEngine& operator=(const NetworkEngine&) = delete;
	};
}
