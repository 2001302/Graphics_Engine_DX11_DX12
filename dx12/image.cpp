#define _CRT_SECURE_NO_WARNINGS
#include "image.h"
#include <DirectXTexEXR.h>
#include <algorithm>
#include <dxgi.h>    // DXGIFactory
#include <dxgi1_4.h> // DXGIFactory4
#include <fp16.h>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

namespace graphics {
void ReadEXRImage(const std::string filename, std::vector<uint8_t> &image,
                  int &width, int &height, DXGI_FORMAT &pixelFormat) {

    if (!std::filesystem::exists(filename))
        return;

    const std::wstring wFilename(filename.begin(), filename.end());

    DirectX::TexMetadata metadata;
    ASSERT_FAILED(GetMetadataFromEXRFile(wFilename.c_str(), metadata));

    DirectX::ScratchImage scratchImage;
    ASSERT_FAILED(
        DirectX::LoadFromEXRFile(wFilename.c_str(), NULL, scratchImage));

    width = static_cast<int>(metadata.width);
    height = static_cast<int>(metadata.height);
    pixelFormat = metadata.format;

    std::cout << filename << " " << metadata.width << " " << metadata.height
              << metadata.format << std::endl;

    image.resize(scratchImage.GetPixelsSize());
    memcpy(image.data(), scratchImage.GetPixels(), image.size());

    std::vector<float> f32(image.size() / 2);
    uint16_t *f16 = (uint16_t *)image.data();
    for (int i = 0; i < image.size() / 2; i++) {
        f32[i] = fp16_ieee_to_fp32_value(f16[i]);
    }

    const float minValue = *std::min_element(f32.begin(), f32.end());
    const float maxValue = *std::max_element(f32.begin(), f32.end());

    std::cout << minValue << " " << maxValue << std::endl;

    // f16 = (uint16_t *)image.data();
    // for (int i = 0; i < image.size() / 2; i++) {
    //     f16[i] = fp16_ieee_from_fp32_value(f32[i] * 2.0f);
    // }
}

void ReadImage(const std::string filename, std::vector<uint8_t> &image,
               int &width, int &height) {

    if (!std::filesystem::exists(filename))
        return;

    int channels;

    unsigned char *img =
        stbi_load(filename.c_str(), &width, &height, &channels, 0);

    // assert(channels == 4);

    std::cout << filename << " " << width << " " << height << " " << channels
              << std::endl;

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
        std::cout << "Cannot read " << channels << " channels" << std::endl;
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

Image Image::Read(const std::string path,
                  ComPtr<ID3D12GraphicsCommandList> command_list,
                  const bool usSRGB) {
    Image image;

    if (!std::filesystem::exists(path)) {
        return image;
    }

    image.format =
        usSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

    std::string ext(path.end() - 3, path.end());
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   (int (*)(int))std::toupper);

    if (ext == "exr") {
        ReadEXRImage(path, image.buffer, image.width, image.height,
                     image.format);
    } else {
        ReadImage(path, image.buffer, image.width, image.height);
    }

    return image;
};
Image Image::Read(const std::string path1, const std::string path2,
                  ComPtr<ID3D12GraphicsCommandList> command_list,
                  const bool usSRGB) {
    Image image;

    if (!std::filesystem::exists(path1) && !std::filesystem::exists(path2)) {
        return image;
    } else if (std::filesystem::exists(path1) &&
               !std::filesystem::exists(path2)) {
        image = Read(path1, command_list, usSRGB);
        return image;
    } else if (!std::filesystem::exists(path1) &&
               std::filesystem::exists(path2)) {
        image = Read(path2, command_list, usSRGB);
        return image;
    }

    image.format =
        usSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

    ReadImage(path1, path2, image.buffer, image.width, image.height);

    return image;
};

Image Image::ReadMetallicRoughness(
    const std::string metallic, const std::string roughness,
    ComPtr<ID3D12GraphicsCommandList> command_list, const bool usSRGB) {
    Image image;

    if (!metallic.empty() && (metallic == roughness)) {
        image = Read(metallic, command_list, false);
    } else {
        int mWidth = 0, mHeight = 0;
        int rWidth = 0, rHeight = 0;
        std::vector<uint8_t> mImage;
        std::vector<uint8_t> rImage;

        if (!metallic.empty()) {
            ReadImage(metallic, mImage, mWidth, mHeight);
        }

        if (!roughness.empty()) {
            ReadImage(roughness, rImage, rWidth, rHeight);
        }

        if (!metallic.empty() && !roughness.empty()) {
            assert(mWidth == rWidth);
            assert(mHeight == rHeight);
        }

        std::vector<uint8_t> combinedImage(size_t(mWidth * mHeight) * 4);
        std::fill(combinedImage.begin(), combinedImage.end(), 0);

        for (size_t i = 0; i < size_t(mWidth * mHeight); i++) {
            if (rImage.size())
                combinedImage[4 * i + 1] = rImage[4 * i]; // Green = Roughness
            if (mImage.size())
                combinedImage[4 * i + 2] = mImage[4 * i]; // Blue = Metalness
        }
    }

    return image;
}
} // namespace graphics