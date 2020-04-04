#ifndef CARDS_STANDARD_CARDS_HPP_INCLUDED
#define CARDS_STANDARD_CARDS_HPP_INCLUDED
#include <algorithm>

namespace Cards
{
	namespace Standard
	{
		enum class Kind
		{
			Heart,
			Clover,
			Tile,
			Pike,
		};
		enum class Value
		{
			Two,
			Three,
			Four,
			Five,
			Six,
			Seven,
			Eight,
			Nine,
			Ten,
			Jack,
			Queen,
			King,
			Ace,
		};

		struct Card
		{
			Card(Kind kind, Value value);

			Kind kind;
			Value value;

			//friend bool operator==(const Card&, const Card&) = default;
			//friend bool operator!=(const Card&, const Card&) = default;
		};
		bool operator==(const Card& c1, const Card& c2);
		bool operator!=(const Card& c1, const Card& c2);

		constexpr const unsigned short FullDeckSize = 52;
		std::vector<Card> createFullDeck();

		/////////////////////////////////// Sorting utils ///////////////////////////////////

		// TODO Use std::strong_ordering
		int compareAceTop(const Value& v1, const Value& v2);

		template <typename Deck>
		void organizeByKindAceTop(Deck& deck);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////// Implementation //////////////////////////////////////////////////////
/**
	\brief Group cards in a deck by kind (alternating colors for better UI) and then sort them by value (Ace being better).
	\tparam Deck Type of deck.
	\param deck Deck of cards.

	TODO: Add concept to Deck.
*/
template <typename Deck>
void Cards::Standard::organizeByKindAceTop(Deck& deck)
{
	std::sort(deck.begin(), deck.end(), [](const Card& c1, const Card& c2) -> bool {
		if(c1.kind < c2.kind)
			return true;
		if(c1.kind > c2.kind)
			return false;
		return compareAceTop(c1.value, c2.value) < 0;
	});
}
#endif