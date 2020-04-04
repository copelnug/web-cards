#include "Exception.hpp"

// TODO Use std::format
// TODO Store the offending player in the exception

Cards::IllegalChoice::IllegalChoice(unsigned short player) :
	runtime_error{"Player {} made illegal choice"}
{}
Cards::NotPlayerTurn::NotPlayerTurn(unsigned short wrongPlayer) :
	runtime_error{"Not player {} made illegal choice"}
{}
Cards::ActionOutOfStep::ActionOutOfStep() :
	runtime_error{"Player {} tried to do an action out of step"}
{}
Cards::ActionOutOfStep::ActionOutOfStep(unsigned short player) :
	ActionOutOfStep{}
{}
Cards::GameFinished::GameFinished() :
	runtime_error{"Player {} tried to play after the game finished"}
{}
Cards::GameFinished::GameFinished(unsigned short player) :
	GameFinished{}
{}