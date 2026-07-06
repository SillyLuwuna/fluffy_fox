#include "networking/tcp_client.hpp"
#include "event2/thread.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <thread>

using fluffy_fox::networking::TcpClient;

TcpClient::TcpClient() :
	connect_event_(),
	disconnect_event_(),
	on_data_received_(),
	base_(nullptr),
	event_buffer_(nullptr),
	event_buffer_mutex_(),
	connected_(false),
	is_connecting_or_disconnecting_mutex_(),
	event_thread_()
{
	evthread_use_pthreads();
	base_ = event_base_new();
	if (!base_)
	{
		throw std::runtime_error("failed to create event base");
	}
}

TcpClient::~TcpClient()
{
	disconnect();

	if (base_)
	{
		event_base_free(base_);
	}
}

bool TcpClient::connect(const std::string& ip, uint16_t port)
{
	std::lock_guard<std::mutex> lock(is_connecting_or_disconnecting_mutex_);

	if (connected_) return true;

	std::lock_guard<std::mutex> lock_buffer(event_buffer_mutex_);

	evutil_socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		std::cerr << "failed to create socket\n";
		return false;
	}

	evutil_make_socket_nonblocking(sock);

	hostent* host = gethostbyname(ip.c_str());
	if (!host)
	{
		std::cerr << "failed to resolve hostname: " << host << "\n";
		evutil_closesocket(sock);
		return false;
	}

	sockaddr_in sin;
	std::memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	std::memcpy(&sin.sin_addr, host->h_addr_list[0], host->h_length);

	event_buffer_ = bufferevent_socket_new(base_, sock, BEV_OPT_CLOSE_ON_FREE);
	if (!event_buffer_)
	{
		std::cerr << "failed to create buffer event\n";
		evutil_closesocket(sock);
		return false;
	}

	bufferevent_setcb(event_buffer_, on_read, nullptr, on_event, this);
	bufferevent_enable(event_buffer_, EV_READ | EV_WRITE);

	int success = bufferevent_socket_connect(event_buffer_, (sockaddr*)&sin, sizeof(sin));
	if (success < 0)
	{
		std::cerr << "failed to connect\n";
		bufferevent_free(event_buffer_);
		event_buffer_ = nullptr;
		return false;
	}

	connected_ = true;

	event_thread_ = std::jthread(&TcpClient::run, this);

	return true;
}

void TcpClient::disconnect()
{
	std::lock_guard<std::mutex> lock(is_connecting_or_disconnecting_mutex_);

	if (!connected_) return;

	event_base_loopbreak(base_);

	{
		std::lock_guard<std::mutex> lock(event_buffer_mutex_);
		if (event_buffer_)
		{
			bufferevent_free(event_buffer_);
			event_buffer_ = nullptr;
		}
	}

	if (event_thread_.joinable())
	{
		event_thread_.request_stop();
		event_thread_.join();
	}

	connected_ = false;

	disconnect_event_.invoke();
}

bool TcpClient::is_connected() const
{
	return connected_;
}

void TcpClient::run()
{
	event_base_dispatch(base_);
}

void TcpClient::send_data(const std::vector<uint8_t>& data)
{
	send_data(data.data(), data.size());
}

void TcpClient::send_data(const uint8_t* data, size_t size)
{
	std::lock_guard<std::mutex> lock(event_buffer_mutex_);

	if (event_buffer_ && connected_)
	{
		int success = bufferevent_write(event_buffer_, data, size);
		if (success < 0)
		{
			std::cerr << "could not send message\n";
		}
	}
}

void TcpClient::on_read(bufferevent* buffer, void* ctx)
{
	TcpClient* client = static_cast<TcpClient*>(ctx);
	evbuffer* input = bufferevent_get_input(buffer);

	size_t len = evbuffer_get_length(input);
	if (len == 0) return;

	std::vector<uint8_t> data(len);
	evbuffer_remove(input, data.data(), len);

	client->on_data_received_.invoke(data);
}

void TcpClient::on_event(bufferevent*, short events, void* ctx)
{
	TcpClient* client = static_cast<TcpClient*>(ctx);

	if (events & BEV_EVENT_CONNECTED)
	{
		std::cout << "connected to server\n";
		client->connect_event_.invoke();
	}

	if (events & BEV_EVENT_ERROR)
	{
		std::cerr << "socket error: " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()) << "\n";
	}

	if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR))
	{
		std::cout << "disconnected from server\n";
		client->disconnect();
	}
}
