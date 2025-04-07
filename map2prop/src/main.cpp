#include <iostream>
#include <filesystem>
#include "logging.h"
#include "config.h"

using Logging::Logger;


int main(int argc, char** argv)
{
    Logger& logger = Logger::getLogger("map2prop");
    logger.setLevel(Logging::LogLevel::LOG_DEBUG);
    logger.debug(("Hello world from " + logger.getName()).c_str());

    logger.info("Checking args...");

    M2PConfig::handleArgs(argc, argv);
    using M2PConfig::config;

    if (!config.input.empty())
        std::cout << config.input << std::endl;

    std::cout << Logging::LogLevelName[Logging::loglevel_console] << std::endl;
}
