#pragma once

#include <fstream>
#include <filesystem>
#include <string>
#include <map>

namespace M2PConfig
{
    class ConfigFile
    {
    public:
        ConfigFile(const std::filesystem::path& configFile = "config.ini");
        ~ConfigFile();

        void setGameConfig(const std::string& gameConfig);
        std::string getConfig(const std::string& key) const;
        std::vector<std::filesystem::path>& getWadList();
    private:
        std::filesystem::path m_filepath;
        std::ifstream m_file;
        std::string m_currentGameConfig;
        std::map<std::string, std::map<std::string, std::string>> m_gameConfigs;
        std::map<std::string, std::vector<std::filesystem::path>> m_wadLists;

        bool createConfigFile();
        void replaceToken(std::string&) const;
    };
}
