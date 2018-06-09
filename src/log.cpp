//
// Created by Sixzeroo on 2018/6/8.
//

#include <cstdlib>
#include <ctime>
#include <vector>

#include "log.h"

std::ofstream Logger::_debug_log_file;
std::ofstream Logger::_info_log_file;
std::ofstream Logger::_warn_log_file;
std::ofstream Logger::_error_log_file;
std::ofstream Logger::_log_file;

void init_logger(const std::string debug_log_file,
                 const std::string info_log_file,
                 const std::string warn_log_file,
                 const std::string error_log_file,
                 const std::string log_file )
{
    Logger::_debug_log_file.open(debug_log_file.c_str());
    Logger::_info_log_file.open(info_log_file.c_str());
    Logger::_warn_log_file.open(warn_log_file.c_str());
    Logger::_error_log_file.open(error_log_file.c_str());
    Logger::_log_file.open(log_file.c_str());
}

std::ostream& Logger::get_stream(LOG_LEVEL log_level) {
    if(log_level == DEBUG) return _debug_log_file;
    if(log_level == INFO) return _info_log_file;
    if(log_level == WARN) return _warn_log_file;
    if(log_level == ERROR) return _error_log_file;
    return std::cout;
}

std::ostream& Logger::write_log(LOG_LEVEL log_level, const int line, const std::string function) {
    time_t tm;
    time(&tm);
    char time_string[128];
    ctime_r(&tm, time_string);
    std::string s_time_string = time_string;
    s_time_string.pop_back();

    std::vector<std::string> le(ERROR + 1);
    le[DEBUG] = "DEBUG";
    le[INFO] = "INFO";
    le[WARN] = "WARN";
    le[ERROR] = "ERROR";

    char buff[4096];
    snprintf(buff, sizeof(buff), "[%s] function (%s) ; line : %d", s_time_string.c_str(), function.c_str(), line);
    std::string msg = buff;

    return get_stream(log_level) << msg << std::endl << std::flush;
}

Logger::~Logger() {
    get_stream(_log_level) << std::endl << std::flush;
}