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

using Logging::g_isVirtual;
bool g_isVirtual = enableVirtualTerminal();


using namespace Logging;


Logger::Logger(const std::string& name)
{
    m_name = name;
    m_loglevel = DEFAULT_LOG_LEVEL;
}

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

FileHandler::FileHandler(const std::filesystem::path& logdir, const LogLevel& loglevel)
{
    m_loglevel = loglevel;
    m_logdir = logdir;
}
Logging::FileHandler::~FileHandler()
{
    if (m_logfile.is_open())
        m_logfile.close();
}
void FileHandler::setLogDir(const std::filesystem::path& logdir) { m_logdir = logdir; }
