#pragma once

#include <string>
#include <sstream>
#include <algorithm>
#include <vector>

namespace M2PUtils
{
	static inline const char* c_DEFAULT_TRIM = " \t";

	std::string toLowerCase(std::string);
	std::string toUpperCase(std::string);
	std::vector<std::string> split(std::string&, const char = ' ');
	void trim(std::string&, const char* = c_DEFAULT_TRIM);
	bool strToBool(std::string);
	void replaceToken(std::string&, const std::string&, const std::string&);

	template<typename T>
	bool contains(std::vector<T> vect, T needle)
	{
		return std::find(vect.begin(), vect.end(), needle) != vect.end();
	}
}
