#pragma once
#include <string>

namespace StringFunction {

	std::string replaceString(std::string subject, const std::string &search, const std::string &replace);
	std::string replaceFirstCapital(std::string& str);
}
