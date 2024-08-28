#define _CRT_SECURE_NO_WARNINGS

#include "texture.h"
#include <DirectXTexEXR.h>
#include <algorithm>
#include <directxtk/DDSTextureLoader.h>
#include <dxgi.h>    // DXGIFactory
#include <dxgi1_4.h> // DXGIFactory4
#include <fp16.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

using namespace std;
using namespace DirectX;

namespace core {
void ReadEXRImage(const std::string filename, std::vector<uint8_t> &image,
                  int &width, int &height, DXGI_FORMAT &pixelFormat) {

    const std::wstring wFilename(filename.begin(), filename.end());

    TexMetadata metadata;
    dx12::ThrowIfFailed(GetMetadataFromEXRFile(wFilename.c_str(), metadata));

    ScratchImage scratchImage;
    dx12::ThrowIfFailed(LoadFromEXRFile(wFilename.c_str(), NULL, scratchImage));

    width = static_cast<int>(metadata.width);
    height = static_cast<int>(metadata.height);
    pixelFormat = metadata.format;

    cout << filename << " " << metadata.width << " " << metadata.height
         << metadata.format << endl;

    image.resize(scratchImage.GetPixelsSize());
    memcpy(image.data(), scratchImage.GetPixels(), image.size());

    vector<float> f32(image.size() / 2);
    uint16_t *f16 = (uint16_t *)image.data();
    for (int i = 0; i < image.size() / 2; i++) {
        f32[i] = fp16_ieee_to_fp32_value(f16[i]);
    }

    const float minValue = *std::min_element(f32.begin(), f32.end());
    const float maxValue = *std::max_element(f32.begin(), f32.end());

    cout << minValue << " " << maxValue << endl;

    // f16 = (uint16_t *)image.data();
    // for (int i = 0; i < image.size() / 2; i++) {
    //     f16[i] = fp16_ieee_from_fp32_value(f32[i] * 2.0f);
    // }
}

void ReadImage(const std::string filename, std::vector<uint8_t> &image,
               int &width, int &height) {

    int channels;

    unsigned char *img =
        stbi_load(filename.c_str(), &width, &height, &channels, 0);

    // assert(channels == 4);

    cout << filename << " " << width << " " << height << " " << channels
         << endl;

    // to 4 chanel
    image.resize(width * height * 4);

    if (channels == 1) {
        for (size_t i = 0; i < width * height; i++) {
            uint8_t g = img[i * channels + 0];
            for (size_t c = 0; c < 4; c++) {
                image[4 * i + c] = g;
            }
        }
    } else if (channels == 2) {
        for (size_t i = 0; i < width * height; i++) {
            for (size_t c = 0; c < 2; c++) {
                image[4 * i + c] = img[i * channels + c];
            }
            image[4 * i + 2] = 255;
            image[4 * i + 3] = 255;
        }
    } else if (channels == 3) {
        for (size_t i = 0; i < width * height; i++) {
            for (size_t c = 0; c < 3; c++) {
                image[4 * i + c] = img[i * channels + c];
            }
            image[4 * i + 3] = 255;
        }
    } else if (channels == 4) {
        for (size_t i = 0; i < width * height; i++) {
            for (size_t c = 0; c < 4; c++) {
                image[4 * i + c] = img[i * channels + c];
            }
        }
    } else {
        std::cout << "Cannot read " << channels << " channels" << endl;
    }

    delete[] img;
}

void ReadImage(const std::string albedoFilename,
               const std::string opacityFilename, std::vector<uint8_t> &image,
               int &width, int &height) {

    ReadImage(albedoFilename, image, width, height);

    std::vector<uint8_t> opacityImage;
    int opaWidth, opaHeight;

    ReadImage(opacityFilename, opacityImage, opaWidth, opaHeight);

    assert(width == opaWidth && height == opaHeight);

    for (int j = 0; j < height; j++)
        for (int i = 0; i < width; i++) {
            image[3 + 4 * i + 4 * width * j] =
                opacityImage[4 * i + 4 * width * j]; // Copy alpha channel
        }
}

// todo : neet to optimize
void CreateTextureHelper(Image* image,
                         ComPtr<ID3D12Resource> &texture,
                         ComPtr<ID3D12GraphicsCommandList> command_list) {
    // 1.resource creation
    D3D12_RESOURCE_DESC txtDesc;
    ZeroMemory(&txtDesc, sizeof(txtDesc));
    txtDesc.Width = image->width;
    txtDesc.Height = image->height;
    txtDesc.MipLevels = 0;
    txtDesc.DepthOrArraySize = 1;
    txtDesc.Format = image->format;
    txtDesc.SampleDesc.Count = 1;
    txtDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    txtDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
    HRESULT hr = dx12::GpuCore::Instance().device->CreateCommittedResource(
        &heapProperties, D3D12_HEAP_FLAG_NONE, &txtDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
        IID_PPV_ARGS(texture.GetAddressOf()));

    // 2.upload heap
    const uint64_t uploadBufferSize =
        GetRequiredIntermediateSize(texture.Get(), 0, 1);

    auto heap_property = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto buffer_size = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    dx12::ThrowIfFailed(
        dx12::GpuCore::Instance().device->CreateCommittedResource(
            &heap_property, D3D12_HEAP_FLAG_NONE, &buffer_size,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&image->upload)));

    // copy data to upload heap
    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = image->buffer.data();
    textureData.RowPitch = image->width * 4;
    textureData.SlicePitch = textureData.RowPitch * image->height;

    UpdateSubresources(command_list.Get(), texture.Get(), image->upload.Get(),
                       0, 0, 1, &textureData);

    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    command_list->ResourceBarrier(1, &barrier);
}

Texture::Texture(const std::string filename, const bool usSRGB,
                 ComPtr<ID3D12GraphicsCommandList> command_list) {

    image = std::make_shared<Image>();
    image->format =
        usSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

    string ext(filename.end() - 3, filename.end());
    std::transform(ext.begin(), ext.end(), ext.begin(), std::tolower);

    if (ext == "exr") {
        ReadEXRImage(filename, image->buffer, image->width, image->height,
                     image->format);
    } else {
        ReadImage(filename, image->buffer, image->width, image->height);
    }

    CreateTextureHelper(image.get(), texture, command_list);
};

Texture::Texture(const std::string albedoFilename,
                 const std::string opacityFilename, const bool usSRGB,
                 ComPtr<ID3D12GraphicsCommandList> command_list) {

    image = std::make_shared<Image>();
    image->format =
        usSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

    ReadImage(albedoFilename, opacityFilename, image->buffer, image->width,
              image->height);

    CreateTextureHelper(image.get(), texture, command_list);
};
} // namespace core