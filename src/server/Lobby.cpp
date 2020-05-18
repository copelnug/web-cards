#include "Lobby.hpp"

#include <boost/make_shared.hpp>

Lobby::Lobby(Server* server, std::string name) :
	server_{std::move(server)},
	name_{std::move(name)}
{}
void Lobby::utilsSendTo(const std::multimap<std::string, boost::weak_ptr<WebsocketSession>>& connections, const std::string& session, std::string message, boost::asio::yield_context yield)
{
	auto [it, end] = connections.equal_range(session);

	auto msg = boost::make_shared<std::string>(std::move(message));
	for(; it != end; ++it)
	{
		if(auto conn = it->second.lock())
		{
			conn->send(msg, yield);
		}
	}
}
void Lobby::utilsSendToAll(const std::multimap<std::string, boost::weak_ptr<WebsocketSession>>& connections, std::string message, boost::asio::yield_context yield)
{
	auto msg = boost::make_shared<std::string>(std::move(message));
	for(auto& [s, ptr] : connections)
	{
		if(auto conn = ptr.lock())
		{
			conn->send(msg, yield);
		}
	}
}