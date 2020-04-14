#include "WebsocketSession.hpp"

#include "Server.hpp"

namespace
{
	void error(const std::string_view& message)
	{
		std::cerr << message << std::endl;
	}
	using queue_type = std::queue<boost::shared_ptr<const std::string>>;
	bool addToQueue(queue_type& queue, std::mutex& mut, boost::shared_ptr<const std::string>&& message)
	{
		std::lock_guard<std::mutex> l{mut};
		queue.push(std::move(message));
		return queue.size() == 1;
	}
	boost::shared_ptr<const std::string> front(queue_type& queue, std::mutex& mut)
	{
		std::lock_guard<std::mutex> l{mut};
		return queue.front();
	}
	bool popQueue(queue_type& queue, std::mutex& mut)
	{
		std::lock_guard<std::mutex> l{mut};
		queue.pop();
		return !queue.empty();
	}
}
WebsocketSession::WebsocketSession(Server* server, boost::asio::ip::tcp::socket socket, std::string session, boost::asio::yield_context yield) :
	server_{std::move(server)},
	session_{std::move(session)},
	ws_{std::move(socket)},
	yield_{yield}
{}
WebsocketSession::~WebsocketSession()
{
	server_->removeConnection(session_, endpoint_, yield_);
}
void WebsocketSession::close()
{
	ws_.lowest_layer().close();
}
void WebsocketSession::run(boost::beast::http::request<boost::beast::http::string_body> initial_request, boost::asio::yield_context yield)
{
	endpoint_ = initial_request.target().to_string();

	/*for(auto const& field : initial_request)
		std::cout << "\t" << field.name() << "=" << field.value() << std::endl;//*/
	
	auto id = initial_request["User-Agent"];
	id.remove_prefix(id.rfind(' '));

	boost::beast::error_code ec;

	ws_.async_accept(initial_request, yield[ec]);
	if(ec) return error("Async accept failure");

	server_->acceptConnection(this->shared_from_this(), yield);

	for(;;)
	{
		buffer_.consume(buffer_.size());
		ws_.async_read(buffer_, yield[ec]);
		if(ec == boost::beast::websocket::error::closed)
			break;
		if(ec) return error("Websocket async read failure");

		std::istream is{&buffer_};
		server_->onMessage(this->shared_from_this(), is, yield);
	}
}
void WebsocketSession::send(boost::shared_ptr<const std::string> message, boost::asio::yield_context yield)
{
	if(addToQueue(queue_, mut_, std::move(message)))
	{
		do
		{
			boost::beast::error_code ec;

			const auto& current = front(queue_, mut_);
			boost::asio::const_buffer buffer{current->c_str(), current->size()};
			ws_.async_write(buffer, yield[ec]);
			if(ec) error("Websocket async write failure");
		} while(popQueue(queue_, mut_));
	}
}