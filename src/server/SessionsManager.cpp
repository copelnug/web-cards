#include "SessionsManager.hpp"

#include <boost/make_shared.hpp>

#include "WebsocketSession.hpp"

void SessionsManager::sessionRegister(boost::weak_ptr<WebsocketSession> session)
{
	std::lock_guard<std::mutex> l{mut_};
	sessions_.insert(std::move(session));
}
void SessionsManager::sessionRemove(boost::weak_ptr<WebsocketSession> session)
{
	std::lock_guard<std::mutex> l{mut_};
	sessions_.erase(session);
}
void SessionsManager::broadcast(std::string&& message, boost::asio::yield_context yield)
{
	std::vector<boost::weak_ptr<WebsocketSession>> sessions;
	{
		std::lock_guard<std::mutex> l{mut_};
		sessions.insert(sessions.begin(), sessions_.begin(), sessions_.end());
	}
	auto messageData = boost::make_shared<const std::string>(std::move(message));
	// TODO Do the send concurrently.
	for(auto& weakSession : sessions)
	{
		if(auto session = weakSession.lock())
		{
			session->send(messageData, yield);
		}
	}
}