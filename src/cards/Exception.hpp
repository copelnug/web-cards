#ifndef CARDS_EXCEPTION_HPP_INCLUDED
#define CARDS_EXCEPTION_HPP_INCLUDED
#include <stdexcept>

namespace Cards
{
	struct IllegalChoice : public std::runtime_error
	{
		IllegalChoice(unsigned short player);
	};
	struct NotPlayerTurn : public std::runtime_error
	{
		NotPlayerTurn(unsigned short wrongPlayer);
	};
	struct ActionOutOfStep : public std::runtime_error
	{
		ActionOutOfStep();
		ActionOutOfStep(unsigned short player);
	};
	struct GameFinished : public std::runtime_error
	{
		GameFinished();
		GameFinished(unsigned short player);
	};
}

#endif