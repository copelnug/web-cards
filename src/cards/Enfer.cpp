#include "Enfer.hpp"
#include "Exception.hpp"
#include "StandardCards.hpp"
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace
{
	Cards::Enfer::Hand createShuffledDeck(std::mt19937_64& random)
	{
		auto deck = Cards::Standard::createFullDeck();
		std::shuffle(deck.begin(), deck.end(), random);
		return deck;
	}
}

Cards::Enfer::PlayerIter::PlayerIter(unsigned short nbPlayers, unsigned short starting) :
	nbPlayers_{nbPlayers},
	current_{starting}
{
	*this += 0; // Ensure we're not past the end.
}
Cards::Enfer::PlayerIter& Cards::Enfer::PlayerIter::operator+=(unsigned short advance)
{
	current_ += advance;
	current_ = current_ % nbPlayers_;
	return *this;
}
Cards::Enfer::PlayerIter Cards::Enfer::PlayerIter::operator+(unsigned short advance) const
{
	PlayerIter copy{*this};
	copy += advance;
	return copy;
}
Cards::Enfer::Round::PlayerStatus::PlayerStatus() :
	target{},
	obtained{0}
{}
Cards::Enfer::Round::PlayerStatus::PlayerStatus(unsigned short target, unsigned short obtained) :
	target{target},
	obtained{obtained}
{}
Cards::Enfer::Round::Round(Hand shuffledDeck, unsigned short playersCount, unsigned short startingPlayer, unsigned short roundNumber) :
	handStartingPlayer_{startingPlayer},
	currentPlayer_{playersCount, startingPlayer}
{
	if(shuffledDeck.size() != Standard::FullDeckSize)
		throw std::logic_error("Trying to create a round with an invalid deck");

	auto nbCards = numberOfCardsForRound(playersCount, roundNumber);
	for(unsigned short i = 0; i < playersCount; ++i)
	{
		Hand hand{shuffledDeck.end() - nbCards, shuffledDeck.end()};
		shuffledDeck.erase(shuffledDeck.end() - nbCards, shuffledDeck.end());
		Standard::organizeByKindAceTop(hand);
		hands_.emplace_back(std::move(hand));
		status_.emplace_back();
	}
	
	if(nbCards == roundNumber && !shuffledDeck.empty())
	{
		strong_ = shuffledDeck.back();
		shuffledDeck.pop_back();
	}
}
Cards::Enfer::Round::Round(std::vector<Hand> hands, std::vector<PlayerStatus> status, std::optional<Card> strong, Hand played, unsigned short handStartingPlayer) :
	hands_{std::move(hands)},
	status_{std::move(status)},
	strong_{std::move(strong)},
	currentHand_{std::move(played)},
	handStartingPlayer_{handStartingPlayer},
	currentPlayer_{static_cast<unsigned short>(hands_.size()), handStartingPlayer_}
{
	handStartingPlayer_ += played.size();
}
Cards::Enfer::State Cards::Enfer::Round::state() const
{
	for(auto& status : status_)
		if(!status.target)
			return State::SetTarget;
	for(auto& hand : hands_)
		if(!hand.empty())
			return State::Play;
	return State::Finished;
}
unsigned short Cards::Enfer::Round::handStartingPlayer() const
{
	return handStartingPlayer_;
}
unsigned short Cards::Enfer::Round::currentPlayer() const
{
	return currentPlayer_;
}
unsigned short Cards::Enfer::Round::nbPlayers() const
{
	return hands_.size();
}
const std::vector<Cards::Enfer::Hand>& Cards::Enfer::Round::hands() const
{
	return hands_;
}
const std::vector<Cards::Enfer::Round::PlayerStatus>& Cards::Enfer::Round::status() const
{
	return status_;
}
const std::optional<Cards::Enfer::Card>& Cards::Enfer::Round::strong() const
{
	return strong_;
}
const Cards::Enfer::Hand& Cards::Enfer::Round::played() const
{
	return currentHand_;
}
void Cards::Enfer::Round::play(unsigned short player, Card card)
{
	switch(state())
	{
		case State::Play:
			break;
		case State::SetTarget:
		case State::GotoNext:
		case State::Finished:
			throw ActionOutOfStep{player};
	}
	if(player != currentPlayer_)
		throw NotPlayerTurn{player};
	if(player >= nbPlayers())
		throw std::logic_error("Player not in game tried to play.");

	auto& hand = hands_[player];

	auto it = std::find(hand.begin(), hand.end(), card);
	if(it == hand.end())
		throw IllegalChoice{player};

	// If we're not the first card (hand full or empty), check if we can play it
	if(currentHand_.size() != nbPlayers() && !currentHand_.empty())
	{
		if(!canPlay(hand, card, currentHand_.front()))
			throw IllegalChoice{player};
	}

	// Clear the previous hand if needed. We want to remember it to show to players until the new one start.
	if(currentHand_.size() == nbPlayers())
	{
		currentHand_.clear();
		handStartingPlayer_ = player;
	}

	// Move card to represent playing it
	hand.erase(it);
	currentHand_.emplace_back(std::move(card));

	// If we finished the current hand: Update state
	if(currentHand_.size() == nbPlayers())
	{
		// Find winner
		unsigned short winner = 0;
		for(unsigned short i = 1; i < currentHand_.size(); ++i)
		{
			if(stronger(currentHand_[i], currentHand_[winner], strong_))
			{
				winner = i;
			}
		}

		// Update everything
		auto indexWinner = PlayerIter(nbPlayers(), handStartingPlayer_) + winner;
		status_[indexWinner].obtained += 1;
		currentPlayer_ = indexWinner;
	}
	else
	{
		currentPlayer_ += 1;
	}
}
void Cards::Enfer::Round::setTarget(unsigned short player, unsigned short target)
{
	switch(state())
	{
		case State::SetTarget:
			break;
		case State::Play:
		case State::GotoNext:
		case State::Finished:
			throw ActionOutOfStep{player};
	}
	if(player != currentPlayer_)
		throw NotPlayerTurn{player};
	if(player >= nbPlayers())
		throw std::logic_error{"Player not in game tried to play."};

	if(target > hands_[player].size())
		throw IllegalChoice{player};
	status_[player].target = target;
	currentPlayer_ += 1;
}
Cards::Enfer::Game::ScoreCase::ScoreCase() :
	target{0},
	points{0},
	succeed{false}
{}
Cards::Enfer::Game::ScoreCase::ScoreCase(unsigned short target, unsigned short points, bool succeed) :
	target{target},
	points{points},
	succeed{succeed}
{}
Cards::Enfer::Game::Game(unsigned short numberOfPlayers, std::seed_seq& randomSeed) :
	randomEngine_{randomSeed},
	numberOfPlayers_{numberOfPlayers},
	currentRoundNumber_{1},
	currentRound_{createShuffledDeck(randomEngine_), numberOfPlayers_, 0, currentRoundNumber_}
{}
Cards::Enfer::Game::Game(unsigned short numberOfPlayers, std::vector<RoundScore> scores, Round currentRound, unsigned short currentRoundNumber, std::seed_seq& randomSeed) :
	randomEngine_{randomSeed},
	scores_{std::move(scores)},
	numberOfPlayers_{numberOfPlayers},
	currentRoundNumber_{currentRoundNumber},
	currentRound_{std::move(currentRound)}
{}
unsigned short Cards::Enfer::Game::roundNbCards() const
{
	return numberOfCardsForRound(numberOfPlayers_, currentRoundNumber_);
}
unsigned short Cards::Enfer::Game::playerStartingForRound(unsigned short roundNumber) const
{
	PlayerIter it{numberOfPlayers_, 0}; // Player 0 start
	it += (roundNumber - 1); // Minus 1 because the round start at 1
	return it;
}
const Cards::Enfer::Game::ScoreCase& Cards::Enfer::Game::scoreFor(unsigned short player, unsigned short roundNumber) const
{
	if(roundNumber > scores_.size() || roundNumber == 0)
		throw std::logic_error("Invalid round number for Cards::Enfer::Game::scoreFor");
	if(player >= numberOfPlayers_)
		throw std::logic_error("Invalid player for Cards::Enfer::Game::scoreFor");

	return scores_[roundNumber-1][player];
}
unsigned short Cards::Enfer::Game::scoredRound() const
{
	return scores_.size();
}
const Cards::Enfer::Round::PlayerStatus& Cards::Enfer::Game::roundState(unsigned short player) const
{
	if(player >= numberOfPlayers_)
		throw std::logic_error("Invalid player for Cards::Enfer::Game::roundState");
	
	return currentRound_.status()[player];
}
Cards::Enfer::State Cards::Enfer::Game::state() const
{
	auto s = currentRound_.state();
	switch(s)
	{
		case State::SetTarget:
		case State::Play:
			break;
		case State::GotoNext:
		case State::Finished:
		{
			auto maxRound = numberOfRounds(numberOfPlayers_);
			if(currentRoundNumber_ >= maxRound)
				return State::Finished;
			return State::GotoNext;
		}
	}
	return s;
}
unsigned short Cards::Enfer::Game::handStartingPlayer() const
{
	return currentRound_.handStartingPlayer();
}
const Cards::Enfer::Hand& Cards::Enfer::Game::currentHand() const
{
	return currentRound_.played();
}
const std::optional<Cards::Enfer::Card> Cards::Enfer::Game::strong() const
{
	return currentRound_.strong();
}
const Cards::Enfer::Hand& Cards::Enfer::Game::playerHand(unsigned short player) const
{
	if(player >= numberOfPlayers_)
		throw std::logic_error("Invalid player for Cards::Enfer::Game::playerHand");
	
	return currentRound_.hands()[player];
}
unsigned short Cards::Enfer::Game::currentPlayer() const
{
	return currentRound_.currentPlayer();
}
void Cards::Enfer::Game::play(unsigned short player, Card card)
{
	if(state() == State::Finished)
		throw GameFinished{player};
	
	currentRound_.play(player, std::move(card));

	if(currentRound_.state() != State::Finished)
		return;
	
	// TODO Should we be exception safe for the end of this function? Exceptions should not happen there, but...
	RoundScore roundScore;
	for(unsigned short i = 0; i < numberOfPlayers_; ++i)
	{
		auto& state = roundState(i);

		ScoreCase score;
		score.succeed = (state.obtained == state.target);
		score.target = state.target.value();
		score.points = ( score.succeed ? 10 + score.target : 0 );

		if(currentRoundNumber_ > 1)
		{
			score.points += scoreFor(i, currentRoundNumber_ - 1).points;
		}
		roundScore.emplace_back(std::move(score));
	}
	scores_.emplace_back(std::move(roundScore));
}
void Cards::Enfer::Game::setTarget(unsigned short player, unsigned short target)
{
	if(state() == State::Finished)
		throw GameFinished{player};

	currentRound_.setTarget(player, target);
}
void Cards::Enfer::Game::gotoNextRound()
{
	switch(state())
	{
		case State::GotoNext:
			break;
		case State::SetTarget:
		case State::Play:
			throw ActionOutOfStep{};
		case State::Finished:
			throw GameFinished{};
	}

	auto maxRound = numberOfRounds(numberOfPlayers_);
	if(currentRoundNumber_ >= maxRound)
		return; // Game done

	++currentRoundNumber_;
	currentRound_ = Round{createShuffledDeck(randomEngine_), numberOfPlayers_, playerStartingForRound(currentRoundNumber_), currentRoundNumber_};
}
bool Cards::Enfer::canPlay(const Hand& hand, const Card& choice, const Card& firstCard)
{
	auto it = std::find(hand.begin(), hand.end(), choice);
	if(it == hand.end())
		return false; // Don't have the card. Can't play it.

	if(choice.kind == firstCard.kind)
		return true;

	for(const Card& c : hand)
	{
		if(firstCard.kind == c.kind)
			return false; // You must play a card of the same kind if possible.
	}
	return true; // You don't have any card of the requested kind.
}
bool Cards::Enfer::stronger(const Card& newcard, const Card& toBeat, const std::optional<Card>& stronger)
{
	if(newcard.kind != toBeat.kind)
	{
		// Is the newcard of the stronger kind ?
		if(stronger && stronger->kind == newcard.kind)
			return true;
		
		// Can't beat it otherwise
		return false;
	}
	return compareAceTop(newcard.value, toBeat.value) > 0;
}
unsigned short Cards::Enfer::numberOfRounds(unsigned short nbPlayers)
{
	// We need to remove 1 to keep one card for the stronger
	unsigned int roundWithStronger = (Standard::FullDeckSize - 1) / nbPlayers;
	// But we play one more round without a stronger kind
	return roundWithStronger + 1;
}
/**
	\brief Number of cards for the specified round.

	Throw exception if roundnumber > numberOfRounds(nbPlayers) or equal to 0
*/
unsigned short Cards::Enfer::numberOfCardsForRound(unsigned short nbPlayers, unsigned short roundNumber)
{
	if(roundNumber == 0)
		throw std::logic_error{"Cards::Enfer::numberOfCardsForRound round number of 0"};
	if(roundNumber > numberOfRounds(nbPlayers))
		throw std::logic_error{"Cards::Enfer::numberOfCardsForRound round number too high"};// TODO Use std::format or fmtlib to add the params

	if(roundNumber * nbPlayers > Standard::FullDeckSize)
		return Standard::FullDeckSize / nbPlayers;
	return roundNumber;
}
std::string Cards::Enfer::roundTitle(unsigned short nbPlayers, unsigned roundNumber)
{
	auto nbCards = numberOfCardsForRound(nbPlayers, roundNumber);

	std::ostringstream out;
	out << nbCards;
	if(roundNumber * nbPlayers >= Standard::FullDeckSize)
		out << '*';
	return std::move(out).str();
}