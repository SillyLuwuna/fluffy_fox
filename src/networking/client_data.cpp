#include "networking/client_data.hpp"

using namespace fluffy_fox::networking;

ClientData::ClientData(const std::string& ip, uint16_t port, struct bufferevent* buffer_event) :
	ip(ip),
	port(port),
	buffer_event(buffer_event),
	fd(bufferevent_getfd(buffer_event))
{
}

std::size_t ClientData::hash() const
{
	return fd;
}
