#ifndef _LOGGER
#define _LOGGER

#include <chrono>
#include <iostream>
#include <string>

namespace common {
class Logger {
  public:
    static void Debug(std::string message) {
        std::cout << message << std::endl;
    };
};
class TimeLogger {
  public:
    static std::chrono::steady_clock::time_point Begin() {
        return std::chrono::high_resolution_clock::now();
    };
    static void End(std::string message,
                    std::chrono::steady_clock::time_point begin) {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
                .count();

        Logger::Debug(message + "Elapsed Time is " + std::to_string(duration) + "ms");
    };
};
} // namespace common

#endif