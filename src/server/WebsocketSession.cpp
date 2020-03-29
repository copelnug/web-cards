#include "WebsocketSession.hpp"

#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <iostream>
namespace
{
	namespace pt = boost::property_tree;
	void error(const std::string_view& message)
	{
		std::cerr << message << std::endl;
	}
}
WebsocketSession::WebsocketSession(boost::asio::ip::tcp::socket socket) :
	ws_{std::move(socket)}
{}
WebsocketSession::~WebsocketSession()
{
}
void WebsocketSession::operator()(boost::beast::http::request<boost::beast::http::string_body> initial_request, boost::asio::yield_context yield)
{
	std::cout << "WS   " << initial_request.method() << ": " << initial_request.target() << std::endl;

	/*for(auto const& field : initial_request)
		std::cout << "\t" << field.name() << "=" << field.value() << std::endl;//*/
	
	auto id = initial_request["User-Agent"];
	id.remove_prefix(id.rfind(' '));

	boost::beast::error_code ec;

	ws_.async_accept(initial_request, yield[ec]);
	if(ec) return error("Async accept failure");

	for(;;)
	{
		buffer_.consume(buffer_.size());
		ws_.async_read(buffer_, yield[ec]);
		if(ec == boost::beast::websocket::error::closed)
			break;
		if(ec) return error("Websocket async read failure");

		std::cout << "WSMSG:" << boost::beast::buffers_to_string(buffer_.data()) << std::endl; // Could use make_printable(buffer.data()) to send to stream, but we will need to read it anyway
		
		std::istream is{&buffer_};
		pt::ptree data;
		pt::read_json(is, data);
		
		auto type = data.get_optional<std::string>("type");
		if(!type)
			error("Websocket unknow message");

		if(*type == "CHAT")
		{
			pt::ptree response;
			response.put("type", "CHAT");
			response.put("message", data.get("message", ""));
			response.put("source", id);

			std::ostream os{&buffer_};
			pt::write_json(os, response);

			ws_.text(ws_.got_text());
			ws_.async_write(buffer_.data(), yield[ec]);
			if(ec) error("Websocket async write failure");
		}
		else
		{
			error("Unknown message");
		}
	}
}