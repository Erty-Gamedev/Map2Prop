#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include "configfile.h"


namespace M2PConfig
{
    struct Config
    {
        std::string input;
        std::string output;
        std::string gameConfig;
        std::string outputName;
        std::string game;
        std::string mod;
        bool autocompile = true;
        bool mapcompile = false;
        bool renameChrome = false;
        int wadCache = 10;
        float smoothing = 60.0f;
        float timeout = 60.0f;
        float qcScale = 1.0f;
        float qcGamma = 1.8f;
        float qcRotate = 0.0f;
        float qcOffset[3] = { 0.0f, 0.0f, 0.0f };
        std::filesystem::path inputFilepath;
        std::filesystem::path inputDir;
        std::filesystem::path outputDir;
        std::filesystem::path steamDir;
        std::filesystem::path studiomdl;
        std::vector<std::filesystem::path> wadList;
    };
    extern Config g_config;

    void handleArgs(int, char**);

    std::filesystem::path gameDir();
    std::filesystem::path modDir();
}
