#ifndef _PATH
#define _PATH

#include "string"

namespace common {
class Path {
  public:
    static std::string Combine(const std::string &path1,
                                const std::string &path2) {
        std::string combined = path1;
        if (combined.back() != '/' && combined.back() != '\\') {
            combined += '/';
        }
        combined += path2;
        return combined;
    }
};
} // namespace common
#endif
