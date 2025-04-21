#include <iostream>
#include <filesystem>
#include "logging.h"
#include "config.h"

using Logging::Logger;


int main(int argc, char** argv)
{
    M2PConfig::handleArgs(argc, argv);
    using M2PConfig::g_config;

    if (!g_config.input.empty())
        std::cout << g_config.input << std::endl;

    std::cout << Logging::LogLevelName[Logging::loglevel_console] << std::endl;
}
