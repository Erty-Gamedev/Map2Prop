#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include "configfile.h"


namespace M2PConfig
{
    enum Extension
    {
        MAP,
        RMF,
        JMF,
        OBJ,
        OL
    };

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
        bool eager = false;
        int wadCache = 10;
        float smoothing = 60.f;
        float timeout = 60.f;
        float clipThreshold = 4.f;
        float qcScale = 1.f;
        float qcGamma = 1.8f;
        float qcRotate = 270.f;
        float qcOffset[3] = { 0.f, 0.f, 0.f };
        Extension extension;
        std::filesystem::path exeDir;
        std::filesystem::path inputFilepath;
        std::filesystem::path inputDir;
        std::filesystem::path outputDir;
        std::filesystem::path steamDir;
        std::filesystem::path studiomdl;
        std::vector<std::filesystem::path> wadList;

        bool isMap() const { return extension == Extension::MAP; }
        bool isObj() const { return extension == Extension::OBJ; }
        bool isRmf() const { return extension == Extension::RMF; }
        bool isJmf() const { return extension == Extension::JMF; }
        bool isOl()  const { return extension == Extension::OL; }

        std::filesystem::path gameDir() const;
        std::filesystem::path modDir() const;
        std::filesystem::path extractDir() const;
    };
    extern Config g_config;

    void handleArgs(int, char**);
}
