#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include "configfile.h"


namespace M2PConfig
{
    static inline const char* USAGE =
#ifdef _WIN32
    "Usage: Map2Prop input [options]\n"
#else
    "Usage: map2prop input [options]\n"
#endif
    R"USAGE(Converts a .map/.rmf/.jmf/.ol or J.A.C.K .obj into GoldSrc .smd files for model creation.

Arguments:
  input                 .map/.rmf/.jmf/.obj/.ol file to convert
  -v | --version        print current version
  -h | --help           show this help message and exit

Options:
  -c | --mapcompile     modify .map input to replace func_map2prop with model entities after compile
  -a | --noautocompile  do not automatically compile the model after conversion
  -o | --output         specify an output directory
  -g | --gameconfig     game config to use from config.ini
  -m | --studiomdl      path to SC studiomdl.exe
  -w | --wadlist        path to text file listing .wad files
  -n | --wadcache       max number of .wad files to keep in memory
  -s | --smoothing      angle threshold for applying smoothing (use 0 to smooth all edges)
  -t | --time           timeout for running studiomdl.exe (default 60.0 seconds)
  --verbose             enable verbose logging
  --renamechrome        rename chrome textures (disables chrome)
  --eager               use eager triangulation algorithm (faster)

QC options:
  --outputname          filename for the finished model
  --scale               scale the model by this amount (default 1.0)
  --gamma               darken/brighten textures (default 1.8)
  --offset x y z        X Y Z offset to apply to the model (default 0 0 0)
  --rotate              rotate the model by this many degrees
)USAGE";


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
