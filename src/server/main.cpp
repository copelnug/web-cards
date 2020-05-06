#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/socket_base.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/file_base.hpp>
#include <boost/beast/core/multi_buffer.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/file_body.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/make_shared.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <string_view>
#include <thread>
#include <utility>

#include "Server.hpp"
#include "WebsocketSession.hpp"

/*
	TODO:
		Response
		Timeout: control_callback ? timer_
		Handle exceptions
		Split websocket and request.
		Json
		Exit code.
*/ 

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

void do_session(Server* server, tcp::socket& socket, boost::asio::yield_context yield)
{
	bool close = false;
	boost::system::error_code ec;

	boost::beast::flat_buffer buffer;

	for(;;)
	{
		http::request<http::string_body> req;
		http::async_read(socket, buffer, req, yield[ec]);
		if(ec == http::error::end_of_stream)
			break;
		if(ec)
			; // TODO Exit in error

		if(boost::beast::websocket::is_upgrade(req))
		{
			// We use shared_ptr because we need the class to stay alive as long as a thread is executing a coroutine in that instance.
			// For example, another thread may be running a coroutine that will broadcast a message to all WebsocketSession. Thus we need that to 
			// finish before we can destroy the WebsocketSession.
			auto ws = server->canAcceptConnection(std::move(socket), req, yield);
			return ws->run(std::move(req), yield);
		}
		
		server->handleRequest(std::move(req), Server::Sender{socket, ec, close, yield});

		if(ec)
			; // TODO
		if(close)
			break;
	}
	socket.shutdown(tcp::socket::shutdown_send, ec);
}
void do_listen(Server* server, boost::asio::io_context& ioc, tcp::endpoint endpoint, boost::asio::yield_context yield)
{
	tcp::acceptor acceptor{ioc};
	acceptor.open(endpoint.protocol());
	acceptor.set_option(boost::asio::socket_base::reuse_address(true));

	acceptor.bind(endpoint);
	acceptor.listen(boost::asio::socket_base::max_listen_connections);

	for(;;)
	{
		boost::system::error_code ec;

		tcp::socket socket(ioc);

		acceptor.async_accept(socket, yield[ec]);
		if(ec)
			; // TODO Error
		boost::asio::spawn(
			acceptor.get_executor(),
			std::bind(
				&do_session,
				server,
				std::move(socket),
				std::placeholders::_1
			)
		);
	}
}

int main()
{
	const short port = 8080;
	const int nb_threads = 2;

	Server server;
	boost::asio::io_context ioc{nb_threads};

	boost::asio::spawn(ioc,
		std::bind(
			&do_listen,
			&server,
			std::ref(ioc),
			tcp::endpoint(boost::asio::ip::make_address("0.0.0.0"), port),
			std::placeholders::_1
		)
	);

	std::vector<std::thread> threads;
	threads.reserve(nb_threads-1);
	for(int i = 0; i < nb_threads-1; ++i)
		threads.emplace_back([&ioc] { ioc.run(); });
	ioc.run();

	// TODO Write clean exit code

	return 0;
}