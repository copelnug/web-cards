#ifndef SERVER_HELPER_HPP_INCLUDED
#define SERVER_HELPER_HPP_INCLUDED
#include <boost/property_tree/ptree.hpp>
#include <sstream>
#include <string>

constexpr const char* TRAD(const char* msg)
{
	// TODO
	return msg;
}

template <typename ...T>
std::string TRAD(T&&... val)
{
	std::ostringstream out;
	(out <<  ... << val);
	return std::move(out).str();
}

namespace Serialize
{
	static constexpr const char* const MSG_ENTRY_TYPE = "type";

	boost::property_tree::ptree statusNode(const std::string& message);
	std::string helper(const boost::property_tree::ptree& msg);
	std::string helperStatus(const std::string& msg);

	std::string hostStart();
	std::string askUsername(const std::string& current = "");

	// Error message
	std::string illegalChoice();
	std::string actionOutOfStep();
	std::string notPlayerTurn();

	// Waiting
	std::string waitingStart(const std::string& username);
	std::string waitingHost();
	std::string endGame();
}

#endif