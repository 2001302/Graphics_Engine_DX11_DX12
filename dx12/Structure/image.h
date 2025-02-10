#ifndef _IMAGE
#define _IMAGE

#include "../pch.h"
#include <filesystem>
#include <iostream>

namespace graphics {
class Image {
  public:
    Image() : width(0), height(0), format(DXGI_FORMAT_UNKNOWN){};
    int Width() { return width; }
    int Height() { return height; }
    std::vector<uint8_t> &Buffer() { return buffer; }
    DXGI_FORMAT Format() { return format; }
    bool IsEmpty() { return width == 0 || height == 0; }

    static Image Read(const std::string path,
                      const bool usSRGB);
    static Image Read(const std::string path1, const std::string path2,
                      const bool usSRGB);
    static Image ReadMetallicRoughness(
        const std::string metallic, const std::string roughness,const bool usSRGB);

  private:
    int width;
    int height;
    std::vector<uint8_t> buffer;
    DXGI_FORMAT format;
};

} // namespace dx12
#endif