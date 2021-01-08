#include "BasicLobby.hpp"

#include "Server.hpp"

namespace
{
	struct SessionComparer
	{
		const std::string_view& sessionId;

		bool operator()(const BasicLobby::PlayerInfo& player)
		{
			return player.sessionId == sessionId;
		}
	};
}
BasicLobby::BasicLobby(Server* server, std::string name, std::string creatorSessionId) :
	Lobby{server, std::move(name)},
	creator_{std::move(creatorSessionId)}
{
}
bool BasicLobby::canJoin(const std::string_view& session)
{
	auto l = lock();

	switch(gameState(l))
	{
		case GameState::Created:
			return true;
		case GameState::Started:
			break;
		case GameState::Finished:
			return false;
	}

	auto it = std::find_if(players_.begin(), players_.end(), SessionComparer{session});
	return it != players_.end();
}

void BasicLobby::join(const boost::shared_ptr<WebsocketSession>& connection, boost::asio::yield_context yield)
{
	// TODO Send when not locked
	std::lock_guard<std::mutex> l{mut_};

	switch(gameState(l))
	{
		case GameState::Finished:
			connection->close();
			return;
		case GameState::Started:
		{
			// Are we in the running game?
			auto it = std::find_if(players_.begin(), players_.end(), SessionComparer{connection->session()});
			if(it == players_.end())
			{
				connection->close();
				return;
			}

			unsigned short playerIndex = it - players_.begin();

			connections_.emplace(connection->session(), connection);

			onPlayerJoin(connection, l, playerIndex, yield);
			return;
		}
		case GameState::Created:
		{
			connections_.emplace(connection->session(), connection);

			auto it = std::find_if(players_.begin(), players_.end(), SessionComparer{connection->session()});

			// New player
			if(it == players_.end())
			{
				auto userInfo = server().getUser(connection->session());
				players_.emplace_back(connection->session(), userInfo ? userInfo->username : "");

				it = std::find_if(players_.begin(), players_.end(), SessionComparer{connection->session()});
			}

			unsigned short playerIndex = it - players_.begin();
			onPlayerJoin(connection, l, playerIndex, yield);
			return;
		}
	}
}
bool BasicLobby::leave(const std::string& session, boost::asio::yield_context yield)
{
	// TODO Send without locking Lobby
	std::lock_guard<std::mutex> l{mut_};

	bool last = true;
	auto [it, end] = connections_.equal_range(session);
	for(; it != end; ++it)
	{
		// Remove any expired session
		if(it->second.expired())
		{
			it = connections_.erase(it);
			break;
		}
		// Can't be the last, we keep at least one connection alive
		last = false;
	}
	// Did we delete the last
	last = last && it == end;

	switch(gameState(l))
	{
		case GameState::Finished:
			return players_.empty();
		case GameState::Started:
			break;
		case GameState::Created:
			if(last)
			{
				players_.erase(std::find_if(players_.begin(), players_.end(), SessionComparer{session}));
				onPlayerLeave(l, yield);
			}
			return players_.empty();
	}
	return false;
}
BasicLobby::Lock BasicLobby::lock()
{
	return Lock{mut_};
}
std::optional<unsigned short> BasicLobby::implGetCreatorIndex(Lock&) const
{
	auto it = std::find_if(players_.begin(), players_.end(), SessionComparer{creator_});
	if(it != players_.end())
		return it - players_.begin();
	return {};
}
std::optional<unsigned short> BasicLobby::getPlayerIndex(Lock&, const std::string& sessionId) const
{
	auto it = std::find_if(players_.begin(), players_.end(), SessionComparer{sessionId});
	if(it == players_.end())
		return {};
	return it - players_.begin();
}
bool BasicLobby::isCreator(const std::string& sessionId) const
{
	// Creator should never change so no need for lock
	return sessionId == creator_;
}
void BasicLobby::setUsername(Lock&, unsigned short index, std::string username)
{
	players_.at(index).username = std::move(username);
}
const std::multimap<std::string, boost::weak_ptr<WebsocketSession>>& BasicLobby::connections(Lock&) const
{
	return connections_;
}
const std::vector<BasicLobby::PlayerInfo>& BasicLobby::players(Lock&) const
{
	return players_;
}