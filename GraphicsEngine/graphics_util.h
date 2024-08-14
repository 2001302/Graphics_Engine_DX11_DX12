#ifndef _GRAPHICSUTIL
#define _GRAPHICSUTIL

#include "graphics_core.h"

namespace graphics {
class Util {
  public:
    static void CreateIndexBuffer(const std::vector<uint32_t> &indices,
                                  ComPtr<ID3D11Buffer> &indexBuffer);

    template <typename T_VERTEX>
    static void CreateVertexBuffer(const std::vector<T_VERTEX> &vertices,
                                   ComPtr<ID3D11Buffer> &vertexBuffer) {

        // D3D11_USAGE enumeration (d3d11.h)
        // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_usage

        D3D11_BUFFER_DESC bufferDesc;
        ZeroMemory(&bufferDesc, sizeof(bufferDesc));
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.ByteWidth = UINT(sizeof(T_VERTEX) * vertices.size());
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = 0; // 0 if no CPU access is necessary.
        bufferDesc.StructureByteStride = sizeof(T_VERTEX);

        D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
        vertexBufferData.pSysMem = vertices.data();
        vertexBufferData.SysMemPitch = 0;
        vertexBufferData.SysMemSlicePitch = 0;

        ThrowIfFailed(GraphicsCore::Instance().device->CreateBuffer(
            &bufferDesc, &vertexBufferData, vertexBuffer.GetAddressOf()));
    }

    template <typename T_CONSTANT>
    static void CreateConstBuffer(const T_CONSTANT &constantBufferData,
                                  ComPtr<ID3D11Buffer> &constantBuffer) {

        static_assert((sizeof(T_CONSTANT) % 16) == 0,
                      "Constant Buffer size must be 16-byte aligned");

        D3D11_BUFFER_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.ByteWidth = sizeof(constantBufferData);
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA initData;
        ZeroMemory(&initData, sizeof(initData));
        initData.pSysMem = &constantBufferData;
        initData.SysMemPitch = 0;
        initData.SysMemSlicePitch = 0;

        ThrowIfFailed(GraphicsCore::Instance().device->CreateBuffer(
            &desc, &initData, constantBuffer.GetAddressOf()));
    }

    template <typename T_DATA>
    static void UpdateBuffer(const std::vector<T_DATA> &bufferData,
                             ComPtr<ID3D11Buffer> &buffer) {

        if (!buffer) {
            std::cout << "UpdateBuffer() buffer was not initialized."
                      << std::endl;
        }

        D3D11_MAPPED_SUBRESOURCE ms;
        GraphicsCore::Instance().device_context->Map(
            buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
        memcpy(ms.pData, bufferData.data(), sizeof(T_DATA) * bufferData.size());
        GraphicsCore::Instance().device_context->Unmap(buffer.Get(), NULL);
    }

    template <typename T_DATA>
    static void UpdateBuffer(const T_DATA &bufferData,
                             ComPtr<ID3D11Buffer> &buffer) {

        if (!buffer) {
            std::cout << "UpdateBuffer() buffer was not initialized."
                      << std::endl;
        }

        D3D11_MAPPED_SUBRESOURCE ms;
        GraphicsCore::Instance().device_context->Map(
            buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
        memcpy(ms.pData, &bufferData, sizeof(bufferData));
        GraphicsCore::Instance().device_context->Unmap(buffer.Get(), NULL);
    }

    static void CreateTexture(

        const std::string filename, const bool usSRGB,
        ComPtr<ID3D11Texture2D> &texture,
        ComPtr<ID3D11ShaderResourceView> &textureResourceView);

    static void CreateTexture(

        const std::string albedoFilename, const std::string opacityFilename,
        const bool usSRGB, ComPtr<ID3D11Texture2D> &texture,
        ComPtr<ID3D11ShaderResourceView> &textureResourceView);

    static void CreateMetallicRoughnessTexture(

        const std::string metallicFiilename,
        const std::string roughnessFilename, ComPtr<ID3D11Texture2D> &texture,
        ComPtr<ID3D11ShaderResourceView> &srv);

    static void CreateTextureArray(

        const std::vector<std::string> filenames,
        ComPtr<ID3D11Texture2D> &texture,
        ComPtr<ID3D11ShaderResourceView> &textureResourceView);

    static void CreateDDSTexture(const wchar_t *filename, const bool isCubeMap,
                                 ComPtr<ID3D11ShaderResourceView> &texResView);

    static void WriteToFile(

        ComPtr<ID3D11Texture2D> &textureToWrite, const std::string filename);

    static void CreateUATexture(const int width, const int height,
                                const DXGI_FORMAT pixelFormat,
                                ComPtr<ID3D11Texture2D> &texture,
                                ComPtr<ID3D11RenderTargetView> &rtv,
                                ComPtr<ID3D11ShaderResourceView> &srv,
                                ComPtr<ID3D11UnorderedAccessView> &uav);

    static void CreateTexture3D(const int width, const int height,
                                const int depth, const DXGI_FORMAT pixelFormat,
                                const std::vector<float> &initData,
                                ComPtr<ID3D11Texture3D> &texture,
                                ComPtr<ID3D11RenderTargetView> &rtv,
                                ComPtr<ID3D11ShaderResourceView> &srv,
                                ComPtr<ID3D11UnorderedAccessView> &uav);

    static ComPtr<ID3D11Texture3D>
    CreateStagingTexture3D(const int width, const int height, const int depth,
                           const DXGI_FORMAT pixelFormat);

    static size_t GetPixelSize(DXGI_FORMAT pixelFormat);

    static void ComputeShaderBarrier();

    static void CreateStructuredBuffer(const UINT numElements,
                                       const UINT sizeElement,
                                       const void *initData,
                                       ComPtr<ID3D11Buffer> &buffer,
                                       ComPtr<ID3D11ShaderResourceView> &srv,
                                       ComPtr<ID3D11UnorderedAccessView> &uav);

    static void CreateStagingBuffer(const UINT numElements,
                                    const UINT sizeElement,
                                    const void *initData,
                                    ComPtr<ID3D11Buffer> &buffer);

    static void CopyToStagingBuffer(ComPtr<ID3D11Buffer> &buffer, UINT size,
                                    void *src);

    static void SetPipelineState(const PipelineState &pso);

    static void SetGlobalConsts(ComPtr<ID3D11Buffer> &globalConstsGPU);
};
} // namespace core
#endif
