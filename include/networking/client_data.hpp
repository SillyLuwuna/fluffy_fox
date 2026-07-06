#pragma once
#include <cstdint>
#include <string>
#include "event2/bufferevent.h"

namespace fluffy_fox::networking
{
	struct ClientData
	{
		std::string ip;
		uint16_t port;
		struct bufferevent* buffer_event;
		int fd;

		ClientData(const std::string& ip, uint16_t port, struct bufferevent* buffer_event);

		std::size_t hash() const;
	};
}

namespace std
{
	template<>
	struct hash<fluffy_fox::networking::ClientData>
	{
		std::size_t operator()(const fluffy_fox::networking::ClientData& client) const
		{
			return client.hash();
		}
	};
}
