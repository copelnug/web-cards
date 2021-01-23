#ifndef CARDS_UNO_HPP_INCLUDED
#define CARDS_UNO_HPP_INCLUDED
#include <optional>
#include <random>
#include <vector>

namespace Cards
{
	namespace Uno
	{
		enum class Color
		{
			Blue,
			Red,
			Yellow,
			Green,
		};
		enum class BasicValue
		{
			Zero = 0,
			One,
			Two,
			Three,
			Four,
			Five,
			Six,
			Seven,
			Eight,
			Nine,
			SkipTurn,
			Plus2,
			Reverse,
		};
		enum class ColorlessValue
		{
			ChangeColor,
			Plus4,
		};

		class BasicCard
		{
			public:
				explicit BasicCard(Color color, BasicValue value);

				const Color& color() const { return color_; }
				const BasicValue& value() const { return value_; }
			private:
				Color color_;
				BasicValue value_;
		};
		bool operator==(const BasicCard& c1, const BasicCard& c2);
		bool operator!=(const BasicCard& c1, const BasicCard& c2);
		class Card
		{
			public:
				explicit Card(Color color, BasicValue value);
				explicit Card(ColorlessValue value);

				const std::optional<BasicCard>& basic() const { return basic_; }
				const std::optional<ColorlessValue>& colorless() const { return colorless_; }
			private:
				std::optional<BasicCard> basic_;
				std::optional<ColorlessValue> colorless_;
		};
		bool operator==(const Card& c1, const Card& c2);
		bool operator!=(const Card& c1, const Card& c2);

		class PlayedCard
		{
			public:
				explicit PlayedCard(Color color, BasicValue value);
				explicit PlayedCard(Color color, ColorlessValue value);

				const std::optional<BasicValue>& basic() const { return basic_; }
				const std::optional<ColorlessValue>& colorless() const { return colorless_; }
				const Color& color() const { return color_; }

			private:
				std::optional<BasicValue> basic_;
				std::optional<ColorlessValue> colorless_;
				Color color_;
		};

		using Hand = std::vector<Card>;

		enum class State
		{
			Finished,
			PlayOrDraw,
			Play,
			Drawing,
		};
		State operator&(State s1, State s2);
		enum class Order
		{
			Normal,
			Reversed,
		};

		class PlayerIter
		{
			public:
				PlayerIter(unsigned short nbPlayers, unsigned short starting);

				operator const unsigned short&() const { return current_; }
				unsigned short count() const { return nb_; }
				Order order() const { return order_; }

				PlayerIter& operator+=(unsigned short advance);
				PlayerIter operator+(unsigned short advance) const;

				void reverse();
				void reset(unsigned short current);
			private:
				unsigned short current_;
				unsigned short nb_;
				Order order_;
		};
		class Game
		{
			public:
				Game(std::mt19937_64 randomEngine, PlayerIter startingPlayer);

				// Action
				void play(unsigned short player, const Card& card, const std::optional<Color>& color);
				void draw(unsigned short player);
				void drawDone(unsigned short player, bool wantToPlay);
				void restart(unsigned short startingPlayer);

				// Infos
				const State& state() const;
				unsigned short currentPlayer() const;
				Order currentOrder() const;
				const Hand& getHand(unsigned short player) const;
				std::vector<Card> drawnCards() const;
				const PlayedCard& currentCard() const;
				
				bool canPlayDrawnCard() const;
				bool canPlayHandCard() const;

			private:
				void readyDraw(unsigned int qty, bool canPlay);
				void checkForEnd();

				std::mt19937_64 randomEngine_;
				std::vector<Card> deck_;
				std::vector<Card> played_;
				std::vector<Hand> hands_;
				PlayedCard lastCard_;
				PlayerIter current_;
				State state_;
				unsigned char drawQty_;
				bool drawCanPlay_;
		};
		class Session
		{
			public:
				class ScoreCase
				{
					public:
						ScoreCase(unsigned int points, bool won);

						unsigned int points() const { return points_; }
						bool won() const { return won_; }
					private:
						unsigned int points_;
						bool won_;
				};
				using RoundScore = std::vector<ScoreCase>;

				Session(std::mt19937_64 randomEngine, unsigned short nbPlayers);

				// Action
				void play(unsigned short player, const Card& card, const std::optional<Color>& color);
				void draw(unsigned short player);
				void drawDone(unsigned short player, bool wantToPlay);
				void nextGame();

				// Infos
				unsigned int nbGames() const;
				unsigned short nbPlayers() const;
				const ScoreCase& scoreFor(unsigned int game, unsigned short player) const;
				const State& state() const;
				unsigned short currentPlayer() const;
				Order currentOrder() const;
				const Hand& getHand(unsigned short player) const;
				std::vector<Card> drawnCards() const;
				const PlayedCard& currentCard() const;

				bool canPlayDrawnCard() const;
				bool canPlayHandCard() const;

			private:
				void updateScoreIfNeeded();

				std::vector<RoundScore> score_;
				PlayerIter startingPlayer_;
				Game game_;
		};

		// Utils
		std::vector<Card> createFullDeck();
		bool canPlay(const Card& card, const Hand& hand, const PlayedCard& on);
		void organize(Hand& hand);
		unsigned int calculatePoints(const Hand& hand);
		unsigned int cardsForNbPlayers(unsigned short players);
	}
}

#endif