#include "UserRepository.hpp"

#include <fstream>
#include <stdexcept>

UserRepository::UserRepository()
{
	std::ifstream input{"users.txt"};
	if(!input.is_open())
		throw std::runtime_error{"TODO Cannot open user repository file"};

	std::string line;
	while(std::getline(input, line))
	{
		auto pos = line.find(' ');
		if(pos == std::string::npos)
			continue;

		users_.emplace(line.substr(0, pos), line.substr(pos+1));
	}
}
bool UserRepository::checkUser(const std::string& user, const std::string& password)
{
	// TODO Implement much better logic. At minimum, we need hashing with a salt
	auto it = users_.find(user);
	if(it == users_.end())
		return false;

	return it->second == password;
}