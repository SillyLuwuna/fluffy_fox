#pragma once
#include <cstdint>
#include <string>
#include "event2/bufferevent.h"

namespace fluffy_fox::networking
{
	struct Connection
	{
		std::string ip;
		uint16_t port;
		std::string endpoint;
		struct bufferevent* buffer_event;
		int fd;

		Connection(const std::string& ip, uint16_t port, struct bufferevent* buffer_event);

		std::size_t hash() const;
		bool operator==(const Connection& other) const;

	private:
		static std::string get_endpoint(std::string ip, uint16_t port);
	};
}

namespace std
{
	template<>
	struct hash<fluffy_fox::networking::Connection>
	{
		std::size_t operator()(const fluffy_fox::networking::Connection& client) const
		{
			return client.hash();
		}
	};
}
