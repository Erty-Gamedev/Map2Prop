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
std::ostream& M2PWad3::operator<<(std::ostream& os, const ImageInfo& info)
{
	os << std::format("ImageInfo({}, {})", info.width, info.height);
	return os;
}


ImageSize Wad3Handler::s_getImageInfo(const std::string& textureName)
{
	if (s_images.contains(textureName))
		return s_images[textureName];

	ImageInfo info{ textureName };
	s_images[textureName] = { info.width, info.height };

	//s_images.insert(std::pair{ textureName, ImageSize(info.width, info.height) });
	return s_images[textureName];
}
std::ostream& M2PWad3::operator<<(std::ostream& os, const ImageSize& size)
{
	os << std::format("ImageSize({}, {})", size.width, size.height);
	return os;
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
	for (const std::filesystem::path& wad : g_config.wadList)
	{
		try
		{
			Wad3Reader& reader = getWad3Reader(wad);
			if (reader.contains(textureName))
			{
				if (!contains(usedWads, wad))
					usedWads.push_back(wad);
				return &reader;
			}
		}
		catch (std::runtime_error&)
		{
			continue;
		}
	}
	return nullptr;
}

ImageSize Wad3Handler::checkTexture(const std::string& textureName)
{
	if (s_images.contains(textureName))
		return s_images[textureName];

	if (isSkipTexture(textureName) || isToolTexture(textureName))
	{
		s_images.insert(std::pair{ textureName, ImageSize(16, 16) });
		return Wad3Handler::s_images[textureName];
	}

	std::string textureFile = textureName + ".bmp";

	Wad3Reader* reader = checkWads(textureName);

	if (std::filesystem::exists(g_config.outputDir / textureFile))
		return s_getImageInfo(textureName);

	if (std::filesystem::exists(g_config.inputDir / textureFile))
	{
		std::filesystem::copy_file(g_config.inputDir / textureFile, g_config.outputDir / textureFile);
		return s_getImageInfo(textureName);
	}

	if (reader == nullptr)
	{
		logger.error("Could not find nor extract texture \"" + textureName
			+ "\" from any .wad packages. Please place a .wad package "
			"containing the texture in the input directory or the chosen game directory "
			"and re-run the application.");
		exit(EXIT_FAILURE);
	}

	logger.info("Extracting " + textureName + " from " + reader->getFilename());

	Wad3MipTex miptex = reader->extract(textureName, g_config.outputDir);

	s_images.insert(std::pair{
		textureName,
		ImageSize(static_cast<int>(miptex.nWidth), static_cast<int>(miptex.nHeight))
	});
	return Wad3Handler::s_images[textureName];
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
