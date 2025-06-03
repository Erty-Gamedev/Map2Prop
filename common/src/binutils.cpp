#include <vector>
#include "binutils.h"

using namespace M2PBinUtils;


char M2PBinUtils::readChar(std::ifstream& file)
{
	char buffer{};
	file.read(&buffer, sizeof(char));
	return buffer;
}

void M2PBinUtils::readChar(std::ifstream& file, char* buffer)
{
	file.read(buffer, sizeof(char));
}

unsigned char M2PBinUtils::readByte(std::ifstream& file)
{
	unsigned char buffer{};
	file.read(reinterpret_cast<char*>(&buffer), sizeof(unsigned char));
	return buffer;
}

std::int32_t M2PBinUtils::readInt(std::ifstream& file)
{
    std::int32_t buffer{};
    file.read(reinterpret_cast<char*>(&buffer), sizeof(std::int32_t));
    return buffer;
}

float M2PBinUtils::readFloat(std::ifstream& file)
{
	float buffer{};
	file.read(reinterpret_cast<char*>(&buffer), sizeof(float));
	return buffer;
}

std::string M2PBinUtils::readNTString(std::ifstream& file, size_t length)
{
	if (length < 1)
		return "";
	std::vector<char> bytes;
	bytes.assign(length, '\0');
	file.read(reinterpret_cast<char*>(bytes.data()), length);
	return std::string{ bytes.data() };
}

std::string M2PBinUtils::readLPString(std::ifstream& file)
{
	unsigned char length = readByte(file);
	return readNTString(file, length);
}

std::string M2PBinUtils::readIntLPString(std::ifstream& file)
{
	std::int32_t length = readInt(file);
	return readNTString(file, length);
}
