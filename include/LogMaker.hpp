
#ifndef __LOGMAKER_HPP__
#define __LOGMAKER_HPP__

#include <string>
#include <fstream>
#include <iostream>
#include <mutex>

#include <stdarg.h>

#define DEBUG_MODE		1

#define LOG_TRACE	   -1
#define LOG_DEBUG		0
#define LOG_INFO		1
#define LOG_WARNING		2
#define LOG_ERROR		3

class LogMaker
{

private:

	std::string _log_path;
	std::mutex _mutex_print;
	std::ofstream _log;
	int _log_level;

public:

	LogMaker(void) = delete;
	LogMaker(std::string log_path);
	~LogMaker(void);
	LogMaker(const LogMaker &other) = delete;
	LogMaker(const LogMaker &&other) noexcept = delete;
	LogMaker &operator=(const LogMaker &other) = delete;
	LogMaker &operator=(LogMaker &&other) noexcept = delete;

	void put_log(int define_event, char const *msg);
	void printf_log(int event_cost, const char *fmt, ...);
	void set_log_level(int level);

};

#endif /* __LOGMAKER_HPP__ */