#ifndef TEST_CARDS_UTILS_STANDARD_CARDS_HPP_INCLUDED
#define TEST_CARDS_UTILS_STANDARD_CARDS_HPP_INCLUDED
#include <catch.hpp>
#include <optional>
#include "StandardCards.hpp"

CATCH_REGISTER_ENUM(Cards::Standard::Kind, 
	Cards::Standard::Kind::Heart, Cards::Standard::Kind::Tile, Cards::Standard::Kind::Clover, Cards::Standard::Kind::Pike
);
CATCH_REGISTER_ENUM(Cards::Standard::Value,
	Cards::Standard::Value::Two, Cards::Standard::Value::Three, Cards::Standard::Value::Four, Cards::Standard::Value::Five,
	Cards::Standard::Value::Six, Cards::Standard::Value::Seven, Cards::Standard::Value::Eight, Cards::Standard::Value::Nine,
	Cards::Standard::Value::Ten, Cards::Standard::Value::Jack, Cards::Standard::Value::Queen, Cards::Standard::Value::King,
	Cards::Standard::Value::Ace
);
namespace Catch {
    template<>
    struct StringMaker<Cards::Standard::Card> {
        static std::string convert( Cards::Standard::Card const& card ) {
			//return std::format("({0},{1})", StringMaker<Cards::Standard::Kind>::convert(card.kind), StringMaker<Cards::Standard::Value>::convert(card.value));
            return "{Kind::" + StringMaker<Cards::Standard::Kind>::convert(card.kind) + ", Value::"
				+ StringMaker<Cards::Standard::Value>::convert(card.value) + '}';
        }
    };
	template<>
	struct StringMaker<std::optional<Cards::Standard::Card>> {
        static std::string convert( std::optional<Cards::Standard::Card> const& card ) {
			if(card)
			{
				return '{' + StringMaker<Cards::Standard::Card>::convert(*card) + '}';
			}
			return "{}";
        }
	};
}

#endif