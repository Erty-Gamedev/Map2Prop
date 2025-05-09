#include "utils.h"


using namespace M2PUtils;

std::string M2PUtils::toLowerCase(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
		return std::tolower(c);
	});
	return str;
}
std::string M2PUtils::toUpperCase(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
		return std::toupper(c);
	});
	return str;
}

std::vector<std::string> M2PUtils::split(const std::string& str, const char delimiter)
{
	std::istringstream strStream{ str };
	std::vector<std::string> segments;
	std::string segment;
	while (std::getline(strStream, segment, delimiter))
	{
		segments.push_back(segment);
	}
	return segments;
}

void M2PUtils::trim(std::string& str, const char* trim)
{
	str.erase(0, str.find_first_not_of(trim));
	str.erase(str.find_last_not_of(trim) + 1);
}

bool M2PUtils::strToBool(std::string str)
{
	str = toLowerCase(str);
	return !str.starts_with('n') && !str.starts_with('f') && str != "0";
}

void M2PUtils::replaceToken(std::string& str, const std::string& token, const std::string& value)
{
	while (str.find(token) != std::string::npos)
		str.replace(str.find(token), token.length(), value);
}
