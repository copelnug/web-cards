#include <catch.hpp>
#include <random>

#include "Enfer.hpp"
#include "LobbyEnfer.hpp"
#include "StandardCards.hpp"

namespace
{
	class StrEqualIgnoreSpaces : public Catch::MatcherBase<std::string>
	{
		std::string expected_;
	public:
		StrEqualIgnoreSpaces(std::string expected) :
			expected_{std::move(expected)}
		{}

		bool match(const std::string& text) const override
		{
			size_t i = 0;
			size_t j = 0;

			for(;;++i, ++j)
			{
				while(i < text.size() && std::isspace(text[i]))
					++i;
				while(j < expected_.size() && std::isspace(expected_[j]))
					++j;

				if(i == text.size() && j == expected_.size())
					return true;
				if(i == text.size() || j == expected_.size())
					return false;

				if(text[i] != expected_[j])
					return false;
			}
		}

		virtual std::string describe() const override
		{
			return "match the JSON (ignoring formatting).";
		}
	};
}
TEST_CASE("Serialize a enfer player list", "[LobbyEnfer][LobbyEnfer_serialize]")
{
	const auto serializePlayerList = LobbyEnfer::serializePlayerList;

	CHECK_THAT(serializePlayerList({"Anna", "Bob", "Charlie", "Damian"}), StrEqualIgnoreSpaces{
		R"_({)_"
		R"_(	"type": "PLAYERS",)_"
		R"_(	"players": [)_"
		R"_(		"Anna",)_"
		R"_(		"Bob",)_"
		R"_(		"Charlie",)_"
		R"_(		"Damian")_"
		R"_(	])_"
		R"_(})_"
	});
	CHECK_THAT(serializePlayerList({"Eve", "Damian", "", "Bob", "Anna"}), StrEqualIgnoreSpaces{
		R"_({)_"
		R"_(	"type": "PLAYERS",)_"
		R"_(	"players": [)_"
		R"_(		"Eve",)_"
		R"_(		"Damian",)_"
		R"_(		"Inconnu",)_"
		R"_(		"Bob",)_"
		R"_(		"Anna")_"
		R"_(	])_"
		R"_(})_"
	});
}
TEST_CASE("Serialize a enfer game message to ask permission to start", "[LobbyEnfer][LobbyEnfer_serialize]")
{
	CHECK_THAT(LobbyEnfer::serializeHostStart(), StrEqualIgnoreSpaces{
		R"_({"type": "ASK_START"})_"
	});
}
TEST_CASE("Serialize a enfer game message to ask for username", "[LobbyEnfer][LobbyEnfer_serialize]")
{
	CHECK_THAT(LobbyEnfer::serializeAskUsername(""), StrEqualIgnoreSpaces{
		R"_({"type": "ASK_USERNAME"})_"
	});
	CHECK_THAT(LobbyEnfer::serializeAskUsername("old"), StrEqualIgnoreSpaces{
		R"_({"type": "ASK_USERNAME", "current": "old"})_"
	});
}
TEST_CASE("Serialize a enfer game state", "[LobbyEnfer][LobbyEnfer_serialize]")
{
	using Cards::Enfer::Game;
	using Cards::Enfer::Hand;
	using Cards::Enfer::Round;
	using Cards::Standard::Card;
	using Cards::Standard::Kind;
	using Cards::Standard::Value;

	using ScoreCase = Cards::Enfer::Game::ScoreCase;

	const auto serializeGameState = LobbyEnfer::serializeGameState;

	std::seed_seq seed{1, 2, 3, 4, 5, 6, 7, 8};
	
	SECTION("Basic round: Start")
	{
		const std::vector<std::string> Players{"Anna", "Bob", "Charlie", "Damian"};
		Round round{
			{
				Hand{ {Kind::Clover, Value::Two}, {Kind::Clover, Value::Ace}, {Kind::Pike, Value::Five} },
				Hand{ {Kind::Clover, Value::Three}, {Kind::Pike, Value::Ace}, {Kind::Pike, Value::Seven} },
				Hand{ {Kind::Clover, Value::Four}, {Kind::Tile, Value::Ace}, {Kind::Pike, Value::Eight} },
				Hand{ {Kind::Clover, Value::Six}, {Kind::Heart, Value::Ace}, {Kind::Pike, Value::Nine} },
			},
			{
				Round::PlayerStatus{0, 0},
				Round::PlayerStatus{},
				Round::PlayerStatus{1},
				Round::PlayerStatus{2,1}
			},
			{ Card{ Kind::Heart, Value::Two} },
			Hand{},
			2
		};
		Game game{4, {
				{ ScoreCase{0, 10, true}, ScoreCase{0, 10, true}, ScoreCase{0, 10, true}, ScoreCase{0, 10, true}},
				{ ScoreCase{1, 21, true}, ScoreCase{0, 20, true}, ScoreCase{0, 20, true}, ScoreCase{1, 10, false}},
			},
			std::move(round),
			3,
			seed
		};

		CHECK_THAT(serializeGameState(Players, game, 0), StrEqualIgnoreSpaces{
			R"_({)_"
			R"_(	"type": "STATE",)_"
			R"_(    "strong": {"kind": "HEART", "value": "TWO"},)_"
			R"_(	"play": [)_"
			R"_(		{)_"
			R"_(			"player": "Charlie",)_"
			R"_(			"state": "0 sur 1")_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Damian",)_"
			R"_(			"state": "1 sur 2")_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Anna",)_"
			R"_(			"state": "0 sur 0")_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Bob",)_"
			R"_(			"state": "?")_"
			R"_(		})_"
			R"_(	],)_"
			R"_(	"hand": [)_"
			R"_(		{"kind": "CLOVER", "value": "TWO", "playable": "true"},)_"
			R"_(		{"kind": "CLOVER", "value": "ACE", "playable": "true"},)_"
			R"_(		{"kind": "PIKE", "value": "FIVE", "playable": "true"})_"
			R"_(	],)_"
			R"_(	"score": [)_"
			R"_(		{)_"
			R"_(			"style": "header",)_"
			R"_(			"data": [)_"
			R"_(				"Cartes",)_"
			R"_(				"Anna",)_"
			R"_(				"Bob",)_"
			R"_(				"Charlie",)_"
			R"_(				"Damian")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"1",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"2",)_"
			R"_(				"1-21",)_"
			R"_(				"0-20",)_"
			R"_(				"0-20",)_"
			R"_(				"1-10")_"
			R"_(			])_"
			R"_(		})_"
			R"_(	])_"
			R"_(})_"
		});
		CHECK_THAT(serializeGameState(Players, game, 1), StrEqualIgnoreSpaces{
			R"_({)_"
			R"_(	"type": "STATE",)_"
			R"_(    "strong": {"kind": "HEART", "value": "TWO"},)_"
			R"_(	"play": [)_"
			R"_(		{)_"
			R"_(			"player": "Charlie",)_"
			R"_(			"state": "0 sur 1")_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Damian",)_"
			R"_(			"state": "1 sur 2")_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Anna",)_"
			R"_(			"state": "0 sur 0")_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Bob",)_"
			R"_(			"state": "?")_"
			R"_(		})_"
			R"_(	],)_"
			R"_(	"hand": [)_"
			R"_(		{"kind": "CLOVER", "value": "THREE", "playable": "true"},)_"
			R"_(		{"kind": "PIKE", "value": "ACE", "playable": "true"},)_"
			R"_(		{"kind": "PIKE", "value": "SEVEN", "playable": "true"})_"
			R"_(	],)_"
			R"_(	"score": [)_"
			R"_(		{)_"
			R"_(			"style": "header",)_"
			R"_(			"data": [)_"
			R"_(				"Cartes",)_"
			R"_(				"Anna",)_"
			R"_(				"Bob",)_"
			R"_(				"Charlie",)_"
			R"_(				"Damian")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"1",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"2",)_"
			R"_(				"1-21",)_"
			R"_(				"0-20",)_"
			R"_(				"0-20",)_"
			R"_(				"1-10")_"
			R"_(			])_"
			R"_(		})_"
			R"_(	])_"
			R"_(})_"
		});
		CHECK_THAT(serializeGameState(Players, game, 2), StrEqualIgnoreSpaces{
			R"_({)_"
			R"_(	"type": "STATE",)_"
			R"_(    "strong": {"kind": "HEART", "value": "TWO"},)_"
			R"_(	"play": [)_"
			R"_(		{)_"
			R"_(			"player": "Charlie",)_"
			R"_(			"state": "0 sur 1")_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Damian",)_"
			R"_(			"state": "1 sur 2")_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Anna",)_"
			R"_(			"state": "0 sur 0")_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Bob",)_"
			R"_(			"state": "?")_"
			R"_(		})_"
			R"_(	],)_"
			R"_(	"hand": [)_"
			R"_(		{"kind": "CLOVER", "value": "FOUR", "playable": "true"},)_"
			R"_(		{"kind": "TILE", "value": "ACE", "playable": "true"},)_"
			R"_(		{"kind": "PIKE", "value": "EIGHT", "playable": "true"})_"
			R"_(	],)_"
			R"_(	"score": [)_"
			R"_(		{)_"
			R"_(			"style": "header",)_"
			R"_(			"data": [)_"
			R"_(				"Cartes",)_"
			R"_(				"Anna",)_"
			R"_(				"Bob",)_"
			R"_(				"Charlie",)_"
			R"_(				"Damian")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"1",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"2",)_"
			R"_(				"1-21",)_"
			R"_(				"0-20",)_"
			R"_(				"0-20",)_"
			R"_(				"1-10")_"
			R"_(			])_"
			R"_(		})_"
			R"_(	])_"
			R"_(})_"
		});
		CHECK_THAT(serializeGameState(Players, game, 3), StrEqualIgnoreSpaces{
			R"_({)_"
			R"_(	"type": "STATE",)_"
			R"_(    "strong": {"kind": "HEART", "value": "TWO"},)_"
			R"_(	"play": [)_"
			R"_(		{)_"
			R"_(			"player": "Charlie",)_"
			R"_(			"state": "0 sur 1")_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Damian",)_"
			R"_(			"state": "1 sur 2")_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Anna",)_"
			R"_(			"state": "0 sur 0")_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Bob",)_"
			R"_(			"state": "?")_"
			R"_(		})_"
			R"_(	],)_"
			R"_(	"hand": [)_"
			R"_(		{"kind": "CLOVER", "value": "SIX", "playable": "true"},)_"
			R"_(		{"kind": "HEART", "value": "ACE", "playable": "true"},)_"
			R"_(		{"kind": "PIKE", "value": "NINE", "playable": "true"})_"
			R"_(	],)_"
			R"_(	"score": [)_"
			R"_(		{)_"
			R"_(			"style": "header",)_"
			R"_(			"data": [)_"
			R"_(				"Cartes",)_"
			R"_(				"Anna",)_"
			R"_(				"Bob",)_"
			R"_(				"Charlie",)_"
			R"_(				"Damian")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"1",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"2",)_"
			R"_(				"1-21",)_"
			R"_(				"0-20",)_"
			R"_(				"0-20",)_"
			R"_(				"1-10")_"
			R"_(			])_"
			R"_(		})_"
			R"_(	])_"
			R"_(})_"
		});
	}
	SECTION("In progress")
	{
		const std::vector<std::string> Players{"Anna", "Bob", "Charlie", "Damian"};
		Round round{
			{
				Hand{ {Kind::Clover, Value::Two}, {Kind::Clover, Value::Ace}, {Kind::Pike, Value::Five} },
				Hand{ {Kind::Clover, Value::Three}, {Kind::Pike, Value::Ace}, {Kind::Pike, Value::Seven} },
				Hand{ {Kind::Clover, Value::Four}, {Kind::Pike, Value::Eight} },
				Hand{ {Kind::Heart, Value::Ace}, {Kind::Pike, Value::Nine} },
			},
			{
				Round::PlayerStatus{0, 0},
				Round::PlayerStatus{},
				Round::PlayerStatus{1},
				Round::PlayerStatus{2,1}
			},
			{ Card{ Kind::Heart, Value::Two} },
			Hand{{Kind::Tile, Value::Ace}, {Kind::Clover, Value::Six}},
			2
		};

		Game game{4, {
				{ ScoreCase{0, 10, true}, ScoreCase{0, 10, true}, ScoreCase{0, 10, true}, ScoreCase{0, 10, true}},
				{ ScoreCase{1, 21, true}, ScoreCase{0, 20, true}, ScoreCase{0, 20, true}, ScoreCase{1, 10, false}},
			},
			std::move(round),
			3,
			seed
		};

		CHECK_THAT(serializeGameState(Players, game, 2), StrEqualIgnoreSpaces{
			R"_({)_"
			R"_(	"type": "STATE",)_"
			R"_(    "strong": {"kind": "HEART", "value": "TWO"},)_"
			R"_(	"play": [)_"
			R"_(		{)_"
			R"_(			"player": "Charlie",)_"
			R"_(			"state": "0 sur 1",)_"
            R"_(			"card": {)_"
			R"_(				"kind": "TILE",)_"
			R"_(				"value": "ACE")_"
			R"_(			})_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Damian",)_"
			R"_(			"state": "1 sur 2",)_"
            R"_(			"card": {)_"
			R"_(				"kind": "CLOVER",)_"
			R"_(				"value": "SIX")_"
			R"_(			})_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Anna",)_"
			R"_(			"state": "0 sur 0")_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Bob",)_"
			R"_(			"state": "?")_"
			R"_(		})_"
			R"_(	],)_"
			R"_(	"hand": [)_"
			R"_(		{"kind": "CLOVER", "value": "FOUR", "playable": "true"},)_"
			R"_(		{"kind": "PIKE", "value": "EIGHT", "playable": "true"})_"
			R"_(	],)_"
			R"_(	"score": [)_"
			R"_(		{)_"
			R"_(			"style": "header",)_"
			R"_(			"data": [)_"
			R"_(				"Cartes",)_"
			R"_(				"Anna",)_"
			R"_(				"Bob",)_"
			R"_(				"Charlie",)_"
			R"_(				"Damian")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"1",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"2",)_"
			R"_(				"1-21",)_"
			R"_(				"0-20",)_"
			R"_(				"0-20",)_"
			R"_(				"1-10")_"
			R"_(			])_"
			R"_(		})_"
			R"_(	])_"
			R"_(})_"
		});
	}
	SECTION("Round finished")
	{
		const std::vector<std::string> Players{"Anna", "Bob", "Charlie", "Damian"};
		Round round{
			{
				Hand{ {Kind::Clover, Value::Two}, {Kind::Pike, Value::Five} },
				Hand{ {Kind::Clover, Value::Three}, {Kind::Pike, Value::Seven} },
				Hand{ {Kind::Clover, Value::Four}, {Kind::Pike, Value::Eight} },
				Hand{ {Kind::Heart, Value::Ace}, {Kind::Pike, Value::Nine} },
			},
			{
				Round::PlayerStatus{0, 0},
				Round::PlayerStatus{1},
				Round::PlayerStatus{1},
				Round::PlayerStatus{2,1}
			},
			{ Card{ Kind::Heart, Value::Two} },
			Hand{{Kind::Tile, Value::Ace}, {Kind::Clover, Value::Six}, {Kind::Clover, Value::Ace}, {Kind::Pike, Value::Ace}},
			2
		};

		Game game{4, {
				{ ScoreCase{0, 10, true}, ScoreCase{0, 10, true}, ScoreCase{0, 10, true}, ScoreCase{0, 10, true}},
				{ ScoreCase{1, 21, true}, ScoreCase{0, 20, true}, ScoreCase{0, 20, true}, ScoreCase{1, 10, false}},
			},
			std::move(round),
			3,
			seed
		};

		CHECK_THAT(serializeGameState(Players, game, 2), StrEqualIgnoreSpaces{
			R"_({)_"
			R"_(	"type": "STATE",)_"
			R"_(    "strong": {"kind": "HEART", "value": "TWO"},)_"
			R"_(	"play": [)_"
			R"_(		{)_"
			R"_(			"player": "Charlie",)_"
			R"_(			"state": "0 sur 1",)_"
            R"_(			"card": {)_"
			R"_(				"kind": "TILE",)_"
			R"_(				"value": "ACE")_"
			R"_(			})_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Damian",)_"
			R"_(			"state": "1 sur 2",)_"
            R"_(			"card": {)_"
			R"_(				"kind": "CLOVER",)_"
			R"_(				"value": "SIX")_"
			R"_(			})_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Anna",)_"
			R"_(			"state": "0 sur 0",)_"
            R"_(			"card": {)_"
			R"_(				"kind": "CLOVER",)_"
			R"_(				"value": "ACE")_"
			R"_(			})_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Bob",)_"
			R"_(			"state": "0 sur 1",)_"
            R"_(			"card": {)_"
			R"_(				"kind": "PIKE",)_"
			R"_(				"value": "ACE")_"
			R"_(			})_"
			R"_(		})_"
			R"_(	],)_"
			R"_(	"hand": [)_"
			R"_(		{"kind": "CLOVER", "value": "FOUR", "playable": "true"},)_"
			R"_(		{"kind": "PIKE", "value": "EIGHT", "playable": "true"})_"
			R"_(	],)_"
			R"_(	"score": [)_"
			R"_(		{)_"
			R"_(			"style": "header",)_"
			R"_(			"data": [)_"
			R"_(				"Cartes",)_"
			R"_(				"Anna",)_"
			R"_(				"Bob",)_"
			R"_(				"Charlie",)_"
			R"_(				"Damian")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"1",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"2",)_"
			R"_(				"1-21",)_"
			R"_(				"0-20",)_"
			R"_(				"0-20",)_"
			R"_(				"1-10")_"
			R"_(			])_"
			R"_(		})_"
			R"_(	])_"
			R"_(})_"
		});
	}
	SECTION("Game finished")
	{
		const std::vector<std::string> Players{"Anna", "Bob", "Charlie", "Damian", "Eve"};
		Round round{
			{
				Hand{},
				Hand{},
				Hand{},
				Hand{},
				Hand{},
			},
			{
				Round::PlayerStatus{0, 0},
				Round::PlayerStatus{3},
				Round::PlayerStatus{1},
				Round::PlayerStatus{2,1},
				Round::PlayerStatus{2,1}
			},
			{ },
			Hand{Card{ Kind::Heart, Value::Two}, Card{ Kind::Heart, Value::Two}, Card{ Kind::Heart, Value::Two}, Card{ Kind::Heart, Value::Two}, Card{ Kind::Heart, Value::Two}},
			2
		};
		Game game{5, {
				{ ScoreCase{0, 10, true}, ScoreCase{0, 10, true}, ScoreCase{0, 10, true}, ScoreCase{0, 10, true}, ScoreCase{0, 0, false}},
				{ ScoreCase{1, 21, true}, ScoreCase{0, 20, true}, ScoreCase{0, 20, true}, ScoreCase{1, 10, false}, ScoreCase{0, 0, false}},
				{ ScoreCase{2, 33, true}, ScoreCase{0, 30, true}, ScoreCase{0, 30, true}, ScoreCase{1, 21, true}, ScoreCase{0, 0, false}},
				{ ScoreCase{3, 46, true}, ScoreCase{0, 30, false}, ScoreCase{0, 30, false}, ScoreCase{2, 21, false}, ScoreCase{0, 0, false}},
				{ ScoreCase{4, 60, true}, ScoreCase{0, 40, true}, ScoreCase{0, 40, true}, ScoreCase{2, 33, true}, ScoreCase{0, 0, false}},
				{ ScoreCase{5, 75, true}, ScoreCase{0, 50, true}, ScoreCase{0, 50, true}, ScoreCase{3, 33, false}, ScoreCase{0, 0, false}},
				{ ScoreCase{6, 91, true}, ScoreCase{0, 60, true}, ScoreCase{0, 60, true}, ScoreCase{3, 46, true}, ScoreCase{0, 0, false}},
				{ ScoreCase{7, 108, true}, ScoreCase{0, 70, true}, ScoreCase{0, 70, true}, ScoreCase{4, 46, false}, ScoreCase{0, 0, false}},
				{ ScoreCase{8, 126, true}, ScoreCase{0, 80, true}, ScoreCase{0, 80, true}, ScoreCase{4, 60, true}, ScoreCase{0, 0, false}},
				{ ScoreCase{9, 145, true}, ScoreCase{0, 90, true}, ScoreCase{0, 90, true}, ScoreCase{5, 60, false}, ScoreCase{0, 0, false}},
				{ ScoreCase{10, 165, true}, ScoreCase{0, 100, true}, ScoreCase{0, 100, true}, ScoreCase{5, 76, true}, ScoreCase{0, 0, false}},
			},
			std::move(round),
			12,
			seed
		};

		CHECK_THAT(serializeGameState(Players, game, 3), StrEqualIgnoreSpaces{
			R"_({)_"
			R"_(	"type": "STATE",)_"
			R"_(	"play": [)_"
			R"_(		{)_"
			R"_(			"player": "Charlie",)_"
			R"_(			"state": "0 sur 1",)_"
            R"_(			"card": {)_"
			R"_(				"kind": "HEART",)_"
			R"_(				"value": "TWO")_"
			R"_(			})_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Damian",)_"
			R"_(			"state": "1 sur 2",)_"
            R"_(			"card": {)_"
			R"_(				"kind": "HEART",)_"
			R"_(				"value": "TWO")_"
			R"_(			})_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Eve",)_"
			R"_(			"state": "1 sur 2",)_"
            R"_(			"card": {)_"
			R"_(				"kind": "HEART",)_"
			R"_(				"value": "TWO")_"
			R"_(			})_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Anna",)_"
			R"_(			"state": "0 sur 0",)_"
            R"_(			"card": {)_"
			R"_(				"kind": "HEART",)_"
			R"_(				"value": "TWO")_"
			R"_(			})_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"player": "Bob",)_"
			R"_(			"state": "0 sur 3",)_"
            R"_(			"card": {)_"
			R"_(				"kind": "HEART",)_"
			R"_(				"value": "TWO")_"
			R"_(			})_"
			R"_(		})_"
			R"_(	],)_"
			R"_(	"hand": "",)_"
			R"_(	"score": [)_"
			R"_(		{)_"
			R"_(			"style": "header",)_"
			R"_(			"data": [)_"
			R"_(				"Cartes",)_"
			R"_(				"Anna",)_"
			R"_(				"Bob",)_"
			R"_(				"Charlie",)_"
			R"_(				"Damian",)_"
			R"_(				"Eve")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"1",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10",)_"
			R"_(				"0-10",)_"
			R"_(				"0-0")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"2",)_"
			R"_(				"1-21",)_"
			R"_(				"0-20",)_"
			R"_(				"0-20",)_"
			R"_(				"1-10",)_"
			R"_(				"0-0")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"3",)_"
			R"_(				"2-33",)_"
			R"_(				"0-30",)_"
			R"_(				"0-30",)_"
			R"_(				"1-21",)_"
			R"_(				"0-0")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"4",)_"
			R"_(				"3-46",)_"
			R"_(				"0-30",)_"
			R"_(				"0-30",)_"
			R"_(				"2-21",)_"
			R"_(				"0-0")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"5",)_"
			R"_(				"4-60",)_"
			R"_(				"0-40",)_"
			R"_(				"0-40",)_"
			R"_(				"2-33",)_"
			R"_(				"0-0")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"6",)_"
			R"_(				"5-75",)_"
			R"_(				"0-50",)_"
			R"_(				"0-50",)_"
			R"_(				"3-33",)_"
			R"_(				"0-0")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"7",)_"
			R"_(				"6-91",)_"
			R"_(				"0-60",)_"
			R"_(				"0-60",)_"
			R"_(				"3-46",)_"
			R"_(				"0-0")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"8",)_"
			R"_(				"7-108",)_"
			R"_(				"0-70",)_"
			R"_(				"0-70",)_"
			R"_(				"4-46",)_"
			R"_(				"0-0")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"9",)_"
			R"_(				"8-126",)_"
			R"_(				"0-80",)_"
			R"_(				"0-80",)_"
			R"_(				"4-60",)_"
			R"_(				"0-0")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"10",)_"
			R"_(				"9-145",)_"
			R"_(				"0-90",)_"
			R"_(				"0-90",)_"
			R"_(				"5-60",)_"
			R"_(				"0-0")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "normal",)_"
			R"_(			"data": [)_"
			R"_(				"10*",)_"
			R"_(				"10-165",)_"
			R"_(				"0-100",)_"
			R"_(				"0-100",)_"
			R"_(				"5-76",)_"
			R"_(				"0-0")_"
			R"_(			])_"
			R"_(		},)_"
			R"_(		{)_"
			R"_(			"style": "header",)_"
			R"_(			"data": [)_"
			R"_(				"Classement",)_"
			R"_(				"1",)_"
			R"_(				"2",)_"
			R"_(				"2",)_"
			R"_(				"4",)_"
			R"_(				"5")_"
			R"_(			])_"
			R"_(		})_"
			R"_(	])_"
			R"_(})_"
		});
	}
}
TEST_CASE("Serialize a enfer game message to ask for target", "[LobbyEnfer][LobbyEnfer_serialize]")
{
	// TODO Find a json library that support integer.
	CHECK_THAT(LobbyEnfer::serializeAskTarget(4), StrEqualIgnoreSpaces{
		R"_({)_"
		R"_(	"type": "ASK_INTEGER",)_"
		R"_(	"msg":	"Combien de mains ferez vous?",)_"
		R"_(	"min": "0",)_"
		R"_(	"max": "4")_"
		R"_(})_"
	});
	CHECK_THAT(LobbyEnfer::serializeAskTarget(6), StrEqualIgnoreSpaces{
		R"_({)_"
		R"_(	"type": "ASK_INTEGER",)_"
		R"_(	"msg":	"Combien de mains ferez vous?",)_"
		R"_(	"min": "0",)_"
		R"_(	"max": "6")_"
		R"_(})_"
	});
}
TEST_CASE("Serialize a enfer game message to ask the user to play a card", "[LobbyEnfer][LobbyEnfer_serialize]")
{
	CHECK_THAT(LobbyEnfer::serializeAskChooseCard(), StrEqualIgnoreSpaces{
		R"_({"type": "PLAY_CARD"})_"
	});
}
TEST_CASE("Serialize a enfer game message to ask before starting next round", "[LobbyEnfer][LobbyEnfer_serialize]")
{
	CHECK_THAT(LobbyEnfer::serializeAskNextRound(), StrEqualIgnoreSpaces{
		R"_({"type": "ASK_CONFIRM", "msg": "Passez à la prochaine manche?"})_"
	});
}
TEST_CASE("Serialize a enfer game error message", "[LobbyEnfer][LobbyEnfer_serialize]")
{
	SECTION("Illegal choice")
	{
		CHECK_THAT(LobbyEnfer::serializeIllegalChoice(), StrEqualIgnoreSpaces{
			R"_({"type": "INPUT_INVALID", "msg": "Choix invalide. Réessayez SVP."})_"
		});
	}
	SECTION("Illegal choice")
	{
		CHECK_THAT(LobbyEnfer::serializeActionOutOfStep(), StrEqualIgnoreSpaces{
			R"_({"type": "ERROR", "msg": "Erreur. Cette action n’est pas celle attendue."})_"
		});
	}
	SECTION("Illegal choice")
	{
		CHECK_THAT(LobbyEnfer::serializeNotPlayerTurn(), StrEqualIgnoreSpaces{
			R"_({"type": "ERROR", "msg": "Erreur. Ce n’est pas votre tour."})_"
		});
	}
}