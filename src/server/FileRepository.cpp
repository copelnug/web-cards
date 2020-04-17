#include "FileRepository.hpp"

#include <fstream>

namespace
{
	using FileList = std::map<std::string, std::filesystem::path>;

	FileList load(const std::filesystem::path& source)
	{
		FileList result;
		std::ifstream input{source};
		if(!input.is_open())
			throw std::runtime_error{"TODO Cannot open file repository source"};

		std::string line;
		while(std::getline(input, line))
		{
			auto pos = line.find(' ');
			if(pos == std::string::npos)
				continue;

			result.emplace(line.substr(0, pos), line.substr(pos+1));
		}

		return result;
	}
}
FileRepository::FileRepository(std::filesystem::path source) :
	source_{std::move(source)}
{
	files_ = load(source_);
}
std::optional<std::filesystem::path> FileRepository::get(const std::string& key)
{
	std::lock_guard<std::mutex> l{mut_};

	auto it = files_.find(key);
	if(it != files_.end())
		return it->second;

	auto newFiles = load(source_);
	if(newFiles.empty())
		return {};

	std::swap(newFiles, files_);

	it = files_.find(key);
	if(it != files_.end())
		return it->second;

	return {};
}
