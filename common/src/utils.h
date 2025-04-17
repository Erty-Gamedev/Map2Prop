#pragma once

#include <string>
#include <algorithm>
#include <vector>

namespace M2PUtils
{
	std::string toLowerCase(std::string);
	std::string toUpperCase(std::string);

	template<typename T>
	bool contains(std::vector<T> vect, T needle)
	{
		return std::find(vect.begin(), vect.end(), needle) != vect.end();
	}
}
