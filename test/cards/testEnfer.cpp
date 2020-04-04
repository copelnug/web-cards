#include "Enfer.hpp"
#include <algorithm>
#include <catch.hpp>
#include <random>
#include <sstream>
#include "utilsStandardCards.hpp"

#include "Exception.hpp"
#include "StandardCards.hpp"

namespace
{
	Cards::Enfer::Hand sorted(Cards::Enfer::Hand hand)
	{
		Cards::Standard::organizeByKindAceTop(hand);
		return hand;
	}
}
namespace Cards
{
	namespace Enfer
	{
		bool operator==(const Round::PlayerStatus& p1, const Round::PlayerStatus& p2)
		{
			return std::tie(p1.obtained, p1.target) == std::tie(p2.obtained, p2.target);
		}
		bool operator==(const Game::ScoreCase& s1, const Game::ScoreCase& s2)
		{
			return std::tie(s1.points, s1.succeed, s1.target) == std::tie(s2.points, s2.succeed, s2.target);
		}
	}
}
namespace Catch
{
	template<>
	struct StringMaker<Cards::Enfer::Game::ScoreCase> {
        static std::string convert( Cards::Enfer::Game::ScoreCase const& card ) {
			std::ostringstream str;

			str << '{' << card.target << ", " << card.points << " , " << (card.succeed ? "true" : "false") << '}';
			return str.str();
        }
	};
}
TEST_CASE("Enfer: Check if we can play a card", "[cards][cards_enfer]")
{
	using Cards::Enfer::canPlay;
	using Cards::Enfer::Card;
	using Cards::Standard::Kind;
	using Cards::Standard::Value;

	const Card HeartNine{Kind::Heart, Value::Nine};
	const Card TileTen{Kind::Tile, Value::Ten};
	const Card CloverTwo{Kind::Clover,Value::Two};
	const Card CloverSeven{Kind::Clover,Value::Seven};
	const Card CloverQueen{Kind::Clover,Value::Queen};
	const Card PikeFive{Kind::Pike, Value::Five};

	CHECK(canPlay({CloverTwo}, CloverTwo, CloverQueen));
	CHECK_FALSE(canPlay({CloverTwo}, PikeFive, CloverQueen)); // Can't if you don't have the card.
	CHECK(canPlay({PikeFive}, PikeFive, CloverQueen));
	CHECK_FALSE(canPlay({CloverTwo, PikeFive}, PikeFive, CloverQueen)); // Can't if you have the Kind in your hand.
	CHECK(canPlay({TileTen, CloverTwo, PikeFive}, CloverTwo, CloverQueen));
	CHECK(canPlay({TileTen, CloverSeven, CloverTwo, PikeFive}, CloverTwo, CloverQueen)); // Can choose the any value of the correct kind
	CHECK_FALSE(canPlay({TileTen, CloverSeven, {Kind::Clover,Value::Five}, PikeFive}, PikeFive, CloverQueen)); // No mismatch between the two Five
}
TEST_CASE("Enfer: Check if a card beat the previous", "[cards][cards_enfer]")
{
	using Cards::Enfer::stronger;
	using Cards::Enfer::Card;
	using Cards::Standard::Kind;
	using Cards::Standard::Value;

	// Values
	CHECK_FALSE(stronger({Kind::Clover,Value::Three}, {Kind::Clover,Value::Four}, {}));
	CHECK(stronger({Kind::Clover,Value::Ten}, {Kind::Clover,Value::Four}, {}));
	CHECK(stronger({Kind::Clover,Value::Jack}, {Kind::Clover,Value::Ten}, {}));
	CHECK(stronger({Kind::Clover,Value::Queen}, {Kind::Clover,Value::Jack}, {}));
	CHECK(stronger({Kind::Clover,Value::King}, {Kind::Clover,Value::Queen}, {}));
	CHECK(stronger({Kind::Clover,Value::Ace}, {Kind::Clover,Value::King}, {}));
	CHECK_FALSE(stronger({Kind::Clover,Value::King}, {Kind::Clover,Value::Ace}, {}));
	// Kind
	CHECK_FALSE(stronger({Kind::Pike,Value::Ten}, {Kind::Clover,Value::Four}, {}));

	// With stronger specified
	CHECK(stronger({Kind::Pike,Value::Two}, {Kind::Clover,Value::Ace}, Card{Kind::Pike,Value::Three}));
	CHECK_FALSE(stronger({Kind::Pike,Value::Two}, {Kind::Clover,Value::Ace}, Card{Kind::Heart,Value::Three}));
	CHECK_FALSE(stronger({Kind::Pike,Value::Two}, {Kind::Clover,Value::Ace}, Card{Kind::Clover,Value::Three}));
	CHECK_FALSE(stronger({Kind::Clover,Value::Two}, {Kind::Clover,Value::Ace}, Card{Kind::Clover,Value::Three}));
	CHECK(stronger({Kind::Clover,Value::King}, {Kind::Clover,Value::Queen}, Card{Kind::Clover,Value::Three}));
}
TEST_CASE("Enfer: Compute the number of round", "[cards][cards_enfer]")
{
	using Cards::Enfer::numberOfRounds;

	CHECK(numberOfRounds(3) == 18);
	CHECK(numberOfRounds(4) == 13);
	CHECK(numberOfRounds(5) == 11);
	CHECK(numberOfRounds(6) == 9);
	CHECK(numberOfRounds(7) == 8);
	CHECK(numberOfRounds(8) == 7);
}
TEST_CASE("Enfer: Compute the number of cards for a round", "[cards][cards_enfer]")
{
	using Cards::Enfer::numberOfCardsForRound;

	CHECK_THROWS(numberOfCardsForRound(3,0));
	CHECK(numberOfCardsForRound(3, 1) == 1);
	CHECK(numberOfCardsForRound(3, 13) == 13);
	CHECK(numberOfCardsForRound(3, 17) == 17);
	CHECK(numberOfCardsForRound(3, 18) == 17);
	CHECK_THROWS(numberOfCardsForRound(3,19));

	CHECK_THROWS(numberOfCardsForRound(4,0));
	CHECK(numberOfCardsForRound(4, 1) == 1);
	CHECK(numberOfCardsForRound(4, 5) == 5);
	CHECK(numberOfCardsForRound(4, 12) == 12);
	CHECK(numberOfCardsForRound(4, 13) == 13);
	CHECK_THROWS(numberOfCardsForRound(4,14));

	CHECK_THROWS(numberOfCardsForRound(5,0));
	CHECK(numberOfCardsForRound(5, 1) == 1);
	CHECK(numberOfCardsForRound(5, 5) == 5);
	CHECK(numberOfCardsForRound(5, 10) == 10);
	CHECK(numberOfCardsForRound(5, 11) == 10);
	CHECK_THROWS(numberOfCardsForRound(5,12));

	CHECK_THROWS(numberOfCardsForRound(6,0));
	CHECK(numberOfCardsForRound(6, 1) == 1);
	CHECK(numberOfCardsForRound(6, 5) == 5);
	CHECK(numberOfCardsForRound(6, 8) == 8);
	CHECK(numberOfCardsForRound(6, 9) == 8);
	CHECK_THROWS(numberOfCardsForRound(6,10));

	CHECK_THROWS(numberOfCardsForRound(7,0));
	CHECK(numberOfCardsForRound(7, 1) == 1);
	CHECK(numberOfCardsForRound(7, 5) == 5);
	CHECK(numberOfCardsForRound(7, 7) == 7);
	CHECK(numberOfCardsForRound(7, 8) == 7);
	CHECK_THROWS(numberOfCardsForRound(7,9));

	CHECK_THROWS(numberOfCardsForRound(8,0));
	CHECK(numberOfCardsForRound(8, 1) == 1);
	CHECK(numberOfCardsForRound(8, 5) == 5);
	CHECK(numberOfCardsForRound(8, 6) == 6);
	CHECK(numberOfCardsForRound(8, 7) == 6);
	CHECK_THROWS(numberOfCardsForRound(8,8));
}
TEST_CASE("Enfer: Test a round", "[cards][cards_enfer][cards_enfer_round]")
{
	using Cards::Enfer::Card;
	using Cards::Enfer::Hand;
	using Cards::Enfer::Round;
	using Cards::Enfer::State;
	using Cards::Standard::Kind;
	using Cards::Standard::Value;

	SECTION("Invalid round number")
	{
		CHECK_THROWS(Round{Hand{}, 4, 0, 1});
		CHECK_THROWS(Round{Cards::Standard::createFullDeck(), 4, 0, 0});
		CHECK_THROWS(Round{Cards::Standard::createFullDeck(), 4, 0, 14});
		CHECK_THROWS(Round{Cards::Standard::createFullDeck(), 5, 0, 12});
	}
	SECTION("#1 for 4: Initial player win")
	{
		Hand deck{
			{Kind::Heart, Value::Two},  {Kind::Tile, Value::Two},  {Kind::Clover, Value::Two},  {Kind::Pike, Value::Two},
			{Kind::Heart, Value::Three},{Kind::Tile, Value::Three},{Kind::Clover, Value::Three},{Kind::Pike, Value::Three},
			{Kind::Heart, Value::Four}, {Kind::Tile, Value::Four}, {Kind::Clover, Value::Four}, {Kind::Pike, Value::Four},
										{Kind::Tile, Value::Five}, {Kind::Clover, Value::Five},
			{Kind::Heart, Value::Six},  {Kind::Tile, Value::Six},  {Kind::Clover, Value::Six},
			{Kind::Heart, Value::Seven},{Kind::Tile, Value::Seven},{Kind::Clover, Value::Seven},{Kind::Pike, Value::Seven},
			{Kind::Heart, Value::Eight},{Kind::Tile, Value::Eight},{Kind::Clover, Value::Eight},{Kind::Pike, Value::Eight},
			{Kind::Heart, Value::Nine}, {Kind::Tile, Value::Nine}, {Kind::Clover, Value::Nine}, {Kind::Pike, Value::Nine},
			{Kind::Heart, Value::Ten},                             {Kind::Clover, Value::Ten},  {Kind::Pike, Value::Ten},
			{Kind::Heart, Value::Jack}, {Kind::Tile, Value::Jack}, {Kind::Clover, Value::Jack}, {Kind::Pike, Value::Jack},
			{Kind::Heart, Value::Queen},{Kind::Tile, Value::Queen},{Kind::Clover, Value::Queen},{Kind::Pike, Value::Queen},
			{Kind::Heart, Value::King}, {Kind::Tile, Value::King}, {Kind::Clover, Value::King}, {Kind::Pike, Value::King},
			{Kind::Heart, Value::Ace},  {Kind::Tile, Value::Ace},                               {Kind::Pike, Value::Ace},

			{Kind::Heart, Value::Five},
			{Kind::Tile, Value::Ten},
			{Kind::Pike, Value::Six},
			{Kind::Clover, Value::Ace},
			{Kind::Pike, Value::Five},
		};

		const Round::PlayerStatus NoStatus{};
		const Round::PlayerStatus TargetNone{0};
		Hand hand0{ {Kind::Pike, Value::Five} };
		Hand hand1{ {Kind::Clover, Value::Ace} };
		Hand hand2{ {Kind::Pike, Value::Six} };
		Hand hand3{ {Kind::Tile, Value::Ten} };
		Hand played;

		Round round(deck, 4, 2, 1);

		// Check starting status
		CHECK(round.state() == State::SetTarget);
		CHECK(round.handStartingPlayer() == 2);
		CHECK(round.currentPlayer() == 2); // As specified
		CHECK(round.hands() == std::vector<Hand>{hand0, hand1, hand2, hand3});
		CHECK(round.status() == std::vector<Round::PlayerStatus>{
			NoStatus,NoStatus,NoStatus,NoStatus,
		});
		CHECK(round.strong() == Card{Kind::Heart, Value::Five});
		CHECK(round.played() == played);

		SECTION("Invalid moves")
		{
			CHECK_THROWS_AS(round.play(2, Card{Kind::Clover, Value::Two}), Cards::ActionOutOfStep);
			CHECK_THROWS_AS(round.setTarget(3, 0), Cards::NotPlayerTurn);
			CHECK_THROWS_AS(round.setTarget(2, 2), Cards::IllegalChoice);
		}
		SECTION("Start game")
		{
			// Player 2 choose
			REQUIRE_NOTHROW(round.setTarget(2, 0));
			CHECK(round.state() == State::SetTarget);
			CHECK(round.handStartingPlayer() == 2);
			CHECK(round.currentPlayer() == 3);
			CHECK(round.hands() == std::vector<Hand>{hand0, hand1, hand2, hand3});
			CHECK(round.status() == std::vector<Round::PlayerStatus>{
				NoStatus, NoStatus, TargetNone, NoStatus,
			});
			CHECK(round.played() == played);

			// Player 3 choose
			REQUIRE_NOTHROW(round.setTarget(3, 0));
			CHECK(round.state() == State::SetTarget);
			CHECK(round.handStartingPlayer() == 2);
			CHECK(round.currentPlayer() == 0);
			CHECK(round.hands() == std::vector<Hand>{hand0, hand1, hand2, hand3});
			CHECK(round.status() == std::vector<Round::PlayerStatus>{
				NoStatus, NoStatus, TargetNone, TargetNone,
			});
			CHECK(round.played() == played);

			// Player 0 choose
			REQUIRE_NOTHROW(round.setTarget(0, 0));
			CHECK(round.state() == State::SetTarget);
			CHECK(round.handStartingPlayer() == 2);
			CHECK(round.currentPlayer() == 1);
			CHECK(round.hands() == std::vector<Hand>{hand0, hand1, hand2, hand3});
			CHECK(round.status() == std::vector<Round::PlayerStatus>{
				TargetNone, NoStatus, TargetNone, TargetNone,
			});
			CHECK(round.played() == played);

			// Lets try corrupting the data
			CHECK_THROWS_AS(round.play(1, Card{Kind::Clover, Value::Ace}), Cards::ActionOutOfStep);
			CHECK_THROWS_AS(round.setTarget(2, 0), Cards::NotPlayerTurn);
			CHECK_THROWS_AS(round.setTarget(1, 2), Cards::IllegalChoice);

			// Player 1 choose
			REQUIRE_NOTHROW(round.setTarget(1, 1));
			CHECK(round.state() == State::Play);
			CHECK(round.handStartingPlayer() == 2);
			CHECK(round.currentPlayer() == 2);
			CHECK(round.hands() == std::vector<Hand>{hand0, hand1, hand2, hand3});
			CHECK(round.status() == std::vector<Round::PlayerStatus>{
				TargetNone, Round::PlayerStatus{1}, TargetNone, TargetNone,
			});
			CHECK(round.played() == played);

			// Player 2 play
			REQUIRE_NOTHROW(round.play(2, hand2[0]));
			played.push_back(hand2[0]);
			hand2.erase(hand2.begin());
			CHECK(round.state() == State::Play);
			CHECK(round.handStartingPlayer() == 2);
			CHECK(round.currentPlayer() == 3);
			CHECK(round.hands() == std::vector<Hand>{hand0, hand1, hand2, hand3});
			CHECK(round.status() == std::vector<Round::PlayerStatus>{
				TargetNone, Round::PlayerStatus{1}, TargetNone, TargetNone,
			});
			CHECK(round.played() == played);

			// Player 3 play
			REQUIRE_NOTHROW(round.play(3, hand3[0]));
			played.push_back(hand3[0]);
			hand3.erase(hand3.begin());
			CHECK(round.state() == State::Play);
			CHECK(round.handStartingPlayer() == 2);
			CHECK(round.currentPlayer() == 0);
			CHECK(round.hands() == std::vector<Hand>{hand0, hand1, hand2, hand3});
			CHECK(round.status() == std::vector<Round::PlayerStatus>{
				TargetNone, Round::PlayerStatus{1}, TargetNone, TargetNone,
			});
			CHECK(round.played() == played);

			// Player 0 play
			REQUIRE_NOTHROW(round.play(0, hand0[0]));
			played.push_back(hand0[0]);
			hand0.erase(hand0.begin());
			CHECK(round.state() == State::Play);
			CHECK(round.handStartingPlayer() == 2);
			CHECK(round.currentPlayer() == 1);
			CHECK(round.hands() == std::vector<Hand>{hand0, hand1, hand2, hand3});
			CHECK(round.status() == std::vector<Round::PlayerStatus>{
				TargetNone, Round::PlayerStatus{1}, TargetNone, TargetNone,
			});
			CHECK(round.played() == played);

			// Lets try corrupting the data again
			CHECK_THROWS_AS(round.setTarget(1, 0), Cards::ActionOutOfStep);
			CHECK_THROWS_AS(round.play(2, Card{Kind::Clover, Value::Ace}), Cards::NotPlayerTurn);
			CHECK_THROWS_AS(round.play(1, Card{Kind::Clover, Value::Three}), Cards::IllegalChoice); // Not in hand

			// Player 1 play
			REQUIRE_NOTHROW(round.play(1, hand1[0]));
			played.push_back(hand1[0]);
			hand1.erase(hand1.begin());
			CHECK(round.state() == State::Finished);
			CHECK(round.handStartingPlayer() == 2); // Still the same player
			CHECK(round.hands() == std::vector<Hand>{hand0, hand1, hand2, hand3});
			CHECK(round.status() == std::vector<Round::PlayerStatus>{
				TargetNone, Round::PlayerStatus{1}, Round::PlayerStatus{0,1}, TargetNone,
			});
			CHECK(round.played() == played);
		}
	}
	SECTION("#6 for 5")
	{
		std::vector<Card> deck{
			{Kind::Tile, Value::Two}, {Kind::Tile, Value::Three}, {Kind::Tile, Value::Jack}, {Kind::Tile, Value::Queen}, {Kind::Tile, Value::King},

			{Kind::Pike, Value::Four}, {Kind::Pike, Value::Five}, {Kind::Pike, Value::Six}, {Kind::Pike, Value::Queen}, {Kind::Pike, Value::King},

			{Kind::Clover, Value::Two}, {Kind::Clover, Value::Five}, {Kind::Clover, Value::Six}, {Kind::Clover, Value::Nine},
			{Kind::Clover, Value::Ten}, {Kind::Clover, Value::Jack},

			{Kind::Heart, Value::Seven}, {Kind::Heart, Value::Nine}, {Kind::Heart, Value::Jack}, {Kind::Heart, Value::Queen}, {Kind::Heart, Value::King},

			// Strong
			{Kind::Clover, Value::Seven},
			// Hand #4: 2 => 1
			{Kind::Clover, Value::Ace},   {Kind::Pike, Value::Ace},    {Kind::Pike, Value::Two},
			{Kind::Pike, Value::Seven},   {Kind::Pike, Value::Three},  {Kind::Pike, Value::Nine},
			// Hand #3: 0 => 0
			{Kind::Clover, Value::Four},  {Kind::Heart, Value::Two},   {Kind::Tile, Value::Five},
			{Kind::Pike, Value::Eight},   {Kind::Heart, Value::Four},  {Kind::Tile, Value::Six},
			// Hand #2: 1 => 1
			{Kind::Clover, Value::King},  {Kind::Heart, Value::Six},   {Kind::Tile, Value::Seven},
			{Kind::Tile, Value::Nine},    {Kind::Heart, Value::Eight}, {Kind::Heart, Value::Ten},
			// Hand #1: 2 => 3
			{Kind::Clover, Value::Queen}, {Kind::Heart, Value::Ace},   {Kind::Clover, Value::Three},
			{Kind::Clover, Value::Eight}, {Kind::Tile, Value::Eight},  {Kind::Heart, Value::Five},
			// Hand #0: 1 => 1
			{Kind::Tile, Value::Ace},     {Kind::Heart, Value::Three}, {Kind::Pike, Value::Ten},
			{Kind::Pike, Value::Jack},    {Kind::Tile, Value::Ten},    {Kind::Tile, Value::Four},
		};
		Round round(deck, 5, 3, 6);
		Hand played;

		CHECK(round.strong() == Card{Kind::Clover, Value::Seven});

		REQUIRE_NOTHROW(round.setTarget(3, 0));
		REQUIRE_NOTHROW(round.setTarget(4, 2));
		REQUIRE_NOTHROW(round.setTarget(0, 1));
		REQUIRE_NOTHROW(round.setTarget(1, 2));
		REQUIRE_NOTHROW(round.setTarget(2, 1));
		CHECK(round.status() == std::vector<Round::PlayerStatus>{
			{1}, {2}, {1}, {0}, {2},
		});

		// First hand: Just stronger value
		played = { 
			{Kind::Pike, Value::Eight},  // #3
			{Kind::Pike, Value::Ace},    // #4
			{Kind::Pike, Value::Ten},    // #0
			{Kind::Tile, Value::Eight},  // #1
			{Kind::Clover, Value::King}, // #2
		};
		REQUIRE_NOTHROW(round.play(3, played[0]));
		REQUIRE_NOTHROW(round.play(4, played[1]));
		REQUIRE_NOTHROW(round.play(0, played[2]));
		REQUIRE_NOTHROW(round.play(1, played[3]));
		REQUIRE_NOTHROW(round.play(2, played[4]));
		CHECK(round.state() == State::Play);
		CHECK(round.handStartingPlayer() == 3);
		CHECK(round.currentPlayer() == 2);
		CHECK(round.hands() == std::vector<Hand>{
			sorted({{Kind::Tile, Value::Ace}, {Kind::Heart, Value::Three}, {Kind::Pike, Value::Jack}, {Kind::Tile, Value::Ten}, {Kind::Tile, Value::Four}}),
			sorted({{Kind::Clover, Value::Queen}, {Kind::Heart, Value::Ace}, {Kind::Clover, Value::Three}, {Kind::Clover, Value::Eight}, {Kind::Heart, Value::Five}}),
			sorted({{Kind::Heart, Value::Six}, {Kind::Tile, Value::Seven}, {Kind::Tile, Value::Nine}, {Kind::Heart, Value::Eight}, {Kind::Heart, Value::Ten}}),
			sorted({{Kind::Clover, Value::Four}, {Kind::Heart, Value::Two}, {Kind::Tile, Value::Five}, {Kind::Heart, Value::Four}, {Kind::Tile, Value::Six}}),
			sorted({{Kind::Clover, Value::Ace}, {Kind::Pike, Value::Two}, {Kind::Pike, Value::Seven}, {Kind::Pike, Value::Three}, {Kind::Pike, Value::Nine}}),
		});
		CHECK(round.status() == std::vector<Round::PlayerStatus>{
			{1}, {2}, {1,1}, {0}, {2},
		});
		CHECK(round.played() == played);

		// Second hand: Strong card
		played = {
			{Kind::Heart, Value::Six},   // #2
			{Kind::Heart, Value::Four},  // #3
			{Kind::Pike, Value::Two},    // #4
			{Kind::Heart, Value::Three}, // #0
			{Kind::Heart, Value::Ace},   // #1
		};
		REQUIRE_NOTHROW(round.play(2, played[0]));
		CHECK(round.handStartingPlayer() == 2);   // Was reset
		CHECK(round.played() == Hand{played[0]}); // Was reset
		REQUIRE_NOTHROW(round.play(3, played[1]));
		REQUIRE_NOTHROW(round.play(4, played[2]));
		REQUIRE_NOTHROW(round.play(0, played[3]));
		REQUIRE_NOTHROW(round.play(1, played[4]));
		CHECK(round.state() == State::Play);
		CHECK(round.handStartingPlayer() == 2);
		CHECK(round.currentPlayer() == 1);
		CHECK(round.hands() == std::vector<Hand>{
			sorted({{Kind::Tile, Value::Ace}, {Kind::Pike, Value::Jack}, {Kind::Tile, Value::Ten}, {Kind::Tile, Value::Four}}),
			sorted({{Kind::Clover, Value::Queen}, {Kind::Clover, Value::Three}, {Kind::Clover, Value::Eight}, {Kind::Heart, Value::Five}}),
			sorted({{Kind::Tile, Value::Seven}, {Kind::Tile, Value::Nine}, {Kind::Heart, Value::Eight}, {Kind::Heart, Value::Ten}}),
			sorted({{Kind::Clover, Value::Four}, {Kind::Heart, Value::Two}, {Kind::Tile, Value::Five}, {Kind::Tile, Value::Six}}),
			sorted({{Kind::Clover, Value::Ace}, {Kind::Pike, Value::Seven}, {Kind::Pike, Value::Three}, {Kind::Pike, Value::Nine}}),
		});
		CHECK(round.status() == std::vector<Round::PlayerStatus>{
			{1}, {2,1}, {1,1}, {0}, {2},
		});
		CHECK(round.played() == played);

		// Third hand
		played = {
			{Kind::Clover, Value::Eight}, // #1
			{Kind::Heart, Value::Ten},    // #2
			{Kind::Clover, Value::Four},  // #3
			{Kind::Clover, Value::Ace},   // #4
			{Kind::Tile, Value::Ten},     // #0
		};
		REQUIRE_NOTHROW(round.play(1, played[0]));
		REQUIRE_NOTHROW(round.play(2, played[1]));
		REQUIRE_NOTHROW(round.play(3, played[2]));
		REQUIRE_NOTHROW(round.play(4, played[3]));
		REQUIRE_NOTHROW(round.play(0, played[4]));
		CHECK(round.state() == State::Play);
		CHECK(round.handStartingPlayer() == 1);
		CHECK(round.currentPlayer() == 4);
		CHECK(round.hands() == std::vector<Hand>{
			sorted({{Kind::Tile, Value::Ace}, {Kind::Pike, Value::Jack}, {Kind::Tile, Value::Four}}),
			sorted({{Kind::Clover, Value::Queen}, {Kind::Clover, Value::Three}, {Kind::Heart, Value::Five}}),
			sorted({{Kind::Tile, Value::Seven}, {Kind::Tile, Value::Nine}, {Kind::Heart, Value::Eight}}),
			sorted({{Kind::Heart, Value::Two}, {Kind::Tile, Value::Five}, {Kind::Tile, Value::Six}}),
			sorted({{Kind::Pike, Value::Seven}, {Kind::Pike, Value::Three}, {Kind::Pike, Value::Nine}}),
		});
		CHECK(round.status() == std::vector<Round::PlayerStatus>{
			{1}, {2,1}, {1,1}, {0}, {2,1},
		});
		CHECK(round.played() == played);

		// Fourth hand
		played = {
			{Kind::Pike, Value::Nine},  // #4
			{Kind::Pike, Value::Jack},  // #0
			{Kind::Heart, Value::Five}, // #1
			{Kind::Tile, Value::Nine},  // #2
			{Kind::Tile, Value::Six},   // #3
		};
		REQUIRE_NOTHROW(round.play(4, played[0]));
		REQUIRE_NOTHROW(round.play(0, played[1]));
		REQUIRE_NOTHROW(round.play(1, played[2]));
		REQUIRE_NOTHROW(round.play(2, played[3]));
		REQUIRE_NOTHROW(round.play(3, played[4]));
		CHECK(round.state() == State::Play);
		CHECK(round.handStartingPlayer() == 4);
		CHECK(round.currentPlayer() == 0);
		CHECK(round.hands() == std::vector<Hand>{
			sorted({{Kind::Tile, Value::Ace}, {Kind::Tile, Value::Four}}),
			sorted({{Kind::Clover, Value::Queen}, {Kind::Clover, Value::Three}}),
			sorted({{Kind::Tile, Value::Seven}, {Kind::Heart, Value::Eight}}),
			sorted({{Kind::Heart, Value::Two}, {Kind::Tile, Value::Five}}),
			sorted({{Kind::Pike, Value::Seven}, {Kind::Pike, Value::Three}}),
		});
		CHECK(round.status() == std::vector<Round::PlayerStatus>{
			{1,1}, {2,1}, {1,1}, {0}, {2,1},
		});
		CHECK(round.played() == played);

		// Fifth hand
		played = {
			{Kind::Tile, Value::Four},    // #0
			{Kind::Clover, Value::Queen}, // #1
			{Kind::Tile, Value::Seven},   // #2
			{Kind::Tile, Value::Five},    // #3
			{Kind::Pike, Value::Seven},   // #4
		};
		REQUIRE_NOTHROW(round.play(0, played[0]));
		REQUIRE_NOTHROW(round.play(1, played[1]));
		CHECK_THROWS_AS(round.play(2, {Kind::Heart, Value::Eight}), Cards::IllegalChoice); // Oops, have tile
		REQUIRE_NOTHROW(round.play(2, played[2]));
		REQUIRE_NOTHROW(round.play(3, played[3]));
		REQUIRE_NOTHROW(round.play(4, played[4]));
		CHECK(round.state() == State::Play);
		CHECK(round.handStartingPlayer() == 0);
		CHECK(round.currentPlayer() == 1);
		CHECK(round.hands() == std::vector<Hand>{
			sorted({{Kind::Tile, Value::Ace}}),
			sorted({{Kind::Clover, Value::Three}}),
			sorted({{Kind::Heart, Value::Eight}}),
			sorted({{Kind::Heart, Value::Two}}),
			sorted({{Kind::Pike, Value::Three}}),
		});
		CHECK(round.status() == std::vector<Round::PlayerStatus>{
			{1,1}, {2,2}, {1,1}, {0}, {2,1},
		});
		CHECK(round.played() == played);

		// Sixth hand
		played = {
			{Kind::Clover, Value::Three}, // #1
			{Kind::Heart, Value::Eight},  // #2
			{Kind::Heart, Value::Two},    // #3
			{Kind::Pike, Value::Three},   // #4
			{Kind::Tile, Value::Ace},     // #0
		};
		REQUIRE_NOTHROW(round.play(1, played[0]));
		REQUIRE_NOTHROW(round.play(2, played[1]));
		REQUIRE_NOTHROW(round.play(3, played[2]));
		REQUIRE_NOTHROW(round.play(4, played[3]));
		REQUIRE_NOTHROW(round.play(0, played[4]));
		CHECK(round.state() == State::Finished);
		CHECK(round.handStartingPlayer() == 1);
		CHECK(round.hands() == std::vector<Hand>{
			{}, {}, {}, {}, {}
		});
		CHECK(round.status() == std::vector<Round::PlayerStatus>{
			{1,1}, {2,3}, {1,1}, {0}, {2,1},
		});
		CHECK(round.played() == played);

		CHECK(round.strong() == Card{Kind::Clover, Value::Seven});
	}
	SECTION("#8 for 6: No stronger")
	{
		std::vector<Card> deck{
			{Kind::Pike, Value::Ten}, {Kind::Clover, Value::Six}, {Kind::Pike, Value::Three}, {Kind::Heart, Value::Four},

			{Kind::Clover, Value::Seven}, {Kind::Clover, Value::Nine}, {Kind::Clover, Value::Jack}, {Kind::Pike, Value::Seven},
			{Kind::Clover, Value::Three}, {Kind::Pike, Value::Two}, {Kind::Tile, Value::Ace}, {Kind::Tile, Value::Four},
			
			{Kind::Pike, Value::Ace}, {Kind::Tile, Value::Two}, {Kind::Tile, Value::Ten}, {Kind::Heart, Value::Five},
			{Kind::Clover, Value::Four}, {Kind::Tile, Value::Six}, {Kind::Tile, Value::Three}, {Kind::Heart, Value::Queen},
			
			{Kind::Pike, Value::Five}, {Kind::Tile, Value::King}, {Kind::Clover, Value::Five}, {Kind::Clover, Value::Eight},
			{Kind::Heart, Value::Ten}, {Kind::Clover, Value::Ten}, {Kind::Clover, Value::Ace}, {Kind::Heart, Value::King},
			
			{Kind::Heart, Value::Six}, {Kind::Pike, Value::Four}, {Kind::Heart, Value::Three}, {Kind::Pike, Value::Six},
			{Kind::Clover, Value::Two}, {Kind::Tile, Value::Seven}, {Kind::Heart, Value::Two}, {Kind::Tile, Value::Five},
			
			{Kind::Pike, Value::Jack}, {Kind::Heart, Value::Nine}, {Kind::Clover, Value::Queen}, {Kind::Heart, Value::Seven},
			{Kind::Pike, Value::Queen}, {Kind::Pike, Value::Nine}, {Kind::Heart, Value::Eight}, {Kind::Heart, Value::Ace},
			
			{Kind::Tile, Value::Nine}, {Kind::Heart, Value::Jack}, {Kind::Clover, Value::King}, {Kind::Pike, Value::Eight},
			{Kind::Pike, Value::King}, {Kind::Tile, Value::Jack}, {Kind::Tile, Value::Eight}, {Kind::Tile, Value::Queen},
		};
		
		Round round(deck, 6, 5, 9);

		CHECK_FALSE(round.strong());

		REQUIRE(round.hands() == std::vector<Hand>{
			sorted({
				{Kind::Tile, Value::Nine}, {Kind::Heart, Value::Jack}, {Kind::Clover, Value::King}, {Kind::Pike, Value::Eight},
				{Kind::Pike, Value::King}, {Kind::Tile, Value::Jack}, {Kind::Tile, Value::Eight}, {Kind::Tile, Value::Queen},
			}),
			sorted({
				{Kind::Pike, Value::Jack}, {Kind::Heart, Value::Nine}, {Kind::Clover, Value::Queen}, {Kind::Heart, Value::Seven},
				{Kind::Pike, Value::Queen}, {Kind::Pike, Value::Nine}, {Kind::Heart, Value::Eight}, {Kind::Heart, Value::Ace},
			}),
			sorted({
				{Kind::Heart, Value::Six}, {Kind::Pike, Value::Four}, {Kind::Heart, Value::Three}, {Kind::Pike, Value::Six},
				{Kind::Clover, Value::Two}, {Kind::Tile, Value::Seven}, {Kind::Heart, Value::Two}, {Kind::Tile, Value::Five},
			}),
			sorted({
				{Kind::Pike, Value::Five}, {Kind::Tile, Value::King}, {Kind::Clover, Value::Five}, {Kind::Clover, Value::Eight},
				{Kind::Heart, Value::Ten}, {Kind::Clover, Value::Ten}, {Kind::Clover, Value::Ace}, {Kind::Heart, Value::King},
			}),
			sorted({
				{Kind::Pike, Value::Ace}, {Kind::Tile, Value::Two}, {Kind::Tile, Value::Ten}, {Kind::Heart, Value::Five},
				{Kind::Clover, Value::Four}, {Kind::Tile, Value::Six}, {Kind::Tile, Value::Three}, {Kind::Heart, Value::Queen},
			}),
			sorted({
				{Kind::Clover, Value::Seven}, {Kind::Clover, Value::Nine}, {Kind::Clover, Value::Jack}, {Kind::Pike, Value::Seven},
				{Kind::Clover, Value::Three}, {Kind::Pike, Value::Two}, {Kind::Tile, Value::Ace}, {Kind::Tile, Value::Four},
			}),
		});

		REQUIRE_NOTHROW(round.setTarget(5, 1));
		REQUIRE_NOTHROW(round.setTarget(0, 1));
		REQUIRE_NOTHROW(round.setTarget(1, 1));
		REQUIRE_NOTHROW(round.setTarget(2, 0));
		REQUIRE_NOTHROW(round.setTarget(3, 2));
		REQUIRE_NOTHROW(round.setTarget(4, 1));
		CHECK(round.status() == std::vector<Round::PlayerStatus>{
			{1}, {1}, {0}, {2}, {1}, {1},
		});
		CHECK(round.state() == State::Play);

		// First hand
		REQUIRE_NOTHROW(round.play(5, {Kind::Tile, Value::Ace}));
		REQUIRE_NOTHROW(round.play(0, {Kind::Tile, Value::Queen}));
		REQUIRE_NOTHROW(round.play(1, {Kind::Clover, Value::Queen}));
		REQUIRE_NOTHROW(round.play(2, {Kind::Tile, Value::Seven}));
		REQUIRE_NOTHROW(round.play(3, {Kind::Tile, Value::King}));
		REQUIRE_NOTHROW(round.play(4, {Kind::Tile, Value::Ten}));
		CHECK(round.currentPlayer() == 5);

		// Second hand
		REQUIRE_NOTHROW(round.play(5, {Kind::Pike, Value::Two}));
		REQUIRE_NOTHROW(round.play(0, {Kind::Pike, Value::Eight}));
		REQUIRE_NOTHROW(round.play(1, {Kind::Pike, Value::Queen}));
		REQUIRE_NOTHROW(round.play(2, {Kind::Pike, Value::Six}));
		REQUIRE_NOTHROW(round.play(3, {Kind::Pike, Value::Five}));
		REQUIRE_NOTHROW(round.play(4, {Kind::Pike, Value::Ace}));
		CHECK(round.currentPlayer() == 4);

		// Third hand
		REQUIRE_NOTHROW(round.play(4, {Kind::Heart, Value::Five}));
		REQUIRE_NOTHROW(round.play(5, {Kind::Clover, Value::Jack}));
		REQUIRE_NOTHROW(round.play(0, {Kind::Heart, Value::Jack}));
		REQUIRE_NOTHROW(round.play(1, {Kind::Heart, Value::Ace}));
		REQUIRE_NOTHROW(round.play(2, {Kind::Heart, Value::Six}));
		REQUIRE_NOTHROW(round.play(3, {Kind::Heart, Value::Ten}));
		CHECK(round.currentPlayer() == 1);

		// Fourth hand
		REQUIRE_NOTHROW(round.play(1, {Kind::Heart, Value::Seven}));
		REQUIRE_NOTHROW(round.play(2, {Kind::Heart, Value::Three}));
		REQUIRE_NOTHROW(round.play(3, {Kind::Heart, Value::King}));
		REQUIRE_NOTHROW(round.play(4, {Kind::Heart, Value::Queen}));
		REQUIRE_NOTHROW(round.play(5, {Kind::Clover, Value::Nine}));
		REQUIRE_NOTHROW(round.play(0, {Kind::Tile, Value::Jack}));
		CHECK(round.currentPlayer() == 3);

		// Fifth hand
		REQUIRE_NOTHROW(round.play(3, {Kind::Clover, Value::Ace}));
		REQUIRE_NOTHROW(round.play(4, {Kind::Clover, Value::Four}));
		REQUIRE_NOTHROW(round.play(5, {Kind::Clover, Value::Seven}));
		REQUIRE_NOTHROW(round.play(0, {Kind::Clover, Value::King}));
		REQUIRE_NOTHROW(round.play(1, {Kind::Pike, Value::Jack}));
		REQUIRE_NOTHROW(round.play(2, {Kind::Clover, Value::Two}));
		CHECK(round.currentPlayer() == 3);

		// Sixth hand
		REQUIRE_NOTHROW(round.play(3, {Kind::Clover, Value::Five}));
		REQUIRE_NOTHROW(round.play(4, {Kind::Tile, Value::Six}));
		REQUIRE_NOTHROW(round.play(5, {Kind::Clover, Value::Three}));
		REQUIRE_NOTHROW(round.play(0, {Kind::Tile, Value::Nine}));
		REQUIRE_NOTHROW(round.play(1, {Kind::Pike, Value::Nine}));
		REQUIRE_NOTHROW(round.play(2, {Kind::Tile, Value::Five}));
		CHECK(round.currentPlayer() == 3);

		// Seventh hand
		REQUIRE_NOTHROW(round.play(3, {Kind::Clover, Value::Ten}));
		REQUIRE_NOTHROW(round.play(4, {Kind::Tile, Value::Three}));
		REQUIRE_NOTHROW(round.play(5, {Kind::Pike, Value::Seven}));
		REQUIRE_NOTHROW(round.play(0, {Kind::Tile, Value::Eight}));
		REQUIRE_NOTHROW(round.play(1, {Kind::Heart, Value::Nine}));
		REQUIRE_NOTHROW(round.play(2, {Kind::Pike, Value::Four}));
		CHECK(round.currentPlayer() == 3);
		
		// Eight hand
		REQUIRE_NOTHROW(round.play(3, {Kind::Clover, Value::Eight}));
		REQUIRE_NOTHROW(round.play(4, {Kind::Tile, Value::Two}));
		REQUIRE_NOTHROW(round.play(5, {Kind::Tile, Value::Four}));
		REQUIRE_NOTHROW(round.play(0, {Kind::Pike, Value::King}));
		REQUIRE_NOTHROW(round.play(1, {Kind::Heart, Value::Eight}));
		REQUIRE_NOTHROW(round.play(2, {Kind::Heart, Value::Two}));
		CHECK(round.currentPlayer() == 3);
		CHECK(round.state() == State::Finished);

		CHECK(round.status() == std::vector<Round::PlayerStatus>{
			{1}, {1,1}, {0}, {2,5}, {1,1}, {1,1},
		});
	}
}
TEST_CASE("Enfer: Test a Game", "[cards][cards_enfer][cards_enfer_game]")
{
	std::seed_seq seed{1, 2, 3, 4, 5, 6, 7, 8};

	using Cards::Enfer::Card;
	using Cards::Enfer::Game;
	using Cards::Enfer::Hand;
	using Cards::Enfer::Round;
	using Cards::Enfer::State;
	using Cards::Standard::Kind;
	using Cards::Standard::Value;

	SECTION("4 players")
	{
		Game game{4, seed};

		CHECK(game.numberOfPlayers() == 4);
		CHECK(game.currentRound() == 1);
		CHECK(game.roundNbCards() == 1);
		CHECK(game.roundState(0) == Round::PlayerStatus{});
		CHECK(game.roundState(1) == Round::PlayerStatus{});
		CHECK(game.roundState(2) == Round::PlayerStatus{});
		CHECK(game.roundState(3) == Round::PlayerStatus{});
		CHECK(game.state() == State::SetTarget);
		CHECK(game.handStartingPlayer() == 0);
		CHECK(game.currentHand() == Hand{});

		// Round 1
		CHECK(game.strong() == Card{Kind::Heart, Value::Four});
		CHECK(game.playerHand(0) == Hand{{Kind::Pike, Value::Five}});
		CHECK(game.playerHand(1) == Hand{{Kind::Tile, Value::Four}});
		CHECK(game.playerHand(2) == Hand{{Kind::Clover, Value::Three}});
		REQUIRE(game.playerHand(3) == Hand{{Kind::Heart, Value::Ten}});
		REQUIRE_NOTHROW(game.setTarget(0, 0));
		REQUIRE_NOTHROW(game.setTarget(1, 0));
		REQUIRE_NOTHROW(game.setTarget(2, 0));
		REQUIRE_NOTHROW(game.setTarget(3, 1));
		CHECK(game.roundState(0) == Round::PlayerStatus{0});
		CHECK(game.roundState(1) == Round::PlayerStatus{0});
		CHECK(game.roundState(2) == Round::PlayerStatus{0});
		CHECK(game.roundState(3) == Round::PlayerStatus{1});
		CHECK(game.state() == State::Play);
		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::Five}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Four}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Clover, Value::Three}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Heart, Value::Ten}));
		CHECK(game.state() == State::GotoNext);
		CHECK(game.scoreFor(0, 1) == Game::ScoreCase{0, 10, true});
		CHECK(game.scoreFor(1, 1) == Game::ScoreCase{0, 10, true});
		CHECK(game.scoreFor(2, 1) == Game::ScoreCase{0, 10, true});
		CHECK(game.scoreFor(3, 1) == Game::ScoreCase{1, 11, true});
		REQUIRE_THROWS_AS(game.play(1, {Kind::Pike, Value::Ten}), Cards::ActionOutOfStep);
		REQUIRE_THROWS_AS(game.setTarget(2, 0), Cards::ActionOutOfStep);
		REQUIRE_NOTHROW(game.gotoNextRound());

		// Round 2
		CHECK(game.state() == State::SetTarget);
		CHECK(game.currentRound() == 2);
		CHECK(game.roundNbCards() == 2);
		REQUIRE(game.currentPlayer() == 1);
		CHECK(game.strong() == Card{Kind::Clover, Value::King});
		CHECK(game.playerHand(0) == Hand{{Kind::Tile, Value::Ten}, {Kind::Tile, Value::Jack}});
		CHECK(game.playerHand(1) == Hand{{Kind::Pike, Value::Ten}, {Kind::Pike, Value::Jack}});
		CHECK(game.playerHand(2) == Hand{{Kind::Tile, Value::Two}, {Kind::Pike, Value::Nine}});
		REQUIRE(game.playerHand(3) == Hand{{Kind::Heart, Value::Queen}, {Kind::Clover, Value::Four}});

		REQUIRE_THROWS_AS(game.play(1, {Kind::Pike, Value::Ten}), Cards::ActionOutOfStep);
		REQUIRE_THROWS_AS(game.setTarget(2, 0), Cards::NotPlayerTurn);
		REQUIRE_NOTHROW(game.setTarget(1, 0));
		REQUIRE_NOTHROW(game.setTarget(2, 0));
		REQUIRE_NOTHROW(game.setTarget(3, 1));
		REQUIRE_NOTHROW(game.setTarget(0, 0));
		CHECK(game.state() == State::Play);

		CHECK(game.handStartingPlayer() == 1);
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Ten}));
		REQUIRE_THROWS_AS(game.play(2, {Kind::Tile, Value::Two}), Cards::IllegalChoice); // Must play the same kind if possible.
		REQUIRE_THROWS_AS(game.play(3, {Kind::Heart, Value::Queen}), Cards::NotPlayerTurn);
		REQUIRE_THROWS_AS(game.setTarget(2, 0), Cards::ActionOutOfStep);
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Nine}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Heart, Value::Queen}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::Jack}));
		CHECK(game.roundState(0) == Round::PlayerStatus{0});
		CHECK(game.roundState(1) == Round::PlayerStatus{0, 1});
		CHECK(game.roundState(2) == Round::PlayerStatus{0});
		CHECK(game.roundState(3) == Round::PlayerStatus{1});

		CHECK(game.handStartingPlayer() == 1);
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Jack}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Two}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::Four}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::Ten}));

		CHECK(game.scoreFor(0, 1) == Game::ScoreCase{0, 10, true});
		CHECK(game.scoreFor(1, 1) == Game::ScoreCase{0, 10, true});
		CHECK(game.scoreFor(2, 1) == Game::ScoreCase{0, 10, true});
		CHECK(game.scoreFor(3, 1) == Game::ScoreCase{1, 11, true});
		CHECK(game.scoreFor(0, 2) == Game::ScoreCase{0, 20, true});
		CHECK(game.scoreFor(1, 2) == Game::ScoreCase{0, 10, false});
		CHECK(game.scoreFor(2, 2) == Game::ScoreCase{0, 20, true});
		CHECK(game.scoreFor(3, 2) == Game::ScoreCase{1, 22, true});
		REQUIRE_NOTHROW(game.gotoNextRound());

		// Round 3
		CHECK(game.state() == State::SetTarget);
		CHECK(game.strong() == Card{Kind::Tile, Value::Nine});
		CHECK(game.playerHand(0) == Hand{{Kind::Heart, Value::Jack}, {Kind::Pike, Value::Two}, {Kind::Pike, Value::King}});
		CHECK(game.playerHand(1) == Hand{{Kind::Tile, Value::Three}, {Kind::Tile, Value::Queen}, {Kind::Pike, Value::Three}});
		CHECK(game.playerHand(2) == Hand{{Kind::Heart, Value::Four}, {Kind::Clover, Value::Ten}, {Kind::Pike, Value::Eight}});
		REQUIRE(game.playerHand(3) == Hand{{Kind::Clover, Value::Ace}, {Kind::Tile, Value::Five}, {Kind::Pike, Value::Five}});
		REQUIRE(game.currentPlayer() == 2);

		REQUIRE_NOTHROW(game.setTarget(2, 0));
		REQUIRE_NOTHROW(game.setTarget(3, 1));
		REQUIRE_NOTHROW(game.setTarget(0, 0));
		REQUIRE_NOTHROW(game.setTarget(1, 1));
		CHECK(game.state() == State::Play);

		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Four}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Five}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Jack}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Three}));
		CHECK(game.handStartingPlayer() == 2);

		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Five}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::Two}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Three}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Eight}));
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(2, {Kind::Clover, Value::Ten}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::Ace}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::King}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Queen}));

		CHECK(game.scoreFor(0, 3) == Game::ScoreCase{0, 30, true});
		CHECK(game.scoreFor(1, 3) == Game::ScoreCase{1, 21, true});
		CHECK(game.scoreFor(2, 3) == Game::ScoreCase{0, 20, false});
		CHECK(game.scoreFor(3, 3) == Game::ScoreCase{1, 33, true});
		REQUIRE_NOTHROW(game.gotoNextRound());

		// Round 4
		CHECK(game.state() == State::SetTarget);
		CHECK(game.strong() == Card{Kind::Heart, Value::Queen});
		CHECK(game.playerHand(0) == Hand{{Kind::Heart, Value::Jack}, {Kind::Clover, Value::Eight}, {Kind::Clover, Value::Jack}, {Kind::Pike, Value::Ten}});
		CHECK(game.playerHand(1) == Hand{{Kind::Heart, Value::Four}, {Kind::Heart, Value::Five}, {Kind::Clover, Value::Ace}, {Kind::Pike, Value::Five}});
		CHECK(game.playerHand(2) == Hand{{Kind::Heart, Value::Ten}, {Kind::Clover, Value::Four}, {Kind::Tile, Value::Seven}, {Kind::Pike, Value::Eight}});
		REQUIRE(game.playerHand(3) == Hand{{Kind::Tile, Value::Jack}, {Kind::Pike, Value::Four}, {Kind::Pike, Value::Seven}, {Kind::Pike, Value::Ace}});
		REQUIRE(game.currentPlayer() == 3);

		REQUIRE_NOTHROW(game.setTarget(3, 0));
		REQUIRE_NOTHROW(game.setTarget(0, 1));
		REQUIRE_NOTHROW(game.setTarget(1, 2));
		REQUIRE_NOTHROW(game.setTarget(2, 1));

		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Seven}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::Ten}));  // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Five}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Eight}));
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Jack})); // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Heart, Value::Four}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Ten}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Jack}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Jack}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(2, {Kind::Clover, Value::Four}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Ace}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(1, {Kind::Heart, Value::Five})); // Win
		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Seven}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Four}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Eight}));

		CHECK(game.scoreFor(0, 4) == Game::ScoreCase{1, 30, false});
		CHECK(game.scoreFor(1, 4) == Game::ScoreCase{2, 33, true});
		CHECK(game.scoreFor(2, 4) == Game::ScoreCase{1, 20, false});
		CHECK(game.scoreFor(3, 4) == Game::ScoreCase{0, 43, true});
		REQUIRE_NOTHROW(game.gotoNextRound());

		// Round 5
		CHECK(game.state() == State::SetTarget);
		CHECK(game.strong() == Card{Kind::Clover, Value::Four});
		CHECK(game.playerHand(0) == Hand{
			{Kind::Heart, Value::Three}, {Kind::Heart, Value::Six}, {Kind::Heart, Value::King}, {Kind::Tile, Value::Three}, {Kind::Pike, Value::Five}
		});
		CHECK(game.playerHand(1) == Hand{
			{Kind::Heart, Value::Four}, {Kind::Clover, Value::Jack}, {Kind::Tile, Value::Four}, {Kind::Tile, Value::Ten}, {Kind::Pike, Value::Queen}
		});
		CHECK(game.playerHand(2) == Hand{
			{Kind::Heart, Value::Eight}, {Kind::Pike, Value::Four}, {Kind::Pike, Value::Six}, {Kind::Pike, Value::Eight}, {Kind::Pike, Value::Ten}
		});
		REQUIRE(game.playerHand(3) == Hand{
			{Kind::Clover, Value::Nine}, {Kind::Tile, Value::Eight}, {Kind::Tile, Value::Nine}, {Kind::Tile, Value::Queen}, {Kind::Pike, Value::Nine}
		});
		REQUIRE(game.currentPlayer() == 0);

		REQUIRE_NOTHROW(game.setTarget(0, 0));
		REQUIRE_NOTHROW(game.setTarget(1, 2));
		REQUIRE_NOTHROW(game.setTarget(2, 0));
		REQUIRE_NOTHROW(game.setTarget(3, 1));

		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::Three}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Four}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Eight}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Queen})); // Win
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::Nine}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::King}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Jack})); // Win
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Ten}));
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Queen})); // Win
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Eight}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Nine}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::Five}));
		CHECK(game.handStartingPlayer() == 1);

		REQUIRE_NOTHROW(game.play(1, {Kind::Heart, Value::Four})); // Win
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Six}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Nine}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Three}));
		CHECK(game.handStartingPlayer() == 1);

		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Ten})); // Win
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Four}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Eight}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Six}));

		CHECK(game.scoreFor(0, 5) == Game::ScoreCase{0, 40, true});
		CHECK(game.scoreFor(1, 5) == Game::ScoreCase{2, 33, false});
		CHECK(game.scoreFor(2, 5) == Game::ScoreCase{0, 30, true});
		CHECK(game.scoreFor(3, 5) == Game::ScoreCase{1, 54, true});
		REQUIRE_NOTHROW(game.gotoNextRound());

		// Round 6
		CHECK(game.state() == State::SetTarget);
		CHECK(game.strong() == Card{Kind::Pike, Value::Two});
		CHECK(game.playerHand(0) == Hand{
			{Kind::Heart, Value::Jack}, {Kind::Clover, Value::Six}, {Kind::Tile, Value::Four}, {Kind::Tile, Value::Ten},
			{Kind::Pike, Value::Seven}, {Kind::Pike, Value::Nine}
		});
		CHECK(game.playerHand(1) == Hand{
			{Kind::Clover, Value::Seven}, {Kind::Clover, Value::Nine}, {Kind::Clover, Value::Queen},
			{Kind::Tile, Value::Three}, {Kind::Tile, Value::Five}, {Kind::Tile, Value::Six}
		});
		CHECK(game.playerHand(2) == Hand{
			{Kind::Heart, Value::Eight}, {Kind::Heart, Value::Ace}, {Kind::Clover, Value::Five}, {Kind::Clover, Value::King},
			{Kind::Tile, Value::Jack}, {Kind::Tile, Value::Queen}
		});
		REQUIRE(game.playerHand(3) == Hand{
			{Kind::Clover, Value::Eight}, {Kind::Clover, Value::Ten}, {Kind::Clover, Value::Ace},
			{Kind::Pike, Value::Five}, {Kind::Pike, Value::Ten}, {Kind::Pike, Value::Queen}
		});
		REQUIRE(game.currentPlayer() == 1);

		REQUIRE_NOTHROW(game.setTarget(1, 0));
		REQUIRE_NOTHROW(game.setTarget(2, 1));
		REQUIRE_NOTHROW(game.setTarget(3, 3));
		REQUIRE_NOTHROW(game.setTarget(0, 1));

		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Six}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Queen}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Ten})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::Ten}));
		CHECK(game.handStartingPlayer() == 1);

		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Six}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Queen}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Clover, Value::King}));
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::Eight}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::Nine})); // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Nine}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Clover, Value::Five}));
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::Four}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Three}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Jack})); // Win
		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::Ten}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Eight}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Queen})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Jack}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Seven}));
		CHECK(game.handStartingPlayer() == 2);

		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Five}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::Seven})); // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Five}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Ace}));

		CHECK(game.scoreFor(0, 6) == Game::ScoreCase{1, 40, false});
		CHECK(game.scoreFor(1, 6) == Game::ScoreCase{0, 43, true});
		CHECK(game.scoreFor(2, 6) == Game::ScoreCase{1, 41, true});
		CHECK(game.scoreFor(3, 6) == Game::ScoreCase{3, 67, true});
		REQUIRE_NOTHROW(game.gotoNextRound());

		// Round 7
		CHECK(game.state() == State::SetTarget);
		CHECK(game.strong() == Card{Kind::Tile, Value::Ace});
		CHECK(game.playerHand(0) == Hand{
			{Kind::Heart, Value::Four}, {Kind::Heart, Value::Eight}, {Kind::Heart, Value::Ten}, 
			{Kind::Tile, Value::Six}, {Kind::Tile, Value::Jack}, {Kind::Pike, Value::Six}, {Kind::Pike, Value::Ten}
		});
		CHECK(game.playerHand(1) == Hand{
			{Kind::Heart, Value::Nine}, {Kind::Heart, Value::Jack}, {Kind::Clover, Value::Queen}, {Kind::Clover, Value::King}, 
			{Kind::Tile, Value::Two}, {Kind::Pike, Value::Three}, {Kind::Pike, Value::Seven}
		});
		CHECK(game.playerHand(2) == Hand{
			{Kind::Heart, Value::Three}, {Kind::Heart, Value::Queen}, 
			{Kind::Clover, Value::Four}, {Kind::Clover, Value::Seven}, {Kind::Clover, Value::Eight}, 
			{Kind::Tile, Value::Three}, {Kind::Tile, Value::Queen}
		});
		REQUIRE(game.playerHand(3) == Hand{
			{Kind::Heart, Value::Two}, {Kind::Clover, Value::Five},
			{Kind::Tile, Value::Five}, {Kind::Tile, Value::Eight}, {Kind::Tile, Value::Ten}, {Kind::Pike, Value::Jack}, {Kind::Pike, Value::Ace}
		});
		REQUIRE(game.currentPlayer() == 2);

		REQUIRE_NOTHROW(game.setTarget(2, 1));
		REQUIRE_NOTHROW(game.setTarget(3, 3));
		REQUIRE_NOTHROW(game.setTarget(0, 1));
		REQUIRE_NOTHROW(game.setTarget(1, 1));

		REQUIRE_NOTHROW(game.play(2, {Kind::Clover, Value::Seven}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::Five}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::Jack})); // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::King}));
		CHECK(game.handStartingPlayer() == 2);

		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Eight}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Heart, Value::Jack})); // Win
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Three}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Heart, Value::Two}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Three}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Queen}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::Ten}));
		CHECK(game.handStartingPlayer() == 1);

		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Five}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::Six}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Two}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Queen})); // Win
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(2, {Kind::Clover, Value::Four}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Jack} ));
		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::Six}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Queen})); // Win
		CHECK(game.handStartingPlayer() == 2);

		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Seven}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Clover, Value::Eight}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Ten})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Ten}));
		CHECK(game.handStartingPlayer() == 1);

		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Eight})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Four}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Heart, Value::Nine}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Three}));

		CHECK(game.scoreFor(0, 7) == Game::ScoreCase{1, 51, true});
		CHECK(game.scoreFor(1, 7) == Game::ScoreCase{1, 43, false});
		CHECK(game.scoreFor(2, 7) == Game::ScoreCase{1, 52, true});
		CHECK(game.scoreFor(3, 7) == Game::ScoreCase{3, 80, true});
		REQUIRE_NOTHROW(game.gotoNextRound());

		// Round 8
		CHECK(game.state() == State::SetTarget);
		CHECK(game.strong() == Card{Kind::Heart, Value::Two});
		CHECK(game.playerHand(0) == Hand{
			{Kind::Heart, Value::Eight}, {Kind::Heart, Value::Ten}, {Kind::Clover, Value::Jack}, {Kind::Clover, Value::King},
			{Kind::Tile, Value::Queen}, {Kind::Pike, Value::Five}, {Kind::Pike, Value::Six}, {Kind::Pike, Value::Eight}
		});
		CHECK(game.playerHand(1) == Hand{
			{Kind::Heart, Value::Seven}, {Kind::Clover, Value::Seven}, {Kind::Clover, Value::Ten},
			{Kind::Tile, Value::Three}, {Kind::Tile, Value::Seven}, {Kind::Tile, Value::King}, {Kind::Pike, Value::Three}, {Kind::Pike, Value::Four}
		});
		CHECK(game.playerHand(2) == Hand{
			{Kind::Heart, Value::Four}, {Kind::Heart, Value::Five}, {Kind::Heart, Value::Jack}, {Kind::Clover, Value::Six}, {Kind::Clover, Value::Ace},
			{Kind::Tile, Value::Two}, {Kind::Tile, Value::Eight}, {Kind::Pike, Value::Jack}
		});
		REQUIRE(game.playerHand(3) == Hand{
			{Kind::Heart, Value::Ace}, {Kind::Clover, Value::Eight}, {Kind::Clover, Value::Nine}, {Kind::Clover, Value::Queen},
			{Kind::Tile, Value::Four}, {Kind::Tile, Value::Jack}, {Kind::Tile, Value::Ace}, {Kind::Pike, Value::Ten}
		});
		REQUIRE(game.currentPlayer() == 3);

		REQUIRE_NOTHROW(game.setTarget(3, 2));
		REQUIRE_NOTHROW(game.setTarget(0, 2));
		REQUIRE_NOTHROW(game.setTarget(1, 1));
		REQUIRE_NOTHROW(game.setTarget(2, 3));

		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::Queen}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Seven}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Eight}));
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::Nine}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Jack}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Ten}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Clover, Value::Ace})); // Win
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Jack})); // Win
		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Ten}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::Eight}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Four}));
		CHECK(game.handStartingPlayer() == 2);

		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Two}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Four}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Eight})); // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Three}));
		CHECK(game.handStartingPlayer() == 2);

		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::King})); // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Seven}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Clover, Value::Six}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::Queen}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::Five}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Three}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Five})); // Win
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Jack}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Four}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Heart, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Ten}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Heart, Value::Seven}));
		CHECK(game.handStartingPlayer() == 2);

		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::Eight}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::Six}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::King}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Jack})); // Win

		CHECK(game.scoreFor(0, 8) == Game::ScoreCase{2, 63, true});
		CHECK(game.scoreFor(1, 8) == Game::ScoreCase{1, 43, false});
		CHECK(game.scoreFor(2, 8) == Game::ScoreCase{3, 52, false});
		CHECK(game.scoreFor(3, 8) == Game::ScoreCase{2, 92, true});
		REQUIRE_NOTHROW(game.gotoNextRound());

		// Round 9
		CHECK(game.state() == State::SetTarget);
		CHECK(game.strong() == Card{Kind::Tile, Value::Six});
		CHECK(game.playerHand(0) == Hand{
			{Kind::Heart, Value::Two}, {Kind::Heart, Value::Three}, {Kind::Heart, Value::Jack}, {Kind::Heart, Value::Ace},
			{Kind::Clover, Value::Four}, {Kind::Clover, Value::Seven}, {Kind::Clover, Value::King}, {Kind::Tile, Value::Eight},
			{Kind::Pike, Value::Six}
		});
		CHECK(game.playerHand(1) == Hand{
			{Kind::Heart, Value::Queen}, {Kind::Clover, Value::Three}, {Kind::Clover, Value::Ten}, {Kind::Tile, Value::Five}, {Kind::Tile, Value::Queen},
			{Kind::Pike, Value::Three}, {Kind::Pike, Value::Eight}, {Kind::Pike, Value::Queen}, {Kind::Pike, Value::Ace}
		});
		CHECK(game.playerHand(2) == Hand{
			{Kind::Heart, Value::Eight}, {Kind::Heart, Value::Ten}, {Kind::Tile, Value::Nine}, {Kind::Tile, Value::Ten},
			{Kind::Pike, Value::Two}, {Kind::Pike, Value::Four}, {Kind::Pike, Value::Seven}, {Kind::Pike, Value::Ten}, {Kind::Pike, Value::Jack}
		});
		REQUIRE(game.playerHand(3) == Hand{
			{Kind::Heart, Value::Four}, {Kind::Heart, Value::Five}, {Kind::Clover, Value::Nine},
			{Kind::Tile, Value::Two}, {Kind::Tile, Value::Three}, {Kind::Tile, Value::Four}, {Kind::Tile, Value::Seven}, {Kind::Tile, Value::Jack},
			{Kind::Tile, Value::Ace}
		});
		REQUIRE(game.currentPlayer() == 0);

		REQUIRE_NOTHROW(game.setTarget(0, 2));
		REQUIRE_NOTHROW(game.setTarget(1, 2));
		REQUIRE_NOTHROW(game.setTarget(2, 1));
		REQUIRE_NOTHROW(game.setTarget(3, 4));

		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Heart, Value::Queen}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Ten}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Heart, Value::Five}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::King}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Ten}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Ten})); // Win
		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::Nine}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Four}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Four})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::Six}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Queen}));
		CHECK(game.handStartingPlayer() == 2);

		REQUIRE_NOTHROW(game.play(3, {Kind::Heart, Value::Four}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Jack}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Five})); // Win
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Eight}));
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Three}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Jack}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Seven})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Seven}));
		CHECK(game.handStartingPlayer() == 1);

		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Three}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::Eight}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Queen})); // Win
		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Nine}));
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Eight}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Ten}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Jack})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Four}));
		CHECK(game.handStartingPlayer() == 1);

		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Two})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Three}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Ace}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Two}));
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Two}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Three}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Seven}));

		CHECK(game.scoreFor(0, 9) == Game::ScoreCase{2, 63, false});
		CHECK(game.scoreFor(1, 9) == Game::ScoreCase{2, 55, true});
		CHECK(game.scoreFor(2, 9) == Game::ScoreCase{1, 63, true});
		CHECK(game.scoreFor(3, 9) == Game::ScoreCase{4, 92, false});
		REQUIRE_NOTHROW(game.gotoNextRound());

		// Round 10
		CHECK(game.state() == State::SetTarget);
		CHECK(game.strong() == Card{Kind::Heart, Value::Eight});
		CHECK(game.playerHand(0) == Hand{
			{Kind::Heart, Value::Two}, {Kind::Heart, Value::Seven}, {Kind::Heart, Value::Ace}, {Kind::Clover, Value::Queen}, {Kind::Clover, Value::Ace},
			{Kind::Tile, Value::Five}, {Kind::Tile, Value::Ten}, {Kind::Tile, Value::Ace}, {Kind::Pike, Value::Three}, {Kind::Pike, Value::Nine}
		});
		CHECK(game.playerHand(1) == Hand{
			{Kind::Heart, Value::Six}, {Kind::Heart, Value::Jack},
			{Kind::Clover, Value::Four}, {Kind::Clover, Value::Five}, {Kind::Clover, Value::Six}, {Kind::Clover, Value::Jack},
			{Kind::Tile, Value::Three}, {Kind::Tile, Value::Jack}, {Kind::Pike, Value::Six}, {Kind::Pike, Value::Queen}
		});
		CHECK(game.playerHand(2) == Hand{
			{Kind::Heart, Value::Three}, {Kind::Heart, Value::Nine}, {Kind::Heart, Value::Ten}, {Kind::Heart, Value::King},
			{Kind::Clover, Value::Two}, {Kind::Tile, Value::Six}, {Kind::Tile, Value::Seven}, {Kind::Tile, Value::Nine},
			{Kind::Pike, Value::Eight}, {Kind::Pike, Value::King}
		});
		REQUIRE(game.playerHand(3) == Hand{
			{Kind::Heart, Value::Four}, {Kind::Clover, Value::Three}, {Kind::Clover, Value::Seven}, {Kind::Clover, Value::King},
			{Kind::Tile, Value::Eight}, {Kind::Tile, Value::Queen}, {Kind::Tile, Value::King},
			{Kind::Pike, Value::Two}, {Kind::Pike, Value::Four}, {Kind::Pike, Value::Jack}
		});
		REQUIRE(game.currentPlayer() == 1);

		REQUIRE_NOTHROW(game.setTarget(1, 1));
		REQUIRE_NOTHROW(game.setTarget(2, 3));
		REQUIRE_NOTHROW(game.setTarget(3, 1));
		REQUIRE_NOTHROW(game.setTarget(0, 4));

		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Six}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Clover, Value::Two}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::Seven}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Ace})); // Win
		CHECK(game.handStartingPlayer() == 1);

		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Jack}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Nine}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Queen}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Queen}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Jack}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Nine})); // Win
		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::Three}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::King})); // Win
		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Jack}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::Nine}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Queen}));
		CHECK(game.handStartingPlayer() == 2);

		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Seven}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::King})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::Ten}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Three}));
		CHECK(game.handStartingPlayer() == 2);

		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Four}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::Three}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Six}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Eight})); // Win
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Six}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Eight}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::Five}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Heart, Value::Jack})); // Win
		CHECK(game.handStartingPlayer() == 2);

		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Five}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Three}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::King}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Seven})); // Win
		CHECK(game.handStartingPlayer() == 1);

		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Two}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Heart, Value::Six}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Ten})); // Win
		REQUIRE_NOTHROW(game.play(3, {Kind::Heart, Value::Four}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::King}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Two}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Four}));

		CHECK(game.scoreFor(0, 10) == Game::ScoreCase{4, 77, true});
		CHECK(game.scoreFor(1, 10) == Game::ScoreCase{1, 66, true});
		CHECK(game.scoreFor(2, 10) == Game::ScoreCase{3, 63, false});
		CHECK(game.scoreFor(3, 10) == Game::ScoreCase{1, 103, true});
		REQUIRE_NOTHROW(game.gotoNextRound());

		// Round 11
		CHECK(game.state() == State::SetTarget);
		CHECK(game.strong() == Card{Kind::Clover, Value::Four});
		CHECK(game.playerHand(0) == Hand{
			{Kind::Heart, Value::Ace}, {Kind::Clover, Value::Eight}, {Kind::Clover, Value::Ten}, {Kind::Clover, Value::Queen}, {Kind::Clover, Value::Ace},
			{Kind::Tile, Value::Four}, {Kind::Tile, Value::Five}, {Kind::Tile, Value::Nine}, {Kind::Tile, Value::Ten}, {Kind::Tile, Value::King},
			{Kind::Pike, Value::Ace}
		});
		CHECK(game.playerHand(1) == Hand{
			{Kind::Heart, Value::Four}, {Kind::Heart, Value::Five}, {Kind::Heart, Value::King},
			{Kind::Clover, Value::Two}, {Kind::Clover, Value::Seven}, {Kind::Clover, Value::Nine}, {Kind::Clover, Value::King},
			{Kind::Tile, Value::Jack}, {Kind::Pike, Value::Three}, {Kind::Pike, Value::Four}, {Kind::Pike, Value::Seven}
		});
		CHECK(game.playerHand(2) == Hand{
			{Kind::Heart, Value::Two}, {Kind::Heart, Value::Seven}, {Kind::Heart, Value::Eight}, {Kind::Clover, Value::Three},
			{Kind::Tile, Value::Two}, {Kind::Tile, Value::Three}, {Kind::Tile, Value::Eight}, {Kind::Tile, Value::Ace},
			{Kind::Pike, Value::Six}, {Kind::Pike, Value::Ten}, {Kind::Pike, Value::King}
		});
		REQUIRE(game.playerHand(3) == Hand{
			{Kind::Heart, Value::Three}, {Kind::Heart, Value::Nine}, {Kind::Heart, Value::Ten},
			{Kind::Clover, Value::Five}, {Kind::Clover, Value::Six}, {Kind::Tile, Value::Six}, {Kind::Tile, Value::Seven}, {Kind::Tile, Value::Queen},
			{Kind::Pike, Value::Five}, {Kind::Pike, Value::Eight}, {Kind::Pike, Value::Jack}
		});
		REQUIRE(game.currentPlayer() == 2);

		REQUIRE_NOTHROW(game.setTarget(2, 1));
		REQUIRE_NOTHROW(game.setTarget(3, 0));
		REQUIRE_NOTHROW(game.setTarget(0, 6));
		REQUIRE_NOTHROW(game.setTarget(1, 3));

		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Queen}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::Ten}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Jack}));
		CHECK(game.handStartingPlayer() == 2);

		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Seven}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Heart, Value::Three}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Heart, Value::Five}));
		CHECK(game.handStartingPlayer() == 2);

		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Seven}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::King}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Jack}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::Five}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Seven})); // Win
		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Eight}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Seven}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Four}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Ten}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Eight}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Eight})); // Win
		CHECK(game.handStartingPlayer() == 1);

		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::Nine}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Nine})); // Win
		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Three}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Six}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Three}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Six})); // Win
		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Five}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::King}));
		CHECK(game.handStartingPlayer() == 1);

		REQUIRE_NOTHROW(game.play(2, {Kind::Clover, Value::Three}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::Six}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Two}));
		CHECK(game.handStartingPlayer() == 2);

		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::Four})); // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Heart, Value::King}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Two}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Heart, Value::Ten}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Ten}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::King})); // Win
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Eight}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::Five}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(1, {Kind::Heart, Value::Four}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Two}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Heart, Value::Nine}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Queen})); // Win

		CHECK(game.scoreFor(0, 11) == Game::ScoreCase{6, 93, true});
		CHECK(game.scoreFor(1, 11) == Game::ScoreCase{3, 79, true});
		CHECK(game.scoreFor(2, 11) == Game::ScoreCase{1, 63, false});
		CHECK(game.scoreFor(3, 11) == Game::ScoreCase{0, 113, true});
		REQUIRE_NOTHROW(game.gotoNextRound());

		// Round 12
		CHECK(game.state() == State::SetTarget);
		CHECK(game.strong() == Card{Kind::Tile, Value::Eight});
		CHECK(game.playerHand(0) == Hand{
			{Kind::Heart, Value::Two}, {Kind::Heart, Value::Five}, {Kind::Heart, Value::Jack},
			{Kind::Clover, Value::Two}, {Kind::Clover, Value::Eight}, {Kind::Clover, Value::Nine}, {Kind::Clover, Value::Jack},
			{Kind::Tile, Value::Five}, {Kind::Tile, Value::Queen}, {Kind::Pike, Value::Seven}, {Kind::Pike, Value::Nine}, {Kind::Pike, Value::Jack}
		});
		CHECK(game.playerHand(1) == Hand{
			{Kind::Heart, Value::Three}, {Kind::Heart, Value::Six}, {Kind::Clover, Value::Ace},
			{Kind::Tile, Value::Two}, {Kind::Tile, Value::Four}, {Kind::Tile, Value::Nine}, {Kind::Tile, Value::King},
			{Kind::Pike, Value::Three}, {Kind::Pike, Value::Four}, {Kind::Pike, Value::Eight}, {Kind::Pike, Value::Ten}, {Kind::Pike, Value::Queen}
		});
		CHECK(game.playerHand(2) == Hand{
			{Kind::Heart, Value::Four}, {Kind::Heart, Value::Queen}, {Kind::Heart, Value::Ace},
			{Kind::Clover, Value::Three}, {Kind::Clover, Value::Four}, {Kind::Clover, Value::Five}, {Kind::Clover, Value::Six},
			{Kind::Clover, Value::Seven}, {Kind::Clover, Value::Ten}, {Kind::Tile, Value::Three}, {Kind::Tile, Value::Ten}, {Kind::Pike, Value::Two}
		});
		REQUIRE(game.playerHand(3) == Hand{
			{Kind::Heart, Value::Seven}, {Kind::Heart, Value::Eight}, {Kind::Heart, Value::Nine}, {Kind::Heart, Value::Ten},
			{Kind::Clover, Value::Queen}, {Kind::Clover, Value::King}, {Kind::Tile, Value::Seven}, {Kind::Tile, Value::Jack},
			{Kind::Pike, Value::Five}, {Kind::Pike, Value::Six}, {Kind::Pike, Value::King}, {Kind::Pike, Value::Ace}
		});
		REQUIRE(game.currentPlayer() == 3);

		REQUIRE_NOTHROW(game.setTarget(3, 4));
		REQUIRE_NOTHROW(game.setTarget(0, 1));
		REQUIRE_NOTHROW(game.setTarget(1, 3));
		REQUIRE_NOTHROW(game.setTarget(2, 1));

		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::Jack}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Queen}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Two}));
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::King})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::Nine}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Ten}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Clover, Value::Ten}));
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::Queen}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Jack}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(2, {Kind::Clover, Value::Seven}));
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(1, {Kind::Heart, Value::Six}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(3, {Kind::Heart, Value::Ten}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Jack}));
		CHECK(game.handStartingPlayer() == 1);

		REQUIRE_NOTHROW(game.play(2, {Kind::Clover, Value::Six}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::King})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Nine}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Eight}));
		CHECK(game.handStartingPlayer() == 2);

		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Six}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::Seven})); // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Four}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Queen}));
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Five}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Heart, Value::Three}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Four}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Heart, Value::Nine})); // Win
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Five})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Eight}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Three}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Clover, Value::Five}));
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Seven})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::Five}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Four}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Three}));
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Jack}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::Queen})); // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Two}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Ten}));
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Two}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Nine})); // Win
		REQUIRE_NOTHROW(game.play(2, {Kind::Clover, Value::Four}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Heart, Value::Eight}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::King})); // Win
		REQUIRE_NOTHROW(game.play(2, {Kind::Clover, Value::Three}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Heart, Value::Seven}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Two}));

		CHECK(game.scoreFor(0, 12) == Game::ScoreCase{1, 93, false});
		CHECK(game.scoreFor(1, 12) == Game::ScoreCase{3, 92, true});
		CHECK(game.scoreFor(2, 12) == Game::ScoreCase{1, 74, true});
		CHECK(game.scoreFor(3, 12) == Game::ScoreCase{4, 113, false});
		REQUIRE_NOTHROW(game.gotoNextRound());

		// Round 13
		CHECK(game.state() == State::SetTarget);
		CHECK_FALSE(game.strong());
		CHECK(game.playerHand(0) == Hand{
			{Kind::Heart, Value::Nine}, {Kind::Heart, Value::Ten}, {Kind::Heart, Value::Jack}, {Kind::Heart, Value::Queen},
			{Kind::Clover, Value::Two}, {Kind::Clover, Value::Three}, {Kind::Clover, Value::Seven}, {Kind::Clover, Value::Eight},
			{Kind::Clover, Value::Nine}, {Kind::Clover, Value::Ten}, {Kind::Tile, Value::Ace}, {Kind::Pike, Value::King}, {Kind::Pike, Value::Ace}
		});
		CHECK(game.playerHand(1) == Hand{
			{Kind::Heart, Value::Five}, {Kind::Heart, Value::Six}, {Kind::Heart, Value::King},
			{Kind::Clover, Value::Five}, {Kind::Clover, Value::Six}, {Kind::Clover, Value::Ace},
			{Kind::Tile, Value::Two}, {Kind::Tile, Value::Nine}, {Kind::Tile, Value::Ten}, {Kind::Tile, Value::Queen},
			{Kind::Pike, Value::Three}, {Kind::Pike, Value::Four}, {Kind::Pike, Value::Six}
		});
		CHECK(game.playerHand(2) == Hand{
			{Kind::Heart, Value::Two}, {Kind::Heart, Value::Three}, {Kind::Heart, Value::Four}, {Kind::Heart, Value::Ace},
			{Kind::Clover, Value::Queen},
			{Kind::Tile, Value::Three}, {Kind::Tile, Value::Four}, {Kind::Tile, Value::Six}, {Kind::Tile, Value::Jack},
			{Kind::Pike, Value::Five}, {Kind::Pike, Value::Seven}, {Kind::Pike, Value::Ten}, {Kind::Pike, Value::Jack}
		});
		REQUIRE(game.playerHand(3) == Hand{
			{Kind::Heart, Value::Seven}, {Kind::Heart, Value::Eight},
			{Kind::Clover, Value::Four}, {Kind::Clover, Value::Jack}, {Kind::Clover, Value::King},
			{Kind::Tile, Value::Five}, {Kind::Tile, Value::Seven}, {Kind::Tile, Value::Eight}, {Kind::Tile, Value::King},
			{Kind::Pike, Value::Two}, {Kind::Pike, Value::Eight}, {Kind::Pike, Value::Nine}, {Kind::Pike, Value::Queen}
		});
		REQUIRE(game.currentPlayer() == 0);

		REQUIRE_NOTHROW(game.setTarget(0, 4));
		REQUIRE_NOTHROW(game.setTarget(1, 3));
		REQUIRE_NOTHROW(game.setTarget(2, 1));
		REQUIRE_NOTHROW(game.setTarget(3, 3));

		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Six}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Jack}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Nine}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Jack}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Heart, Value::Six}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(3, {Kind::Heart, Value::Eight}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Six}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Eight}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Tile, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Ten}));
		CHECK(game.handStartingPlayer() == 2);

		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Ten}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Heart, Value::King})); // Win
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Four}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Heart, Value::Seven}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Nine}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Four}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::King})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Nine}));
		CHECK(game.handStartingPlayer() == 1);

		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Eight}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Pike, Value::King})); // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Four}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Ten}));
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(0, {Kind::Heart, Value::Queen})); // Win
		REQUIRE_NOTHROW(game.play(1, {Kind::Heart, Value::Five}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Three}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::Jack}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Seven}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Ace})); // Win
		REQUIRE_NOTHROW(game.play(2, {Kind::Clover, Value::Queen}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::Four}));
		CHECK(game.handStartingPlayer() == 0);

		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Queen})); // Win
		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Jack}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Seven}));
		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Ten}));
		CHECK(game.handStartingPlayer() == 1);

		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Six}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Seven}));
		REQUIRE_NOTHROW(game.play(3, {Kind::Clover, Value::King})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Nine}));
		CHECK(game.handStartingPlayer() == 1);

		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Queen})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Eight}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Pike, Value::Three}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Pike, Value::Five}));
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(3, {Kind::Pike, Value::Two})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Three}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Clover, Value::Five}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Tile, Value::Three}));
		CHECK(game.handStartingPlayer() == 3);

		REQUIRE_NOTHROW(game.play(3, {Kind::Tile, Value::Five})); // Win
		REQUIRE_NOTHROW(game.play(0, {Kind::Clover, Value::Two}));
		REQUIRE_NOTHROW(game.play(1, {Kind::Tile, Value::Two}));
		REQUIRE_NOTHROW(game.play(2, {Kind::Heart, Value::Two}));

		CHECK(game.scoreFor(0, 13) == Game::ScoreCase{4, 107, true});
		CHECK(game.scoreFor(1, 13) == Game::ScoreCase{3, 105, true});
		CHECK(game.scoreFor(2, 13) == Game::ScoreCase{1, 85, true});
		CHECK(game.scoreFor(3, 13) == Game::ScoreCase{3, 113, false});

		CHECK(game.state() == State::Finished);
		CHECK(game.currentRound() == 13);

		CHECK_THROWS_AS(game.setTarget(0, 0), Cards::GameFinished);
		CHECK_THROWS_AS(game.play(0, {Kind::Clover, Value::Two}), Cards::GameFinished);
		CHECK_THROWS_AS(game.gotoNextRound(), Cards::GameFinished);

	}
}