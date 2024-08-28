#ifndef _TEXTURE
#define _TEXTURE

#include "../graphics/graphics_util.h"
#include "vertex.h"

namespace core {
struct Image {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> buffer;
    DXGI_FORMAT format;
    ComPtr<ID3D12Resource> upload;
};

struct Texture {
    std::shared_ptr<Image> image;
    ComPtr<ID3D12Resource> texture;
    Texture(){};
    Texture(const std::string filename, const bool usSRGB,
            ComPtr<ID3D12GraphicsCommandList> command_list);
    Texture(const std::string albedoFilename, const std::string opacityFilename,
            const bool usSRGB, ComPtr<ID3D12GraphicsCommandList> command_list);
};
} // namespace core

#endif