#pragma once

#include <string>
#include <filesystem>

struct Vector3D {
    float x, y, z;
};

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
        Vector3D qcOffset;
        std::filesystem::path inputPath;
    };

    extern Config config;

    void handleArgs(int argc, char** argv);
}
