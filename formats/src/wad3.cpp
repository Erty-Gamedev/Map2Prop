
#include "config.h"
#include "logging.h"
#include "wad3.h"
#include "bmp8bpp.h"


static inline Logging::Logger& logger = Logging::Logger::getLogger("wad3reader");
using M2PConfig::config;

using namespace M2PWad3;



static std::string toLowerCase(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
		return std::tolower(c);
	});
	return str;
}
static std::string toUpperCase(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) {
		return std::toupper(c);
	});
	return str;
}


Wad3Reader::Wad3Reader(const std::filesystem::path& filepath)
{
	m_filepath = filepath;
	m_file.open(filepath);
	if (!m_file.is_open() || !m_file.good())
	{
		m_file.close();
		logger.error("Could not open file " + filepath.string());
		exit(EXIT_FAILURE);
	}
}
Wad3Reader::~Wad3Reader()
{
	if (m_file.is_open())
		m_file.close();
}
textureMap Wad3Reader::getTextures() const { return m_textures; }


ImageInfo::ImageInfo(const std::string& textureName)
{
	std::filesystem::path textureFile = config.outputDir / (textureName + ".bmp");
	m_file.open(textureFile, std::ios::binary);
	if (!m_file.is_open() || !m_file.good())
	{
		m_file.close();
		logger.error("Could not find texture " + std::filesystem::absolute(textureFile).string());
		exit(EXIT_FAILURE);
	}
	m_file.seekg(14, std::ios::beg);
	M2PBmp::BMPInfoHeader infoHeader{};
	m_file.read((char*)&infoHeader, sizeof(M2PBmp::BMPInfoHeader));

	width = infoHeader.width;
	height = infoHeader.height;

	m_file.close();
}
ImageInfo::~ImageInfo()
{
	if (m_file.is_open())
		m_file.close();
}

bool Wad3Handler::checkTexture(const std::string& textureName)
{
	return true;
}
bool Wad3Handler::isToolTexture(const std::string& textureName)
{
	return std::find(
		c_TOOLTEXTURES.begin(),
		c_TOOLTEXTURES.end(),
		toLowerCase(textureName)) != c_TOOLTEXTURES.end();
}
bool Wad3Handler::isSkipTexture(const std::string& textureName)
{
	return std::find(
		c_SKIPTEXTURES.begin(),
		c_SKIPTEXTURES.end(),
		toLowerCase(textureName)) != c_SKIPTEXTURES.end();
}

