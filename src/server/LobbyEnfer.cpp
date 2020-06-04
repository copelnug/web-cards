#include "LobbyEnfer.hpp"

#include <boost/make_shared.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <iostream>
#include <optional>
#include <random>
#include <sstream>
#include <stdexcept>

#include "Enfer.hpp"
#include "Exception.hpp"
#include "Server.hpp"
#include "StandardCards.hpp"

namespace
{
	namespace pt = boost::property_tree;
	using Cards::Enfer::State;

	constexpr const char* TRAD(const char* msg)
	{
		// TODO
		return msg;
	}
	struct SessionComparer
	{
		const std::string_view& sessionId;

		bool operator()(const LobbyEnfer::PlayerInfo& player)
		{
			return player.sessionId == sessionId;
		}
	};
	template <typename ...T>
	std::string TRAD(T&&... val)
	{
		std::ostringstream out;
		(out <<  ... << val);
		return std::move(out).str();
	}
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
	std::string serializeStatusHelper(const std::string& msg)
	{
		pt::ptree root;
		root.put(LobbyEnfer::MSG_ENTRY_TYPE, "STATUS");
		root.put("msg", msg);

		std::ostringstream out;
		pt::write_json(out, root);
		return out.str();
	}
}
LobbyEnfer::PlayerInfo::PlayerInfo(std::string sessionId, std::string username) :
	sessionId{std::move(sessionId)},
	username{std::move(username)}
{}
std::optional<unsigned short> LobbyEnfer::implGetCreatorIndex() const
{
	auto it = std::find_if(players_.begin(), players_.end(), SessionComparer{creator_});
	if(it != players_.end())
		return it - players_.begin();
	return {};
}
void LobbyEnfer::implSendTo(const std::string& session, std::string message, boost::asio::yield_context yield)
{
	utilsSendTo(connections_, session, message, yield);
}
void LobbyEnfer::implSendToAll(std::string message, boost::asio::yield_context yield)
{
	utilsSendToAll(connections_, message, yield);
}
void LobbyEnfer::implSendStateToAll(boost::asio::yield_context yield)
{
	if(game_)
	{
		for(unsigned short i = 0; i < players_.size(); ++i)
		{
			implSendTo(players_[i].sessionId, serializeGameState(players_, *game_, i), yield);
		}
	}
}
void LobbyEnfer::implSendNextAction(boost::asio::yield_context yield)
{
	auto creatorIndex = implGetCreatorIndex();

	for(unsigned short i = 0; i < players_.size(); ++i)
	{
		auto msg = serializeCurrentEvent(players_, game_, i, creatorIndex);
		if(msg)
			implSendTo(players_[i].sessionId, *std::move(msg), yield);
	}
}
LobbyEnfer::LobbyEnfer(Server* server, std::string name, std::string creatorSessionId) :
	Lobby{server, std::move(name)},
	creator_{std::move(creatorSessionId)},
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
bool LobbyEnfer::canJoin(const std::string_view& session)
{
	std::lock_guard<std::mutex> l{mut_};

	if(!game_)
		return true;

	if(game_->state() == State::Finished)
		return false;

	auto it = std::find_if(players_.begin(), players_.end(), SessionComparer{session});
	return it != players_.end();
}
void LobbyEnfer::join(const boost::shared_ptr<WebsocketSession>& connection, boost::asio::yield_context yield)
{
	// TODO Send when not locked
	std::lock_guard<std::mutex> l{mut_};
	if(game_)
	{
		if(game_->state() == State::Finished)
		{
			connection->close();
			return;
		}

		// Are we in the running game?
		auto it = std::find_if(players_.begin(), players_.end(), SessionComparer{connection->session()});
		if(it == players_.end())
		{
			connection->close();
			return;
		}

		unsigned short playerIndex = it - players_.begin();

		connections_.emplace(connection->session(), connection);
		connection->send(boost::make_shared<std::string>(serializeGameState(players_, *game_, playerIndex)), yield);
		
		auto msg = serializeCurrentEvent(players_, game_, playerIndex, implGetCreatorIndex());
		if(msg)
			connection->send(boost::make_shared<std::string>(*std::move(msg)), yield);
		return;
	}
	else
	{
		connections_.emplace(connection->session(), connection);

		auto it = std::find_if(players_.begin(), players_.end(), SessionComparer{connection->session()});

		// New player
		if(it == players_.end())
		{
			auto userInfo = server().getUser(connection->session());
			players_.emplace_back(connection->session(), userInfo ? userInfo->username : "");
			implSendToAll(serializePlayerList(players_), yield);

			it = std::find_if(players_.begin(), players_.end(), SessionComparer{connection->session()});
		}

		unsigned short playerIndex = it - players_.begin();
		auto msg = serializeCurrentEvent(players_, game_, playerIndex, implGetCreatorIndex());
		if(msg)
		{
			connection->send(boost::make_shared<std::string>(*std::move(msg)), yield);
		}
	}

}
bool LobbyEnfer::leave(const std::string& session, boost::asio::yield_context yield)
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
	
	if(game_)
	{
		if(game_->state() == State::Finished)
		{
			return players_.empty();
		}
		return false;
	}
	else if(last)
	{
		players_.erase(std::find_if(players_.begin(), players_.end(), SessionComparer{session}));
		implSendToAll(serializePlayerList(players_), yield);
		return players_.empty();
	}
	return false;
}
bool LobbyEnfer::onMessage(const boost::shared_ptr<WebsocketSession>& connection, const boost::property_tree::ptree& message, boost::asio::yield_context yield)
{
	// TODO send without lock
	std::lock_guard<std::mutex> l{mut_};
	try
	{
		auto it = std::find_if(players_.begin(), players_.end(), SessionComparer{connection->session()});
		if(it == players_.end())
			throw std::logic_error{"TODO"};
		unsigned short index = it - players_.begin();

		auto type = message.get_optional<std::string>(MSG_ENTRY_TYPE);
		if(!type)
			throw std::runtime_error{"TODO"};

		if(*type != "START" && *type != "SET_USERNAME" && !game_)
			throw std::runtime_error{"TODO"};

		if(*type == "START")
		{
			if(connection->session() != creator_)
				throw std::runtime_error{"TODO"};

			if(players_.size() < 3)
				throw std::runtime_error{"TODO"};
			// TODO max number

			if(game_)
				throw std::runtime_error{"TODO"};

			std::seed_seq seed{randomEngine_(), randomEngine_(), randomEngine_(), randomEngine_(), randomEngine_(), randomEngine_(), randomEngine_(), randomEngine_()};
			std::shuffle(players_.begin(), players_.end(), randomEngine_);
			game_ = Cards::Enfer::Game{static_cast<unsigned short>(players_.size()), seed};

			implSendStateToAll(yield);
			implSendNextAction(yield);
			implSendToAll(serializeRoundInfos(game_->roundNbCards(), game_->roundState()), yield);
		}
		else if(*type == "TARGET")
		{
			auto target = message.get_optional<unsigned short>("target");
			if(!target)
				throw std::runtime_error{"TODO"};
			game_->setTarget(index, *target);

			implSendStateToAll(yield);
			implSendNextAction(yield);
			implSendToAll(serializeRoundInfos(game_->roundNbCards(), game_->roundState()), yield);
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

			implSendStateToAll(yield);
			implSendNextAction(yield);
		}
		else if(*type == "NEXT")
		{
			// TODO validate player that can do next
			game_->gotoNextRound();

			implSendStateToAll(yield);
			implSendNextAction(yield);
			implSendToAll(serializeRoundInfos(game_->roundNbCards(), game_->roundState()), yield);
		}
		else if(*type == "SET_USERNAME")
		{
			auto username = message.get_optional<std::string>("username");
			if(!username)
				throw std::runtime_error{"TODO"};

			players_[index].username = *std::move(username);
			if(!game_)
			{
				implSendToAll(serializePlayerList(players_), yield);
			}

			auto msg = serializeCurrentEvent(players_, game_, index, implGetCreatorIndex());
			if(msg)
			{
				implSendTo(connection->session(), *std::move(msg), yield);
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
		connection->send(boost::make_shared<std::string>(serializeIllegalChoice()), yield);
		return false;
	}
	catch(const Cards::ActionOutOfStep& ex)
	{
		implSendStateToAll(yield);
		implSendNextAction(yield);
		connection->send(boost::make_shared<std::string>(serializeActionOutOfStep()), yield);
		return false;
	}
	catch(const Cards::NotPlayerTurn& ex)
	{
		implSendStateToAll(yield);
		implSendNextAction(yield);
		connection->send(boost::make_shared<std::string>(serializeNotPlayerTurn()), yield);
		return false;
	}
	catch(...)
	{
		std::cerr << "Unnexpected exception" << std::endl;
		utilsSendToAll(connections_, serializeStatusHelper(TRAD("Erreur serveur. Demandez à l’administrateur si vous pouvez rafraichir la page.")), yield);
		return false;
	}
}
std::string LobbyEnfer::serializePlayerList(const std::vector<PlayerInfo>& players)
{
	pt::ptree msg;
	msg.put(MSG_ENTRY_TYPE, "PLAYERS");

	pt::ptree array;
	pt::ptree node;
	for(auto& player: players)
	{
		if(player.username.empty())
			node.put_value(TRAD("Inconnu"));
		else
			node.put_value(player.username);
		array.push_back(std::make_pair("", node));
	}
	msg.add_child("players", array);

	std::ostringstream out;
	pt::write_json(out, msg);
	return out.str();
}
std::string LobbyEnfer::serializeHostStart()
{
	pt::ptree msg;
	msg.put(MSG_ENTRY_TYPE, "ASK_START");

	std::ostringstream out;
	pt::write_json(out, msg);
	return out.str();
}
std::string LobbyEnfer::serializeAskUsername(const std::string& current)
{
	pt::ptree msg;
	msg.put(MSG_ENTRY_TYPE, "ASK_USERNAME");
	if(!current.empty())
		msg.put("current", current);

	std::ostringstream out;
	pt::write_json(out, msg);
	return out.str();
}
std::string LobbyEnfer::serializeGameState(const std::vector<PlayerInfo>& players, const Cards::Enfer::Game& game, unsigned short player)
{
	pt::ptree msg;
	msg.put(MSG_ENTRY_TYPE, "STATE");

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

	std::ostringstream out;
	pt::write_json(out, msg);
	return out.str();
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
	msg.put(MSG_ENTRY_TYPE, "ASK_INTEGER");
	msg.put("msg", std::move(textMsg));
	msg.put("min", 0);
	msg.put("max", maxCards);

	std::ostringstream out;
	pt::write_json(out, msg);
	return out.str();
}
std::string LobbyEnfer::serializeAskChooseCard(bool newHand)
{
	pt::ptree msg;
	msg.put(MSG_ENTRY_TYPE, "PLAY_CARD");
	if(newHand)
		msg.put("msg", "Choisissez une carte pour commencez la main.");
	else
		msg.put("msg", "Choisissez une carte.");

	std::ostringstream out;
	pt::write_json(out, msg);
	return out.str();
}
std::string LobbyEnfer::serializeAskNextRound()
{
	pt::ptree msg;
	msg.put(MSG_ENTRY_TYPE, "ASK_CONFIRM");
	msg.put("msg", TRAD("Passez à la prochaine manche?"));

	std::ostringstream out;
	pt::write_json(out, msg);
	return out.str();
}
std::optional<std::string> LobbyEnfer::serializeCurrentEvent(const std::vector<PlayerInfo>& players, const std::optional<Cards::Enfer::Game>& game, unsigned short player, const std::optional<unsigned short>& creatorIndex)
{
	if(!game)
	{
		if(players.at(player).username.empty())
			return serializeAskUsername();
		else if(player == creatorIndex)
			return serializeHostStart();
		else if(creatorIndex)
			return serializeWaitingStart(players.at(*creatorIndex).username);
		else
			return serializeWaitingHost();
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
				return serializeWaitingHost();
		case State::Finished:
			return serializeEndGame();
	}
	return {};
}
std::string LobbyEnfer::serializeRoundInfos(unsigned short maxCards, const std::vector<Cards::Enfer::Round::PlayerStatus>& playersStatus)
{
	pt::ptree msg;
	msg.put(MSG_ENTRY_TYPE, "ROUND_INFOS");

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

	std::ostringstream out;
	pt::write_json(out, msg);
	return out.str();
}
std::string LobbyEnfer::serializeIllegalChoice()
{
	pt::ptree msg;
	msg.put(MSG_ENTRY_TYPE, "INPUT_INVALID");
	msg.put("msg", TRAD("Choix invalide. Réessayez SVP."));

	std::ostringstream out;
	pt::write_json(out, msg);
	return out.str();
}
std::string LobbyEnfer::serializeActionOutOfStep()
{
	pt::ptree msg;
	msg.put(MSG_ENTRY_TYPE, "ERROR");
	msg.put("msg", TRAD("Erreur. Cette action n’est pas celle attendue."));

	std::ostringstream out;
	pt::write_json(out, msg);
	return out.str();
}
std::string LobbyEnfer::serializeNotPlayerTurn()
{
	pt::ptree msg;
	msg.put(MSG_ENTRY_TYPE, "ERROR");
	msg.put("msg", TRAD("Erreur. Ce n’est pas votre tour."));

	std::ostringstream out;
	pt::write_json(out, msg);
	return out.str();
}
std::string LobbyEnfer::serializeWaitingStart(const std::string& username)
{
	// TODO Use std::format
	std::ostringstream out;
	out << TRAD("En attente du début de la partie déclaré par ") << username;
	return serializeStatusHelper(std::move(out).str());
}
std::string LobbyEnfer::serializeWaitingTarget(const std::string& username)
{
	// TODO Use std::format
	std::ostringstream out;
	out << TRAD("En attente du nombre de mains ciblées par ") << username;

	return serializeStatusHelper(std::move(out).str());
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

	return serializeStatusHelper(std::move(out).str());
}
std::string LobbyEnfer::serializeWaitingNext(const std::string& username)
{
	// TODO Use std::format
	std::ostringstream out;
	out << TRAD("En attente du début de la prochaine manche déclaré par ") << username;
	return serializeStatusHelper(std::move(out).str());
}
std::string LobbyEnfer::serializeEndGame()
{
	return serializeStatusHelper("La partie est terminée");
}
std::string LobbyEnfer::serializeWaitingHost()
{
	// TODO Use std::format
	std::ostringstream out;
	out << TRAD("En attente du créateur de la partie");
	return serializeStatusHelper(std::move(out).str());
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