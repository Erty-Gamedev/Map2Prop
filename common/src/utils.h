#pragma once

#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <array>

namespace M2PUtils
{
	static inline const char* c_DEFAULT_TRIM = " \t";
	static inline const std::array<std::string, 6> c_WADSKIPLIST{
		"cached", "decals", "fonts",
		"gfx", "spraypaint", "tempdecal"
	};
	static inline const std::array<std::string, 3> c_STEAMPIPES{
		"_addon", "_hd", "_downloads"
	};

	std::string toLowerCase(std::string str);
	std::string toUpperCase(std::string str);
	std::vector<std::string> split(const std::string& str, const char delimiter = ' ');
	void trim(std::string& str, const char* trim = c_DEFAULT_TRIM);
	bool strToBool(std::string str);
	void replaceToken(std::string& str, const std::string& token, const std::string& value);

	template<typename T>
	bool contains(const std::vector<T>& vect, const T& needle)
	{
		return std::find(vect.begin(), vect.end(), needle) != vect.end();
	}
	template<typename T, size_t n>
	bool contains(const std::array<T, n>& arr, const T& needle)
	{
		return std::find(arr.begin(), arr.end(), needle) != arr.end();
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
