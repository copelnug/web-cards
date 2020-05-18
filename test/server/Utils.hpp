#ifndef TEST_SERVER_UTILS_HPP_INCLUDED
#define TEST_SERVER_UTILS_HPP_INCLUDED
#include <catch.hpp>

namespace Test
{
	class StrEqualIgnoreSpaces : public Catch::MatcherBase<std::string>
	{
		std::string expected_;
	public:
		StrEqualIgnoreSpaces(std::string expected) :
			expected_{std::move(expected)}
		{}

		bool match(const std::string& text) const override
		{
			size_t i = 0;
			size_t j = 0;

			for(;;++i, ++j)
			{
				while(i < text.size() && std::isspace(text[i]))
					++i;
				while(j < expected_.size() && std::isspace(expected_[j]))
					++j;

				if(i == text.size() && j == expected_.size())
					return true;
				if(i == text.size() || j == expected_.size())
					return false;

				if(text[i] != expected_[j])
					return false;
			}
		}

		virtual std::string describe() const override
		{
			return "match the JSON (ignoring formatting).";
		}
	};
}
#endif