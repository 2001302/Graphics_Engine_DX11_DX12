#define _CRT_SECURE_NO_WARNINGS
#include "graphics_util.h"

#include <DirectXTexEXR.h>
#include <algorithm>
#include <directxtk/DDSTextureLoader.h>
#include <dxgi.h>    // DXGIFactory
#include <dxgi1_4.h> // DXGIFactory4
#include <fp16.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

namespace dx12 {

using namespace std;
using namespace DirectX;

//void Util::CreateIndexBuffer(const std::vector<uint32_t> &indices,
//                             ComPtr<ID3D12Resource> &indexBuffer) {
//
//    const UINT size = sizeof(indices);
//
//    HRESULT hr = GpuCore::Instance().device->CreateCommittedResource(
//        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
//        &CD3DX12_RESOURCE_DESC::Buffer(size), D3D12_RESOURCE_STATE_GENERIC_READ,
//        nullptr, IID_PPV_ARGS(&indexBuffer));
//}
//
// void ReadEXRImage(const std::string filename, std::vector<uint8_t> &image,
//                  int &width, int &height, DXGI_FORMAT &pixelFormat) {
//
//    const std::wstring wFilename(filename.begin(), filename.end());
//
//    TexMetadata metadata;
//    ThrowIfFailed(GetMetadataFromEXRFile(wFilename.c_str(), metadata));
//
//    ScratchImage scratchImage;
//    ThrowIfFailed(LoadFromEXRFile(wFilename.c_str(), NULL, scratchImage));
//
//    width = static_cast<int>(metadata.width);
//    height = static_cast<int>(metadata.height);
//    pixelFormat = metadata.format;
//
//    cout << filename << " " << metadata.width << " " << metadata.height
//         << metadata.format << endl;
//
//    image.resize(scratchImage.GetPixelsSize());
//    memcpy(image.data(), scratchImage.GetPixels(), image.size());
//
//    vector<float> f32(image.size() / 2);
//    uint16_t *f16 = (uint16_t *)image.data();
//    for (int i = 0; i < image.size() / 2; i++) {
//        f32[i] = fp16_ieee_to_fp32_value(f16[i]);
//    }
//
//    const float minValue = *std::min_element(f32.begin(), f32.end());
//    const float maxValue = *std::max_element(f32.begin(), f32.end());
//
//    cout << minValue << " " << maxValue << endl;
//
//    // f16 = (uint16_t *)image.data();
//    // for (int i = 0; i < image.size() / 2; i++) {
//    //     f16[i] = fp16_ieee_from_fp32_value(f32[i] * 2.0f);
//    // }
//}
//
// void ReadImage(const std::string filename, std::vector<uint8_t> &image,
//               int &width, int &height) {
//
//    int channels;
//
//    unsigned char *img =
//        stbi_load(filename.c_str(), &width, &height, &channels, 0);
//
//    // assert(channels == 4);
//
//    cout << filename << " " << width << " " << height << " " << channels
//         << endl;
//
//    // to 4 chanel
//    image.resize(width * height * 4);
//
//    if (channels == 1) {
//        for (size_t i = 0; i < width * height; i++) {
//            uint8_t g = img[i * channels + 0];
//            for (size_t c = 0; c < 4; c++) {
//                image[4 * i + c] = g;
//            }
//        }
//    } else if (channels == 2) {
//        for (size_t i = 0; i < width * height; i++) {
//            for (size_t c = 0; c < 2; c++) {
//                image[4 * i + c] = img[i * channels + c];
//            }
//            image[4 * i + 2] = 255;
//            image[4 * i + 3] = 255;
//        }
//    } else if (channels == 3) {
//        for (size_t i = 0; i < width * height; i++) {
//            for (size_t c = 0; c < 3; c++) {
//                image[4 * i + c] = img[i * channels + c];
//            }
//            image[4 * i + 3] = 255;
//        }
//    } else if (channels == 4) {
//        for (size_t i = 0; i < width * height; i++) {
//            for (size_t c = 0; c < 4; c++) {
//                image[4 * i + c] = img[i * channels + c];
//            }
//        }
//    } else {
//        std::cout << "Cannot read " << channels << " channels" << endl;
//    }
//
//    delete[] img;
//}
//
// void ReadImage(const std::string albedoFilename,
//               const std::string opacityFilename, std::vector<uint8_t> &image,
//               int &width, int &height) {
//
//    ReadImage(albedoFilename, image, width, height);
//
//    std::vector<uint8_t> opacityImage;
//    int opaWidth, opaHeight;
//
//    ReadImage(opacityFilename, opacityImage, opaWidth, opaHeight);
//
//    assert(width == opaWidth && height == opaHeight);
//
//    for (int j = 0; j < height; j++)
//        for (int i = 0; i < width; i++) {
//            image[3 + 4 * i + 4 * width * j] =
//                opacityImage[4 * i + 4 * width * j]; // Copy alpha channel
//        }
//}
//
// ComPtr<ID3D11Texture2D>
// CreateStagingTexture(const int width,
//                     const int height, const std::vector<uint8_t> &image,
//                     const DXGI_FORMAT pixelFormat =
//                     DXGI_FORMAT_R8G8B8A8_UNORM, const int mipLevels = 1,
//                     const int arraySize = 1) {
//
//    D3D11_TEXTURE2D_DESC txtDesc;
//    ZeroMemory(&txtDesc, sizeof(txtDesc));
//    txtDesc.Width = width;
//    txtDesc.Height = height;
//    txtDesc.MipLevels = mipLevels;
//    txtDesc.ArraySize = arraySize;
//    txtDesc.Format = pixelFormat;
//    txtDesc.SampleDesc.Count = 1;
//    txtDesc.Usage = D3D11_USAGE_STAGING;
//    txtDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
//
//    ComPtr<ID3D11Texture2D> stagingTexture;
//    if (FAILED(GpuCore::Instance().device->CreateTexture2D(
//            &txtDesc, NULL,
//                                       stagingTexture.GetAddressOf()))) {
//        cout << "Failed()" << endl;
//    }
//
//    size_t pixelSize = sizeof(uint8_t) * 4;
//    if (pixelFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) {
//        pixelSize = sizeof(uint16_t) * 4;
//    }
//
//    D3D11_MAPPED_SUBRESOURCE ms;
//    GpuCore::Instance().device_context->Map(stagingTexture.Get(), NULL,
//                                                 D3D11_MAP_WRITE, NULL, &ms);
//    uint8_t *pData = (uint8_t *)ms.pData;
//    for (UINT h = 0; h < UINT(height); h++) {
//        memcpy(&pData[h * ms.RowPitch], &image[h * width * pixelSize],
//               width * pixelSize);
//    }
//    GpuCore::Instance().device_context->Unmap(stagingTexture.Get(), NULL);
//
//    return stagingTexture;
//}
//
// void CreateTextureHelper(const int width,
//                         const int height, const vector<uint8_t> &image,
//                         const DXGI_FORMAT pixelFormat,
//                         ComPtr<ID3D11Texture2D> &texture,
//                         ComPtr<ID3D11ShaderResourceView> &srv) {
//
//    ComPtr<ID3D11Texture2D> stagingTexture = CreateStagingTexture(width,
//                             height, image, pixelFormat);
//
//    D3D11_TEXTURE2D_DESC txtDesc;
//    ZeroMemory(&txtDesc, sizeof(txtDesc));
//    txtDesc.Width = width;
//    txtDesc.Height = height;
//    txtDesc.MipLevels = 0;
//    txtDesc.ArraySize = 1;
//    txtDesc.Format = pixelFormat;
//    txtDesc.SampleDesc.Count = 1;
//    txtDesc.Usage = D3D11_USAGE_DEFAULT; // 스테이징 텍스춰로부터 복사 가능
//    txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
//    txtDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS; // 밉맵 사용
//    txtDesc.CPUAccessFlags = 0;
//
//    // 초기 데이터 없이 텍스춰 생성 (전부 검은색)
//    GpuCore::Instance().device->CreateTexture2D(
//        &txtDesc, NULL, texture.GetAddressOf());
//
//    // 실제로 생성된 MipLevels를 확인해보고 싶을 경우
//    // texture->GetDesc(&txtDesc);
//    // cout << txtDesc.MipLevels << endl;
//
//    // 스테이징 텍스춰로부터 가장 해상도가 높은 이미지 복사
//    GpuCore::Instance().device_context->CopySubresourceRegion(
//        texture.Get(), 0, 0, 0, 0,
//                                   stagingTexture.Get(), 0, NULL);
//
//    // ResourceView 만들기
//    GpuCore::Instance().device->CreateShaderResourceView(
//        texture.Get(), 0, srv.GetAddressOf());
//
//    // 해상도를 낮춰가며 밉맵 생성
//    GpuCore::Instance().device_context->GenerateMips(srv.Get());
//
//    // HLSL 쉐이더 안에서는 SampleLevel() 사용
//}
//
// void Util::CreateMetallicRoughnessTexture(
//    const std::string metallicFilename, const std::string roughnessFilename,
//    ComPtr<ID3D11Texture2D> &texture, ComPtr<ID3D11ShaderResourceView> &srv) {
//
//    // GLTF 방식은 이미 합쳐져 있음
//    if (!metallicFilename.empty() && (metallicFilename == roughnessFilename))
//    {
//        CreateTexture(metallicFilename, false, texture, srv);
//    } else {
//        // 별도 파일일 경우 따로 읽어서 합쳐줍니다.
//
//        // ReadImage()를 활용하기 위해서 두 이미지들을 각각 4채널로 변환 후
//        다시
//        // 3채널로 합치는 방식으로 구현
//        int mWidth = 0, mHeight = 0;
//        int rWidth = 0, rHeight = 0;
//        std::vector<uint8_t> mImage;
//        std::vector<uint8_t> rImage;
//
//        // (거의 없겠지만) 둘 중 하나만 있을 경우도 고려하기 위해 각각 파일명
//        // 확인
//        if (!metallicFilename.empty()) {
//            ReadImage(metallicFilename, mImage, mWidth, mHeight);
//        }
//
//        if (!roughnessFilename.empty()) {
//            ReadImage(roughnessFilename, rImage, rWidth, rHeight);
//        }
//
//        // 두 이미지의 해상도가 같다고 가정
//        if (!metallicFilename.empty() && !roughnessFilename.empty()) {
//            assert(mWidth == rWidth);
//            assert(mHeight == rHeight);
//        }
//
//        vector<uint8_t> combinedImage(size_t(mWidth * mHeight) * 4);
//        fill(combinedImage.begin(), combinedImage.end(), 0);
//
//        for (size_t i = 0; i < size_t(mWidth * mHeight); i++) {
//            if (rImage.size())
//                combinedImage[4 * i + 1] = rImage[4 * i]; // Green = Roughness
//            if (mImage.size())
//                combinedImage[4 * i + 2] = mImage[4 * i]; // Blue = Metalness
//        }
//
//        CreateTextureHelper( mWidth, mHeight, combinedImage,
//                            DXGI_FORMAT_R8G8B8A8_UNORM, texture, srv);
//    }
//}
//
// void Util::CreateTexture(
//                                 const std::string filename, const bool
//                                 usSRGB, ComPtr<ID3D11Texture2D> &tex,
//                                 ComPtr<ID3D11ShaderResourceView> &srv) {
//
//    int width = 0, height = 0;
//    std::vector<uint8_t> image;
//    DXGI_FORMAT pixelFormat =
//        usSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
//
//    string ext(filename.end() - 3, filename.end());
//    std::transform(ext.begin(), ext.end(), ext.begin(), std::tolower);
//
//    if (ext == "exr") {
//        ReadEXRImage(filename, image, width, height, pixelFormat);
//    } else {
//        ReadImage(filename, image, width, height);
//    }
//
//    CreateTextureHelper( width, height, image, pixelFormat, tex,
//                        srv);
//}
//
// void Util::CreateTexture(
//                                 const std::string albedoFilename,
//                                 const std::string opacityFilename,
//                                 const bool usSRGB,
//                                 ComPtr<ID3D11Texture2D> &texture,
//                                 ComPtr<ID3D11ShaderResourceView> &srv) {
//
//    int width = 0, height = 0;
//    std::vector<uint8_t> image;
//    DXGI_FORMAT pixelFormat =
//        usSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
//
//    ReadImage(albedoFilename, opacityFilename, image, width, height);
//
//    CreateTextureHelper(width, height, image, pixelFormat,
//                        texture, srv);
//}
//
// void Util::CreateTextureArray(
//    const std::vector<std::string> filenames, ComPtr<ID3D11Texture2D>
//    &texture, ComPtr<ID3D11ShaderResourceView> &textureResourceView) {
//
//    using namespace std;
//
//    if (filenames.empty())
//        return;
//
//    // 모든 이미지의 width와 height가 같다고 가정합니다.
//
//    // 파일로부터 이미지 여러 개를 읽어들입니다.
//    int width = 0, height = 0;
//    vector<vector<uint8_t>> imageArray;
//    for (const auto &f : filenames) {
//
//        cout << f << endl;
//
//        std::vector<uint8_t> image;
//
//        ReadImage(f, image, width, height);
//
//        imageArray.push_back(image);
//    }
//
//    UINT size = UINT(filenames.size());
//
//    // Texture2DArray를 만듭니다. 이때 데이터를 CPU로부터 복사하지 않습니다.
//    D3D11_TEXTURE2D_DESC txtDesc;
//    ZeroMemory(&txtDesc, sizeof(txtDesc));
//    txtDesc.Width = UINT(width);
//    txtDesc.Height = UINT(height);
//    txtDesc.MipLevels = 0; // 밉맵 레벨 최대
//    txtDesc.ArraySize = size;
//    txtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//    txtDesc.SampleDesc.Count = 1;
//    txtDesc.SampleDesc.Quality = 0;
//    txtDesc.Usage = D3D11_USAGE_DEFAULT; // 스테이징 텍스춰로부터 복사 가능
//    txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
//    txtDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS; // 밉맵 사용
//
//    // 초기 데이터 없이 텍스춰를 만듭니다.
//    GpuCore::Instance().device->CreateTexture2D(&txtDesc, NULL,
//                                                     texture.GetAddressOf());
//
//    // 실제로 만들어진 MipLevels를 확인
//    texture->GetDesc(&txtDesc);
//    // cout << txtDesc.MipLevels << endl;
//
//    // StagingTexture를 만들어서 하나씩 복사합니다.
//    for (size_t i = 0; i < imageArray.size(); i++) {
//
//        auto &image = imageArray[i];
//
//        // StagingTexture는 Texture2DArray가 아니라 Texture2D 입니다.
//        ComPtr<ID3D11Texture2D> stagingTexture = CreateStagingTexture(
//             width, height, image, txtDesc.Format, 1, 1);
//
//        // 스테이징 텍스춰를 텍스춰 배열의 해당 위치에 복사합니다.
//        UINT subresourceIndex =
//            D3D11CalcSubresource(0, UINT(i), txtDesc.MipLevels);
//
//        GpuCore::Instance().device_context->CopySubresourceRegion(
//            texture.Get(), subresourceIndex, 0, 0, 0,
//                                       stagingTexture.Get(), 0, NULL);
//    }
//
//    GpuCore::Instance().device->CreateShaderResourceView(
//        texture.Get(), NULL,
//                                     textureResourceView.GetAddressOf());
//
//    GpuCore::Instance().device_context->GenerateMips(
//        textureResourceView.Get());
//}
//
// void Util::CreateDDSTexture( const wchar_t *filename, bool isCubeMap,
//    ComPtr<ID3D11ShaderResourceView> &textureResourceView) {
//
//    ComPtr<ID3D11Texture2D> texture;
//
//    UINT miscFlags = 0;
//    if (isCubeMap) {
//        miscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
//    }
//
//    // https://github.com/microsoft/DirectXTK/wiki/DDSTextureLoader
//    ThrowIfFailed(CreateDDSTextureFromFileEx(
//        GpuCore::Instance().device.Get(), filename, 0, D3D11_USAGE_DEFAULT,
//        D3D11_BIND_SHADER_RESOURCE, 0, miscFlags, DDS_LOADER_FLAGS(false),
//        (ID3D11Resource **)texture.GetAddressOf(),
//        textureResourceView.GetAddressOf(), NULL));
//}
//
// void Util::WriteToFile(
//                               ComPtr<ID3D11Texture2D> &textureToWrite,
//                               const std::string filename) {
//
//    D3D11_TEXTURE2D_DESC desc;
//    textureToWrite->GetDesc(&desc);
//    desc.SampleDesc.Count = 1;
//    desc.SampleDesc.Quality = 0;
//    desc.BindFlags = 0;
//    desc.MiscFlags = 0;
//    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ; // CPU에서 읽기 가능
//    desc.Usage = D3D11_USAGE_STAGING; // GPU에서 CPU로 보낼 데이터를 임시 보관
//
//    ComPtr<ID3D11Texture2D> stagingTexture;
//    if (FAILED(GpuCore::Instance().device->CreateTexture2D(
//            &desc, NULL,
//                                       stagingTexture.GetAddressOf()))) {
//        cout << "Failed()" << endl;
//    }
//
//    // 참고: 전체 복사할 때
//    // context->CopyResource(stagingTexture.Get(), pTemp.Get());
//
//    // 일부만 복사할 때 사용
//    D3D11_BOX box;
//    box.left = 0;
//    box.right = desc.Width;
//    box.top = 0;
//    box.bottom = desc.Height;
//    box.front = 0;
//    box.back = 1;
//    GpuCore::Instance().device_context->CopySubresourceRegion(
//        stagingTexture.Get(), 0, 0, 0, 0,
//                                   textureToWrite.Get(), 0, &box);
//
//    // R8G8B8A8 이라고 가정
//    std::vector<uint8_t> pixels(desc.Width * desc.Height * 4);
//
//    D3D11_MAPPED_SUBRESOURCE ms;
//    GpuCore::Instance().device_context->Map(stagingTexture.Get(), NULL,
//                                                 D3D11_MAP_READ, NULL,
//                 &ms); // D3D11_MAP_READ 주의
//
//    // 텍스춰가 작을 경우에는
//    // ms.RowPitch가 width * sizeof(uint8_t) * 4보다 클 수도 있어서
//    // for문으로 가로줄 하나씩 복사
//    uint8_t *pData = (uint8_t *)ms.pData;
//    for (unsigned int h = 0; h < desc.Height; h++) {
//        memcpy(&pixels[h * desc.Width * 4], &pData[h * ms.RowPitch],
//               desc.Width * sizeof(uint8_t) * 4);
//    }
//
//    GpuCore::Instance().device_context->Unmap(stagingTexture.Get(), NULL);
//
//    stbi_write_png(filename.c_str(), desc.Width, desc.Height, 4,
//    pixels.data(),
//                   desc.Width * 4);
//
//    cout << filename << endl;
//}
//
// void Util::CreateUATexture(
//                                   const int width, const int height,
//                                   const DXGI_FORMAT pixelFormat,
//                                   ComPtr<ID3D11Texture2D> &texture,
//                                   ComPtr<ID3D11RenderTargetView> &rtv,
//                                   ComPtr<ID3D11ShaderResourceView> &srv,
//                                   ComPtr<ID3D11UnorderedAccessView> &uav) {
//
//    D3D11_TEXTURE2D_DESC txtDesc;
//    ZeroMemory(&txtDesc, sizeof(txtDesc));
//    txtDesc.Width = width;
//    txtDesc.Height = height;
//    txtDesc.MipLevels = 1;
//    txtDesc.ArraySize = 1;
//    txtDesc.Format = pixelFormat; // 주로 FLOAT 사용
//    txtDesc.SampleDesc.Count = 1;
//    txtDesc.Usage = D3D11_USAGE_DEFAULT;
//    txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET
//    |
//                        D3D11_BIND_UNORDERED_ACCESS;
//    txtDesc.MiscFlags = 0;
//    txtDesc.CPUAccessFlags = 0;
//
//    ThrowIfFailed(GpuCore::Instance().device->CreateTexture2D(
//        &txtDesc, NULL, texture.GetAddressOf()));
//    ThrowIfFailed(GpuCore::Instance().device->CreateRenderTargetView(
//        texture.Get(), NULL,
//                                                 rtv.GetAddressOf()));
//    ThrowIfFailed(GpuCore::Instance().device->CreateShaderResourceView(
//        texture.Get(), NULL,
//                                                   srv.GetAddressOf()));
//    ThrowIfFailed(GpuCore::Instance().device->CreateUnorderedAccessView(
//        texture.Get(), NULL,
//                                                    uav.GetAddressOf()));
//}
//
// void Util::ComputeShaderBarrier() {
//
//    // 최대 사용하는 SRV, UAV 갯수가 6개
//    ID3D11ShaderResourceView *nullSRV[6] = {
//        0,
//    };
//    GpuCore::Instance().device_context->CSSetShaderResources(0, 6,
//                                                                  nullSRV);
//    ID3D11UnorderedAccessView *nullUAV[6] = {
//        0,
//    };
//    GpuCore::Instance().device_context->CSSetUnorderedAccessViews(
//        0, 6, nullUAV, NULL);
//}
//
// ComPtr<ID3D11Texture3D> Util::CreateStagingTexture3D( const int width, const
// int height,
//    const int depth, const DXGI_FORMAT pixelFormat) {
//
//    // 스테이징 텍스춰 만들기
//    D3D11_TEXTURE3D_DESC txtDesc;
//    ZeroMemory(&txtDesc, sizeof(txtDesc));
//    txtDesc.Width = width;
//    txtDesc.Height = height;
//    txtDesc.Depth = depth;
//    txtDesc.MipLevels = 1;
//    txtDesc.Format = pixelFormat;
//    txtDesc.Usage = D3D11_USAGE_STAGING;
//    txtDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
//
//    ComPtr<ID3D11Texture3D> stagingTexture;
//    if (FAILED(GpuCore::Instance().device->CreateTexture3D(
//            &txtDesc, NULL,
//                                       stagingTexture.GetAddressOf()))) {
//        cout << "CreateStagingTexture3D() failed." << endl;
//    }
//
//    return stagingTexture;
//}
//
// size_t Util::GetPixelSize(DXGI_FORMAT pixelFormat) {
//
//    switch (pixelFormat) {
//    case DXGI_FORMAT_R16G16B16A16_FLOAT:
//        return sizeof(uint16_t) * 4;
//    case DXGI_FORMAT_R32G32B32A32_FLOAT:
//        return sizeof(uint32_t) * 4;
//    case DXGI_FORMAT_R32_FLOAT:
//        return sizeof(uint32_t) * 1;
//    case DXGI_FORMAT_R8G8B8A8_UNORM:
//        return sizeof(uint8_t) * 4;
//    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
//        return sizeof(uint8_t) * 4;
//    case DXGI_FORMAT_R32_SINT:
//        return sizeof(int32_t) * 1;
//    case DXGI_FORMAT_R16_FLOAT:
//        return sizeof(uint16_t) * 1;
//    }
//
//    cout << "PixelFormat not implemented " << pixelFormat << endl;
//
//    return sizeof(uint8_t) * 4;
//}
//
// void Util::CreateTexture3D(const int width, const int height,
//    const int depth, const DXGI_FORMAT pixelFormat,
//    const vector<float> &initData, ComPtr<ID3D11Texture3D> &texture,
//    ComPtr<ID3D11RenderTargetView> &rtv, ComPtr<ID3D11ShaderResourceView>
//    &srv, ComPtr<ID3D11UnorderedAccessView> &uav) {
//
//    D3D11_TEXTURE3D_DESC txtDesc;
//    ZeroMemory(&txtDesc, sizeof(txtDesc));
//    txtDesc.Width = width;
//    txtDesc.Height = height;
//    txtDesc.Depth = depth;
//    txtDesc.MipLevels = 1;
//    txtDesc.Format = pixelFormat;
//    txtDesc.Usage = D3D11_USAGE_DEFAULT;
//    txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET
//    |
//                        D3D11_BIND_UNORDERED_ACCESS;
//    txtDesc.MiscFlags = 0;
//    txtDesc.CPUAccessFlags = 0;
//
//    if (initData.size() > 0) {
//        size_t pixelSize = GetPixelSize(pixelFormat);
//        D3D11_SUBRESOURCE_DATA bufferData;
//        ZeroMemory(&bufferData, sizeof(bufferData));
//        bufferData.pSysMem = initData.data();
//        bufferData.SysMemPitch = UINT(width * pixelSize);
//        bufferData.SysMemSlicePitch = UINT(width * height * pixelSize);
//        ThrowIfFailed(GpuCore::Instance().device->CreateTexture3D(
//            &txtDesc, &bufferData,
//                                              texture.GetAddressOf()));
//    } else {
//        ThrowIfFailed(GpuCore::Instance().device->CreateTexture3D(
//            &txtDesc, NULL, texture.GetAddressOf()));
//    }
//
//    ThrowIfFailed(GpuCore::Instance().device->CreateRenderTargetView(
//        texture.Get(), NULL,
//                                                 rtv.GetAddressOf()));
//    ThrowIfFailed(GpuCore::Instance().device->CreateShaderResourceView(
//        texture.Get(), NULL,
//                                                   srv.GetAddressOf()));
//    ThrowIfFailed(GpuCore::Instance().device->CreateUnorderedAccessView(
//        texture.Get(), NULL,
//                                                    uav.GetAddressOf()));
//}
//
// void Util::CreateStructuredBuffer( const UINT numElements,
//    const UINT sizeElement, const void *initData, ComPtr<ID3D11Buffer>
//    &buffer, ComPtr<ID3D11ShaderResourceView> &srv,
//    ComPtr<ID3D11UnorderedAccessView> &uav) {
//
//    D3D11_BUFFER_DESC bufferDesc;
//    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
//    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
//    bufferDesc.ByteWidth = numElements * sizeElement;
//    bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | // Compute Shader
//                           D3D11_BIND_SHADER_RESOURCE;   // Vertex Shader
//    bufferDesc.StructureByteStride = sizeElement;
//    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
//
//    // 참고: Structured는 D3D11_BIND_VERTEX_BUFFER로 사용 불가
//
//    if (initData) {
//        D3D11_SUBRESOURCE_DATA bufferData;
//        ZeroMemory(&bufferData, sizeof(bufferData));
//        bufferData.pSysMem = initData;
//        ThrowIfFailed(GpuCore::Instance().device->CreateBuffer(
//            &bufferDesc, &bufferData,
//                                           buffer.GetAddressOf()));
//    } else {
//        ThrowIfFailed(GpuCore::Instance().device->CreateBuffer(
//            &bufferDesc, NULL, buffer.GetAddressOf()));
//    }
//
//    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
//    ZeroMemory(&uavDesc, sizeof(uavDesc));
//    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
//    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
//    uavDesc.Buffer.NumElements = numElements;
//    GpuCore::Instance().device->CreateUnorderedAccessView(
//        buffer.Get(), &uavDesc,
//                                      uav.GetAddressOf());
//
//    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
//    ZeroMemory(&srvDesc, sizeof(srvDesc));
//    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
//    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
//    srvDesc.BufferEx.NumElements = numElements;
//    GpuCore::Instance().device->CreateShaderResourceView(
//        buffer.Get(), &srvDesc,
//                                     srv.GetAddressOf());
//}
//
// void Util::CreateStagingBuffer(
//                                       const UINT numElements,
//                                       const UINT sizeElement,
//                                       const void *initData,
//                                       ComPtr<ID3D11Buffer> &buffer) {
//
//    D3D11_BUFFER_DESC desc;
//    ZeroMemory(&desc, sizeof(desc));
//    desc.ByteWidth = numElements * sizeElement;
//    desc.Usage = D3D11_USAGE_STAGING;
//    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
//    desc.StructureByteStride = sizeElement;
//
//    if (initData) {
//        D3D11_SUBRESOURCE_DATA bufferData;
//        ZeroMemory(&bufferData, sizeof(bufferData));
//        bufferData.pSysMem = initData;
//        ThrowIfFailed(GpuCore::Instance().device->CreateBuffer(
//            &desc, &bufferData, buffer.GetAddressOf()));
//    } else {
//        ThrowIfFailed(GpuCore::Instance().device->CreateBuffer(
//            &desc, NULL, buffer.GetAddressOf()));
//    }
//}
//
// void Util::SetPipelineState(const PSO &pso) {
//
//    GpuCore::Instance().device_context->VSSetShader(
//        pso.vertex_shader.Get(), 0, 0);
//    GpuCore::Instance().device_context->PSSetShader(pso.pixel_shader.Get(),
//                                                         0, 0);
//    GpuCore::Instance().device_context->HSSetShader(pso.hull_shader.Get(),
//                                                         0, 0);
//    GpuCore::Instance().device_context->DSSetShader(
//        pso.domain_shader.Get(), 0, 0);
//    GpuCore::Instance().device_context->GSSetShader(
//        pso.geometry_shader.Get(), 0, 0);
//    GpuCore::Instance().device_context->IASetInputLayout(
//        pso.input_layout.Get());
//    GpuCore::Instance().device_context->RSSetState(
//        pso.rasterizer_state.Get());
//    GpuCore::Instance().device_context->OMSetBlendState(
//        pso.blend_state.Get(), pso.blend_factor, 0xffffffff);
//    GpuCore::Instance().device_context->OMSetDepthStencilState(
//        pso.depth_stencil_state.Get(), pso.stencil_ref);
//    GpuCore::Instance().device_context->IASetPrimitiveTopology(
//        pso.primitive_topology);
//}
//
// void Util::SetGlobalConsts(ComPtr<ID3D11Buffer> &globalConstsGPU) {
//    // 쉐이더와 일관성 유지 cbuffer GlobalConstants : register(b0)
//    GpuCore::Instance().device_context->VSSetConstantBuffers(
//        0, 1, globalConstsGPU.GetAddressOf());
//    GpuCore::Instance().device_context->PSSetConstantBuffers(
//        0, 1, globalConstsGPU.GetAddressOf());
//    GpuCore::Instance().device_context->GSSetConstantBuffers(
//        0, 1, globalConstsGPU.GetAddressOf());
//}
//
// void Util::CopyToStagingBuffer(ComPtr<ID3D11Buffer> &buffer, UINT size,
//                                void *src) {
//    D3D11_MAPPED_SUBRESOURCE ms;
//    GpuCore::Instance().device_context->Map(buffer.Get(), NULL,
//                                                 D3D11_MAP_WRITE, NULL, &ms);
//    memcpy(ms.pData, src, size);
//    GpuCore::Instance().device_context->Unmap(buffer.Get(), NULL);
//}

} // namespace dx12