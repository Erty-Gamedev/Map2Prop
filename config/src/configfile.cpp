#include "config.h"
#include "configfile.h"
#include "logging.h"
#include "utils.h"


using M2PConfig::g_config;

static Logging::Logger& logger = Logging::Logger::getLogger("config");

// TODO: TOML style arrays?


M2PConfig::ConfigFile::ConfigFile(const std::filesystem::path& configFile)
{
    m_filepath = g_config.exeDir / configFile;
#ifdef _DEBUG
    if (!std::filesystem::exists(m_filepath))
        m_filepath = configFile;
#endif
    m_gameConfigs = {};
    m_wadLists = {};
    m_gameConfigs["default"] = {};
    m_wadLists["default"] = {};

    m_file.open(m_filepath, std::ios::in);
    if (!m_file.is_open() || !m_file.good())
    {
        m_file.close();
        logger.info("Could not read config file \"" + std::filesystem::absolute(m_filepath).string() + "\", creating default config file...");

        m_filepath = g_config.exeDir / configFile;
        if (!createConfigFile())
        {
            logger.warning("Could not create default config file \"" + m_filepath.string() + "\"");
            return;
        }
    }


    std::string line, key, value;
    std::string currentConfig = "default";
    line.reserve(1024);
    int lineNumber = 0;
    while (std::getline(m_file, line))
    {
        ++lineNumber;

        // Skip comments and empty lines
        if (line.starts_with("//") || line.starts_with(";")
            || line.starts_with("#") || line.empty())
            continue;

        const std::vector<std::string>& parts = M2PUtils::split(line, '=');
        if (parts.empty())
        {
            logger.warning(std::format(
                "Malformed property at line {} in {}", lineNumber, m_filepath.string()));
            continue;
        }

        key = parts.at(0);
        M2PUtils::trim(key);

        if (key.starts_with('['))
        {
            currentConfig = key.substr(key.find('[') + 1, key.find(']') - 1);
            continue;
        }

        value = (parts.size() > 1) ? parts.at(1) : "";
        M2PUtils::trim(value);
        replaceToken(value);

        // Ignore empty values
        if (value.empty())
            continue;

        if (key == "wad list")
        {
            if (value.ends_with(','))
                value = value.substr(0, value.size() - 1);

            m_wadLists[currentConfig].emplace_back(value);
            while (std::getline(m_file, line))
            {
                ++lineNumber;

                M2PUtils::trim(line);
                if (line.ends_with(','))
                    line = line.substr(0, line.size() - 1);

                if (line.empty() || !line.ends_with(".wad"))
                    break;

                replaceToken(line);

                m_wadLists[currentConfig].emplace_back(line);
            }
        }
        else
        {
            m_gameConfigs[currentConfig][key] = value;
        }
    }
}
M2PConfig::ConfigFile::~ConfigFile()
{
    if (m_file.is_open())
        m_file.close();
}

void M2PConfig::ConfigFile::setGameConfig(const std::string& gameConfig)
{
    m_currentGameConfig = gameConfig;
}
std::string M2PConfig::ConfigFile::getConfig(const std::string& key) const
{
    if (m_gameConfigs.contains(m_currentGameConfig)
        && m_gameConfigs.at(m_currentGameConfig).contains(key))
        return m_gameConfigs.at(m_currentGameConfig).at(key);

    if (!m_gameConfigs.at("default").contains(key))
        return "";

    return m_gameConfigs.at("default").at(key);
}
std::vector<std::filesystem::path>& M2PConfig::ConfigFile::getWadList()
{
    if (m_wadLists.contains(m_currentGameConfig))
        return m_wadLists[m_currentGameConfig];
    return m_wadLists["default"];
}


void M2PConfig::ConfigFile::replaceToken(std::string& str) const
{
    size_t tokenStart = str.find("%(");
    size_t tokenEnd = str.find(")s");

    if (tokenStart == std::string::npos || tokenEnd == std::string::npos)
        return;

    std::string key = str.substr(tokenStart + 2, tokenEnd - tokenStart - 2);
    std::string token = str.substr(tokenStart, tokenEnd - tokenStart + 2);

    std::string value = getConfig(key);
    if (!value.empty())
        M2PUtils::replaceToken(str, token, value);
}

bool M2PConfig::ConfigFile::createConfigFile()
{
    m_file.open(m_filepath, std::ios::out);
    if (!m_file.is_open() || !m_file.good())
    {
        m_file.close();
        return false;
    }

    m_file << R"CONFIG([default]
smoothing threshold = 60.0
rename chrome = no
output directory = converted
steam directory = C:\Program Files (x86)\Steam
game config = halflife
studiomdl = %(steam directory)s\steamapps\common\Sven Co-op SDK\modelling\studiomdl.exe
autocompile = yes
timeout = 60.0
autoexit = no
wad cache = 10
wad list = 
;Example wad list:
;wad list = %(steam directory)s\steamapps\common\Half-Life\valve\halflife.wad,
;           %(steam directory)s\steamapps\common\Half-Life\valve\liquids.wad,

[halflife]
game = Half-Life
mod = valve

[svencoop]
game = Sven Co-op
mod = svencoop

[cstrike]
game = Half-Life
mod = cstrike
)CONFIG";
    m_file.close();

    // Re-open in read
    m_file.open(m_filepath, std::ios::in);
    return m_file.is_open() && m_file.good();
}
