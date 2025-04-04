#include <iostream>
#include <filesystem>
#include "logging.h"
#include "config.h"


int main(int argc, char** argv)
{
    Logging::debug("Hello World!");
    Logging::info("Checking args...");

    M2PConfig::handleArgs(argc, argv);
    using M2PConfig::config;

    if (!config.input.empty())
        std::cout << config.input << std::endl;

    std::cout << Logging::LogLevelName[Logging::loglevel_console] << std::endl;
}
