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
	Lobby{server}
{}
std::optional<std::string> GlobalLobby::getHtmlFile(const std::string_view& session) const
{
	auto user = server().getUser(session);

	if(user.guest)
		return "login.html";
	return "index.html";
}
void GlobalLobby::join(const boost::shared_ptr<WebsocketSession>& connection, boost::asio::yield_context yield)
{
	auto user = server().getUser(connection->session());

	if(user.guest)
		connection->close();

}
bool GlobalLobby::leave(const std::string&, boost::asio::yield_context)
{
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
		auto name = server().addLobby(std::make_shared<LobbyEnfer>(&server(), connection->session()));

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