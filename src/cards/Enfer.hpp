#ifndef CARDS_ENFER_HPP_INCLUDED
#define CARDS_ENFER_HPP_INCLUDED
#include <optional>
#include <random>
#include <vector>

#include "Exception.hpp"
#include "StandardCards.hpp"

namespace Cards
{
	namespace Enfer
	{
		using Card = Cards::Standard::Card;
		using Hand = std::vector<Card>;

		enum class State
		{
			SetTarget,
			Play,
			GotoNext,
			Finished,
		};

		class PlayerIter
		{
			unsigned short nbPlayers_;
			unsigned short current_;
		public:
			PlayerIter(unsigned short nbPlayers, unsigned short starting);

			operator const unsigned short&() const { return current_; }

			PlayerIter& operator+=(unsigned short advance);
			PlayerIter operator+(unsigned short advance) const;
		};

    	class Round
		{
		public:
			struct PlayerStatus
			{
				PlayerStatus();
				PlayerStatus(unsigned short target, unsigned short obtained = 0);

				std::optional<unsigned short> target;
				unsigned short obtained;
			};
		private:
			std::vector<Hand> hands_;
            std::vector<PlayerStatus> status_;
            std::optional<Card> strong_;
			Hand currentHand_;
			unsigned short handStartingPlayer_;
			PlayerIter currentPlayer_;
		public:
			Round(Hand shuffledDeck, unsigned short playersCount, unsigned short startingPlayer, unsigned short roundNumber);
			// For tests and maybe restore
			Round(std::vector<Hand> hands, std::vector<PlayerStatus> status, std::optional<Card> strong, Hand played, unsigned short handStartingPlayer);
			
			State state() const;
			unsigned short handStartingPlayer() const;
			unsigned short currentPlayer() const;
			unsigned short nbPlayers() const;

			const std::vector<Hand>& hands() const;
			const std::vector<PlayerStatus>& status() const;
			const std::optional<Card>& strong() const;
			const Hand& played() const;

			void play(unsigned short player, const Card card);
			void setTarget(unsigned short player, unsigned short target);
		};

		class Game
		{
		public:
			struct ScoreCase
			{
				ScoreCase();
				ScoreCase(unsigned short target, unsigned short points, bool succeed);

				unsigned short target;
				unsigned short points;
				bool succeed;
			};
			using RoundScore = std::vector<ScoreCase>;
		private:
			std::mt19937_64 randomEngine_;
			std::vector<RoundScore> scores_;
			unsigned short numberOfPlayers_;
			unsigned short currentRoundNumber_;
			Round currentRound_;

		public:
			Game(unsigned short numberOfPlayers, std::seed_seq& randomSeed);
			// For test or maybe for load
			Game(unsigned short numberOfPlayers, std::vector<RoundScore> scores, Round currentRound, unsigned short currentRoundNumber, std::seed_seq& randomSeed);

			unsigned short numberOfPlayers() const { return numberOfPlayers_; }
			unsigned short currentRound() const { return currentRoundNumber_; }
			unsigned short roundNbCards() const;
			unsigned short playerStartingForRound(unsigned short roundNumber) const;

			// Game state
			const ScoreCase& scoreFor(unsigned short player, unsigned short roundNumber) const;
			unsigned short scoredRound() const;
			const Round::PlayerStatus& roundState(unsigned short player) const;
			State state() const;

			// Current hand
			unsigned short handStartingPlayer() const;
			const Hand& currentHand() const;
			const std::optional<Card> strong() const;

			// Player info
			const Hand& playerHand(unsigned short player) const;
			unsigned short currentPlayer() const;

			// Action
			void play(unsigned short player, Card card);
			void setTarget(unsigned short player, unsigned short target);
			void gotoNextRound();
		};

		/////////////////////////////////// Utils ///////////////////////////////////
		bool canPlay(const Hand& hand, const Card& choice, const Card& firstCard);
		bool stronger(const Card& newcard, const Card& toBeat, const std::optional<Card>& stronger);
		unsigned short numberOfRounds(unsigned short nbPlayers);
		unsigned short numberOfCardsForRound(unsigned short nbPlayers, unsigned short roundNumber);
		std::string roundTitle(unsigned short nbPlayers, unsigned roundNumber);
	}
}

#endif