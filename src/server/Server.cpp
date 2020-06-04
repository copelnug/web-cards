#include "Server.hpp"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/make_shared.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <mutex>
#include <random>
#include <string_view>

#include "GlobalLobby.hpp"
#include "Lobby.hpp"
#include "WebsocketSession.hpp"

namespace
{
	namespace pt = boost::property_tree;

	constexpr const char* const GLOBAL_ENDPOINT = "/";

	void error(const std::string_view& message)
	{
		std::cerr << message << std::endl;
	}
	// TODO Unittest
	std::optional<std::string> extractSession(const boost::beast::string_view& s)
	{
		std::vector<std::string> parts;
		boost::split(parts, s, boost::is_any_of(";")); // TODO can we use string_view?

		for(auto& part : parts)
		{
			auto pos = part.find('=');
			if(part.substr(0, pos) == "session") // TODO Use string_view
			{
				return part.substr(pos+1);
			}
		}
		return {};
	}
	template <typename Resp>
	void setSession(Resp& resp, std::optional<std::string>& receivedSession, std::string session)
	{
		// TODO Find a better way to handle this
		if(!receivedSession)
		{
			resp.set(boost::beast::http::field::set_cookie, "session=" + session); // TODO Format
		}
	}
	std::optional<std::pair<std::string, std::string>> parseUserPass(const boost::beast::string_view& s)
	{
		// TODO Unittest
		std::pair<std::string,std::string> result;
		std::vector<std::string> parts;
		boost::split(parts, s, boost::is_any_of("&=")); // TODO can we use string_view?

		if( (parts.size() % 2) != 0)
			return {};

		for(unsigned int i = 0; i < parts.size(); i += 2)
		{
			if(parts[i] == "user")
				result.first = parts[i+1];
			else if(parts[i] == "pwd")
				result.second = parts[i+1];
		}

		if(result.first.empty() || result.second.empty())
			return {};
		return result;
	}
	std::string getFileFormat(const std::filesystem::path& path)
	{
		auto ext = path.extension();
		if(ext == ".html")
			return "text/html; charset=utf-8";
		if(ext == ".svg")
			return "image/svg+xml";
		if(ext == ".png")
			return "image/png";
		return "text; charset=utf-8";
	}
}
Server::Sender::Sender(boost::asio::ip::tcp::socket& socket, boost::system::error_code& ec, bool& close, boost::asio::yield_context yield) :
	socket_{socket},
	ec_{ec},
	yield_{yield},
	close_{close}
{}
Server::Server(boost::asio::io_context& ioc) :
	ioc_{ioc},
	fileRepository_{"files.lst"}
{
	std::random_device r;
	std::seed_seq seed{r(), r(), r(), r(), r(), r(), r(), r()};
	random_.seed(seed);

	auto global = std::make_shared<GlobalLobby>(this);
	lobbies_.emplace(GLOBAL_ENDPOINT, global);
	observers_.emplace_back(global);
}
std::shared_ptr<Lobby> Server::getLobby(const boost::beast::string_view& endpoint)
{
	return getLobby(std::string{endpoint});
}
std::shared_ptr<Lobby> Server::getLobby(const std::string_view& endpoint)
{
	return getLobby(std::string{endpoint});
}
std::shared_ptr<Lobby> Server::getLobby(const std::string& endpoint)
{
	std::lock_guard<std::mutex> l{mut_};

	auto it = lobbies_.find(endpoint);
	if(it == lobbies_.end())
		return {};
	return it->second;
}
void Server::setUser(const std::string& sessionId, User user)
{
	std::lock_guard<std::mutex> l{mut_};
	users_[sessionId] = std::move(user);
}
void Server::handleRequest(boost::beast::http::request<boost::beast::http::string_body> request, Sender&& sender)
{
	try {
		std::cout << "HTTP " << request.method() << ": " << request.target() << std::endl;

		auto inputSession = extractSession(request[boost::beast::http::field::cookie]);
		auto session = inputSession ? *inputSession : generateSessionId();

		if(request.method() == boost::beast::http::verb::get)
		{
			std::optional<std::filesystem::path> file = fileRepository_.get(request.target().to_string());

			if(!file)
			{
				auto lobby = getLobby(request.target());
				if(lobby)
					file = lobby->getHtmlFile(session);
			}
			
			if(file)
			{
				boost::beast::http::file_body::value_type body;
				boost::beast::error_code ec;
				body.open(file->c_str(), boost::beast::file_mode::scan, ec);
				if(ec) return error("Request read failed.");

				auto const size = body.size();

				boost::beast::http::response<boost::beast::http::file_body> res{
					std::piecewise_construct,
					std::make_tuple(std::move(body)),
					std::make_tuple(boost::beast::http::status::ok, request.version())
				};
				res.set(boost::beast::http::field::server, "0.1");
				res.set(boost::beast::http::field::content_type, getFileFormat(*file));
				setSession(res, inputSession, session);
				res.content_length(size);
				res.keep_alive(request.keep_alive());
				return sender(std::move(res));
			}
			// TODO Utils functions to populate the response...
			boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::not_found, request.version()};
			res.set(boost::beast::http::field::server, "0.1");
			res.set(boost::beast::http::field::content_type, "text/html");
			setSession(res, inputSession, session);
			res.keep_alive(request.keep_alive());
			res.body() = "The resource '" + request.target().to_string() + "' was not found.";
			res.prepare_payload();
			return sender(std::move(res));
		}
		else if(request.method() == boost::beast::http::verb::post)
		{
			// TODO Validate URL

			auto userPass = parseUserPass(request.body());
			if(!userPass || !usersRepository_.checkUser(userPass->first, userPass->second))
			{
				// TODO Find a way to add error message
				boost::beast::http::response<boost::beast::http::empty_body> res{boost::beast::http::status::see_other, request.version()};
				res.set(boost::beast::http::field::server, "0.1");
				res.set(boost::beast::http::field::location, "/");
				res.keep_alive(request.keep_alive());
				res.prepare_payload();
				return sender(std::move(res));
			}

			User user;
			user.username = userPass->first;
			setUser(session, std::move(user));

			// Redirect to prevent refresh problem: https://developer.mozilla.org/en-US/docs/Web/HTTP/Redirections
			boost::beast::http::response<boost::beast::http::empty_body> res{boost::beast::http::status::see_other, request.version()};
			res.set(boost::beast::http::field::server, "0.1");
			res.set(boost::beast::http::field::location, "/"); // TODO Do we want hardcoded? Do we want to redirect / to index.html and login.html?
			res.keep_alive(request.keep_alive());
			res.prepare_payload();
			return sender(std::move(res));
		}

		boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::bad_request, request.version()};
		res.set(boost::beast::http::field::server, "0.1");
		res.set(boost::beast::http::field::content_type, "text/html");
		setSession(res, inputSession, session);
		res.keep_alive(request.keep_alive());
		res.body() = "Unsupported operation";
		res.prepare_payload();
		return sender(std::move(res));
	}
	catch(const std::exception& ex)
	{
		std::cerr << "Request exception: " << ex.what() << std::endl;
		
		boost::beast::http::response<boost::beast::http::string_body> res{boost::beast::http::status::internal_server_error, request.version()};
		res.set(boost::beast::http::field::server, "0.1");
		res.set(boost::beast::http::field::content_type, "text/html");
		// Can't know if we have a session.
		res.keep_alive(request.keep_alive());
		res.body() = "Internal server error";
		res.prepare_payload();
		return sender(std::move(res));
	}
}
boost::shared_ptr<WebsocketSession> Server::canAcceptConnection(boost::asio::ip::tcp::socket socket, const boost::beast::http::request<boost::beast::http::string_body>& initial_request, boost::asio::yield_context yield)
{
	std::cout << "WS   " << initial_request.method() << ": " << initial_request.target() << std::endl;

	auto inputSession = extractSession(initial_request[boost::beast::http::field::cookie]);
	if(!inputSession)
	{
		boost::system::error_code ec;
		socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
		return {};
	}

	if(initial_request.target() != GLOBAL_ENDPOINT)
	{
		if(!getLobby(initial_request.target().to_string()))
		{
			boost::system::error_code ec;
			socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
			return {};
		}
	}
	// TODO We could inherit from websocket session to add information. ex: server, session and endpoint
	return boost::make_shared<WebsocketSession>(this, std::move(socket), *inputSession, yield);
}
void Server::acceptConnection(boost::shared_ptr<WebsocketSession> connection, boost::asio::yield_context yield)
{
	std::shared_ptr<Lobby> lobby = getLobby(connection->endpoint());
	if(lobby)
	{
		lobby->join(connection, yield);
	}
}
void Server::removeConnection(const std::string& session, const std::string& endpoint, boost::asio::yield_context yield)
{
	std::shared_ptr<Lobby> lobby = getLobby(endpoint);
	if(lobby)
	{
		if(lobby->leave(session, yield))
		{
			std::lock_guard<std::mutex> l{mut_};
			lobbies_.erase(endpoint);
			for(auto& observer : observers_)
				observer->onLobbyDelete(endpoint, yield);
		}
	}
}
void Server::onMessage(boost::shared_ptr<WebsocketSession> connection, std::istream& content, boost::asio::yield_context yield)
{
	pt::ptree data;
	pt::read_json(content, data);

	auto lobby = getLobby(connection->endpoint());
	if(!lobby)
	{
		connection->close();
		return;
	}
	
	if(lobby->onMessage(connection, data, yield))
	{
		std::lock_guard<std::mutex> l{mut_};
		lobbies_.erase(connection->endpoint());
		for(auto& observer : observers_)
			observer->onLobbyDelete(connection->endpoint(), yield);
	}
}
std::string Server::addLobby(std::shared_ptr<Lobby> lobby, boost::asio::yield_context yield)
{
	std::lock_guard<std::mutex> l{mut_};

	// TODO make better.
	const static std::vector<const char*> Words = { "Red", "Orange", "Yellow", "Green", "Blue", "Purple"};
	std::uniform_int_distribution<unsigned short> dist{0, static_cast<unsigned short>(Words.size() - 1)};

	std::string name;
	do
	{
		name = "/game/";
		name += Words[dist(random_)];
		name += Words[dist(random_)];
		name += Words[dist(random_)];
	} while (lobbies_.count(name) != 0);

	lobbies_.emplace(name, lobby);
	for(auto& observer : observers_)
		observer->onLobbyAdd(name, lobby, yield);
	return name;
}
std::optional<Server::User> Server::getUser(const std::string_view& sessionId)
{
	return getUser(std::string{sessionId});
}
std::optional<Server::User> Server::getUser(const std::string& sessionId)
{
	std::lock_guard<std::mutex> l{mut_};
	auto it = users_.find(sessionId);
	if(it != users_.end())
		return it->second;
	return {};
}
std::string Server::generateSessionId()
{
	// TODO make way better
	std::lock_guard<std::mutex> l{mut_};

	std::string id;
	std::uniform_int_distribution<char> dist{0, 25};
	for(unsigned int i = 0; i < 30; ++i)
	{
		id += 'a' + dist(random_);
	}
	return id;
}