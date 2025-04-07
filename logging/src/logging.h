#pragma once

#include <iostream>
#include <filesystem>
#include <string>
#include <map>



namespace Logging
{
	enum class LogLevel
	{
		LOG_DEBUG,
		LOG_INFO,
		LOG_WARNING,
		LOG_ERROR,
	};

#ifdef _DEBUG
	static inline const LogLevel DEFAULT_LOG_LEVEL = LogLevel::LOG_DEBUG;
#else
	static inline const LogLevel DEFAULT_LOG_LEVEL = LogLevel::LOG_INFO;
#endif

	class ConsoleHandler
	{
	private:
		LogLevel m_loglevel;
	public:
		ConsoleHandler();
		ConsoleHandler(LogLevel loglevel);
		void setLevel(LogLevel loglevel);
		void log(LogLevel level, const char* message) const;
	};
	class FileHandler
	{
	private:
		LogLevel m_loglevel;
		std::filesystem::path m_logdir;
	public:
		FileHandler();
		FileHandler(std::filesystem::path logdir, LogLevel loglevel);
		void setLevel(LogLevel loglevel);
		void setLogDir(std::filesystem::path logdir);
		void log(LogLevel level, const char* message) const;
	};

	class Logger
	{
	public:
		static inline std::map<LogLevel, const char*> s_logLevelName{
			{LogLevel::LOG_DEBUG, "DEBUG"},
			{LogLevel::LOG_INFO, "INFO"},
			{LogLevel::LOG_WARNING, "WARNING"},
			{LogLevel::LOG_ERROR, "ERROR"},
		};
		static Logger& getLogger(std::string loggerName);

		Logger(std::string name)
		{
			m_name = name;
			m_loglevel = DEFAULT_LOG_LEVEL;
			m_consoleHandler = Logger::s_defaultConsoleHandler;
			m_fileHandler = Logger::s_defaultFileHandler;
		}
		Logger() : Logger("logger") {}
		Logger(const Logger&) = delete;

		void log(LogLevel level, const char* message);
		void debug(const char* message);
		void info(const char* message);
		void warning(const char* message);
		void error(const char* message);

		std::string getName();
		void setLevel(LogLevel loglevel);
		void setConsoleHandlerLevel(LogLevel loglevel);
		void setFileHandlerLevel(LogLevel loglevel);
	private:
		static inline std::map<std::string, Logger> s_loggers {};
		static inline ConsoleHandler s_defaultConsoleHandler { DEFAULT_LOG_LEVEL };
		static inline FileHandler s_defaultFileHandler{ "/logs", LogLevel::LOG_WARNING };

		std::string m_name;
		LogLevel m_loglevel = DEFAULT_LOG_LEVEL;
		ConsoleHandler m_consoleHandler;
		FileHandler m_fileHandler;
	};

}
