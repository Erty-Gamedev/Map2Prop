#include "bmp8bpp.h"
#include "logging.h"


static inline Logging::Logger& logger = Logging::Logger::getLogger("bmp8bpp");


M2PBmp::BMP8Bpp::BMP8Bpp(int width, int height)
{
	int size = width * height;
	m_header = {
		.filesize = uint32_t(c_BMPSIZEHEADER + size),
		.dataOffset = c_BMPSIZEHEADER
	};
	m_infoHeader = {
		uint32_t(40), uint32_t(width), uint32_t(height),
		uint16_t(1), uint16_t(8), uint32_t(0), uint32_t(size),
		3780, // Horizontal pixels per meter
		3780, // Vertical pixels per meter
		256,  // Number of colours used (always 256 here)
		256   // Number of important colours (always 256 here)
	};
}
M2PBmp::BMP8Bpp::~BMP8Bpp()
{
	if (m_file.is_open())
		m_file.close();
}

bool M2PBmp::BMP8Bpp::save(const std::filesystem::path& filepath)
{
	m_file.open(filepath, std::ios::binary);
	if (!m_file.is_open() || !m_file.good())
	{
		m_file.close();
		logger.error("Could not create file " + filepath.string());
		exit(EXIT_FAILURE);
	}

	m_file.write((char*)&m_header, sizeof(m_header));
	m_file.write((char*)&m_infoHeader, sizeof(m_infoHeader));
	m_file.write((char*)&m_palette[0], c_BMPPALETTESIZE * 4);
	m_file.write((char*)&m_data[0], static_cast<size_t>(m_infoHeader.width) * m_infoHeader.height);

	m_file.close();

	return m_file.good();
}
