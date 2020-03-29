#ifndef SERVER_WEBSOCKET_SESSION_HPP_INCLUDED
#define SERVER_WEBSOCKET_SESSION_HPP_INCLUDED

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket/stream.hpp>

class WebsocketSession
{
	boost::asio::streambuf buffer_;
	boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws_;
public:

	WebsocketSession(boost::asio::ip::tcp::socket socket);
	WebsocketSession(WebsocketSession&& source) = delete;
	~WebsocketSession();

	void operator()(boost::beast::http::request<boost::beast::http::string_body> initial_request, boost::asio::yield_context yield);
};

#endif