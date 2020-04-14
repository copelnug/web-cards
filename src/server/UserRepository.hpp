#ifndef SERVER_USER_REPOSITORY_HPP_INCLUDED
#define SERVER_USER_REPOSITORY_HPP_INCLUDED
#include <map>
#include <string>

class UserRepository
{
	std::map<std::string, std::string> users_;
public:
	UserRepository();

	// TODO Use different string for password?
	bool checkUser(const std::string& user, const std::string& password);
};

#endif