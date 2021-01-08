#ifndef SERVER_BASIC_LOBBY_HPP_INCLUDED
#define SERVER_BASIC_LOBBY_HPP_INCLUDED
#include <algorithm>

#include "Lobby.hpp"

class BasicLobby : public Lobby
{
	public:
		struct PlayerInfo
		{
			PlayerInfo(std::string sessionId, std::string username);
			
			std::string sessionId;
			std::string username;
		};
		enum class GameState
		{
			Created,
			Started,
			Finished,
		};
		using Lock = std::lock_guard<std::mutex>;

		BasicLobby(Server* server, std::string name, std::string creatorSessionId);

		virtual bool canJoin(const std::string_view& session) override;
		virtual void join(const boost::shared_ptr<WebsocketSession>& connection, boost::asio::yield_context yield) override;
		virtual bool leave(const std::string& session, boost::asio::yield_context yield) override;

	protected:
		Lock lock();

		virtual GameState gameState(Lock&) const = 0;
		virtual void onPlayerJoin(const boost::shared_ptr<WebsocketSession>& connection, Lock& l, unsigned short playerIndex, boost::asio::yield_context yield) = 0;
		virtual void onPlayerLeave(Lock& l, boost::asio::yield_context yield) = 0;

		std::optional<unsigned short> implGetCreatorIndex(Lock&) const;
		std::optional<unsigned short> getPlayerIndex(Lock&, const std::string& sessionId) const;
		bool isCreator(const std::string& sessionId) const;
		void setUsername(Lock&, unsigned short index, std::string username);
		const std::multimap<std::string, boost::weak_ptr<WebsocketSession>>& connections(Lock&) const;
		const std::vector<PlayerInfo>& players(Lock&) const;

		template <typename Random>
		void reorderPlayers(Lock&, Random& r)
		{
			std::shuffle(players_.begin(), players_.end(), r);
		}

	private:
		std::multimap<std::string, boost::weak_ptr<WebsocketSession>> connections_;
		std::vector<PlayerInfo> players_;
		std::string creator_;
		std::mutex mut_;
};

template <typename T>
std::optional<T> fromJsonString(const std::string&);
template <typename T>
std::optional<T> fromJsonString(const std::optional<std::string>& text)
{
	if(!text)
		return {};
	return fromJsonString<T>(*text);
}
template <typename T>
std::optional<T> fromJsonString(const boost::optional<std::string>& text)
{
	if(!text)
		return {};
	return fromJsonString<T>(*text);
}

#endif