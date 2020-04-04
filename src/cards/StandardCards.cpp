#include "StandardCards.hpp"

#include <utility>

Cards::Standard::Card::Card(Kind kind, Value value) :
	kind{std::move(kind)},
	value{std::move(value)}
{}
int Cards::Standard::compareAceTop(const Value& v1, const Value& v2)
{
	return static_cast<int>(v1) - static_cast<int>(v2);
}
bool Cards::Standard::operator==(const Card& c1, const Card& c2)
{
	return c1.kind == c2.kind && c1.value == c2.value;
}
bool Cards::Standard::operator!=(const Card& c1, const Card& c2)
{
	return !(c1 == c2);
}
std::vector<Cards::Standard::Card> Cards::Standard::createFullDeck()
{
	// TODO Implement some kind of reflexion?
	const auto kinds = {Kind::Heart, Kind::Tile, Kind::Clover, Kind::Pike};
	const auto values = {
		Value::Two,
		Value::Three,
		Value::Four,
		Value::Five,
		Value::Six,
		Value::Seven,
		Value::Eight,
		Value::Nine,
		Value::Ten,
		Value::Jack,
		Value::Queen,
		Value::King,
		Value::Ace
	};

	std::vector<Card> result;
	result.reserve(kinds.size() * values.size());
	for(auto kind : kinds)
		for(auto value : values)
			result.emplace_back(kind, value);
	return result;
}