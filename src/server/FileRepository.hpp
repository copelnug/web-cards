#ifndef SERVER_FILE_REPOSITORY_HPP_INCLUDED
#define SERVER_FILE_REPOSITORY_HPP_INCLUDED
#include <filesystem>
#include <map>
#include <mutex>
#include <optional>
#include <string>

class FileRepository
{
	std::map<std::string, std::filesystem::path> files_;
	std::filesystem::path source_;
	std::mutex mut_;

public:
	FileRepository(std::filesystem::path source);

	std::optional<std::filesystem::path> get(const std::string& key);
};

#endif