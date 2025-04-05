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
        std::string outputname;
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
        std::filesystem::path inputPath;
    };

    extern Config config;

    void handleArgs(int argc, char** argv);
}
