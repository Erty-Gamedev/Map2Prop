#include "utils.h"
#include "config.h"
#include "logging.h"
#include "wad3.h"
#include "bmp8bpp.h"


static inline Logging::Logger& logger = Logging::Logger::getLogger("wad3reader");
using M2PConfig::g_config;

using namespace M2PWad3;
using namespace M2PUtils;
using namespace M2PBmp;


Wad3Reader::Wad3Reader(const std::filesystem::path& filepath)
{
	m_filepath = filepath;
	open();

	Wad3Header header{};
	m_file.read(reinterpret_cast<char*>(&header), sizeof(Wad3Header));

	if (strncmp(header.szMagic, "WAD3", 4))
	{
		m_file.close();
		logger.warning("Invalid file type: \""
			+ std::filesystem::absolute(filepath).string() + "\" is not a valid WAD3 package.");
		throw std::runtime_error("Invalid file type");
	}

	m_file.seekg(header.nDirOffset, std::ios::beg);
	m_dirEntries.assign(header.nDir, {});
	for (int i = 0; i < header.nDir; ++i)
		m_file.read(reinterpret_cast<char*>(&(m_dirEntries[i])), sizeof(Wad3DirEntry));

	m_file.close();
}
Wad3Reader::~Wad3Reader()
{
	if (m_file.is_open())
		m_file.close();
}
void M2PWad3::Wad3Reader::open()
{
	m_file.open(m_filepath, std::ios::binary);
	if (!m_file.is_open() || !m_file.good())
	{
		m_file.close();
		logger.error("Could not open file " + m_filepath.string());
		exit(EXIT_FAILURE);
	}
}
std::string Wad3Reader::getFilename() const
{
	return m_filepath.filename().string();
}
Wad3DirEntry* Wad3Reader::getDirEntry(const std::string& textureName)
{
	for (Wad3DirEntry& dirEntry : m_dirEntries)
	{
		if (M2PUtils::toLowerCase(textureName) == M2PUtils::toLowerCase(dirEntry.szName))
			return &dirEntry;
	}
	return nullptr;
}
bool Wad3Reader::contains(const std::string& textureName)
{
	return getDirEntry(textureName) != nullptr;
}
Wad3MipTex Wad3Reader::extract(const std::string& textureName, const std::filesystem::path& outdir)
{
	// Read texture data from WAD

	Wad3DirEntry* dirEntry = getDirEntry(textureName);
	if (dirEntry == nullptr)
	{
		throw std::runtime_error("Could not extract \"" + textureName + "\" from " + getFilename());
	}

	if (dirEntry->nType != EntryType::MIPTEX)
	{;
		throw std::runtime_error("Texture \"" + textureName + "\" is not a MipTex type");
	}

	open();

	m_file.seekg(dirEntry->nFilePos);

	Wad3MipTex miptex{};
	m_file.read((char*)&miptex, sizeof(Wad3MipTex));

	size_t width = miptex.nWidth;
	size_t height = miptex.nHeight;
	size_t textureSize = width * height;
	std::vector<unsigned char> data(textureSize, {});

	m_file.seekg(dirEntry->nFilePos + miptex.nOffsets[0]);

	m_file.read((char*)data.data(), textureSize);  // Read mipmap 0

	m_file.seekg((width >> 1) * (height >> 1), std::ios::cur);  // Skip mipmap 1
	m_file.seekg((width >> 2) * (height >> 2), std::ios::cur);  // Skip mipmap 2
	m_file.seekg((width >> 3) * (height >> 3), std::ios::cur);  // Skip mipmap 3
	m_file.seekg(sizeof(int16_t), std::ios::cur); // Skip colours used (always 256 here)

	unsigned char palette[c_PALETTESIZE]{};
	m_file.read((char*)&palette[0], c_PALETTESIZE);

	m_file.close();


	// Prepare data for BMP

	BMP8Bpp bmp(static_cast<int>(width), static_cast<int>(height));
	bmp.m_data = std::vector<unsigned char>(textureSize);

	// Vertically flip data

	size_t currentPos = (height - 1) * width;
	for (int i = 0; i < height; ++i)
	{
		std::copy_n(&data[currentPos], width, &bmp.m_data[width * i]);
		currentPos -= width;
	}

	// Convert palette from RGB to BGRA

	bmp.m_palette = std::vector<unsigned char>(c_BMPPALETTESIZE * 4);
	BGRA bgra;
	for (int i = 0, j = 0; i < c_BMPPALETTESIZE; ++i)
	{
		bgra = {
			palette[i * 3 + 2],
			palette[i * 3 + 1],
			palette[i * 3],
			0x00
		};
		std::copy_n((char*) &bgra, sizeof(BGRA), &bmp.m_palette[i * sizeof(BGRA)]);
	}


	// Save BMP

	std::filesystem::path filepath = outdir / (std::string{ miptex.szName } + ".bmp");
	bmp.save(filepath);

	return miptex;
}
