#pragma once

#include <string>
#include <array>
#include <filesystem>
#include <fstream>
#include "configfile.h"


namespace M2PConfig
{
    static inline const std::array<std::string, 1> c_SUPPORTED_FORMATS{
        ".map"/*, ".obj", ".rmf", ".jmf", ".ol"*/
    };

    struct Config
    {
        std::string input;
        std::string output;
        std::string gameConfig;
        std::string outputName;
        std::string game;
        std::string mod;
        std::string extension;
        bool autocompile = true;
        bool mapcompile = false;
        bool renameChrome = false;
        bool eager = false;
        int wadCache = 10;
        float smoothing = 60.0f;
        float timeout = 60.0f;
        float qcScale = 1.0f;
        float qcGamma = 1.8f;
        float qcRotate = 270.0f;
        float qcOffset[3] = { 0.0f, 0.0f, 0.0f };
        std::filesystem::path exeDir;
        std::filesystem::path inputFilepath;
        std::filesystem::path inputDir;
        std::filesystem::path outputDir;
        std::filesystem::path steamDir;
        std::filesystem::path studiomdl;
        std::vector<std::filesystem::path> wadList;

        bool isMap() const { return extension == ".map"; }
        bool isObj() const { return extension == ".obj"; }
        bool isRmf() const { return extension == ".rmf"; }
        bool isJmf() const { return extension == ".jmf"; }
        bool isOl()  const { return extension == ".ol"; }

    };
    extern Config g_config;

    void handleArgs(int, char**);

    std::filesystem::path gameDir();
    std::filesystem::path modDir();
    std::filesystem::path extractDir();
    bool isMap();
    bool isObj();
}
