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
	std::vector<std::string> split(const std::string& str, const char delimiter = ' ');
	void trim(std::string& str, const char* trim = c_DEFAULT_TRIM);
	bool strToBool(std::string str);
	void replaceToken(std::string& str, const std::string& token, const std::string& value);

	template<typename T>
	bool contains(const std::vector<T>& vect, const T& needle)
	{
		return std::find(vect.begin(), vect.end(), needle) != vect.end();
	}
	template<typename T>
	void extendVector(std::vector<T>& first, const std::vector<T>& second)
	{
		first.reserve(second.size());
		first.insert(first.end(), second.begin(), second.end());
	}

	template<typename T>
	void extendVectorUnique(std::vector<T>& first, const std::vector<T>& second)
	{
		for (const T& item : second)
		{
			if (!contains(first, item))
				first.push_back(item);
		}
	}

}
