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
