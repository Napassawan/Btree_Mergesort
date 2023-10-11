#pragma once

#include <string>
#include <vector>
#include <optional>

class OptParse {
private:
	std::vector<std::string> tokens;
public:
	OptParse (int argc, char** argv);
	
	std::optional<std::reference_wrapper<const std::string>> GetOptionParam(const std::string& opt) const;
	bool OptionExists(const std::string& opt) const;
};