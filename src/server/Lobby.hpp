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
	std::string name_;
public:
	Lobby(Server* server, std::string name);
	virtual ~Lobby() = default;

	const std::string& name() const { return name_; }

	Server& server() const { return *server_; }

	virtual std::optional<std::string> getHtmlFile(const std::string_view& session) const = 0;

	virtual bool canJoin(const std::string_view& session) = 0;
	virtual void join(const boost::shared_ptr<WebsocketSession>& connection, boost::asio::yield_context yield) = 0;
	virtual bool leave(const std::string& session, boost::asio::yield_context yield) = 0;
	virtual bool onMessage(const boost::shared_ptr<WebsocketSession>& connection, const boost::property_tree::ptree& message, boost::asio::yield_context yield) = 0;

	static void utilsSendTo(const std::multimap<std::string, boost::weak_ptr<WebsocketSession>>& connections, const std::string& session, std::string message, boost::asio::yield_context yield);
	static void utilsSendToAll(const std::multimap<std::string, boost::weak_ptr<WebsocketSession>>& connections, std::string message, boost::asio::yield_context yield);
};

#endif