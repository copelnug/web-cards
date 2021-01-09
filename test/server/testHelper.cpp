#include <catch.hpp>

#include "Helper.hpp"
#include "Utils.hpp"

namespace
{
	using Test::StrEqualIgnoreSpaces;
}

TEST_CASE("Serialize a game message to ask permission to start", "[Serialize]")
{
	CHECK_THAT(Serialize::hostStart(), StrEqualIgnoreSpaces{
		R"_({"type": "ASK_START"})_"
	});
}
TEST_CASE("Serialize a game message to ask for username", "[Serialize]")
{
	CHECK_THAT(Serialize::askUsername(""), StrEqualIgnoreSpaces{
		R"_({"type": "ASK_USERNAME"})_"
	});
	CHECK_THAT(Serialize::askUsername("old"), StrEqualIgnoreSpaces{
		R"_({"type": "ASK_USERNAME", "current": "old"})_"
	});
}
TEST_CASE("Serialize a game error message", "[Serialize]")
{
	SECTION("Illegal choice")
	{
		CHECK_THAT(Serialize::illegalChoice(), StrEqualIgnoreSpaces{
			R"_({"type": "INPUT_INVALID", "msg": "Choix invalide. Réessayez SVP."})_"
		});
	}
	SECTION("Illegal choice")
	{
		CHECK_THAT(Serialize::actionOutOfStep(), StrEqualIgnoreSpaces{
			R"_({"type": "ERROR", "msg": "Erreur. Cette action n’est pas celle attendue."})_"
		});
	}
	SECTION("Illegal choice")
	{
		CHECK_THAT(Serialize::notPlayerTurn(), StrEqualIgnoreSpaces{
			R"_({"type": "ERROR", "msg": "Erreur. Ce n’est pas votre tour."})_"
		});
	}
}
TEST_CASE("Serialize a game waiting messages", "[Serialize]")
{
	SECTION("Waiting for start")
	{
		CHECK_THAT(Serialize::waitingStart("Unittest"), StrEqualIgnoreSpaces{
			R"_({"type": "STATUS", "msg": "En attente du début de la partie déclaré par Unittest"})_"
		});
	}
	SECTION("End game")
	{
		CHECK_THAT(Serialize::endGame(), StrEqualIgnoreSpaces{
			R"_({"type": "STATUS", "msg": "La partie est terminée", "actions": [)_"
			R"_(	{"type": "HOME", "label": "Retourner à la page principale"})_"
			R"_(]})_"
		});
	}
	SECTION("Waiting for host")
	{
		CHECK_THAT(Serialize::waitingHost(), StrEqualIgnoreSpaces{
			R"_({"type": "STATUS", "msg": "En attente du créateur de la partie"})_"
		});
	}
}