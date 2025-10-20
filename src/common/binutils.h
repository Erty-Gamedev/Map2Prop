#pragma once

#include <string>
#include <fstream>

namespace M2PBinUtils
{
	char readChar(std::ifstream& file);
	void readChar(std::ifstream& file, char *buffer);
	unsigned char readByte(std::ifstream& file);
	std::int32_t readInt(std::ifstream& file);
	float readFloat(std::ifstream& file);
	std::string readNTString(std::ifstream& file, size_t length);
	std::string readLPString(std::ifstream& file);
	std::string readIntLPString(std::ifstream& file);
}