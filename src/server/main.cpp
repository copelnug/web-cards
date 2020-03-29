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
#include <boost/beast/websocket/detail/error.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <string_view>
#include <thread>
#include <utility>

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

#include <iostream>
void error(const std::string_view& message)
{
	std::cerr << message << std::endl;
}
template <typename Send>
void handle_request(http::request<http::string_body> request, Send send)
{
	std::cout << "HTTP " << request.method() << ": " << request.target() << std::endl;
	if(request.method() == http::verb::get)
	{
		if(request.target() == "/")
		{
			http::file_body::value_type body;
			boost::beast::error_code ec;
			body.open("index.html", boost::beast::file_mode::scan, ec);
			if(ec) return error("Request read failed.");

			auto const size = body.size();

			http::response<http::file_body> res{
				std::piecewise_construct,
				std::make_tuple(std::move(body)),
				std::make_tuple(http::status::ok, request.version())
			};
			res.set(http::field::server, "0.1");
			res.set(http::field::content_type, "text/html; charset=utf-8");
			res.content_length(size);
			res.keep_alive(request.keep_alive());
			return send(std::move(res));
		}
		// TODO Utils functions to populate the response...
		http::response<http::string_body> res{http::status::not_found, request.version()};
		res.set(http::field::server, "0.1");
		res.set(http::field::content_type, "text/html");
		res.keep_alive(request.keep_alive());
		res.body() = "The resource '" + request.target().to_string() + "' was not found.";
		res.prepare_payload();
		return send(std::move(res));
	}

	http::response<http::string_body> res{http::status::bad_request, request.version()};
	res.set(http::field::server, "0.1");
	res.set(http::field::content_type, "text/html");
	res.keep_alive(request.keep_alive());
	res.body() = "Unsupported operation";
	res.prepare_payload();
	return send(std::move(res));
}
void do_session(tcp::socket& socket, boost::asio::yield_context yield)
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
			WebsocketSession ws{std::move(socket)};
			return ws(std::move(req), yield);
		}

		handle_request(std::move(req), [&close, &socket, &ec, yield](auto&& response) -> void {
			http::async_write(socket, response, yield[ec]);
			close = response.need_eof();
		});
		if(ec)
			; // TODO
		if(close)
			break;
	}
	socket.shutdown(tcp::socket::shutdown_send, ec);
}
void do_listen(boost::asio::io_context& ioc, tcp::endpoint endpoint, boost::asio::yield_context yield)
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
			acceptor.get_executor().context(),
			std::bind(
				&do_session,
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

	boost::asio::io_context ioc{nb_threads};

	boost::asio::spawn(ioc,
		std::bind(
			&do_listen,
			std::ref(ioc),
			tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), port),
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