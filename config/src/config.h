#pragma once

#include <string>
#include <filesystem>


namespace M2PConfig
{
    struct Config
    {
        std::string input;
        std::string output;
        std::string gameConfig;
        std::string studiomdl;
        std::string wadList;
        std::string outputName;
        bool autocompile;
        bool mapcompile;
        bool renameChrome;
        int wadCache;
        float smoothing;
        float timeout;
        float qcScale;
        float qcGamma;
        float qcRotate;
        float qcOffset[3];
        std::filesystem::path inputFilepath;
        std::filesystem::path inputDir;
        std::filesystem::path outputDir;
    };

    extern Config config;

    void handleArgs(int argc, char** argv);
}
