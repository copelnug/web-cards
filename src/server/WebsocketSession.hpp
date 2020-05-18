#ifndef SERVER_WEBSOCKET_SESSION_HPP_INCLUDED
#define SERVER_WEBSOCKET_SESSION_HPP_INCLUDED

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <mutex>
#include <queue>

class Server;

class WebsocketSession : public boost::enable_shared_from_this<WebsocketSession>
{
	Server* server_;
	std::string session_;
	std::string endpoint_;
	boost::asio::streambuf buffer_;
	boost::beast::websocket::stream<boost::beast::tcp_stream> ws_;
	std::queue<boost::shared_ptr<const std::string>> queue_; /**< @brief Queue of message to send. Use shared_ptr to save memory on broadcast. */
	boost::asio::yield_context yield_; // TODO Remove yield in function signature?
	std::mutex mut_;
public:

	WebsocketSession(Server* server, boost::asio::ip::tcp::socket socket, std::string session, boost::asio::yield_context yield);
	WebsocketSession(WebsocketSession&& source) = delete;
	~WebsocketSession();

	void close();

	void run(boost::beast::http::request<boost::beast::http::string_body> initial_request, boost::asio::yield_context yield);
	void send(boost::shared_ptr<const std::string> message, boost::asio::yield_context yield);

	const std::string& session() const { return session_; }
	const std::string& endpoint() const { return endpoint_; }
};

#endif