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

// void Util::CreateIndexBuffer(const std::vector<uint32_t> &indices,
//                              ComPtr<ID3D12Resource> &indexBuffer) {
//
//     const UINT size = sizeof(indices);
//
//     HRESULT hr = GpuCore::Instance().device->CreateCommittedResource(
//         &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
//         D3D12_HEAP_FLAG_NONE, &CD3DX12_RESOURCE_DESC::Buffer(size),
//         D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
//         IID_PPV_ARGS(&indexBuffer));
// }
//
//  void ReadEXRImage(const std::string filename, std::vector<uint8_t> &image,
//                   int &width, int &height, DXGI_FORMAT &pixelFormat) {
//
//     const std::wstring wFilename(filename.begin(), filename.end());
//
//     TexMetadata metadata;
//     ThrowIfFailed(GetMetadataFromEXRFile(wFilename.c_str(), metadata));
//
//     ScratchImage scratchImage;
//     ThrowIfFailed(LoadFromEXRFile(wFilename.c_str(), NULL, scratchImage));
//
//     width = static_cast<int>(metadata.width);
//     height = static_cast<int>(metadata.height);
//     pixelFormat = metadata.format;
//
//     cout << filename << " " << metadata.width << " " << metadata.height
//          << metadata.format << endl;
//
//     image.resize(scratchImage.GetPixelsSize());
//     memcpy(image.data(), scratchImage.GetPixels(), image.size());
//
//     vector<float> f32(image.size() / 2);
//     uint16_t *f16 = (uint16_t *)image.data();
//     for (int i = 0; i < image.size() / 2; i++) {
//         f32[i] = fp16_ieee_to_fp32_value(f16[i]);
//     }
//
//     const float minValue = *std::min_element(f32.begin(), f32.end());
//     const float maxValue = *std::max_element(f32.begin(), f32.end());
//
//     cout << minValue << " " << maxValue << endl;
//
//     // f16 = (uint16_t *)image.data();
//     // for (int i = 0; i < image.size() / 2; i++) {
//     //     f16[i] = fp16_ieee_from_fp32_value(f32[i] * 2.0f);
//     // }
// }
//
//  void ReadImage(const std::string filename, std::vector<uint8_t> &image,
//                int &width, int &height) {
//
//     int channels;
//
//     unsigned char *img =
//         stbi_load(filename.c_str(), &width, &height, &channels, 0);
//
//     // assert(channels == 4);
//
//     cout << filename << " " << width << " " << height << " " << channels
//          << endl;
//
//     // to 4 chanel
//     image.resize(width * height * 4);
//
//     if (channels == 1) {
//         for (size_t i = 0; i < width * height; i++) {
//             uint8_t g = img[i * channels + 0];
//             for (size_t c = 0; c < 4; c++) {
//                 image[4 * i + c] = g;
//             }
//         }
//     } else if (channels == 2) {
//         for (size_t i = 0; i < width * height; i++) {
//             for (size_t c = 0; c < 2; c++) {
//                 image[4 * i + c] = img[i * channels + c];
//             }
//             image[4 * i + 2] = 255;
//             image[4 * i + 3] = 255;
//         }
//     } else if (channels == 3) {
//         for (size_t i = 0; i < width * height; i++) {
//             for (size_t c = 0; c < 3; c++) {
//                 image[4 * i + c] = img[i * channels + c];
//             }
//             image[4 * i + 3] = 255;
//         }
//     } else if (channels == 4) {
//         for (size_t i = 0; i < width * height; i++) {
//             for (size_t c = 0; c < 4; c++) {
//                 image[4 * i + c] = img[i * channels + c];
//             }
//         }
//     } else {
//         std::cout << "Cannot read " << channels << " channels" << endl;
//     }
//
//     delete[] img;
// }
//
//  void ReadImage(const std::string albedoFilename,
//                const std::string opacityFilename, std::vector<uint8_t>
//                &image, int &width, int &height) {
//
//     ReadImage(albedoFilename, image, width, height);
//
//     std::vector<uint8_t> opacityImage;
//     int opaWidth, opaHeight;
//
//     ReadImage(opacityFilename, opacityImage, opaWidth, opaHeight);
//
//     assert(width == opaWidth && height == opaHeight);
//
//     for (int j = 0; j < height; j++)
//         for (int i = 0; i < width; i++) {
//             image[3 + 4 * i + 4 * width * j] =
//                 opacityImage[4 * i + 4 * width * j]; // Copy alpha channel
//         }
// }
//
//  ComPtr<ID3D11Texture2D>
//  CreateStagingTexture(const int width,
//                      const int height, const std::vector<uint8_t> &image,
//                      const DXGI_FORMAT pixelFormat =
//                      DXGI_FORMAT_R8G8B8A8_UNORM, const int mipLevels = 1,
//                      const int arraySize = 1) {
//
//     D3D11_TEXTURE2D_DESC txtDesc;
//     ZeroMemory(&txtDesc, sizeof(txtDesc));
//     txtDesc.Width = width;
//     txtDesc.Height = height;
//     txtDesc.MipLevels = mipLevels;
//     txtDesc.ArraySize = arraySize;
//     txtDesc.Format = pixelFormat;
//     txtDesc.SampleDesc.Count = 1;
//     txtDesc.Usage = D3D11_USAGE_STAGING;
//     txtDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
//
//     ComPtr<ID3D11Texture2D> stagingTexture;
//     if (FAILED(GpuCore::Instance().device->CreateTexture2D(
//             &txtDesc, NULL,
//                                        stagingTexture.GetAddressOf()))) {
//         cout << "Failed()" << endl;
//     }
//
//     size_t pixelSize = sizeof(uint8_t) * 4;
//     if (pixelFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) {
//         pixelSize = sizeof(uint16_t) * 4;
//     }
//
//     D3D11_MAPPED_SUBRESOURCE ms;
//     GpuCore::Instance().device_context->Map(stagingTexture.Get(), NULL,
//                                                  D3D11_MAP_WRITE, NULL, &ms);
//     uint8_t *pData = (uint8_t *)ms.pData;
//     for (UINT h = 0; h < UINT(height); h++) {
//         memcpy(&pData[h * ms.RowPitch], &image[h * width * pixelSize],
//                width * pixelSize);
//     }
//     GpuCore::Instance().device_context->Unmap(stagingTexture.Get(), NULL);
//
//     return stagingTexture;
// }
//
//  void CreateTextureHelper(const int width,
//                          const int height, const vector<uint8_t> &image,
//                          const DXGI_FORMAT pixelFormat,
//                          ComPtr<ID3D11Texture2D> &texture,
//                          ComPtr<ID3D11ShaderResourceView> &srv) {
//
//     ComPtr<ID3D11Texture2D> stagingTexture = CreateStagingTexture(width,
//                              height, image, pixelFormat);
//
//     D3D11_TEXTURE2D_DESC txtDesc;
//     ZeroMemory(&txtDesc, sizeof(txtDesc));
//     txtDesc.Width = width;
//     txtDesc.Height = height;
//     txtDesc.MipLevels = 0;
//     txtDesc.ArraySize = 1;
//     txtDesc.Format = pixelFormat;
//     txtDesc.SampleDesc.Count = 1;
//     txtDesc.Usage = D3D11_USAGE_DEFAULT; // ?ㅽ뀒?댁쭠 ?띿뒪異곕줈遺??蹂듭궗 媛??
//     txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE |
//     D3D11_BIND_RENDER_TARGET; txtDesc.MiscFlags =
//     D3D11_RESOURCE_MISC_GENERATE_MIPS; // 諛됰㏊ ?ъ슜 txtDesc.CPUAccessFlags =
//     0;
//
//     // 珥덇린 ?곗씠???놁씠 ?띿뒪異??앹꽦 (?꾨? 寃???
//     GpuCore::Instance().device->CreateTexture2D(
//         &txtDesc, NULL, texture.GetAddressOf());
//
//     // ?ㅼ젣濡??앹꽦??MipLevels瑜??뺤씤?대낫怨??띠쓣 寃쎌슦
//     // texture->GetDesc(&txtDesc);
//     // cout << txtDesc.MipLevels << endl;
//
//     // ?ㅽ뀒?댁쭠 ?띿뒪異곕줈遺??媛???댁긽?꾧? ?믪? ?대?吏 蹂듭궗
//     GpuCore::Instance().device_context->CopySubresourceRegion(
//         texture.Get(), 0, 0, 0, 0,
//                                    stagingTexture.Get(), 0, NULL);
//
//     // ResourceView 留뚮뱾湲?
//     GpuCore::Instance().device->CreateShaderResourceView(
//         texture.Get(), 0, srv.GetAddressOf());
//
//     // ?댁긽?꾨? ??떠媛硫?諛됰㏊ ?앹꽦
//     GpuCore::Instance().device_context->GenerateMips(srv.Get());
//
//     // HLSL ?먯씠???덉뿉?쒕뒗 SampleLevel() ?ъ슜
// }
//
//  void Util::CreateMetallicRoughnessTexture(
//     const std::string metallicFilename, const std::string roughnessFilename,
//     ComPtr<ID3D11Texture2D> &texture, ComPtr<ID3D11ShaderResourceView> &srv)
//     {
//
//     // GLTF 諛⑹떇? ?대? ?⑹퀜???덉쓬
//     if (!metallicFilename.empty() && (metallicFilename == roughnessFilename))
//     {
//         CreateTexture(metallicFilename, false, texture, srv);
//     } else {
//         // 蹂꾨룄 ?뚯씪??寃쎌슦 ?곕줈 ?쎌뼱???⑹퀜以띾땲??
//
//         // ReadImage()瑜??쒖슜?섍린 ?꾪빐?????대?吏?ㅼ쓣 媛곴컖 4梨꾨꼸濡?蹂????
//         ?ㅼ떆
//         // 3梨꾨꼸濡??⑹튂??諛⑹떇?쇰줈 援ы쁽
//         int mWidth = 0, mHeight = 0;
//         int rWidth = 0, rHeight = 0;
//         std::vector<uint8_t> mImage;
//         std::vector<uint8_t> rImage;
//
//         // (嫄곗쓽 ?녾쿋吏留? ??以??섎굹留??덉쓣 寃쎌슦??怨좊젮?섍린 ?꾪빐 媛곴컖 ?뚯씪紐?
//         // ?뺤씤
//         if (!metallicFilename.empty()) {
//             ReadImage(metallicFilename, mImage, mWidth, mHeight);
//         }
//
//         if (!roughnessFilename.empty()) {
//             ReadImage(roughnessFilename, rImage, rWidth, rHeight);
//         }
//
//         // ???대?吏???댁긽?꾧? 媛숇떎怨?媛??
//         if (!metallicFilename.empty() && !roughnessFilename.empty()) {
//             assert(mWidth == rWidth);
//             assert(mHeight == rHeight);
//         }
//
//         vector<uint8_t> combinedImage(size_t(mWidth * mHeight) * 4);
//         fill(combinedImage.begin(), combinedImage.end(), 0);
//
//         for (size_t i = 0; i < size_t(mWidth * mHeight); i++) {
//             if (rImage.size())
//                 combinedImage[4 * i + 1] = rImage[4 * i]; // Green =
//                 Roughness
//             if (mImage.size())
//                 combinedImage[4 * i + 2] = mImage[4 * i]; // Blue = Metalness
//         }
//
//         CreateTextureHelper( mWidth, mHeight, combinedImage,
//                             DXGI_FORMAT_R8G8B8A8_UNORM, texture, srv);
//     }
// }
//
//  void Util::CreateTexture(
//                                  const std::string filename, const bool
//                                  usSRGB, ComPtr<ID3D11Texture2D> &tex,
//                                  ComPtr<ID3D11ShaderResourceView> &srv) {
//
//     int width = 0, height = 0;
//     std::vector<uint8_t> image;
//     DXGI_FORMAT pixelFormat =
//         usSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB :
//         DXGI_FORMAT_R8G8B8A8_UNORM;
//
//     string ext(filename.end() - 3, filename.end());
//     std::transform(ext.begin(), ext.end(), ext.begin(), std::tolower);
//
//     if (ext == "exr") {
//         ReadEXRImage(filename, image, width, height, pixelFormat);
//     } else {
//         ReadImage(filename, image, width, height);
//     }
//
//     CreateTextureHelper( width, height, image, pixelFormat, tex,
//                         srv);
// }
//
//  void Util::CreateTexture(
//                                  const std::string albedoFilename,
//                                  const std::string opacityFilename,
//                                  const bool usSRGB,
//                                  ComPtr<ID3D11Texture2D> &texture,
//                                  ComPtr<ID3D11ShaderResourceView> &srv) {
//
//     int width = 0, height = 0;
//     std::vector<uint8_t> image;
//     DXGI_FORMAT pixelFormat =
//         usSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB :
//         DXGI_FORMAT_R8G8B8A8_UNORM;
//
//     ReadImage(albedoFilename, opacityFilename, image, width, height);
//
//     CreateTextureHelper(width, height, image, pixelFormat,
//                         texture, srv);
// }
//
//  void Util::CreateTextureArray(
//     const std::vector<std::string> filenames, ComPtr<ID3D11Texture2D>
//     &texture, ComPtr<ID3D11ShaderResourceView> &textureResourceView) {
//
//     using namespace std;
//
//     if (filenames.empty())
//         return;
//
//     // 紐⑤뱺 ?대?吏??width? height媛 媛숇떎怨?媛?뺥빀?덈떎.
//
//     // ?뚯씪濡쒕????대?吏 ?щ윭 媛쒕? ?쎌뼱?ㅼ엯?덈떎.
//     int width = 0, height = 0;
//     vector<vector<uint8_t>> imageArray;
//     for (const auto &f : filenames) {
//
//         cout << f << endl;
//
//         std::vector<uint8_t> image;
//
//         ReadImage(f, image, width, height);
//
//         imageArray.push_back(image);
//     }
//
//     UINT size = UINT(filenames.size());
//
//     // Texture2DArray瑜?留뚮벊?덈떎. ?대븣 ?곗씠?곕? CPU濡쒕???蹂듭궗?섏? ?딆뒿?덈떎.
//     D3D11_TEXTURE2D_DESC txtDesc;
//     ZeroMemory(&txtDesc, sizeof(txtDesc));
//     txtDesc.Width = UINT(width);
//     txtDesc.Height = UINT(height);
//     txtDesc.MipLevels = 0; // 諛됰㏊ ?덈꺼 理쒕?
//     txtDesc.ArraySize = size;
//     txtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//     txtDesc.SampleDesc.Count = 1;
//     txtDesc.SampleDesc.Quality = 0;
//     txtDesc.Usage = D3D11_USAGE_DEFAULT; // ?ㅽ뀒?댁쭠 ?띿뒪異곕줈遺??蹂듭궗 媛??
//     txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE |
//     D3D11_BIND_RENDER_TARGET; txtDesc.MiscFlags =
//     D3D11_RESOURCE_MISC_GENERATE_MIPS; // 諛됰㏊ ?ъ슜
//
//     // 珥덇린 ?곗씠???놁씠 ?띿뒪異곕? 留뚮벊?덈떎.
//     GpuCore::Instance().device->CreateTexture2D(&txtDesc, NULL,
//                                                      texture.GetAddressOf());
//
//     // ?ㅼ젣濡?留뚮뱾?댁쭊 MipLevels瑜??뺤씤
//     texture->GetDesc(&txtDesc);
//     // cout << txtDesc.MipLevels << endl;
//
//     // StagingTexture瑜?留뚮뱾?댁꽌 ?섎굹??蹂듭궗?⑸땲??
//     for (size_t i = 0; i < imageArray.size(); i++) {
//
//         auto &image = imageArray[i];
//
//         // StagingTexture??Texture2DArray媛 ?꾨땲??Texture2D ?낅땲??
//         ComPtr<ID3D11Texture2D> stagingTexture = CreateStagingTexture(
//              width, height, image, txtDesc.Format, 1, 1);
//
//         // ?ㅽ뀒?댁쭠 ?띿뒪異곕? ?띿뒪異?諛곗뿴???대떦 ?꾩튂??蹂듭궗?⑸땲??
//         UINT subresourceIndex =
//             D3D11CalcSubresource(0, UINT(i), txtDesc.MipLevels);
//
//         GpuCore::Instance().device_context->CopySubresourceRegion(
//             texture.Get(), subresourceIndex, 0, 0, 0,
//                                        stagingTexture.Get(), 0, NULL);
//     }
//
//     GpuCore::Instance().device->CreateShaderResourceView(
//         texture.Get(), NULL,
//                                      textureResourceView.GetAddressOf());
//
//     GpuCore::Instance().device_context->GenerateMips(
//         textureResourceView.Get());
// }
//
//  void Util::CreateDDSTexture( const wchar_t *filename, bool isCubeMap,
//     ComPtr<ID3D11ShaderResourceView> &textureResourceView) {
//
//     ComPtr<ID3D11Texture2D> texture;
//
//     UINT miscFlags = 0;
//     if (isCubeMap) {
//         miscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
//     }
//
//     // https://github.com/microsoft/DirectXTK/wiki/DDSTextureLoader
//     ThrowIfFailed(CreateDDSTextureFromFileEx(
//         GpuCore::Instance().device.Get(), filename, 0, D3D11_USAGE_DEFAULT,
//         D3D11_BIND_SHADER_RESOURCE, 0, miscFlags, DDS_LOADER_FLAGS(false),
//         (ID3D11Resource **)texture.GetAddressOf(),
//         textureResourceView.GetAddressOf(), NULL));
// }
//
//  void Util::WriteToFile(
//                                ComPtr<ID3D11Texture2D> &textureToWrite,
//                                const std::string filename) {
//
//     D3D11_TEXTURE2D_DESC desc;
//     textureToWrite->GetDesc(&desc);
//     desc.SampleDesc.Count = 1;
//     desc.SampleDesc.Quality = 0;
//     desc.BindFlags = 0;
//     desc.MiscFlags = 0;
//     desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ; // CPU?먯꽌 ?쎄린 媛??
//     desc.Usage = D3D11_USAGE_STAGING; // GPU?먯꽌 CPU濡?蹂대궪 ?곗씠?곕? ?꾩떆
//     蹂닿?
//
//     ComPtr<ID3D11Texture2D> stagingTexture;
//     if (FAILED(GpuCore::Instance().device->CreateTexture2D(
//             &desc, NULL,
//                                        stagingTexture.GetAddressOf()))) {
//         cout << "Failed()" << endl;
//     }
//
//     // 李멸퀬: ?꾩껜 蹂듭궗????
//     // context->CopyResource(stagingTexture.Get(), pTemp.Get());
//
//     // ?쇰?留?蹂듭궗?????ъ슜
//     D3D11_BOX box;
//     box.left = 0;
//     box.right = desc.Width;
//     box.top = 0;
//     box.bottom = desc.Height;
//     box.front = 0;
//     box.back = 1;
//     GpuCore::Instance().device_context->CopySubresourceRegion(
//         stagingTexture.Get(), 0, 0, 0, 0,
//                                    textureToWrite.Get(), 0, &box);
//
//     // R8G8B8A8 ?대씪怨?媛??
//     std::vector<uint8_t> pixels(desc.Width * desc.Height * 4);
//
//     D3D11_MAPPED_SUBRESOURCE ms;
//     GpuCore::Instance().device_context->Map(stagingTexture.Get(), NULL,
//                                                  D3D11_MAP_READ, NULL,
//                  &ms); // D3D11_MAP_READ 二쇱쓽
//
//     // ?띿뒪異곌? ?묒쓣 寃쎌슦?먮뒗
//     // ms.RowPitch媛 width * sizeof(uint8_t) * 4蹂대떎 ???섎룄 ?덉뼱??
//     // for臾몄쑝濡?媛濡쒖쨪 ?섎굹??蹂듭궗
//     uint8_t *pData = (uint8_t *)ms.pData;
//     for (unsigned int h = 0; h < desc.Height; h++) {
//         memcpy(&pixels[h * desc.Width * 4], &pData[h * ms.RowPitch],
//                desc.Width * sizeof(uint8_t) * 4);
//     }
//
//     GpuCore::Instance().device_context->Unmap(stagingTexture.Get(), NULL);
//
//     stbi_write_png(filename.c_str(), desc.Width, desc.Height, 4,
//     pixels.data(),
//                    desc.Width * 4);
//
//     cout << filename << endl;
// }
//
//  void Util::CreateUATexture(
//                                    const int width, const int height,
//                                    const DXGI_FORMAT pixelFormat,
//                                    ComPtr<ID3D11Texture2D> &texture,
//                                    ComPtr<ID3D11RenderTargetView> &rtv,
//                                    ComPtr<ID3D11ShaderResourceView> &srv,
//                                    ComPtr<ID3D11UnorderedAccessView> &uav) {
//
//     D3D11_TEXTURE2D_DESC txtDesc;
//     ZeroMemory(&txtDesc, sizeof(txtDesc));
//     txtDesc.Width = width;
//     txtDesc.Height = height;
//     txtDesc.MipLevels = 1;
//     txtDesc.ArraySize = 1;
//     txtDesc.Format = pixelFormat; // 二쇰줈 FLOAT ?ъ슜
//     txtDesc.SampleDesc.Count = 1;
//     txtDesc.Usage = D3D11_USAGE_DEFAULT;
//     txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET
//     |
//                         D3D11_BIND_UNORDERED_ACCESS;
//     txtDesc.MiscFlags = 0;
//     txtDesc.CPUAccessFlags = 0;
//
//     ThrowIfFailed(GpuCore::Instance().device->CreateTexture2D(
//         &txtDesc, NULL, texture.GetAddressOf()));
//     ThrowIfFailed(GpuCore::Instance().device->CreateRenderTargetView(
//         texture.Get(), NULL,
//                                                  rtv.GetAddressOf()));
//     ThrowIfFailed(GpuCore::Instance().device->CreateShaderResourceView(
//         texture.Get(), NULL,
//                                                    srv.GetAddressOf()));
//     ThrowIfFailed(GpuCore::Instance().device->CreateUnorderedAccessView(
//         texture.Get(), NULL,
//                                                     uav.GetAddressOf()));
// }
//
//  void Util::ComputeShaderBarrier() {
//
//     // 理쒕? ?ъ슜?섎뒗 SRV, UAV 媛?닔媛 6媛?
//     ID3D11ShaderResourceView *nullSRV[6] = {
//         0,
//     };
//     GpuCore::Instance().device_context->CSSetShaderResources(0, 6,
//                                                                   nullSRV);
//     ID3D11UnorderedAccessView *nullUAV[6] = {
//         0,
//     };
//     GpuCore::Instance().device_context->CSSetUnorderedAccessViews(
//         0, 6, nullUAV, NULL);
// }
//
//  ComPtr<ID3D11Texture3D> Util::CreateStagingTexture3D( const int width, const
//  int height,
//     const int depth, const DXGI_FORMAT pixelFormat) {
//
//     // ?ㅽ뀒?댁쭠 ?띿뒪異?留뚮뱾湲?
//     D3D11_TEXTURE3D_DESC txtDesc;
//     ZeroMemory(&txtDesc, sizeof(txtDesc));
//     txtDesc.Width = width;
//     txtDesc.Height = height;
//     txtDesc.Depth = depth;
//     txtDesc.MipLevels = 1;
//     txtDesc.Format = pixelFormat;
//     txtDesc.Usage = D3D11_USAGE_STAGING;
//     txtDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
//
//     ComPtr<ID3D11Texture3D> stagingTexture;
//     if (FAILED(GpuCore::Instance().device->CreateTexture3D(
//             &txtDesc, NULL,
//                                        stagingTexture.GetAddressOf()))) {
//         cout << "CreateStagingTexture3D() failed." << endl;
//     }
//
//     return stagingTexture;
// }
//
//  size_t Util::GetPixelSize(DXGI_FORMAT pixelFormat) {
//
//     switch (pixelFormat) {
//     case DXGI_FORMAT_R16G16B16A16_FLOAT:
//         return sizeof(uint16_t) * 4;
//     case DXGI_FORMAT_R32G32B32A32_FLOAT:
//         return sizeof(uint32_t) * 4;
//     case DXGI_FORMAT_R32_FLOAT:
//         return sizeof(uint32_t) * 1;
//     case DXGI_FORMAT_R8G8B8A8_UNORM:
//         return sizeof(uint8_t) * 4;
//     case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
//         return sizeof(uint8_t) * 4;
//     case DXGI_FORMAT_R32_SINT:
//         return sizeof(int32_t) * 1;
//     case DXGI_FORMAT_R16_FLOAT:
//         return sizeof(uint16_t) * 1;
//     }
//
//     cout << "PixelFormat not implemented " << pixelFormat << endl;
//
//     return sizeof(uint8_t) * 4;
// }
//
//  void Util::CreateTexture3D(const int width, const int height,
//     const int depth, const DXGI_FORMAT pixelFormat,
//     const vector<float> &initData, ComPtr<ID3D11Texture3D> &texture,
//     ComPtr<ID3D11RenderTargetView> &rtv, ComPtr<ID3D11ShaderResourceView>
//     &srv, ComPtr<ID3D11UnorderedAccessView> &uav) {
//
//     D3D11_TEXTURE3D_DESC txtDesc;
//     ZeroMemory(&txtDesc, sizeof(txtDesc));
//     txtDesc.Width = width;
//     txtDesc.Height = height;
//     txtDesc.Depth = depth;
//     txtDesc.MipLevels = 1;
//     txtDesc.Format = pixelFormat;
//     txtDesc.Usage = D3D11_USAGE_DEFAULT;
//     txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET
//     |
//                         D3D11_BIND_UNORDERED_ACCESS;
//     txtDesc.MiscFlags = 0;
//     txtDesc.CPUAccessFlags = 0;
//
//     if (initData.size() > 0) {
//         size_t pixelSize = GetPixelSize(pixelFormat);
//         D3D11_SUBRESOURCE_DATA bufferData;
//         ZeroMemory(&bufferData, sizeof(bufferData));
//         bufferData.pSysMem = initData.data();
//         bufferData.SysMemPitch = UINT(width * pixelSize);
//         bufferData.SysMemSlicePitch = UINT(width * height * pixelSize);
//         ThrowIfFailed(GpuCore::Instance().device->CreateTexture3D(
//             &txtDesc, &bufferData,
//                                               texture.GetAddressOf()));
//     } else {
//         ThrowIfFailed(GpuCore::Instance().device->CreateTexture3D(
//             &txtDesc, NULL, texture.GetAddressOf()));
//     }
//
//     ThrowIfFailed(GpuCore::Instance().device->CreateRenderTargetView(
//         texture.Get(), NULL,
//                                                  rtv.GetAddressOf()));
//     ThrowIfFailed(GpuCore::Instance().device->CreateShaderResourceView(
//         texture.Get(), NULL,
//                                                    srv.GetAddressOf()));
//     ThrowIfFailed(GpuCore::Instance().device->CreateUnorderedAccessView(
//         texture.Get(), NULL,
//                                                     uav.GetAddressOf()));
// }
//
//  void Util::CreateStructuredBuffer( const UINT numElements,
//     const UINT sizeElement, const void *initData, ComPtr<ID3D11Buffer>
//     &buffer, ComPtr<ID3D11ShaderResourceView> &srv,
//     ComPtr<ID3D11UnorderedAccessView> &uav) {
//
//     D3D11_BUFFER_DESC bufferDesc;
//     ZeroMemory(&bufferDesc, sizeof(bufferDesc));
//     bufferDesc.Usage = D3D11_USAGE_DEFAULT;
//     bufferDesc.ByteWidth = numElements * sizeElement;
//     bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | // Compute Shader
//                            D3D11_BIND_SHADER_RESOURCE;   // Vertex Shader
//     bufferDesc.StructureByteStride = sizeElement;
//     bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
//
//     // 李멸퀬: Structured??D3D11_BIND_VERTEX_BUFFER濡??ъ슜 遺덇?
//
//     if (initData) {
//         D3D11_SUBRESOURCE_DATA bufferData;
//         ZeroMemory(&bufferData, sizeof(bufferData));
//         bufferData.pSysMem = initData;
//         ThrowIfFailed(GpuCore::Instance().device->CreateBuffer(
//             &bufferDesc, &bufferData,
//                                            buffer.GetAddressOf()));
//     } else {
//         ThrowIfFailed(GpuCore::Instance().device->CreateBuffer(
//             &bufferDesc, NULL, buffer.GetAddressOf()));
//     }
//
//     D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
//     ZeroMemory(&uavDesc, sizeof(uavDesc));
//     uavDesc.Format = DXGI_FORMAT_UNKNOWN;
//     uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
//     uavDesc.Buffer.NumElements = numElements;
//     GpuCore::Instance().device->CreateUnorderedAccessView(
//         buffer.Get(), &uavDesc,
//                                       uav.GetAddressOf());
//
//     D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
//     ZeroMemory(&srvDesc, sizeof(srvDesc));
//     srvDesc.Format = DXGI_FORMAT_UNKNOWN;
//     srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
//     srvDesc.BufferEx.NumElements = numElements;
//     GpuCore::Instance().device->CreateShaderResourceView(
//         buffer.Get(), &srvDesc,
//                                      srv.GetAddressOf());
// }
//
//  void Util::CreateStagingBuffer(
//                                        const UINT numElements,
//                                        const UINT sizeElement,
//                                        const void *initData,
//                                        ComPtr<ID3D11Buffer> &buffer) {
//
//     D3D11_BUFFER_DESC desc;
//     ZeroMemory(&desc, sizeof(desc));
//     desc.ByteWidth = numElements * sizeElement;
//     desc.Usage = D3D11_USAGE_STAGING;
//     desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
//     desc.StructureByteStride = sizeElement;
//
//     if (initData) {
//         D3D11_SUBRESOURCE_DATA bufferData;
//         ZeroMemory(&bufferData, sizeof(bufferData));
//         bufferData.pSysMem = initData;
//         ThrowIfFailed(GpuCore::Instance().device->CreateBuffer(
//             &desc, &bufferData, buffer.GetAddressOf()));
//     } else {
//         ThrowIfFailed(GpuCore::Instance().device->CreateBuffer(
//             &desc, NULL, buffer.GetAddressOf()));
//     }
// }
//
//  void Util::SetPipelineState(const PSO &pso) {
//
//     GpuCore::Instance().device_context->VSSetShader(
//         pso.vertex_shader.Get(), 0, 0);
//     GpuCore::Instance().device_context->PSSetShader(pso.pixel_shader.Get(),
//                                                          0, 0);
//     GpuCore::Instance().device_context->HSSetShader(pso.hull_shader.Get(),
//                                                          0, 0);
//     GpuCore::Instance().device_context->DSSetShader(
//         pso.domain_shader.Get(), 0, 0);
//     GpuCore::Instance().device_context->GSSetShader(
//         pso.geometry_shader.Get(), 0, 0);
//     GpuCore::Instance().device_context->IASetInputLayout(
//         pso.input_layout.Get());
//     GpuCore::Instance().device_context->RSSetState(
//         pso.rasterizer_state.Get());
//     GpuCore::Instance().device_context->OMSetBlendState(
//         pso.blend_state.Get(), pso.blend_factor, 0xffffffff);
//     GpuCore::Instance().device_context->OMSetDepthStencilState(
//         pso.depth_stencil_state.Get(), pso.stencil_ref);
//     GpuCore::Instance().device_context->IASetPrimitiveTopology(
//         pso.primitive_topology);
// }
//
//  void Util::SetGlobalConsts(ComPtr<ID3D11Buffer> &globalConstsGPU) {
//     // ?먯씠?붿? ?쇨????좎? cbuffer GlobalConstants : register(b0)
//     GpuCore::Instance().device_context->VSSetConstantBuffers(
//         0, 1, globalConstsGPU.GetAddressOf());
//     GpuCore::Instance().device_context->PSSetConstantBuffers(
//         0, 1, globalConstsGPU.GetAddressOf());
//     GpuCore::Instance().device_context->GSSetConstantBuffers(
//         0, 1, globalConstsGPU.GetAddressOf());
// }
//
//  void Util::CopyToStagingBuffer(ComPtr<ID3D11Buffer> &buffer, UINT size,
//                                 void *src) {
//     D3D11_MAPPED_SUBRESOURCE ms;
//     GpuCore::Instance().device_context->Map(buffer.Get(), NULL,
//                                                  D3D11_MAP_WRITE, NULL, &ms);
//     memcpy(ms.pData, src, size);
//     GpuCore::Instance().device_context->Unmap(buffer.Get(), NULL);
// }

void CheckResult(HRESULT hr, ID3DBlob *errorBlob) {
    if (FAILED(hr)) {
        if ((hr & D3D11_ERROR_FILE_NOT_FOUND) != 0) {
            std::cout << "File not found." << std::endl;
        }
        if (errorBlob) {
            std::cout << "Shader compile error\n"
                      << (char *)errorBlob->GetBufferPointer() << std::endl;
        }
    }
}

void Util::CreateVertexShader(
    ComPtr<ID3D12Device> &device, std::wstring filename,
    ComPtr<ID3DBlob> &m_vertexShader,
    const std::vector<D3D_SHADER_MACRO> shaderMacros) {

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), shaderMacros.empty() ? NULL : shaderMacros.data(),
        D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", compileFlags, 0,
        &m_vertexShader, &errorBlob);
    CheckResult(hr, errorBlob.Get());
}

void Util::CreatePixelShader(ComPtr<ID3D12Device> &device,
                             std::wstring filename, ComPtr<ID3DBlob> &shader) {
    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "ps_5_0", compileFlags, 0, &shader, &errorBlob);
    CheckResult(hr, errorBlob.Get());
}
} // namespace dx12
