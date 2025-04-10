#include "logging.h"
#include <wchar.h>
#include <windows.h>


/**
 * Check if we can enable virtual terminal (needed for ANSI escape sequences)
 * From: https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences#example-of-enabling-virtual-terminal-processing
 */
static bool enableVirtualTerminal()
{
    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    DWORD dwOriginalOutMode = 0;
    DWORD dwOriginalInMode = 0;
    if (!GetConsoleMode(hOut, &dwOriginalOutMode))
    {
        return false;
    }
    if (!GetConsoleMode(hIn, &dwOriginalInMode))
    {
        return false;
    }

    DWORD dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
    DWORD dwRequestedInModes = ENABLE_VIRTUAL_TERMINAL_INPUT;

    DWORD dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
    if (!SetConsoleMode(hOut, dwOutMode))
    {
        // We failed to set both modes, try to step down mode gracefully.
        dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
        if (!SetConsoleMode(hOut, dwOutMode))
        {
            // Failed to set any VT mode, can't do anything here.
            return false;
        }
    }

    DWORD dwInMode = dwOriginalInMode | dwRequestedInModes;
    if (!SetConsoleMode(hIn, dwInMode))
    {
        // Failed to set VT input mode, can't do anything here.
        return false;
    }

    return true;
}

static bool isVirtual = enableVirtualTerminal();


using namespace Logging;


Logger::Logger(const std::string& name)
{
    m_name = name;
    m_loglevel = DEFAULT_LOG_LEVEL;
}
void Logger::log(const LogLevel& level, const char* message)
{
    if (level < m_loglevel) { return; }
    m_consoleHandler.log(level, message);
    m_fileHandler.log(level, message);
}
void Logger::debug(const char* message) { log(LogLevel::LOG_DEBUG, message); }
void Logger::info(const char* message) { log(LogLevel::LOG_INFO, message); }
void Logger::warning(const char* message) { log(LogLevel::LOG_WARNING, message); }
void Logger::warn(const char* message) { Logger::warning(message); }
void Logger::error(const char* message) { log(LogLevel::LOG_ERROR, message); }

std::string Logger::getName() const { return m_name; }

void Logger::setLevel(const LogLevel& loglevel)
{
    m_loglevel = loglevel;
}
void Logger::setConsoleHandlerLevel(const LogLevel& loglevel)
{
    m_consoleHandler.setLevel(loglevel);
}
void Logger::setFileHandlerLevel(const LogLevel& loglevel)
{
    m_fileHandler.setLevel(loglevel);
}


Logger& Logger::getLogger(const std::string& loggerName)
{
    if (Logger::s_loggers.contains(loggerName))
        return Logger::s_loggers[loggerName];

    Logger::s_loggers.insert(std::pair{ loggerName, loggerName });
    return Logger::s_loggers[loggerName];
}



LogHandler::LogHandler() { m_loglevel = LogLevel::LOG_INFO; }
LogHandler::LogHandler(const LogLevel& loglevel) { m_loglevel = loglevel; }
void LogHandler::setLevel(const LogLevel& loglevel) { m_loglevel = loglevel; }

void ConsoleHandler::log(const LogLevel& level, const char* message)
{
    if (level < m_loglevel) { return; }

    std::ostream& os = (level == LogLevel::LOG_DEBUG) ? std::clog : std::cout;
    const char* levelName = Logger::s_logLevelName[level];

    if (!isVirtual)
    {
        os << levelName << ": " << message << std::endl;
        return;
    }

    os << "\033[1m" << levelName << "\033[0m";
    switch (level)
    {
    case LogLevel::LOG_INFO:
        os << "\033[36m";
        break;
    case LogLevel::LOG_WARNING:
        os << "\033[1;33m";
        break;
    case LogLevel::LOG_ERROR:
        os << "\033[1;31m";
        break;
    }

    os << message << "\033[0m" << std::endl;
}

FileHandler::FileHandler(const std::filesystem::path& filepath, const LogLevel& loglevel)
{
    m_loglevel = loglevel;
    m_logdir = filepath;
    m_logfile.open(filepath / "log.txt", std::ios::app);
    if (!m_logfile.is_open() || !m_logfile.good())
    {
        m_logfile.close();
        m_fileError = true;
        std::cerr << "###  Log Error: Could not open log file \""
            << std::filesystem::absolute(m_logdir).string() << "\"  ###" << std::endl;
    }
}
Logging::FileHandler::~FileHandler()
{
    if (m_logfile.is_open())
        m_logfile.close();
}
void FileHandler::setLogDir(const std::filesystem::path& logdir) { m_logdir = logdir; }
void FileHandler::log(const LogLevel& level, const char* message)
{
    if (level < m_loglevel || m_fileError) { return; }

    if (!m_logdirChecked && !std::filesystem::exists(m_logdir) && !std::filesystem::is_directory(m_logdir))
    {
        if (!std::filesystem::create_directories(m_logdir)) {
            std::cerr << "###  Log Error: Could not create log directory \""
                << std::filesystem::absolute(m_logdir).string() << "\"  ###" << std::endl;
            return;
        }
    }
    m_logdirChecked = true;

    m_logfile << Logger::s_logLevelName[level] << message << std::endl;
}
