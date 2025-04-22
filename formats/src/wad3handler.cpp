#include "config.h"
#include "logging.h"
#include "utils.h"
#include "bmp8bpp.h"
#include "wad3handler.h"


static inline Logging::Logger& logger = Logging::Logger::getLogger("wad3reader");

using M2PConfig::g_config;
using namespace M2PWad3;
using namespace M2PUtils;


ImageInfo::ImageInfo(const std::pair<int, int>& size)
{
	this->width = size.first;
	this->height = size.second;
}
ImageInfo::ImageInfo(const std::string& textureName)
{
	std::filesystem::path textureFile = g_config.outputDir / (textureName + ".bmp");
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


ImageInfo& Wad3Handler::s_getImageInfo(const std::string& textureName)
{
	if (s_images.contains(textureName))
		return s_images[textureName];

	s_images.insert(std::pair{ textureName, textureName });
	return s_images[textureName];
}

std::vector<std::filesystem::path>& Wad3Handler::getWadList()
{
	if (!m_wadpathList.empty())
		return m_wadpathList;

	if (!g_config.wadList.empty())
	{
		M2PUtils::extendVectorUnique(m_wadpathList, g_config.wadList);
	}

	return m_wadpathList;
}

Wad3Reader& Wad3Handler::getWad3Reader(const std::filesystem::path& wad)
{
	if (!m_wadCache.contains(wad))
	{
		// Remove the first element before inserting a new element if cache is full
		if (g_config.wadCache > 0 && m_wadCache.size() >= g_config.wadCache)
			m_wadCache.erase(m_wadCache.begin());
		m_wadCache.insert(std::pair(wad, wad));
	}
	return m_wadCache[wad];
}

Wad3Reader* Wad3Handler::checkWads(const std::string& textureName)
{
	std::vector<std::filesystem::path> wadList = getWadList();
	for (const std::filesystem::path& wad : wadList)
	{
		Wad3Reader& reader = getWad3Reader(wad);
		if (reader.containsTexture(textureName))
		{
			if (!contains(m_usedWads, wad))
				m_usedWads.push_back(wad);

		}
	}
	return nullptr;
}

ImageInfo& Wad3Handler::checkTexture(const std::string& textureName)
{
	if (s_images.contains(textureName))
		return s_images[textureName];

	if (isSkipTexture(textureName) || isToolTexture(textureName))
	{
		s_images.insert(std::pair{ textureName, std::pair(16, 16) });
		return Wad3Handler::s_images[textureName];
	}

	std::string textureFile = textureName + ".bmp";

	if (std::filesystem::exists(g_config.outputDir / textureFile))
		return s_getImageInfo(textureName);

	if (std::filesystem::exists(g_config.inputDir / textureFile))
	{
		std::filesystem::copy_file(g_config.inputDir / textureFile, g_config.outputDir / textureFile);
		return s_getImageInfo(textureName);
	}

	Wad3Reader* reader = checkWads(textureName);

	if (reader == nullptr)
	{
		logger.error("Could not find nor extract texture \"" + textureName
			+ "\" from any .wad packages. Please place a .wad package "
			"containing the texture in the input directory or the chosen game directory "
			"and re-run the application.");
		exit(EXIT_FAILURE);
	}

	logger.info("Extracting " + textureName + " from " + reader->getFilename());

	ImageInfo* info = new ImageInfo();
	return *info;
}
bool Wad3Handler::isToolTexture(const std::string& textureName)
{
	return contains(c_TOOLTEXTURES, toLowerCase(textureName));
}
bool Wad3Handler::isSkipTexture(const std::string& textureName)
{
	return contains(c_SKIPTEXTURES, toLowerCase(textureName));
}

bool Wad3Handler::hasMissingTextures() const { return m_missingTextures; }
