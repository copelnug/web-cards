#include "Lobby.hpp"

Lobby::Lobby(Server* server) :
	server_{std::move(server)}
{}