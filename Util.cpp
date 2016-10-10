#include <string>
#include <iostream>
#include "Util.h"

namespace StringFunction {
	std::string replaceString(std::string subject, const std::string &search, const std::string &replace) {
		size_t pos = 0;
		while ((pos = subject.find(search, pos)) != std::string::npos) {
			subject.replace(pos, search.length(), replace);
			pos += replace.length();
		}
		return subject;
	}

	std::string replaceFirstCapital(std::string& str)
	{
		std::string subject = str;
		if (!str.compare(" the "))
		{
			int a = 9;
		}
		if ((subject[0] == ' '))
			subject[1] = subject.at(1) - 32;
		else
		{

		}
		return subject;
	}
}
