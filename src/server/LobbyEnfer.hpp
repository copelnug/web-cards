#ifndef SERVER_LOBBY_ENFER_HPP_INCLUDED
#define SERVER_LOBBY_ENFER_HPP_INCLUDED
#include <boost/asio/spawn.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <mutex>
#include <optional>
#include <random>
#include <string_view>

#include "Enfer.hpp"
#include "Lobby.hpp"
#include "StandardCards.hpp"

class LobbyEnfer : public Lobby
{
	std::multimap<std::string, boost::weak_ptr<WebsocketSession>> connections_;
	std::optional<Cards::Enfer::Game> game_;
	std::vector<std::string> players_;
	std::string creator_;
	std::mt19937_64 randomEngine_;
	std::mutex mut_;

	unsigned short implGetCreatorIndex() const;

	void implSendTo(const std::string& session, std::string message, boost::asio::yield_context yield);
	void implSendToAll(std::string message, boost::asio::yield_context yield);
	void implSendStateToAll(boost::asio::yield_context yield);
	void implSendNextAction(boost::asio::yield_context yield);
public:
	LobbyEnfer(Server* server, std::string creatorSessionId);

	virtual std::optional<std::string> getHtmlFile(const std::string_view& session) const override;

	virtual void join(const boost::shared_ptr<WebsocketSession>& connection, boost::asio::yield_context yield) override;
	virtual bool leave(const std::string& session, boost::asio::yield_context yield) override;
	virtual bool onMessage(const boost::shared_ptr<WebsocketSession>& connection, const boost::property_tree::ptree& message, boost::asio::yield_context yield) override;

	// Messages
	static std::string serializePlayerList(const std::vector<std::string>& usernames);
	static std::string serializeHostStart();
	static std::string serializeAskUsername(const std::string& current = "");
	static std::string serializeGameState(const std::vector<std::string>& usernames, const Cards::Enfer::Game& game, unsigned short player);
	static std::string serializeAskTarget(unsigned short maxCards, const std::vector<Cards::Enfer::Round::PlayerStatus>& playersStatus);
	static std::string serializeAskChooseCard();
	static std::string serializeAskNextRound();

	static std::optional<std::string> serializeCurrentEvent(const std::vector<std::string>& usernames, const std::optional<Cards::Enfer::Game>& game, unsigned short player, unsigned short creatorIndex);
	
	// Error message
	static std::string serializeIllegalChoice();
	static std::string serializeActionOutOfStep();
	static std::string serializeNotPlayerTurn();

	// Status messages
	static std::string serializeWaitingStart(const std::string& username);
	static std::string serializeWaitingTarget(const std::string& username);
	static std::string serializeWaitingChoose(const std::string& username);
	static std::string serializeWaitingNext(const std::string& username);
	static std::string serializeEndGame();
};

std::string toJsonString(const Cards::Enfer::Round::PlayerStatus& status);

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

const char* toJsonString(const Cards::Standard::Kind& kind);
template <>
std::optional<Cards::Standard::Kind> fromJsonString<Cards::Standard::Kind>(const std::string& text);

const char* toJsonString(const Cards::Standard::Value& value);
template <>
std::optional<Cards::Standard::Value> fromJsonString<Cards::Standard::Value>(const std::string& text);

#endif