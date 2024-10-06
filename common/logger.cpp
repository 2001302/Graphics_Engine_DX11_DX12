#include "logger.h"

namespace common {
void Logger::Debug(std::string message) {

    time_t timer = time(NULL);
    struct tm t;
    localtime_s(&t, &timer);

    std::cout << std::to_string(t.tm_year + 1900) + "-" +
                     std::to_string(t.tm_mon + 1) + "-" +
                     std::to_string(t.tm_mday) + "T" +
                     std::to_string(t.tm_hour) + ":" +
                     std::to_string(t.tm_min) + ":" + std::to_string(t.tm_sec) +
                     "|"
              << message << std::endl;
};
} // namespace common