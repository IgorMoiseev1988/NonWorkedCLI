
#include "LogMaker.hpp"

LogMaker::LogMaker(std::string log_path) {
	_log_path = log_path;
#ifdef DEBUG_MODE

#else
	_log.open(_log_path, std::ios::app);
    if (!_log.good()) {
		throw std::string("can't open logfile");
	}
#endif
}

LogMaker::~LogMaker(void){}

void LogMaker::put_log(int define_event, char const *msg) {
    if (define_event < _log_level) return;

	std::string tmp;
	time_t rawtime;
    struct tm *info;
    char buffer[80];
    time(&rawtime);
    info = localtime(&rawtime);
	strftime(buffer, 80, "%F - %I:%M%p", info);
	
    switch (define_event) {
    case (LOG_TRACE):   tmp = "[" + std::string(buffer) + "] " + "TRACE: " + msg;	break;
    case (LOG_DEBUG):   tmp = "[" + std::string(buffer) + "] " + "DEBUG: " + msg;	break;
    case (LOG_INFO):    tmp = "[" + std::string(buffer) + "] " + "INFO: " + msg;	break;
    case (LOG_WARNING): tmp = "[" + std::string(buffer) + "] " + "WARNING: " + msg;	break;
    case (LOG_ERROR):   tmp = "[" + std::string(buffer) + "] " + "ERROR: " + msg;	break;
    }
	std::lock_guard<std::mutex> lock(_mutex_print);
#ifdef DEBUG_MODE
	std::cout << tmp << '\n';
#else
	_log << tmp << '\n';
    _log.flush();
#endif
}

void LogMaker::printf_log(int event_cost, const char *fmt, ...) {
	char tmp[1024] = {0};
	va_list argp;
	va_start(argp, fmt);
	vsprintf(tmp, fmt, argp);
	va_end(argp);
	put_log(event_cost, tmp);
}

void LogMaker::set_log_level(int level) {
	_log_level = level;
}
