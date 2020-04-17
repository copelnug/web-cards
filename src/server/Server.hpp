#ifndef SERVER_SERVER_HPP_INCLUDED
#define SERVER_SERVER_HPP_INCLUDED
#include <boost/asio//ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <map>
#include <mutex>
#include <random>

#include "FileRepository.hpp"
#include "UserRepository.hpp"

// TODO forward declare as much as possible of boost

class Lobby;
class WebsocketSession;

class Server
{
public:
	using SessionId = std::string;
	using Endpoint = std::string;
	class User
	{
	public:
		std::string username;
		bool guest = true;
	};
	class Sender
	{
		boost::asio::ip::tcp::socket& socket_;
		boost::system::error_code& ec_;
		boost::asio::yield_context yield_;
		bool& close_;
	public:
		Sender(boost::asio::ip::tcp::socket& socket, boost::system::error_code& ec, bool& close, boost::asio::yield_context yield);

		template <typename Response>
		void operator()(Response response)
		{
			boost::beast::http::async_write(socket_, response, yield_[ec_]);
			close_ = response.need_eof();
		}
	};

private:
	// TODO Timeout on session and lobbies
	UserRepository usersRepository_;
	FileRepository fileRepository_;
	std::map<SessionId, User> users_;
	std::map<Endpoint, std::shared_ptr<Lobby>> lobbies_;
	std::mt19937_64 random_;
	std::mutex mut_;

	std::shared_ptr<Lobby> getLobby(const boost::beast::string_view& endpoint);
	std::shared_ptr<Lobby> getLobby(const std::string_view& endpoint);
	std::shared_ptr<Lobby> getLobby(const std::string& endpoint);
	void setUser(const std::string& sessionId, User user);
public:
	Server();

	void handleRequest(boost::beast::http::request<boost::beast::http::string_body> request, Sender&& sender);

	boost::shared_ptr<WebsocketSession> canAcceptConnection(boost::asio::ip::tcp::socket socket, const boost::beast::http::request<boost::beast::http::string_body>& initial_request, boost::asio::yield_context yield);
	void acceptConnection(boost::shared_ptr<WebsocketSession> connection, boost::asio::yield_context yield);
	void removeConnection(const std::string& session, const std::string& endpoint, boost::asio::yield_context yield);

	void onMessage(boost::shared_ptr<WebsocketSession> connection, std::istream& content, boost::asio::yield_context yield);

	std::string addLobby(std::shared_ptr<Lobby> lobby);
	void setUsername(const std::string& session, std::string username);
	User getUser(const std::string_view& sessionId);
	User getUser(const std::string& sessionId);
	std::vector<std::string> translateUsers(const std::vector<std::string>& sessions);

	std::string generateSessionId();
};

#endif