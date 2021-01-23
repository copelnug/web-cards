#include "Uno.hpp"
#include "Exception.hpp"
#include <algorithm>
#include <stdexcept>

Cards::Uno::BasicCard::BasicCard(Color color, BasicValue value) :
	color_{color},
	value_{value}
{}
bool Cards::Uno::operator==(const BasicCard& c1, const BasicCard& c2)
{
	return std::tie(c1.color(), c1.value()) == std::tie(c2.color(), c2.value());
}
bool Cards::Uno::operator!=(const BasicCard& c1, const BasicCard& c2)
{
	return !(c1 == c2);
}
Cards::Uno::Card::Card(Color color, BasicValue value) :
	basic_{BasicCard{color, value}}
{}
Cards::Uno::Card::Card(ColorlessValue value) :
	colorless_{value}
{}
bool Cards::Uno::operator==(const Card& c1, const Card& c2)
{
	return std::tie(c1.basic(), c1.colorless()) == std::tie(c2.basic(), c2.colorless());
}
bool Cards::Uno::operator!=(const Cards::Uno::Card& c1, const Cards::Uno::Card& c2)
{
	return !(c1 == c2);
}
Cards::Uno::PlayedCard::PlayedCard(Color color, BasicValue value) :
	basic_{value},
	color_{color}
{}
Cards::Uno::PlayedCard::PlayedCard(Color color, ColorlessValue value) :
	colorless_{value},
	color_{color}
{}
Cards::Uno::PlayerIter::PlayerIter(unsigned short nbPlayers, unsigned short starting) :
	current_{starting},
	nb_{nbPlayers},
	order_{Order::Normal}
{}
Cards::Uno::PlayerIter& Cards::Uno::PlayerIter::operator+=(unsigned short advance)
{
	while(advance >= nb_)
		advance -= nb_;

	switch(order_)
	{
		case Order::Normal:
		{
			current_ += advance;
			if(current_ >= nb_)
				current_ -= nb_;
			break;
		}
		case Order::Reversed:
		{
			if(current_ < advance)
				current_ += nb_;
			current_ -= advance;
			break;
		}
	}
	return *this;
}
Cards::Uno::PlayerIter Cards::Uno::PlayerIter::operator+(unsigned short advance) const
{
	PlayerIter cpy{*this};
	cpy += advance;
	return cpy;
}
void Cards::Uno::PlayerIter::reverse()
{
	switch(order_)
	{
		case Order::Normal:
			order_ = Order::Reversed;
			break;
		case Order::Reversed:
			order_ = Order::Normal;
			break;
	}
}
void Cards::Uno::PlayerIter::reset(unsigned short current)
{
	order_ = Order::Normal;
	current_ = current;
	while(current >= nb_)
		current -= nb_;
}
Cards::Uno::Game::Game(std::mt19937_64 randomEngine, PlayerIter startingPlayer) :
	randomEngine_{std::move(randomEngine)},
	lastCard_{Color::Blue, BasicValue::Zero}, // Will be overwritten by restart()
	current_{std::move(startingPlayer)},
	state_{State::Finished}, // Needed for restart
	drawQty_{0},
	drawCanPlay_{false}
{
	hands_.resize(current_.count());  
	restart(current_);
}
void Cards::Uno::Game::play(unsigned short player, const Card& card, const std::optional<Color>& color)
{
	// Validate player & state
	if(player != current_)
		throw NotPlayerTurn{player};

	switch(state_)
	{
		case State::Play:
		case State::PlayOrDraw:
			break;
		case State::Drawing:
			if(drawCanPlay_)
				break;
			throw ActionOutOfStep{player};
		case State::Finished:
			throw ActionOutOfStep{player};
	}

	// Validate other inputs
	if(card.colorless() && !color)
		throw IllegalChoice{player};

	// Validate choice
	auto& hand = hands_[player];

	switch(state_)
	{
		case State::Play:
		case State::PlayOrDraw:
		{
			auto it = std::find(hand.begin(), hand.end(), card);
			if(it != hand.end())
			{
				if(!canPlay(card, hand, lastCard_))
					throw IllegalChoice{player};
				hand.erase(it);
				break;
			}
		}
		// fallthrough
		case State::Drawing:
			if(state_ == State::Play)
				throw IllegalChoice{player};
			else if(state_ != State::Drawing || drawCanPlay_)
			{
				auto new_card = drawnCards();
				if(new_card.size() != 1)
					throw IllegalChoice{player};
				auto it = std::find(new_card.begin(), new_card.end(), card);
				if(it == new_card.end())
					throw IllegalChoice{player};
				if(!canPlay(card, hand, lastCard_))
					throw IllegalChoice{player};
				deck_.pop_back();
				break;
			}
		case State::Finished:
			throw ActionOutOfStep{player};
	}
	
	// Play the card
	played_.push_back(card);

	if(card.colorless())
		lastCard_ = PlayedCard{color.value(), card.colorless().value()};
	else
		lastCard_ = PlayedCard{card.basic().value().color(), card.basic().value().value()};
	state_ = State::PlayOrDraw;
	
	if(card.basic())
	{
		switch(card.basic().value().value())
		{
			case BasicValue::Reverse:
				current_.reverse();
				break;
			case BasicValue::SkipTurn:
				current_ += 1;
				break;
			case BasicValue::Plus2:
				readyDraw(2, false);
				break;
			default:
				break;
		}
	}
	else if(card.colorless())
	{
		switch(card.colorless().value())
		{
			case ColorlessValue::Plus4:
				readyDraw(4, false);
				break;
			default:
				break;
		}
	}
	else
		throw std::runtime_error{"TODO"};
	current_ += 1;

	checkForEnd();
}
void Cards::Uno::Game::draw(unsigned short player)
{
	// Validate player & state
	if(player != current_)
		throw NotPlayerTurn{player};

	switch(state_)
	{
		case State::PlayOrDraw:
			break;
		case State::Play:
			throw IllegalChoice{player};
		case State::Drawing:
		case State::Finished:
			throw ActionOutOfStep{player};
	}

	// Set state
	readyDraw(1, true);
}
void Cards::Uno::Game::drawDone(unsigned short player, bool wantToPlay)
{
	// Validate player & state
	if(player != current_)
		throw NotPlayerTurn{player};

	switch(state_)
	{
		case State::Drawing:
			break;
		case State::Play:
		case State::PlayOrDraw:
		case State::Finished:
			throw ActionOutOfStep{player};
	}

	if(wantToPlay && !drawCanPlay_)
		throw IllegalChoice{player};

	// Do actions
	auto& hand = hands_[player];

	auto new_cards = drawnCards();
	hand.insert(hand.begin(), new_cards.begin(), new_cards.end());
	organize(hand);

	auto start = deck_.end()-new_cards.size();
	auto end = deck_.end();
	deck_.erase(start, end);

	// Set state
	if(wantToPlay)
	{
		state_ = State::Play;
		// Stay on same player if he want to play a card
	}
	else
	{
		state_ = State::PlayOrDraw;
		current_ += 1;
	}

	// In case somebody just played a +2/+4 as a last card, check for the end here
	checkForEnd();
}
void Cards::Uno::Game::restart(unsigned short startingPlayer)
{
	// Validate state
	switch(state_)
	{
		case State::Drawing:
		case State::Play:
		case State::PlayOrDraw:
			throw ActionOutOfStep{};
		case State::Finished:
			break;
	}

	// Rebuild deck
	deck_ = createFullDeck();
	std::shuffle(deck_.begin(), deck_.end(), randomEngine_);

	// Make hands
	for(unsigned short i = 0; i < hands_.size(); ++i)
	{
		if(deck_.size() < 8)
			throw std::runtime_error{"TODO"};

		auto start = deck_.end() - cardsForNbPlayers(hands_.size());
		auto end = deck_.end();

		hands_[i] = Hand{start, end};
		deck_.erase(start, end);

		organize(hands_[i]);
	}

	// Choose first card
	do
	{
		if(deck_.empty())
			throw std::runtime_error{"TODO No cards left"};
		auto card = deck_.back();
		deck_.pop_back();

		played_.emplace_back(std::move(card));
	} while(played_.back().colorless()); // Can't start with colorless
	lastCard_ = PlayedCard{played_.back().basic().value().color(), played_.back().basic().value().value()};

	// Set state
	state_ = State::PlayOrDraw;
	current_.reset(startingPlayer);

	// Play the first card
	switch(lastCard_.basic().value())
	{
		case BasicValue::SkipTurn:
			current_ += 1;
			break;
		case BasicValue::Reverse:
			current_.reverse();
			current_ += 2; // Because the previous player is considered to have played that card.
			break;
		case BasicValue::Plus2:
			readyDraw(2, false);
			break;
		default:
			break;
	}
}
const Cards::Uno::State& Cards::Uno::Game::state() const
{
	return state_;
}
unsigned short Cards::Uno::Game::currentPlayer() const
{
	return current_;
}
Cards::Uno::Order Cards::Uno::Game::currentOrder() const
{
	return current_.order();
}
const Cards::Uno::Hand& Cards::Uno::Game::getHand(unsigned short player) const
{
	if(player >= hands_.size())
		throw std::runtime_error("TODO");

	return hands_[player];
}
std::vector<Cards::Uno::Card> Cards::Uno::Game::drawnCards() const
{
	unsigned int qty = drawQty_;
	if(drawQty_ > deck_.size())
		qty = deck_.size();

	auto start = deck_.end()-qty;
	auto end = deck_.end();
	std::vector<Card> result{start, end};
	organize(result);
	return result;
}
const Cards::Uno::PlayedCard& Cards::Uno::Game::currentCard() const
{
	return lastCard_;
}
bool Cards::Uno::Game::canPlayDrawnCard() const
{
	if(state_ != Cards::Uno::State::Drawing)
		return false;
	if(!drawCanPlay_)
		return false;

	for(auto& card : drawnCards())
	{
		if(canPlay(card, getHand(currentPlayer()), lastCard_))
			return true;
	}
	return false;
}
bool Cards::Uno::Game::canPlayHandCard() const
{
	if(state_ != Cards::Uno::State::PlayOrDraw && !(state_ == Cards::Uno::State::Drawing && drawCanPlay_))
		return false;

	auto hand = getHand(currentPlayer());
	for(auto& card : hand)
	{
		if(canPlay(card, hand, lastCard_))
			return true;
	}
	return false;
}
void Cards::Uno::Game::readyDraw(unsigned int qty, bool canPlay)
{
	drawQty_ = qty;
	drawCanPlay_ = canPlay;
	state_ = State::Drawing;

	if(deck_.size() >= qty)
		return;
	
	if(played_.size() <= 1)
		return;

	auto begin = played_.begin();
	auto end = played_.end() - 1; // Keep last card
	std::shuffle(begin, end, randomEngine_);

	deck_.insert(deck_.begin(), begin, end);
	played_.erase(begin, end);
}
void Cards::Uno::Game::checkForEnd()
{
	switch(state_)
	{
		case State::Drawing: // Can finish while still drawing
		case State::Finished:
			return;
		case State::Play:
		case State::PlayOrDraw:
			break;
	}
	for(unsigned int i = 0; i < hands_.size(); ++i)
	{
		if(hands_[i].empty())
		{
			state_ = State::Finished;
			return;
		}
	}
}
Cards::Uno::Session::ScoreCase::ScoreCase(unsigned int points, bool won) :
	points_{points},
	won_{won}
{}
Cards::Uno::Session::Session(std::mt19937_64 randomEngine, unsigned short nbPlayers) :
	startingPlayer_{nbPlayers, 0},
	game_{std::move(randomEngine), startingPlayer_}
{
	startingPlayer_ += 1;
}
void Cards::Uno::Session::play(unsigned short player, const Card& card, const std::optional<Color>& color)
{
	game_.play(player, card, color);

	updateScoreIfNeeded();
}
void Cards::Uno::Session::draw(unsigned short player)
{
	game_.draw(player);
}
void Cards::Uno::Session::drawDone(unsigned short player, bool wantToPlay)
{
	game_.drawDone(player, wantToPlay);
	updateScoreIfNeeded();
}
void Cards::Uno::Session::nextGame()
{
	game_.restart(startingPlayer_);
	startingPlayer_ += 1;
}
unsigned int Cards::Uno::Session::nbGames() const
{
	return score_.size();
}
unsigned short Cards::Uno::Session::nbPlayers() const
{
	return startingPlayer_.count();
}
const Cards::Uno::Session::ScoreCase& Cards::Uno::Session::scoreFor(unsigned int game, unsigned short player) const
{
	return score_.at(game).at(player);
}
const Cards::Uno::State& Cards::Uno::Session::state() const
{
	return game_.state();
}
unsigned short Cards::Uno::Session::currentPlayer() const
{
	return game_.currentPlayer();
}
Cards::Uno::Order Cards::Uno::Session::currentOrder() const
{
	return game_.currentOrder();
}
const Cards::Uno::Hand& Cards::Uno::Session::getHand(unsigned short player) const
{
	return game_.getHand(player);
}
std::vector<Cards::Uno::Card> Cards::Uno::Session::drawnCards() const
{
	return game_.drawnCards();
}
const Cards::Uno::PlayedCard& Cards::Uno::Session::currentCard() const
{
	return game_.currentCard();
}
bool Cards::Uno::Session::canPlayDrawnCard() const
{
	return game_.canPlayDrawnCard();
}
bool Cards::Uno::Session::canPlayHandCard() const
{
	return game_.canPlayHandCard();
}
void Cards::Uno::Session::updateScoreIfNeeded()
{
	if(game_.state() != State::Finished)
		return;

	RoundScore score;
	for(unsigned int i = 0; i < nbPlayers(); ++i)
	{
		auto& hand = getHand(i);
		score.emplace_back(ScoreCase{calculatePoints(hand), hand.empty()});
	}
	score_.emplace_back(std::move(score));
}
std::vector<Cards::Uno::Card> Cards::Uno::createFullDeck()
{
	const static auto deck = []()->std::vector<Card> {
		const auto list_basic_double = {
			BasicValue::One,
			BasicValue::Two,
			BasicValue::Three,
			BasicValue::Four,
			BasicValue::Five,
			BasicValue::Six,
			BasicValue::Seven,
			BasicValue::Eight,
			BasicValue::Nine,
			BasicValue::SkipTurn,
			BasicValue::Plus2,
			BasicValue::Reverse,
		};
		const auto list_color = {
			Color::Blue,
			Color::Red,
			Color::Yellow,
			Color::Green,
		};

		std::vector<Card> result;
		for(auto color : list_color)
		{
			result.emplace_back(Card{color, BasicValue::Zero});
			for(auto& val : list_basic_double)
			{
				// Present twice
				result.emplace_back(Card{color, val});
				result.emplace_back(Card{color, val});
			}
		}

		for(int i = 0; i < 4; ++i)
		{
			result.emplace_back(Card{ColorlessValue::ChangeColor});
			result.emplace_back(Card{ColorlessValue::Plus4});
		}
		return result;
	}();
	return deck;
}
bool Cards::Uno::canPlay(const Card& card, const Hand& hand, const PlayedCard& on)
{
	if(card.basic())
	{
		if(card.basic()->color() == on.color())
			return true;
		if(on.basic() && card.basic()->value() == on.basic().value())
			return true;
	}
	else
	{
		switch(card.colorless().value())
		{
			case ColorlessValue::ChangeColor:
				return true;
			case ColorlessValue::Plus4:
			{
				if(on.colorless() && on.colorless().value() == ColorlessValue::Plus4)
					return true;
				
				for(auto& c : hand)
				{
					if(c == card)
						continue;
					
					if(c.basic() && c.basic()->color() == on.color())
					{
						return false;
					}
				}
				return true;
			}

		}
	}
	return false;
}
void Cards::Uno::organize(Hand& hand)
{
	std::sort(hand.begin(), hand.end(), [](const Card& c1, const Card& c2) -> bool {
		if(c1.basic() && c2.basic())
		{
			if(c1.basic()->color() < c2.basic()->color())
				return true;
			if(c1.basic()->color() > c2.basic()->color())
				return false;

			return c1.basic()->value() < c2.basic()->value();
		}
		else if(c1.basic())
			return true;
		else if(c2.basic())
			return false;
		else
		{
			return c1.colorless().value() < c2.colorless().value();
		}
	});
}
unsigned int Cards::Uno::calculatePoints(const Hand& hand)
{
	unsigned int total = 0;

	for(auto& card : hand)
	{
		if(card.colorless())
			total += 50;
		else
		{
			switch(card.basic()->value())
			{
				case BasicValue::Zero:
				case BasicValue::One:
				case BasicValue::Two:
				case BasicValue::Three:
				case BasicValue::Four:
				case BasicValue::Five:
				case BasicValue::Six:
				case BasicValue::Seven:
				case BasicValue::Eight:
				case BasicValue::Nine:
					total += static_cast<unsigned int>(card.basic()->value());
					break;
				case BasicValue::SkipTurn:
				case BasicValue::Plus2:
				case BasicValue::Reverse:
					total += 20;
					break;
			}
		}
	}

	return total;
}
unsigned int Cards::Uno::cardsForNbPlayers(unsigned short players)
{
	if(players <= 8)
		return 7;
	if(players > 15)
		throw std::runtime_error{"TODO Not enough cards for the number of players"};
	return 6;
}