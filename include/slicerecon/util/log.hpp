#pragma once

#include <iostream>
#include <map>
#include <mutex>
#include <sstream>

#include <string.h>

#define __FILENAME__                                                           \
    (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOG_FILE                                                               \
    slicerecon::util::color["black_bg"]                                        \
        << slicerecon::util::color["cyan"] << "(" << __FILENAME__ << ":"       \
        << __LINE__ << ")" << slicerecon::util::color["nc"] << " "

namespace slicerecon::util {

enum class lvl { info, warning, error, none_set };

struct end_log_ {};
extern end_log_ end_log;

static std::map<std::string, std::string> color{
    {"black", "\033[1;30m"},    {"red", "\033[1;31m"},
    {"green", "\033[1;32m"},    {"yellow", "\033[1;33m"},
    {"blue", "\033[1;34m"},     {"magenta", "\033[1;35m"},
    {"cyan", "\033[1;36m"},     {"white", "\033[1;37m"},
    {"black_bg", "\033[1;40m"}, {"red_bg", "\033[1;41m"},
    {"green_bg", "\033[1;42m"}, {"yellow_bg", "\033[1;43m"},
    {"blue_bg", "\033[1;44m"},  {"magenta_bg", "\033[1;45m"},
    {"cyan_bg", "\033[1;46m"},  {"white_bg", "\033[1;47m"},
    {"nc", "\033[0m"}};

struct logger {
    logger& operator<<(lvl level) {
        level_ = level;
        return *this;
    }

    template <typename T>
    logger& operator<<(T&& rhs) {
        line << rhs;
        return *this;
    }

    void operator<<(end_log_ unused) {
        (void)unused;

        switch (level_) {
        case lvl::info:
            std::cout << color["black_bg"] << color["yellow"] << "INFO";
            break;
        case lvl::warning:
            std::cout << color["black_bg"] << color["blue"] << "WARNING";
            break;
        case lvl::error:
            std::cout << color["black_bg"] << color["red"] << "ERROR";
            break;
        default:
            std::cout << "LOG";
            break;
        };

        std::cout << color["nc"] << " " << line.str() << "\n";
        level_ = lvl::none_set;

        // reset the string stream
        line = std::stringstream{};
    }

    std::stringstream line;
    lvl level_ = lvl::none_set;
};

extern logger log;

} // namespace slicerecon::util
