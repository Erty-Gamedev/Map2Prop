#pragma once

#include <fstream>
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
		LOG_LOG,
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
		virtual void log(const LogLevel& level, const char* message) = 0;
	};

	class ConsoleHandler : LogHandler
	{
	public:
		using LogHandler::LogHandler;
		using LogHandler::setLevel;
		void log(const LogLevel& level, const char* message) override;
	};
	class FileHandler : LogHandler
	{
	private:
		bool m_fileError = false;
		bool m_logdirChecked = false;
		bool m_logfileChecked = false;
		std::filesystem::path m_logdir;
		std::ofstream m_logfile;
	public:
		FileHandler(const std::filesystem::path& logdir, const LogLevel& loglevel);
		FileHandler() : FileHandler("logs", LogLevel::LOG_WARNING) {}
		~FileHandler();
		using LogHandler::setLevel;
		void setLogDir(const std::filesystem::path& logdir);
		void log(const LogLevel& level, const char* message);
	};

	class Logger
	{
	public:
		static inline const std::map<LogLevel, const char*> s_logLevelName{
			{LogLevel::LOG_DEBUG,   "DEBUG:   "},
			{LogLevel::LOG_LOG,     ""         },
			{LogLevel::LOG_INFO,    "INFO:    "},
			{LogLevel::LOG_WARNING, "WARNING: "},
			{LogLevel::LOG_ERROR,   "ERROR:   "},
		};
		static Logger& getLogger(const std::string& loggerName);

		Logger(const std::string& name);
		Logger() : Logger("logger") {}
		Logger(const Logger&) = delete;

		void debug(const char* message);
		void log(const char* message);
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
		static inline FileHandler s_defaultFileHandler;

		std::string m_name;
		LogLevel m_loglevel = DEFAULT_LOG_LEVEL;
		ConsoleHandler& m_consoleHandler = s_defaultConsoleHandler;
		FileHandler& m_fileHandler = s_defaultFileHandler;

		void _log(const LogLevel& level, const char* message);
	};


	namespace Styling
	{
		static inline const char* RESET = "\033[0m";

		static inline const char* FG_BLACK =	"\033[30m";
		static inline const char* FG_RED =		"\033[31m";
		static inline const char* FG_GREEN =	"\033[32m";
		static inline const char* FG_YELLOW =	"\033[33m";
		static inline const char* FG_BLUE =		"\033[34m";
		static inline const char* FG_MAGENTA =	"\033[35m";
		static inline const char* FG_CYAN =		"\033[36m";
		static inline const char* FG_WHITE =	"\033[37m";

		static inline const char* FG_BRIGHT_BLACK =		"\033[90m";
		static inline const char* FG_BRIGHT_RED =		"\033[91m";
		static inline const char* FG_BRIGHT_GREEN =		"\033[92m";
		static inline const char* FG_BRIGHT_YELLOW =	"\033[93m";
		static inline const char* FG_BRIGHT_BLUE =		"\033[94m";
		static inline const char* FG_BRIGHT_MAGENTA =	"\033[95m";
		static inline const char* FG_BRIGHT_CYAN =		"\033[96m";
		static inline const char* FG_BRIGHT_WHITE =		"\033[97m";

		static inline const char* STYLE_BOLD =			"\033[1m"; // works
		static inline const char* STYLE_DIM =			"\033[2m";
		static inline const char* STYLE_ITALIC =		"\033[3m";
		static inline const char* STYLE_UNDERLINE =		"\033[4m"; // works
		static inline const char* STYLE_BLINKING =		"\033[5m";
		static inline const char* STYLE_REVERSE =		"\033[6m";
		static inline const char* STYLE_HIDDEN =		"\033[8m";
		static inline const char* STYLE_STRIKETHROUGH =	"\033[8m";
	}

}
