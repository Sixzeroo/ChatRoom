//
// Created by Sixzeroo on 2018/6/8.
//

#ifndef SRC_LOG_H
#define SRC_LOG_H

#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>

#define LOG(log_level) \
Logger(log_level).write_log(log_level, __LINE__, __FUNCTION__)


enum LOG_LEVEL {
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4
};

// 初始化日志文件
void init_logger(const std::string profix,
                 const std::string debug_log_file,
                 const std::string info_log_file,
                 const std::string warn_log_file,
                 const std::string error_log_file,
                 const std::string log_file );

void set_logger_mode(int mode);

class Logger {
    friend void init_logger(const std::string profix,
                            const std::string debug_log_file,
                            const std::string info_log_file,
                            const std::string warn_log_file,
                            const std::string error_log_file,
                            const std::string log_file );

    friend void set_logger_mode(int mode);

private:
    LOG_LEVEL _log_level;

    static int _mode;

    static std::ostream& get_stream(LOG_LEVEL log_level);

    // 通过文件输入输出流的形式写入文件
    static std::ofstream _debug_log_file;
    static std::ofstream _info_log_file;
    static std::ofstream _warn_log_file;
    static std::ofstream _error_log_file;
    static std::ofstream _log_file;


public:
    Logger(LOG_LEVEL log_level) : _log_level(log_level) {};

    ~Logger();

    // 设置日志存储模式，为1时全部存储在一个文件里
    void set_logger_mode(int mode);

    // 静态成员函数
    static std::ostream& write_log(LOG_LEVEL log_level, const int line, const std::string function);

};

#endif //SRC_LOG_H
