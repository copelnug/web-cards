#include <catch.hpp>

#include "Uno.hpp"

TEST_CASE("Uno: Check a full deck", "[cards][cards_uno]")
{
	CHECK(Cards::Uno::createFullDeck().size() == 108);
}
TEST_CASE("Uno: Calculate point on a hand", "[cards][cards_uno]")
{
	using Cards::Uno::BasicValue;
	using Cards::Uno::ColorlessValue;
	using Cards::Uno::Color;
	using Cards::Uno::Card;
	
	using Cards::Uno::calculatePoints;

	CHECK(calculatePoints({ Card{Color::Blue, BasicValue::Zero} }) == 0);
	CHECK(calculatePoints({ Card{Color::Red, BasicValue::One} }) == 1);
	CHECK(calculatePoints({ Card{Color::Blue, BasicValue::Two} }) == 2);
	CHECK(calculatePoints({ Card{Color::Yellow, BasicValue::Three} }) == 3);
	CHECK(calculatePoints({ Card{Color::Blue, BasicValue::Four} }) == 4);
	CHECK(calculatePoints({ Card{Color::Green, BasicValue::Five} }) == 5);
	CHECK(calculatePoints({ Card{Color::Blue, BasicValue::Six} }) == 6);
	CHECK(calculatePoints({ Card{Color::Blue, BasicValue::Seven} }) == 7);
	CHECK(calculatePoints({ Card{Color::Blue, BasicValue::Eight} }) == 8);
	CHECK(calculatePoints({ Card{Color::Blue, BasicValue::Nine} }) == 9);
	CHECK(calculatePoints({ Card{Color::Yellow, BasicValue::SkipTurn} }) == 20);
	CHECK(calculatePoints({ Card{Color::Green, BasicValue::Plus2} }) == 20);
	CHECK(calculatePoints({ Card{Color::Red, BasicValue::Reverse} }) == 20);

	CHECK(calculatePoints({ Card{ColorlessValue::ChangeColor} }) == 50);
	CHECK(calculatePoints({ Card{ColorlessValue::Plus4} }) == 50);

	CHECK(calculatePoints({
		Card{Color::Blue, BasicValue::Four},
		Card{Color::Yellow, BasicValue::SkipTurn},
		Card{Color::Blue, BasicValue::Eight},
		Card{ColorlessValue::ChangeColor},
		Card{ColorlessValue::ChangeColor},
		Card{Color::Blue, BasicValue::Zero},
	}) == 132);
}
TEST_CASE("Uno: Organize a hand", "[cards][cards_uno]")
{
	// TODO
}