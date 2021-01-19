#include "LobbyUno.hpp"

#include <boost/property_tree/ptree.hpp>
#include <exception>
#include <iostream>
#include <stdexcept>

#include "BasicLobby.hpp"
#include "Exception.hpp"
#include "Helper.hpp"
#include "Uno.hpp"

namespace
{
	const std::optional<unsigned short> ALL_PLAYERS;
	namespace pt = boost::property_tree;

	pt::ptree buildScoreNode(const Cards::Uno::Session::ScoreCase& score)
	{
		pt::ptree node;

		node.put("points", score.points());
		node.put("won", score.won());

		return node;
	}
	pt::ptree createCardNode(const Cards::Uno::Card& card)
	{
		pt::ptree node;
		if(card.basic())
		{
			node.put("value", toJsonString(card.basic()->value()));
			node.put("color", toJsonString(card.basic()->color()));
		}
		else
		{
			node.put("value", toJsonString(*card.colorless()));
		}
		return node;
	}
}

LobbyUno::LobbyUno(Server* server, std::string name, std::string creatorSessionId) :
	BasicLobby{server, std::move(name), std::move(creatorSessionId)}
{
	// TODO Get seed from caller
	std::random_device r;
	std::seed_seq seed{r(), r(), r(), r(), r(), r(), r(), r()};
	randomEngine_.seed(seed);
}
std::optional<std::string> LobbyUno::getHtmlFile(const std::string_view& session) const
{
	return "game/uno.html";
}
bool LobbyUno::onMessage(const boost::shared_ptr<WebsocketSession>& connection, const boost::property_tree::ptree& message, boost::asio::yield_context yield)
{
	auto l = lock();

	unsigned short index = getPlayerIndex(l, connection->session()).value();

	try
	{
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

			reorderPlayers(l, randomEngine_);
			std::seed_seq seed{randomEngine_(), randomEngine_(), randomEngine_(), randomEngine_(), randomEngine_(), randomEngine_(), randomEngine_(), randomEngine_()};
			game_ = Cards::Uno::Session{std::mt19937_64{seed}, static_cast<unsigned short>(players(l).size())};

			utilsSendToAll(connections(l), serializeScore(*game_, players(l)), yield);
			refreshPlay(l, ALL_PLAYERS, yield);
		}
		else if(*type == "PLAY")
		{
			auto color = fromJsonString<Cards::Uno::Color>(message.get_optional<std::string>("card.color"));
			if(color)
			{
				auto value = fromJsonString<Cards::Uno::BasicValue>(message.get_optional<std::string>("card.value"));
				if(!value)
					throw std::runtime_error{"TODO"};
				game_->play(index, Cards::Uno::Card{*color, *value}, {});
			}
			else 
			{
				color = fromJsonString<Cards::Uno::Color>(message.get_optional<std::string>("color"));
				if(!color)
					throw std::runtime_error{"TODO"};
				auto value = fromJsonString<Cards::Uno::ColorlessValue>(message.get_optional<std::string>("card.value"));
				if(!value)
					throw std::runtime_error{"TODO"};
				game_->play(index, Cards::Uno::Card{*value}, color);
			}

			refreshPlay(l, index, yield);
			if(game_->state() == Cards::Uno::State::Finished)
				utilsSendToAll(connections(l), serializeScore(*game_, players(l)), yield);
		}
		else if(*type == "DRAW")
		{
			game_->draw(index);
			utilsSendTo(connections(l), players(l)[index].sessionId, serializeCurrentTask(*game_, players(l), index, implGetCreatorIndex(l)), yield);
		}
		else if(*type == "DRAW_CONFIRM")
		{
			auto wantToPlay = message.get<bool>("play", false);
			game_->drawDone(index, wantToPlay);
			refreshPlay(l, index, yield);

			// Update score if the last action was a PLUS_X 
			if(game_->state() == Cards::Uno::State::Finished)
				utilsSendToAll(connections(l), serializeScore(*game_, players(l)), yield);
		}
		else if(*type == "NEXT")
		{
			if(index != implGetCreatorIndex(l))
				throw std::runtime_error{"TODO: Only creator can goto next."};
			game_->nextGame();

			utilsSendToAll(connections(l), serializeScore(*game_, players(l)), yield);
			refreshPlay(l, ALL_PLAYERS, yield);
		}
		else if(*type == "END")
		{
			if(index != implGetCreatorIndex(l))
				throw std::runtime_error{"TODO: Only creator can end game."};
			utilsSendToAll(connections(l), Serialize::endGame(), yield);
			return true;
		}
		else if(*type == "SET_USERNAME")
		{
			auto username = message.get_optional<std::string>("username");
			if(!username)
				throw std::runtime_error{"TODO"};

			setUsername(l, index, *std::move(username));

			utilsSendToAll(connections(l), serializePlayers(game_, players(l)), yield);
			auto creator = implGetCreatorIndex(l);
			if(creator)
			{
				utilsSendTo(connections(l), connection->session(), Serialize::waitingStart(players(l).at(*creator).username), yield);
			}
			else
			{
				utilsSendTo(connections(l), connection->session(), Serialize::waitingHost(), yield);
			}
		}
		else
		{
			std::cerr << "Unknown type: " << *type << std::endl;
			throw std::runtime_error{"TODO"};
		}

		return false;
	}
	// TODO Handle exceptions specific to game and error messages 
	catch(const Cards::IllegalChoice& ex)
	{
		connection->send(boost::make_shared<std::string>(Serialize::illegalChoice()), yield);
		return false;
	}
	catch(const Cards::ActionOutOfStep& ex)
	{
		refreshPlay(l, index, yield);
		connection->send(boost::make_shared<std::string>(Serialize::actionOutOfStep()), yield);
		return false;
	}
	catch(const Cards::NotPlayerTurn& ex)
	{
		refreshPlay(l, index, yield);
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
std::string LobbyUno::serializeScore(const Cards::Uno::Session& game, const std::vector<PlayerInfo>& players)
{
	pt::ptree msg;
	msg.put(Serialize::MSG_ENTRY_TYPE, "SCORE");
	pt::ptree array;

	// Score header
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
	std::vector<unsigned int> total(game.nbPlayers(), 0);
	for(unsigned short i = 0; i < game.nbGames(); ++i)
	{
		row.clear();
		data.clear();
		row.put("style", "normal");

		node.clear();
		node.put_value(i+1);
		data.push_back(std::make_pair("", node));
		
		for(unsigned short j = 0; j < game.nbPlayers(); ++j)
		{
			auto score = game.scoreFor(i, j);
			total[j] += score.points();
			data.push_back(std::make_pair("", buildScoreNode(score)));
		}
		row.add_child("data", data);
		array.push_back(std::make_pair("", row));
	}

	// Totals
	row.clear();
	data.clear();
	row.put("style", "header");

	node.clear();
	node.put_value(TRAD("Total"));
	data.push_back(std::make_pair("", node));

	for(unsigned short i = 0; i < total.size(); ++i)
	{
		node.clear();
		node.put_value(total[i]);
		data.push_back(std::make_pair("", node));
	}
	row.add_child("data", data);
	array.push_back(std::make_pair("", row));

	// Ranking
	row.clear();
	data.clear();
	row.put("style", "header");
	
	node.clear();
	node.put_value(TRAD("Classement"));
	data.push_back(std::make_pair("", node));

	std::vector<unsigned int> score{total.begin(), total.end()};
	std::sort(score.begin(), score.end(), [](unsigned int p1, unsigned int p2) {
		return p1 < p2;
	});

	for(unsigned short i = 0; i < total.size(); ++i)
	{
		auto points = total[i];
		
		auto it = std::find(score.begin(), score.end(), points);
		
		std::ostringstream out;
		out << (it - score.begin() + 1);
		
		node.clear();
		node.put_value(std::move(out).str());
		data.push_back(std::make_pair("", node));
	}
	row.add_child("data", data);
	array.push_back(std::make_pair("", row));

	msg.add_child("score", std::move(array));

	return Serialize::helper(msg);
}
std::string LobbyUno::serializeHand(const Cards::Uno::Session& game, unsigned short player)
{
	bool isPlaying = game.currentPlayer() == player;

	pt::ptree msg;
	msg.put(Serialize::MSG_ENTRY_TYPE, "HAND");

	pt::ptree array;
	auto& hand = game.getHand(player);
	for(auto& card : hand)
	{
		auto node = createCardNode(card);
		if(isPlaying)
			node.put("playable", Cards::Uno::canPlay(card, hand, game.currentCard()));

		array.push_back(std::make_pair("", node));
	}
	msg.add_child("hand", std::move(array));

	if(isPlaying)
	{
		msg.put("playing", true);
		msg.put("msg", "Choisissez une carte.");
		msg.put("can_draw", game.state() == Cards::Uno::State::PlayOrDraw);
	}

	return Serialize::helper(msg);
}
std::string LobbyUno::serializePlayers(const std::optional<Cards::Uno::Session>& game, const std::vector<PlayerInfo>& players)
{
	pt::ptree msg;
	msg.put(Serialize::MSG_ENTRY_TYPE, "PLAYERS");

	pt::ptree array;
	for(unsigned int i = 0; i < players.size(); ++i)
	{
		pt::ptree node;
		node.put("id", i);
		node.put("player", players[i].displayUsername());
		if(game)
		{
			node.put("cards", game->getHand(i).size());
		}
		array.push_back(std::make_pair("", node));
	}
	msg.add_child("players", std::move(array));

	if(game)
	{
		msg.put("current", game->currentPlayer());
		msg.put("order", toJsonString(game->currentOrder()));
	}

	return Serialize::helper(msg);
}
std::string LobbyUno::serializeCurrentCard(const Cards::Uno::PlayedCard& card)
{
	pt::ptree msg;
	msg.put(Serialize::MSG_ENTRY_TYPE, "CURRENT_CARD");

	if(card.basic())
	{
		msg.put("card.value", toJsonString(*card.basic()));
		msg.put("card.color", toJsonString(card.color()));
	}
	else
	{
		msg.put("card.value", toJsonString(*card.colorless()));
		msg.put("color", toJsonString(card.color()));
	}

	return Serialize::helper(msg);
}
std::string LobbyUno::serializeCurrentTask(const Cards::Uno::Session& game, const std::vector<PlayerInfo>& players, unsigned short player, const std::optional<unsigned short>& creatorId)
{
	switch(game.state())
	{
		case Cards::Uno::State::Finished:
			break;
		case Cards::Uno::State::Play:
		case Cards::Uno::State::PlayOrDraw:
		{
			if(player == game.currentPlayer())
			{
				return serializeHand(game, player);
			}
			else
			{
				std::ostringstream out;
				out << TRAD("En attente du choix de ") << players.at(game.currentPlayer()).username;
				return Serialize::helperStatus(std::move(out).str());
			}
		}
		case Cards::Uno::State::Drawing:
		{
			if(player == game.currentPlayer())
			{
				pt::ptree msg;
				msg.put(Serialize::MSG_ENTRY_TYPE, "DRAW");

				pt::ptree array;
				auto cards = game.drawnCards();
				for(auto& card : cards)
				{
					array.push_back(std::make_pair("", createCardNode(card)));
				}
				msg.add_child("cards", std::move(array));

				msg.put("playable.drawn", game.canPlayDrawnCard());
				msg.put("playable.hand", game.canPlayHandCard());

				return Serialize::helper(msg);
			}
			else
			{
				std::ostringstream out;
				unsigned int nb = game.drawnCards().size();
				if(nb == 1)
					out << players.at(game.currentPlayer()).username << TRAD(" pige une carte.");
				else
					out << players.at(game.currentPlayer()).username << TRAD(" pige ") << nb << TRAD(" cartes."); // TODO std::format
				return Serialize::helperStatus(std::move(out).str());
			}
		}
	}

	if(player == creatorId)
	{
		return serializeAskNextRound();
	}
	else
	{
		return Serialize::helperStatus("La partie est terminée. En attente de la nouvelle partie.");
	}
}
std::string LobbyUno::serializeAskNextRound()
{
	pt::ptree msg;
	msg.put(Serialize::MSG_ENTRY_TYPE, "ASK_CONFIRM");
	msg.put("msg", TRAD("Commencer une nouvelle partie?"));

	return Serialize::helper(msg);
}
BasicLobby::GameState LobbyUno::gameState(Lock&) const
{
	if(!game_)
		return GameState::Created;
	return GameState::Started; // Technically never finished until the game is deleted.
}
void LobbyUno::onPlayerJoin(const boost::shared_ptr<WebsocketSession>& connection, Lock& l, unsigned short playerIndex, boost::asio::yield_context yield)
{
	if(game_)
	{
		auto& session = players(l)[playerIndex].sessionId;
		utilsSendTo(connections(l), session, serializeScore(*game_, players(l)), yield);
		utilsSendTo(connections(l), session, serializePlayers(game_, players(l)), yield);
		utilsSendTo(connections(l), session, serializeCurrentCard(game_->currentCard()), yield);
		utilsSendTo(connections(l), session, serializeHand(*game_, playerIndex), yield);
		utilsSendTo(connections(l), session, serializeCurrentTask(*game_, players(l), playerIndex, implGetCreatorIndex(l)), yield);
	}
	else
	{
		utilsSendToAll(connections(l), serializePlayers(game_, players(l)), yield);

		auto creator = implGetCreatorIndex(l);
		auto& player = players(l)[playerIndex];
		if(player.username.empty())
		{
			utilsSendTo(connections(l), player.sessionId, Serialize::askUsername(), yield);
		}
		else if(playerIndex == creator)
		{
			utilsSendTo(connections(l), player.sessionId, Serialize::hostStart(), yield);
		}
		else if(creator)
		{
			utilsSendTo(connections(l), player.sessionId, Serialize::waitingStart(players(l).at(*creator).username), yield);
		}
		else
		{
			utilsSendTo(connections(l), player.sessionId, Serialize::waitingHost(), yield);
		}
	}
}
void LobbyUno::onPlayerLeave(Lock& l, boost::asio::yield_context yield)
{
	if(!game_)
	{
		utilsSendToAll(connections(l), serializePlayers(game_, players(l)), yield);
	}
}
void LobbyUno::refreshPlay(Lock& l, const std::optional<unsigned short>& changedPlayer, boost::asio::yield_context yield)
{
	utilsSendToAll(connections(l), serializePlayers(game_, players(l)), yield);
	utilsSendToAll(connections(l), serializeCurrentCard(game_->currentCard()), yield);
	for(unsigned short i = 0; i < players(l).size(); ++i)
	{
		auto& session = players(l)[i].sessionId;
		if(!changedPlayer || changedPlayer == i)
		{
			// Send the hand only when it changed
			utilsSendTo(connections(l), session, serializeHand(*game_, i), yield);
		}
		utilsSendTo(connections(l), session, serializeCurrentTask(*game_, players(l), i, implGetCreatorIndex(l)), yield);
	}
}
const char* toJsonString(const Cards::Uno::Order& order)
{
	switch(order)
	{
		case Cards::Uno::Order::Normal:
			break;
		case Cards::Uno::Order::Reversed:
			return "REVERSED";
	}
	return "NORMAL";
}
const char* toJsonString(const Cards::Uno::Color& color)
{
	switch(color)
	{
		case Cards::Uno::Color::Blue:
			return "BLUE";
		case Cards::Uno::Color::Green:
			return "GREEN";
		case Cards::Uno::Color::Red:
			return "RED";
		case Cards::Uno::Color::Yellow:
			break;
	}
	return "YELLOW";
}
template <>
std::optional<Cards::Uno::Color> fromJsonString<Cards::Uno::Color>(const std::string& text)
{
	using Cards::Uno::Color;
	const static std::vector<Color> Values{
		Color::Blue, Color::Green, Color::Red, Color::Yellow,
	};
	for(auto& value : Values)
	{
		if(text == toJsonString(value))
			return value;
	}
	return {};
}
const char* toJsonString(const Cards::Uno::BasicValue& value)
{
	using Cards::Uno::BasicValue;
	switch(value)
	{
		case BasicValue::Zero:
			return "ZERO";
		case BasicValue::One:
			return "ONE";
		case BasicValue::Two:
			return "TWO";
		case BasicValue::Three:
			return "THREE";
		case BasicValue::Four:
			return "FOUR";
		case BasicValue::Five:
			return "FIVE";
		case BasicValue::Six:
			return "SIX";
		case BasicValue::Seven:
			return "SEVEN";
		case BasicValue::Eight:
			return "EIGHT";
		case BasicValue::Nine:
			return "NINE";
		case BasicValue::SkipTurn:
			return "SKIP";
		case BasicValue::Plus2:
			return "PLUS_2";
		case BasicValue::Reverse:
			break;
	}
	return "REVERSE";
}
template <>
std::optional<Cards::Uno::BasicValue> fromJsonString<Cards::Uno::BasicValue>(const std::string& text)
{
	using Cards::Uno::BasicValue;
	const static std::vector<BasicValue> Values{
		BasicValue::Zero, BasicValue::One, BasicValue::Two, BasicValue::Three, BasicValue::Four,
		BasicValue::Five, BasicValue::Six, BasicValue::Seven, BasicValue::Eight, BasicValue::Nine,
		BasicValue::SkipTurn, BasicValue::Plus2,BasicValue::Reverse,
	};
	for(auto& value : Values)
	{
		if(text == toJsonString(value))
			return value;
	}
	return {};
}
const char* toJsonString(const Cards::Uno::ColorlessValue& value)
{
	using Cards::Uno::ColorlessValue;
	switch(value)
	{
		case ColorlessValue::ChangeColor:
			break;
		case ColorlessValue::Plus4:
			return "PLUS_4";
	}
	return "CHANGE";
}
template <>
std::optional<Cards::Uno::ColorlessValue> fromJsonString<Cards::Uno::ColorlessValue>(const std::string& text)
{
	using Cards::Uno::ColorlessValue;
	const static std::vector<ColorlessValue> Values{
		ColorlessValue::ChangeColor,
		ColorlessValue::Plus4,
	};
	for(auto& value : Values)
	{
		if(text == toJsonString(value))
			return value;
	}
	return {};
}