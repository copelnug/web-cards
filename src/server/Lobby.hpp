#ifndef SERVER_LOBBY_HPP_INCLUDED
#define SERVER_LOBBY_HPP_INCLUDED
#include <boost/asio/spawn.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>
#include <optional>
#include <string>
#include "WebsocketSession.hpp"

class Server;

class Lobby
{
	Server* server_;
public:
	static constexpr const char* const MSG_ENTRY_TYPE = "type";

	Lobby(Server* server);
	virtual ~Lobby() = default;

	Server& server() const { return *server_; }

	virtual std::optional<std::string> getHtmlFile(const std::string_view& session) const = 0;

	virtual void join(const boost::shared_ptr<WebsocketSession>& connection, boost::asio::yield_context yield) = 0;
	virtual bool leave(const std::string& session, boost::asio::yield_context yield) = 0;
	virtual bool onMessage(const boost::shared_ptr<WebsocketSession>& connection, const boost::property_tree::ptree& message, boost::asio::yield_context yield) = 0;
};

#endif