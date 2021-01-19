#ifndef SERVER_LOBBY_UNO_HPP_INCLUDED
#define SERVER_LOBBY_UNO_HPP_INCLUDED
#include "BasicLobby.hpp"
#include "Uno.hpp"

class LobbyUno : public BasicLobby
{
	public:
		LobbyUno(Server* server, std::string name, std::string creatorSessionId);

		virtual std::optional<std::string> getHtmlFile(const std::string_view& session) const override;

		virtual bool onMessage(const boost::shared_ptr<WebsocketSession>& connection, const boost::property_tree::ptree& message, boost::asio::yield_context yield) override;

		static std::string serializeScore(const Cards::Uno::Session& game, const std::vector<PlayerInfo>& players);
		static std::string serializeHand(const Cards::Uno::Session& game, unsigned short player);
		static std::string serializePlayers(const std::optional<Cards::Uno::Session>& game, const std::vector<PlayerInfo>& players);
		static std::string serializeCurrentCard(const Cards::Uno::PlayedCard& card);
		static std::string serializeCurrentTask(const Cards::Uno::Session& game, const std::vector<PlayerInfo>& players, unsigned short player, const std::optional<unsigned short>& creatorId);
		static std::string serializeAskNextRound();

	protected:
		virtual GameState gameState(Lock&) const override;
		virtual void onPlayerJoin(const boost::shared_ptr<WebsocketSession>& connection, Lock& l, unsigned short playerIndex, boost::asio::yield_context yield) override;
		virtual void onPlayerLeave(Lock& l, boost::asio::yield_context yield) override;

	private:
          void refreshPlay(Lock &l, const std::optional<unsigned short> &handOnlyForPlayer, boost::asio::yield_context yield);

          std::optional<Cards::Uno::Session> game_;
          std::mt19937_64 randomEngine_;
};

const char* toJsonString(const Cards::Uno::Order& order);

const char* toJsonString(const Cards::Uno::Color& color);
template <>
std::optional<Cards::Uno::Color> fromJsonString<Cards::Uno::Color>(const std::string& text);

const char* toJsonString(const Cards::Uno::BasicValue& value);
template <>
std::optional<Cards::Uno::BasicValue> fromJsonString<Cards::Uno::BasicValue>(const std::string& text);

const char* toJsonString(const Cards::Uno::ColorlessValue& value);
template <>
std::optional<Cards::Uno::ColorlessValue> fromJsonString<Cards::Uno::ColorlessValue>(const std::string& text);

#endif