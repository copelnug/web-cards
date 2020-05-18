#ifndef SERVER_GLOBAL_LOBBY_HPP_INCLUDED
#define SERVER_GLOBAL_LOBBY_HPP_INCLUDED
#include <boost/asio/spawn.hpp>
#include <boost/shared_ptr.hpp>
#include "Lobby.hpp"
#include "Server.hpp"
#include "WebsocketSession.hpp"

class GlobalLobby : public Lobby, public Server::LobbyObserver
{
public:
	struct LobbyInfo
	{
		std::string url;
		std::shared_ptr<Lobby> lobby;
	};
private:
	std::multimap<std::string, boost::weak_ptr<WebsocketSession>> connections_;
	std::vector<LobbyInfo> lobbies_;
	std::mutex mut_;
public:
	GlobalLobby(Server* server);

	virtual std::optional<std::string> getHtmlFile(const std::string_view& session) const override;

	virtual bool canJoin(const std::string_view& session) override;
	virtual void join(const boost::shared_ptr<WebsocketSession>& connection, boost::asio::yield_context yield) override;
	virtual bool leave(const std::string& session, boost::asio::yield_context yield) override;
	virtual bool onMessage(const boost::shared_ptr<WebsocketSession>& connection, const boost::property_tree::ptree& message, boost::asio::yield_context yield) override;

	virtual void onLobbyAdd(const std::string_view& lobbyId, std::shared_ptr<Lobby> lobby, boost::asio::yield_context yield) override;
	virtual void onLobbyDelete(const std::string_view& lobbyId, boost::asio::yield_context yield) override;

	static std::string serializeLobbyAdd(const LobbyInfo& lobby, const std::string_view& sessionId);
	static std::string serializeLobbyRemove(const std::string_view& lobbyId);
	static std::vector<std::string> serializeLobbyList(const std::vector<LobbyInfo>& lobbies, const std::string_view& sessionId);
};
#endif