#ifndef SERVER_LOBBY_ENFER_HPP_INCLUDED
#define SERVER_LOBBY_ENFER_HPP_INCLUDED
#include <boost/asio/spawn.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <mutex>
#include <optional>
#include <random>
#include <string_view>

#include "BasicLobby.hpp"
#include "Enfer.hpp"
#include "StandardCards.hpp"

class LobbyEnfer : public BasicLobby
{
public:
private:

	std::optional<Cards::Enfer::Game> game_;
	std::mt19937_64 randomEngine_;

	void implSendTo(Lock& l, const std::string& session, std::string message, boost::asio::yield_context yield);
	void implSendToAll(Lock& l, std::string message, boost::asio::yield_context yield);
	void implSendStateToAll(Lock& l, boost::asio::yield_context yield);
	void implSendNextAction(Lock& l, boost::asio::yield_context yield);

protected:
	virtual GameState gameState(Lock&) const override;
	virtual void onPlayerJoin(const boost::shared_ptr<WebsocketSession>& connection, Lock& l, unsigned short playerIndex, boost::asio::yield_context yield) override;
	virtual void onPlayerLeave(Lock& l, boost::asio::yield_context yield) override;
public:
	LobbyEnfer(Server* server, std::string name, std::string creatorSessionId);

	virtual std::optional<std::string> getHtmlFile(const std::string_view& session) const override;

	virtual bool onMessage(const boost::shared_ptr<WebsocketSession>& connection, const boost::property_tree::ptree& message, boost::asio::yield_context yield) override;

	// Messages
	static std::string serializePlayerList(const std::vector<PlayerInfo>& players);
	static std::string serializeHostStart();
	static std::string serializeAskUsername(const std::string& current = "");
	static std::string serializeGameState(const std::vector<PlayerInfo>& players, const Cards::Enfer::Game& game, unsigned short player);
	static std::string serializeAskTarget(unsigned short maxCards, const std::vector<Cards::Enfer::Round::PlayerStatus>& playersStatus);
	static std::string serializeAskChooseCard(bool newHand);
	static std::string serializeAskNextRound();

	static std::optional<std::string> serializeCurrentEvent(const std::vector<PlayerInfo>& players, const std::optional<Cards::Enfer::Game>& game, unsigned short player, const std::optional<unsigned short>& creatorIndex);
	static std::string serializeRoundInfos(unsigned short maxCards, const std::vector<Cards::Enfer::Round::PlayerStatus>& playersStatus);
	
	// Error message
	static std::string serializeIllegalChoice();
	static std::string serializeActionOutOfStep();
	static std::string serializeNotPlayerTurn();

	// Status messages
	static std::string serializeWaitingStart(const std::string& username);
	static std::string serializeWaitingTarget(const std::string& username);
	static std::string serializeWaitingChoose(const std::string& username, bool isFirstHand, bool newHand);
	static std::string serializeWaitingNext(const std::string& username);
	static std::string serializeEndGame();
	static std::string serializeWaitingHost();
};

std::string toJsonString(const Cards::Enfer::Round::PlayerStatus& status);

const char* toJsonString(const Cards::Standard::Kind& kind);
template <>
std::optional<Cards::Standard::Kind> fromJsonString<Cards::Standard::Kind>(const std::string& text);

const char* toJsonString(const Cards::Standard::Value& value);
template <>
std::optional<Cards::Standard::Value> fromJsonString<Cards::Standard::Value>(const std::string& text);

#endif