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