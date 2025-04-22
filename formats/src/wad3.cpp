#include "utils.h"
#include "config.h"
#include "logging.h"
#include "wad3.h"


static inline Logging::Logger& logger = Logging::Logger::getLogger("wad3reader");
using M2PConfig::g_config;

using namespace M2PWad3;
using namespace M2PUtils;


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
std::string Wad3Reader::getFilename() const
{
	return m_filepath.filename().string();
}
bool Wad3Reader::containsTexture(const std::string& textureName)
{
	return m_textures.contains(textureName);
}
