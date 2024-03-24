//
// Created by rolando on 5/5/17.
//

#ifndef DRC_SIM_C_LOGGER_H
#define DRC_SIM_C_LOGGER_H

#include <iostream>

class Logger {

public:
    static void log(const std::string log_name, const int log_level, const std::string message, va_list args);
    static void info(const std::string log_name, const std::string message, ...);
    static void debug(const std::string log_name, const std::string message, ...);
    static void extra(const std::string log_name, const std::string message, ...);
    static void finer(const std::string log_name, const std::string message, ...);
    static void verbose(const std::string log_name, const std::string message, ...);
    static void error(const std::string log_name, const std::string message, ...);
    static void set_level(int log_level);
    static int get_level();

    static const int INFO = 0;
    static const int DEBUG = 1;
    static const int EXTRA = 2;
    static const int FINER = 3;
    static const int VERBOSE = 4;
    static const int ERROR = 5;

    static const std::string DRC;
    static const std::string SERVER;
    static const std::string VIDEO;
    static const std::string AUDIO;
    static const std::string CONFIG;

    static bool is_level_enabled(const int level);

    // static const char * to_hex(unsigned char *data, size_t size);

private:
    static int log_level;

    static char *get_level_str(const int log_level);
};


#endif //DRC_SIM_C_LOGGER_H
