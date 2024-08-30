#ifndef _TEXTURE
#define _TEXTURE

#include "../graphics/graphics_util.h"
#include "vertex.h"
#include <filesystem>
#include <iostream>

namespace core {
struct Image {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> buffer;
    DXGI_FORMAT format;
    ComPtr<ID3D12Resource> upload;
};

struct Texture {
    Texture() : image(0), texture(0), is_initialized(false){};
    bool InitAsTexture(const std::string filename, const bool usSRGB,
                       ComPtr<ID3D12GraphicsCommandList> command_list);
    bool InitAsTexture(const std::string albedoFilename,
                       const std::string opacityFilename, const bool usSRGB,
                       ComPtr<ID3D12GraphicsCommandList> command_list);
    bool InitAsMetallicRoughnessTexture(
        const std::string metallicFilename, const std::string roughnessFilename,
        ComPtr<ID3D12GraphicsCommandList> command_list);
    static bool InitAsDDSTexture(const wchar_t file_name, bool isCubeMap,
                                 ComPtr<ID3D12Resource> texture);

    std::shared_ptr<Image> image;
    ComPtr<ID3D12Resource> texture;
    bool is_initialized;
};
} // namespace core

#endif