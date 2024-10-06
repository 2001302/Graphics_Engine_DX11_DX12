#ifndef _TIME_STAMP
#define _TIME_STAMP

#include <chrono>

namespace common {
class TimeStamp {
  public:
    float DeltaTime() {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - before)
                .count();
        before = now;
        return duration * 0.001f;
    }

  private:
    std::chrono::steady_clock::time_point before;
};

} // namespace common

#endif