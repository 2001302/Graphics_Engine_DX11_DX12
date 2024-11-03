#ifndef _TIME_STAMP
#define _TIME_STAMP

#include <chrono>

namespace common {
class TimeStamp {
  public:
    float DeltaTime() {
        if (first_frame) {
            first_frame = false;
            auto now = std::chrono::high_resolution_clock::now();
            before = now;
            return 0.0f;
        } else {
            auto now = std::chrono::high_resolution_clock::now();
            auto duration =
                std::chrono::duration_cast<std::chrono::microseconds>(now -
                                                                      before)
                    .count();
            before = now;
            return duration * 0.000001f;
        }
    }

  private:
    std::chrono::steady_clock::time_point before;
    bool first_frame = true;
};
} // namespace common

#endif