#include "LobbyEnfer.hpp"

#include <boost/make_shared.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <optional>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string_view>

#include "BasicLobby.hpp"
#include "Enfer.hpp"
#include "Exception.hpp"
#include "Helper.hpp"
#include "Server.hpp"
#include "StandardCards.hpp"

namespace
{
	namespace pt = boost::property_tree;
	using Cards::Enfer::State;

	pt::ptree buildScoreNode(const Cards::Enfer::Game::ScoreCase& score)
	{
		pt::ptree node;

		node.put("points", score.points);
		if(score.succeed)
			node.put("status", "success");
		else
			node.put("status", "failure");
		node.put("target", score.target);

		return node;
	}
}
void LobbyEnfer::implSendTo(Lock& l, const std::string& session, std::string message, boost::asio::yield_context yield)
{
	utilsSendTo(connections(l), session, message, yield);
}
void LobbyEnfer::implSendToAll(Lock& l, std::string message, boost::asio::yield_context yield)
{
	utilsSendToAll(connections(l), message, yield);
}
void LobbyEnfer::implSendStateToAll(Lock& l, boost::asio::yield_context yield)
{
	if(game_)
	{
		for(unsigned short i = 0; i < players(l).size(); ++i)
		{
			implSendTo(l, players(l)[i].sessionId, serializeGameState(players(l), *game_, i), yield);
		}
	}
}
void LobbyEnfer::implSendNextAction(Lock& l, boost::asio::yield_context yield)
{
	auto creatorIndex = implGetCreatorIndex(l);

	for(unsigned short i = 0; i < players(l).size(); ++i)
	{
		auto msg = serializeCurrentEvent(players(l), game_, i, creatorIndex);
		if(msg)
			implSendTo(l, players(l)[i].sessionId, *std::move(msg), yield);
	}
}
void LobbyEnfer::onPlayerJoin(const boost::shared_ptr<WebsocketSession>& connection, Lock& l, unsigned short playerIndex, boost::asio::yield_context yield)
{
	if(game_)
	{
		connection->send(boost::make_shared<std::string>(serializeGameState(players(l), *game_, playerIndex)), yield);
		auto msg = serializeCurrentEvent(players(l), game_, playerIndex, implGetCreatorIndex(l));
		if(msg)
			connection->send(boost::make_shared<std::string>(*std::move(msg)), yield);

		connection->send(boost::make_shared<std::string>(serializeRoundInfos(game_->roundNbCards(), game_->roundState())), yield);
	}
	else
	{
		implSendToAll(l, serializePlayerList(players(l)), yield);
		auto msg = serializeCurrentEvent(players(l), game_, playerIndex, implGetCreatorIndex(l));
		if(msg)
		{
			connection->send(boost::make_shared<std::string>(*std::move(msg)), yield);
		}
	}
}
void LobbyEnfer::onPlayerLeave(Lock& l, boost::asio::yield_context yield)
{
	if(!game_)
	{
		implSendToAll(l, serializePlayerList(players(l)), yield);
	}
}
BasicLobby::GameState LobbyEnfer::gameState(Lock&) const
{
	if(!game_)
		return GameState::Created;
	if(game_->state() == Cards::Enfer::State::Finished)
		return GameState::Finished;
	return GameState::Started;
}
LobbyEnfer::LobbyEnfer(Server* server, std::string name, std::string creatorSessionId) :
	BasicLobby{server, std::move(name), std::move(creatorSessionId)},
	randomEngine_{}
{
	// TODO Get seed from caller
	std::random_device r;
	std::seed_seq seed{r(), r(), r(), r(), r(), r(), r(), r()};
	randomEngine_.seed(seed);
}
std::optional<std::string> LobbyEnfer::getHtmlFile(const std::string_view&) const
{
	return "game/enfer.html";
}
bool LobbyEnfer::onMessage(const boost::shared_ptr<WebsocketSession>& connection, const boost::property_tree::ptree& message, boost::asio::yield_context yield)
{
	// TODO send without lock
	auto l = lock();
	try
	{
		unsigned short index = getPlayerIndex(l, connection->session()).value(); // TODO Special message?

		auto type = message.get_optional<std::string>(Serialize::MSG_ENTRY_TYPE);
		if(!type)
			throw std::runtime_error{"TODO"};

		if(*type != "START" && *type != "SET_USERNAME" && !game_)
			throw std::runtime_error{"TODO"};

		if(*type == "START")
		{
			if(!isCreator(connection->session()))
				throw std::runtime_error{"TODO"};

			if(players(l).size() < 3)
				throw std::runtime_error{"TODO"};
			// TODO max number

			if(game_)
				throw std::runtime_error{"TODO"};

			std::seed_seq seed{randomEngine_(), randomEngine_(), randomEngine_(), randomEngine_(), randomEngine_(), randomEngine_(), randomEngine_(), randomEngine_()};
			reorderPlayers(l, randomEngine_);
			game_ = Cards::Enfer::Game{static_cast<unsigned short>(players(l).size()), seed};

			implSendStateToAll(l, yield);
			implSendNextAction(l, yield);
			implSendToAll(l, serializeRoundInfos(game_->roundNbCards(), game_->roundState()), yield);
		}
		else if(*type == "TARGET")
		{
			auto target = message.get_optional<unsigned short>("target");
			if(!target)
				throw std::runtime_error{"TODO"};
			game_->setTarget(index, *target);

			implSendStateToAll(l, yield);
			implSendNextAction(l, yield);
			implSendToAll(l, serializeRoundInfos(game_->roundNbCards(), game_->roundState()), yield);
		}
		else if(*type == "PLAY")
		{
			auto kind = fromJsonString<Cards::Standard::Kind>(message.get_optional<std::string>("card.kind"));
			if(!kind)
				throw std::runtime_error{"TODO"};
			auto value = fromJsonString<Cards::Standard::Value>(message.get_optional<std::string>("card.value"));
			if(!value)
				throw std::runtime_error{"TODO"};
		
			game_->play(index, {*kind, *value});

			implSendStateToAll(l, yield);
			implSendNextAction(l, yield);
		}
		else if(*type == "NEXT")
		{
			if(index != implGetCreatorIndex(l))
				throw std::runtime_error{"TODO: Only creator can goto next."};

			game_->gotoNextRound();

			implSendStateToAll(l, yield);
			implSendNextAction(l, yield);
			implSendToAll(l, serializeRoundInfos(game_->roundNbCards(), game_->roundState()), yield);
		}
		else if(*type == "SET_USERNAME")
		{
			auto username = message.get_optional<std::string>("username");
			if(!username)
				throw std::runtime_error{"TODO"};

			setUsername(l, index, *std::move(username));
			if(!game_)
			{
				implSendToAll(l, serializePlayerList(players(l)), yield);
			}

			auto msg = serializeCurrentEvent(players(l), game_, index, implGetCreatorIndex(l));
			if(msg)
			{
				implSendTo(l, connection->session(), *std::move(msg), yield);
			}
		}
		else
		{
			std::cerr << "Unknown type: " << *type << std::endl;
			throw std::runtime_error{"TODO"};
		}

		return game_ && game_->state() == State::Finished;
	}
	// TODO Handle exceptions specific to game and error messages 
	catch(const Cards::IllegalChoice& ex)
	{
		connection->send(boost::make_shared<std::string>(Serialize::illegalChoice()), yield);
		return false;
	}
	catch(const Cards::ActionOutOfStep& ex)
	{
		implSendStateToAll(l, yield);
		implSendNextAction(l, yield);
		connection->send(boost::make_shared<std::string>(Serialize::actionOutOfStep()), yield);
		return false;
	}
	catch(const Cards::NotPlayerTurn& ex)
	{
		implSendStateToAll(l, yield);
		implSendNextAction(l, yield);
		connection->send(boost::make_shared<std::string>(Serialize::notPlayerTurn()), yield);
		return false;
	}
	catch(const std::exception& ex)
	{
		std::cerr << "Unnexpected exception: " << ex.what() << std::endl;
		connection->send(boost::make_shared<std::string>(Serialize::helperStatus(TRAD("Erreur serveur. Demandez à l’administrateur si vous pouvez rafraichir la page."))), yield);
		return false;
	}
	catch(...)
	{
		std::cerr << "Unnexpected exception" << std::endl;
		connection->send(boost::make_shared<std::string>(Serialize::helperStatus(TRAD("Erreur serveur. Demandez à l’administrateur si vous pouvez rafraichir la page."))), yield);
		return false;
	}
}
std::string LobbyEnfer::serializePlayerList(const std::vector<PlayerInfo>& players)
{
	pt::ptree msg;
	msg.put(Serialize::MSG_ENTRY_TYPE, "PLAYERS");

	pt::ptree array;
	pt::ptree node;
	for(auto& player: players)
	{
		node.put_value(player.displayUsername());
		array.push_back(std::make_pair("", node));
	}
	msg.add_child("players", array);

	return Serialize::helper(msg);
}
std::string LobbyEnfer::serializeGameState(const std::vector<PlayerInfo>& players, const Cards::Enfer::Game& game, unsigned short player)
{
	pt::ptree msg;
	msg.put(Serialize::MSG_ENTRY_TYPE, "STATE");

	if(game.strong())
	{
		msg.put("strong.kind", toJsonString(game.strong()->kind));
		msg.put("strong.value", toJsonString(game.strong()->value));
	}

	// Let's input the currently played cards
	auto startingPlayer = game.handStartingPlayer();
	pt::ptree array;
	for(unsigned short i = 0; i < game.numberOfPlayers(); ++i)
	{
		pt::ptree node;
		Cards::Enfer::PlayerIter p{game.numberOfPlayers(), startingPlayer};
		p += i;

		node.put("player", players[p].username);
		node.put("state", toJsonString(game.roundState(p)));
		if(game.currentHand().size() > i)
		{
			node.put("card.kind", toJsonString(game.currentHand()[i].kind));
			node.put("card.value", toJsonString(game.currentHand()[i].value));
		}

		switch(game.state())
		{
			case State::SetTarget:
			case State::Play:
				if(game.currentPlayer() == p)
					node.put("status", "current");
				break;
			case State::GotoNext:
			case State::Finished:
			{
				auto result = game.roundState(p);
				if(result.target && *result.target == result.obtained)
					node.put("status", "success");
				else
					node.put("status", "failure");
				break;
			}
		}
		
		array.push_back(std::make_pair("", node));
	}
	msg.add_child("play", std::move(array));


	// Player hand
	array.clear();
	const auto& hand = game.playerHand(player);
	for(auto& card : hand)
	{
		pt::ptree node;
		node.put("kind", toJsonString(card.kind));
		node.put("value", toJsonString(card.value));

		if(game.isNewHandStarting() || Cards::Enfer::canPlay(hand, card, game.currentHand().front())) // If we're not starting a new hand. Their must be a card played so front() is safe.
		{
			node.put("playable", true);
		}
		array.push_back(std::make_pair("", node));

	}
	msg.add_child("hand", std::move(array));

	// Score header
	array.clear();
	pt::ptree row;
	row.put("style", "header");
	pt::ptree data;
	pt::ptree node;
	node.put_value(TRAD("Cartes"));
	data.push_back(std::make_pair("", node));
	for(auto& player : players)
	{
		node.clear();
		node.put_value(player.username);
		data.push_back(std::make_pair("", node));
	}
	row.add_child("data", data);
	array.push_back(std::make_pair("", row));

	// Main score
	for(unsigned short i = 1; i <= game.scoredRound(); ++i)
	{
		row.clear();
		data.clear();
		row.put("style", "normal");

		node.clear();
		node.put_value(Cards::Enfer::roundTitle(game.numberOfPlayers(), i, game.maxRound()));
		data.push_back(std::make_pair("", node));
		
		for(unsigned short j = 0; j < game.numberOfPlayers(); ++j)
		{
			data.push_back(std::make_pair("", buildScoreNode(game.scoreFor(j, i))));
		}
		row.add_child("data", data);
		array.push_back(std::make_pair("", row));
	}

	// Ranking
	if(game.scoredRound() > 0)
	{
		row.clear();
		data.clear();
		row.put("style", "header");
		
		node.clear();
		node.put_value(TRAD("Classement"));
		data.push_back(std::make_pair("", node));

		std::vector<unsigned short> score;
		for(unsigned short i = 0; i < game.numberOfPlayers(); ++i)
		{
			score.push_back(game.scoreFor(i, game.scoredRound()).points);
		}
		std::sort(score.begin(), score.end(), [](unsigned short p1, unsigned short p2) {
			return p1 > p2;
		});

		for(unsigned short i = 0; i < game.numberOfPlayers(); ++i)
		{
			auto points = game.scoreFor(i, game.scoredRound()).points;
			
			auto it = std::find(score.begin(), score.end(), points);
			
			std::ostringstream out;
			out << (it - score.begin() + 1);
			
			node.clear();
			node.put_value(std::move(out).str());
			data.push_back(std::make_pair("", node));
		}
		row.add_child("data", data);
		array.push_back(std::make_pair("", row));
	}

	msg.add_child("score", std::move(array));

	return Serialize::helper(msg);
}
std::string LobbyEnfer::serializeAskTarget(unsigned short maxCards, const std::vector<Cards::Enfer::Round::PlayerStatus>& playersStatus)
{
	unsigned short nbPlayerLeft = playersStatus.size();
	unsigned short nbTaken = 0;

	for(auto& status : playersStatus)
	{
		if(status.target)
		{
			--nbPlayerLeft;
			nbTaken += *status.target;
		}
	}

	std::string textMsg;
	if(nbTaken < maxCards)
	{
		if(maxCards == 1)
			textMsg += TRAD("La main est toujours disponible");
		else if(maxCards == nbTaken + 1)
			textMsg += TRAD("Il reste une main sur ", maxCards);
		else
			textMsg += TRAD("Il reste ", (maxCards - nbTaken), " mains sur ", maxCards);
	}
	else if(nbTaken == maxCards)
	{
		if(maxCards == 1)
			textMsg += TRAD("La main a déjà été revendiquée");
		else
			textMsg += TRAD("Les ", maxCards, " mains ont déjà été revendiquées");
	}
	else
	{
		if(maxCards == 1)
			textMsg += TRAD("La main a déjà été revendiquée par ", nbTaken, " joueurs");
		else
			textMsg += TRAD(nbTaken, " mains sur ", maxCards, " ont été revendiquées");
	}

	if(nbPlayerLeft == 1)
		textMsg += TRAD(" et il ne reste aucun autre joueur.");
	else if(maxCards == 1)
		textMsg += TRAD(" et vous êtes ", nbPlayerLeft, " joueurs restant.");
	else if(nbTaken >= maxCards)
		textMsg += TRAD(" alors que vous êtes ", nbPlayerLeft, " joueurs restant.");
	else
		textMsg += TRAD(" pour ", nbPlayerLeft, " joueurs.");

	if(maxCards == 1)
		textMsg += TRAD(" Pensez-vous la faire?");
	else
		textMsg += TRAD(" Combien pensez-vous en faire?");
	
	pt::ptree msg;
	msg.put(Serialize::MSG_ENTRY_TYPE, "ASK_INTEGER");
	msg.put("msg", std::move(textMsg));
	msg.put("min", 0);
	msg.put("max", maxCards);

	return Serialize::helper(msg);
}
std::string LobbyEnfer::serializeAskChooseCard(bool newHand)
{
	pt::ptree msg;
	msg.put(Serialize::MSG_ENTRY_TYPE, "PLAY_CARD");
	if(newHand)
		msg.put("msg", "Choisissez une carte pour commencez la main.");
	else
		msg.put("msg", "Choisissez une carte.");

	return Serialize::helper(msg);
}
std::string LobbyEnfer::serializeAskNextRound()
{
	pt::ptree msg;
	msg.put(Serialize::MSG_ENTRY_TYPE, "ASK_CONFIRM");
	msg.put("msg", TRAD("Passez à la prochaine manche?"));

	return Serialize::helper(msg);
}
std::optional<std::string> LobbyEnfer::serializeCurrentEvent(const std::vector<PlayerInfo>& players, const std::optional<Cards::Enfer::Game>& game, unsigned short player, const std::optional<unsigned short>& creatorIndex)
{
	if(!game)
	{
		if(players.at(player).username.empty())
			return Serialize::askUsername();
		else if(player == creatorIndex)
			return Serialize::hostStart();
		else if(creatorIndex)
			return Serialize::waitingStart(players.at(*creatorIndex).username);
		else
			return Serialize::waitingHost();
	}

	switch(game->state())
	{
		case State::SetTarget:
			if(player == game->currentPlayer())
				return serializeAskTarget(game->roundNbCards(), game->roundState());
			else
				return serializeWaitingTarget(players.at(game->currentPlayer()).username);
		case State::Play:
			if(player == game->currentPlayer())
				return serializeAskChooseCard(game->isNewHandStarting());
			else
				return serializeWaitingChoose(players.at(game->currentPlayer()).username, game->isFirstHandInRound(), game->isNewHandStarting());
		case State::GotoNext:
			if(player == creatorIndex)
				return serializeAskNextRound();
			else if(creatorIndex)
				return serializeWaitingNext(players.at(*creatorIndex).username);
			else
				return Serialize::waitingHost();
		case State::Finished:
			return Serialize::endGame();
	}
	return {};
}
std::string LobbyEnfer::serializeRoundInfos(unsigned short maxCards, const std::vector<Cards::Enfer::Round::PlayerStatus>& playersStatus)
{
	pt::ptree msg;
	msg.put(Serialize::MSG_ENTRY_TYPE, "ROUND_INFOS");

	pt::ptree array;
	pt::ptree line;

	line.put_value(TRAD("Mains: ", maxCards));
	array.push_back(std::make_pair("", line));

	unsigned short taken = 0;
	for(auto& status : playersStatus)
		if(status.target)
			taken += *status.target;

	line = pt::ptree{};
	line.put_value(TRAD("Prises: ", taken));
	array.push_back(std::make_pair("", line));

	msg.add_child("msg", std::move(array));

	return Serialize::helper(msg);
}
std::string LobbyEnfer::serializeWaitingTarget(const std::string& username)
{
	// TODO Use std::format
	std::ostringstream out;
	out << TRAD("En attente du nombre de mains ciblées par ") << username;

	return Serialize::helperStatus(std::move(out).str());
}
std::string LobbyEnfer::serializeWaitingChoose(const std::string& username, bool isFirstHand, bool newHand)
{
	// TODO Use std::format
	std::ostringstream out;
	if(newHand)
	{
		if(isFirstHand)
			out << TRAD("En attente de la première carte de ", username);
		else
			out << TRAD("Main gagnée par ", username, ". En attente de la nouvelle main.");
	}
	else
		out << TRAD("En attente de la carte jouée par ") << username;

	return Serialize::helperStatus(std::move(out).str());
}
std::string LobbyEnfer::serializeWaitingNext(const std::string& username)
{
	// TODO Use std::format
	std::ostringstream out;
	out << TRAD("En attente du début de la prochaine manche déclaré par ") << username;
	return Serialize::helperStatus(std::move(out).str());
}
std::string toJsonString(const Cards::Enfer::Round::PlayerStatus& status)
{
	// TODO Use std::format or similar
	if(status.target)
	{
		std::stringstream out;
		out << status.obtained;
		out << TRAD(" sur ");
		out << *status.target;
		return std::move(out).str();
	}
	return TRAD("?");
}
const char* toJsonString(const Cards::Standard::Kind& kind)
{
	using Cards::Standard::Kind;
	switch(kind)
	{
		case Kind::Clover:
			return "CLOVER";
		case Kind::Heart:
			return "HEART";
		case Kind::Pike:
			return "PIKE";
		case Kind::Tile:
			return "TILE";
	}
	return "";
}
template <>
std::optional<Cards::Standard::Kind> fromJsonString<Cards::Standard::Kind>(const std::string& text)
{
	// TODO Make cleaner

	using Cards::Standard::Kind;
	const static std::vector<Kind> Kinds{Kind::Clover, Kind::Heart, Kind::Pike, Kind::Tile};
	for(auto& kind : Kinds)
		if(toJsonString(kind) == text)
			return kind;
	return {};
}
const char* toJsonString(const Cards::Standard::Value& value)
{
	using Cards::Standard::Value;
	switch(value)
	{
		case Value::Two: return "TWO";
		case Value::Three: return "THREE";
		case Value::Four: return "FOUR";
		case Value::Five: return "FIVE";
		case Value::Six: return "SIX";
		case Value::Seven: return "SEVEN";
		case Value::Eight: return "EIGHT";
		case Value::Nine: return "NINE";
		case Value::Ten: return "TEN";
		case Value::Jack: return "JACK";
		case Value::Queen: return "QUEEN";
		case Value::King: return "KING";
		case Value::Ace: return "ACE";
	}
	return "";
}
template <>
std::optional<Cards::Standard::Value> fromJsonString<Cards::Standard::Value>(const std::string& text)
{
	// TODO Make cleaner: Reflexion? a isIn function?

	using Cards::Standard::Value;
	const static std::vector<Value> Values{
		Value::Two, Value::Three, Value::Four, Value::Five, Value::Six, Value::Seven, Value::Eight, Value::Nine,
		Value::Ten, Value::Jack, Value::Queen, Value::King, Value::Ace
	};
	for(auto& value : Values)
		if(toJsonString(value) == text)
			return value;
	return {};

}