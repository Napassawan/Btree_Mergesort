#include "mygetopt.hpp"

#include <algorithm>

// Modified from https://stackoverflow.com/a/868894

OptParse::OptParse(int argc, char** argv)
{
	for (int i = 1; i < argc; ++i)
		tokens.push_back(std::string(argv[i]));
}

std::optional<std::reference_wrapper<const std::string>>
OptParse::GetOptionParam(const std::string& opt) const
{
	auto itr = std::find(tokens.begin(), tokens.end(), opt);
	auto itrNext = std::next(itr);
	
	if (itr != tokens.end() && itrNext != tokens.end()) {
		return *itrNext;
	}
	
	return {};
}

bool OptParse::OptionExists(const std::string& opt) const
{
	auto itr = std::find(tokens.begin(), tokens.end(), opt);
	return itr != tokens.end();
}
