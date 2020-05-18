#include "GlobalLobby.hpp"

#include <boost/asio/spawn.hpp>
#include <boost/make_shared.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "LobbyEnfer.hpp"
#include "Server.hpp"
#include "WebsocketSession.hpp"

namespace
{
	namespace pt = boost::property_tree;
}
GlobalLobby::GlobalLobby(Server* server) :
	Lobby{server, ""}
{}
std::optional<std::string> GlobalLobby::getHtmlFile(const std::string_view& session) const
{
	auto user = server().getUser(session);
	if(user)
		return "index-user.html";
	return "index-guest.html";
}
bool GlobalLobby::canJoin(const std::string_view&)
{
	return true;
}
void GlobalLobby::join(const boost::shared_ptr<WebsocketSession>& connection, boost::asio::yield_context yield)
{
	auto user = server().getUser(connection->session());

	std::lock_guard<std::mutex> l{mut_};
	connections_.emplace(connection->session(), connection);

	for(auto& msg : serializeLobbyList(lobbies_, connection->session()))
	{
		connection->send(boost::make_shared<std::string>(std::move(msg)), yield);
	}
}
bool GlobalLobby::leave(const std::string& session, boost::asio::yield_context)
{
	std::lock_guard<std::mutex> l{mut_};

	auto [it, end] = connections_.equal_range(session);
	for(; it != end; ++it)
	{
		// Remove any expired session
		if(it->second.expired())
		{
			it = connections_.erase(it);
			break;
		}
	}

	return false;
}
bool GlobalLobby::onMessage(const boost::shared_ptr<WebsocketSession>& connection, const boost::property_tree::ptree& message, boost::asio::yield_context yield)
{
	auto type = message.get_optional<std::string>(MSG_ENTRY_TYPE);
	if(!type)
		return false; // TODO Error

	auto user = server().getUser(connection->session());

	if(*type == "START")
	{
		if(!user)
			return false; // TODO Error
		auto gameName = message.get_optional<std::string>("name");
		if(!gameName || gameName->empty() || gameName->length() > 50) // TODO Constant? or exception from the constructor?
			return false; // TODO Error

		auto name = server().addLobby(std::make_shared<LobbyEnfer>(&server(), *gameName, connection->session()), yield);

		pt::ptree msg;
		msg.put(MSG_ENTRY_TYPE, "STARTED");
		msg.put("game", name);

		std::ostringstream out;
		pt::write_json(out, msg);

		// TODO: Wrap session & yield in an object.
		connection->send(boost::make_shared<std::string>(std::move(out).str()), yield);
	}
	return false;
}
void GlobalLobby::onLobbyAdd(const std::string_view& lobbyId, std::shared_ptr<Lobby> lobby, boost::asio::yield_context yield)
{
	std::lock_guard<std::mutex> l{mut_};

	lobbies_.emplace_back(LobbyInfo{std::string{lobbyId}, lobby});

	std::string lastSession;
	boost::shared_ptr<const std::string> message;

	for(auto& session : connections_)
	{
		if(session.first != lastSession)
		{
			message = boost::make_shared<const std::string>(serializeLobbyAdd(LobbyInfo{std::string{lobbyId}, lobby}, session.first));
			lastSession = session.first;
		}
		if(auto conn = session.second.lock())
		{
			conn->send(message, yield);
		}
	}
}
void GlobalLobby::onLobbyDelete(const std::string_view& lobbyId, boost::asio::yield_context yield)
{
	std::lock_guard<std::mutex> l{mut_};

	auto it = std::find_if(lobbies_.begin(), lobbies_.end(), [&](const LobbyInfo& info) {
		return info.url == lobbyId;
	});
	if(it != lobbies_.end())
		lobbies_.erase(it);

	auto message = boost::make_shared<const std::string>(serializeLobbyRemove(lobbyId));

	for(auto& session : connections_)
	{
		if(auto conn = session.second.lock())
		{
			conn->send(message, yield);
		}
	}
}
std::string GlobalLobby::serializeLobbyAdd(const LobbyInfo& lobby, const std::string_view& sessionId)
{
	pt::ptree msg;
	msg.put(MSG_ENTRY_TYPE, "LOBBY_ADD");
	msg.put("id", lobby.url);
	msg.put("name", lobby.lobby->name());

	if(lobby.lobby->canJoin(sessionId))
	{
		msg.put("url", lobby.url);
	}

	std::ostringstream out;
	pt::write_json(out, msg);
	return std::move(out).str();
}
std::string GlobalLobby::serializeLobbyRemove(const std::string_view& lobbyId)
{
	pt::ptree msg;
	msg.put(MSG_ENTRY_TYPE, "LOBBY_REMOVE");
	msg.put("id", lobbyId);

	std::ostringstream out;
	pt::write_json(out, msg);
	return std::move(out).str();
}
std::vector<std::string> GlobalLobby::serializeLobbyList(const std::vector<LobbyInfo>& lobbies, const std::string_view& sessionId)
{
	std::vector<std::string> messages;
	for(auto& lobby : lobbies)
	{
		messages.emplace_back(serializeLobbyAdd(lobby, sessionId));
	}
	return messages;
}