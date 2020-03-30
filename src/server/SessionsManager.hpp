#ifndef SERVER_SESSIONS_MANAGER_HPP_INCLUDED
#define SERVER_SESSIONS_MANAGER_HPP_INCLUDED
#include <boost/asio/spawn.hpp>
#include <boost/weak_ptr.hpp>
#include <mutex>
#include <set>

class WebsocketSession;

class SessionsManager
{
	std::set<boost::weak_ptr<WebsocketSession>> sessions_;
	std::mutex mut_;
public:
	SessionsManager() = default;
	SessionsManager(SessionsManager&&) = delete;
	SessionsManager(const SessionsManager&) = delete;

	void sessionRegister(boost::weak_ptr<WebsocketSession> session);
	void sessionRemove(boost::weak_ptr<WebsocketSession> session);

	void broadcast(std::string&& message, boost::asio::yield_context yield);
};

#endif