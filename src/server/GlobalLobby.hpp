#ifndef SERVER_GLOBAL_LOBBY_HPP_INCLUDED
#define SERVER_GLOBAL_LOBBY_HPP_INCLUDED
#include <boost/asio/spawn.hpp>
#include <boost/shared_ptr.hpp>
#include "Lobby.hpp"
#include "WebsocketSession.hpp"

class GlobalLobby : public Lobby
{
public:
	GlobalLobby(Server* server);

	virtual std::optional<std::string> getHtmlFile(const std::string_view& session) const override;

	virtual void join(const boost::shared_ptr<WebsocketSession>& connection, boost::asio::yield_context yield) override;
	virtual bool leave(const std::string& session, boost::asio::yield_context yield) override;
	virtual bool onMessage(const boost::shared_ptr<WebsocketSession>& connection, const boost::property_tree::ptree& message, boost::asio::yield_context yield) override;
};
#endif