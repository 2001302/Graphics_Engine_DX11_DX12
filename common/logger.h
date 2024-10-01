#ifndef _LOGGER
#define _LOGGER
#define _CRT_SECURE_NO_WARNINGS

#include <chrono>
#include <iostream>
#include <string>

namespace common {
class Logger {
  public:
    static void Debug(std::string message) {

        time_t timer = time(NULL);
        struct tm t;
        localtime_s(&t, &timer);

        std::cout << std::to_string(t.tm_year + 1900) + "-" +
                         std::to_string(t.tm_mon + 1) + "-" +
                         std::to_string(t.tm_mday) + "T" +
                         std::to_string(t.tm_hour) + ":" +
                         std::to_string(t.tm_min) + ":" +
                         std::to_string(t.tm_sec) + "|"
                  << message << std::endl;
    };
};

class ScopeStopWatch {
  public:
    ScopeStopWatch(std::string message) : message(message) {
        begin = std::chrono::high_resolution_clock::now();
    };
    ~ScopeStopWatch() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
                .count();

        Logger::Debug(message + ": " + std::to_string(duration) + "ms");
    };

  private:
    std::string message;
    std::chrono::steady_clock::time_point begin;
};
} // namespace common

#endif