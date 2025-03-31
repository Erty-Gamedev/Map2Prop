#pragma once

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

	extern LogLevel loglevel_console;
	extern LogLevel loglevel_file;

	static std::map<LogLevel, const char*> LogLevelName{
		{LogLevel::LOG_DEBUG, "DEBUG"},
		{LogLevel::LOG_INFO, "INFO"},
		{LogLevel::LOG_WARNING, "WARNING"},
		{LogLevel::LOG_ERROR, "ERROR"},
	};

	void log(Logging::LogLevel level, const std::string& message);
	void debug(const std::string& message);
	void info(const std::string& message);
	void warning(const std::string& message);
	void error(const std::string& message);
}
