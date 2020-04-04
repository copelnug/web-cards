#include "StandardCards.hpp"
#include <catch.hpp>
#include "utilsStandardCards.hpp"

TEST_CASE("Compare helper for standard card values", "[cards][cards_standard][cards_standard_value]")
{
	using Cards::Standard::Value;

	const std::vector<Value> ordered{
		Value::Two,
		Value::Three,
		Value::Four,
		Value::Five,
		Value::Six,
		Value::Seven,
		Value::Eight,
		Value::Nine,
		Value::Jack,
		Value::Queen,
		Value::King,
		Value::Ace,
	};

	for(unsigned int i = 0; i < ordered.size(); ++i)
	{
		for(unsigned int j = 0; j < ordered.size(); ++j)
		{
			if(i == j)
				CHECK(compareAceTop(ordered[i], ordered[j]) == 0);
			else if(i < j)
				CHECK(compareAceTop(ordered[i], ordered[j]) < 0);
			else
				CHECK(compareAceTop(ordered[i], ordered[j]) > 0);
		}
	}
}
TEST_CASE("Build a full deck", "[cards][cards_standard]")
{
	using Cards::Standard::Card;
	using Cards::Standard::Kind;
	using Cards::Standard::Value;

	std::vector<Card> expected{
		{Kind::Heart, Value::Two},  {Kind::Tile, Value::Two},  {Kind::Clover, Value::Two},  {Kind::Pike, Value::Two},
		{Kind::Heart, Value::Three},{Kind::Tile, Value::Three},{Kind::Clover, Value::Three},{Kind::Pike, Value::Three},
		{Kind::Heart, Value::Four}, {Kind::Tile, Value::Four}, {Kind::Clover, Value::Four}, {Kind::Pike, Value::Four},
		{Kind::Heart, Value::Five}, {Kind::Tile, Value::Five}, {Kind::Clover, Value::Five}, {Kind::Pike, Value::Five},
		{Kind::Heart, Value::Six},  {Kind::Tile, Value::Six},  {Kind::Clover, Value::Six},  {Kind::Pike, Value::Six},
		{Kind::Heart, Value::Seven},{Kind::Tile, Value::Seven},{Kind::Clover, Value::Seven},{Kind::Pike, Value::Seven},
		{Kind::Heart, Value::Eight},{Kind::Tile, Value::Eight},{Kind::Clover, Value::Eight},{Kind::Pike, Value::Eight},
		{Kind::Heart, Value::Nine}, {Kind::Tile, Value::Nine}, {Kind::Clover, Value::Nine}, {Kind::Pike, Value::Nine},
		{Kind::Heart, Value::Ten},  {Kind::Tile, Value::Ten},  {Kind::Clover, Value::Ten},  {Kind::Pike, Value::Ten},
		{Kind::Heart, Value::Jack}, {Kind::Tile, Value::Jack}, {Kind::Clover, Value::Jack}, {Kind::Pike, Value::Jack},
		{Kind::Heart, Value::Queen},{Kind::Tile, Value::Queen},{Kind::Clover, Value::Queen},{Kind::Pike, Value::Queen},
		{Kind::Heart, Value::King}, {Kind::Tile, Value::King}, {Kind::Clover, Value::King}, {Kind::Pike, Value::King},
		{Kind::Heart, Value::Ace},  {Kind::Tile, Value::Ace},  {Kind::Clover, Value::Ace},  {Kind::Pike, Value::Ace},
	};

	auto result = Cards::Standard::createFullDeck();

	auto sorter = [](const Card& c1, const Card& c2) -> bool {
		return std::tie(c1.kind, c1.value) < std::tie(c2.kind, c2.value);
	};
	std::sort(expected.begin(), expected.end(), sorter);
	std::sort(result.begin(), result.end(), sorter);

	CHECK(result.size() == 52);
	CHECK(result == expected);
}
TEST_CASE("Check equality of cards", "[cards][cards_standard][cards_standard_card]")
{
	using Cards::Standard::Card;
	using Cards::Standard::Kind;
	using Cards::Standard::Value;

	CHECK(Card(Kind::Heart, Value::Two) == Card(Kind::Heart, Value::Two));
	CHECK_FALSE(Card(Kind::Heart, Value::Two) != Card(Kind::Heart, Value::Two));

	// Kind
	CHECK_FALSE(Card(Kind::Tile, Value::Two) == Card(Kind::Heart, Value::Two));
	CHECK(Card(Kind::Tile, Value::Two) != Card(Kind::Heart, Value::Two));

	CHECK_FALSE(Card(Kind::Clover, Value::Two) == Card(Kind::Heart, Value::Two));
	CHECK(Card(Kind::Clover, Value::Two) != Card(Kind::Heart, Value::Two));

	CHECK_FALSE(Card(Kind::Pike, Value::Two) == Card(Kind::Heart, Value::Two));
	CHECK(Card(Kind::Pike, Value::Two) != Card(Kind::Heart, Value::Two));

	// Number
	CHECK_FALSE(Card(Kind::Heart, Value::Two) == Card(Kind::Heart, Value::Ace));
	CHECK(Card(Kind::Heart, Value::Two) != Card(Kind::Heart, Value::Ace));
}
TEST_CASE("Order a deck of cards by kind", "[cards][cards_standard]")
{
	using Cards::Standard::Card;
	using Cards::Standard::Kind;
	using Cards::Standard::Value;

	std::vector<Card> input{
		{Kind::Clover, Value::Ace},
		{Kind::Heart, Value::Two},
		{Kind::Pike, Value::King},
		{Kind::Clover, Value::King},
		{Kind::Pike, Value::Five},
		{Kind::Tile, Value::Ace},
		{Kind::Heart, Value::Four},
		{Kind::Clover, Value::Jack},
		{Kind::Pike, Value::Ten},
		{Kind::Clover, Value::Queen},
		{Kind::Heart, Value::Ace},
	};
	// To note, the colors alternate to be easier to read in an UI.
	const std::vector<Card> Expected {
		{Kind::Heart, Value::Two},
		{Kind::Heart, Value::Four},
		{Kind::Heart, Value::Ace},
		{Kind::Clover, Value::Jack},
		{Kind::Clover, Value::Queen},
		{Kind::Clover, Value::King},
		{Kind::Clover, Value::Ace},
		{Kind::Tile, Value::Ace},
		{Kind::Pike, Value::Five},
		{Kind::Pike, Value::Ten},
		{Kind::Pike, Value::King},
	};

	organizeByKindAceTop(input);
	CHECK(input == Expected);
}