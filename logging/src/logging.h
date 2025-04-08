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

	class LogHandler
	{
	protected:
		LogLevel m_loglevel;
	public:
		LogHandler();
		LogHandler(const LogLevel& loglevel);
		void setLevel(const LogLevel& loglevel);
		virtual void log(const LogLevel& level, const char* message) const = 0;
	};

	class ConsoleHandler : LogHandler
	{
	public:
		using LogHandler::LogHandler;
		using LogHandler::setLevel;
		void log(const LogLevel& level, const char* message) const override;
	};
	class FileHandler : LogHandler
	{
	private:
		std::filesystem::path m_logdir;
	public:
		FileHandler();
		FileHandler(const std::filesystem::path& logdir, const LogLevel& loglevel);
		using LogHandler::setLevel;
		void setLogDir(const std::filesystem::path& logdir);
		void log(const LogLevel& level, const char* message) const;
	};

	class Logger
	{
	public:
		static inline std::map<LogLevel, const char*> s_logLevelName{
			{LogLevel::LOG_DEBUG,   "DEBUG:   "},
			{LogLevel::LOG_INFO,    "INFO:    "},
			{LogLevel::LOG_WARNING, "WARNING: "},
			{LogLevel::LOG_ERROR,   "ERROR:   "},
		};
		static Logger& getLogger(const std::string& loggerName);

		Logger(const std::string& name)
		{
			m_name = name;
			m_loglevel = DEFAULT_LOG_LEVEL;
			m_consoleHandler = Logger::s_defaultConsoleHandler;
			m_fileHandler = Logger::s_defaultFileHandler;
		}
		Logger() : Logger("logger") {}
		Logger(const Logger&) = delete;

		void log(const LogLevel& level, const char* message);
		void debug(const char* message);
		void info(const char* message);
		void warning(const char* message);
		void warn(const char* message);
		void error(const char* message);

		std::string getName() const;
		void setLevel(const LogLevel& loglevel);
		void setConsoleHandlerLevel(const LogLevel& loglevel);
		void setFileHandlerLevel(const LogLevel& loglevel);
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
