#include <catch.hpp>
#include <set>

#include "GlobalLobby.hpp"
#include "Utils.hpp"

namespace
{
	using LobbyInfo = GlobalLobby::LobbyInfo;
	using Test::StrEqualIgnoreSpaces;

	struct TestLobby : public Lobby
	{
		std::set<std::string> sessions;

		TestLobby(std::string name) :
			Lobby{nullptr, std::move(name)}
		{}

		virtual std::optional<std::string> getHtmlFile(const std::string_view& session) const override
		{
			FAIL("Should not be called (getHtmlFile).");
			return "Unittest";
		}
		virtual bool canJoin(const std::string_view& session) override
		{
			return sessions.count(std::string{session}) > 0;
		}
		virtual void join(const boost::shared_ptr<WebsocketSession>& connection, boost::asio::yield_context yield) override
		{ FAIL("Should not be called (join)."); }
		virtual bool leave(const std::string& session, boost::asio::yield_context yield) override
		{ FAIL("Should not be called (leave)."); return false; }
		virtual bool onMessage(const boost::shared_ptr<WebsocketSession>& connection, const boost::property_tree::ptree& message, boost::asio::yield_context yield) override
		{ FAIL("Should not be called (onMessage)."); return false; }
	};
}

TEST_CASE("Test GlobalLobby::serializeLobbyAdd", "[GlobalLobby][GlobalLobby_serialize]")
{
	const auto serializeLobbyAdd = GlobalLobby::serializeLobbyAdd;
	const std::string s1{"abc"};
	const std::string s2{"def"};
	const std::string s3{"ghi"};

	auto l1 = std::make_shared<TestLobby>("First lobby");
	l1->sessions.emplace(s1);
	l1->sessions.emplace(s2);

	CHECK_THAT(serializeLobbyAdd(LobbyInfo{"lobby_one", l1}, s1), StrEqualIgnoreSpaces{
		R"_({)_"
		R"_(	"type": "LOBBY_ADD",)_"
		R"_(	"id": "lobby_one",)_"
		R"_(	"name": "First lobby",)_"
		R"_(	"url": "lobby_one")_"
		R"_(})_"
	});
	CHECK_THAT(serializeLobbyAdd(LobbyInfo{"lobby_one", l1}, s2), StrEqualIgnoreSpaces{
		R"_({)_"
		R"_(	"type": "LOBBY_ADD",)_"
		R"_(	"id": "lobby_one",)_"
		R"_(	"name": "First lobby",)_"
		R"_(	"url": "lobby_one")_"
		R"_(})_"
	});
	CHECK_THAT(serializeLobbyAdd(LobbyInfo{"lobby_one", l1}, s3), StrEqualIgnoreSpaces{
		R"_({)_"
		R"_(	"type": "LOBBY_ADD",)_"
		R"_(	"id": "lobby_one",)_"
		R"_(	"name": "First lobby")_"
		R"_(})_"
	});
}
TEST_CASE("Test GlobalLobby::serializeLobbyRemove", "[GlobalLobby][GlobalLobby_serialize]")
{
	const auto serializeLobbyRemove = GlobalLobby::serializeLobbyRemove;
	const std::string l1{"lobby_one"};
	const std::string l2{"lobby_two"};

	CHECK_THAT(serializeLobbyRemove(l1), StrEqualIgnoreSpaces{
		R"_({)_"
		R"_(	"type": "LOBBY_REMOVE",)_"
		R"_(	"id": "lobby_one")_"
		R"_(})_"
	});
	CHECK_THAT(serializeLobbyRemove(l2), StrEqualIgnoreSpaces{
		R"_({)_"
		R"_(	"type": "LOBBY_REMOVE",)_"
		R"_(	"id": "lobby_two")_"
		R"_(})_"
	});
}
TEST_CASE("Test GlobalLobby::serializeLobbyList", "[GlobalLobby][GlobalLobby_serialize]")
{
	const auto serializeLobbyAdd = GlobalLobby::serializeLobbyAdd;
	const auto serializeLobbyList = GlobalLobby::serializeLobbyList;
	const std::string s1{"abc"};
	const std::string s2{"def"};
	const std::string s3{"ghi"};

	auto l1 = std::make_shared<TestLobby>("First lobby");
	l1->sessions.emplace(s1);
	l1->sessions.emplace(s2);

	auto l2 = std::make_shared<TestLobby>("Second lobby");
	l2->sessions.emplace(s2);

	auto l3 = std::make_shared<TestLobby>("Third lobby");
	l3->sessions.emplace(s3);

	std::vector<LobbyInfo> lobbies{
		{"lobby_one", l1},
		{"lobby_two", l2},
		{"lobby_three", l3},
	};

	CHECK(serializeLobbyList(lobbies, s1) == std::vector<std::string>{
		serializeLobbyAdd(lobbies[0], s1),
		serializeLobbyAdd(lobbies[1], s1),
		serializeLobbyAdd(lobbies[2], s1),
	});
	CHECK(serializeLobbyList(lobbies, s2) == std::vector<std::string>{
		serializeLobbyAdd(lobbies[0], s2),
		serializeLobbyAdd(lobbies[1], s2),
		serializeLobbyAdd(lobbies[2], s2),
	});
	CHECK(serializeLobbyList(lobbies, s3) == std::vector<std::string>{
		serializeLobbyAdd(lobbies[0], s3),
		serializeLobbyAdd(lobbies[1], s3),
		serializeLobbyAdd(lobbies[2], s3),
	});
}