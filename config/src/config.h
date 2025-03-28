#pragma once

#include <string>
#include <filesystem>

struct Vector3D {
    float x, y, z;
};

namespace Config
{
    extern std::string input;
    extern std::string output;
    extern std::string gameConfig;
    extern std::string studiomdl;
    extern std::string wadList;
    extern std::string outputname;
    extern bool autocompile;
    extern bool mapcompile;
    extern bool renameChrome;
    extern int wadCache;
    extern float smoothing;
    extern float timeout;
    extern float qcScale;
    extern float qcGamma;
    extern float qcRotate;
    extern Vector3D qcOffset;
    extern std::filesystem::path inputPath;

    void handleArgs(int argc, char** argv);
}
