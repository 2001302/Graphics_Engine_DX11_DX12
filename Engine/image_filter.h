#ifndef _IMAGEFILTER
#define _IMAGEFILTER

#include "common_struct.h"
#include "entity.h"

namespace dx11 {

using DirectX::SimpleMath::Vector4;
using Microsoft::WRL::ComPtr;

class ImageFilter {
  public:
    ImageFilter(){};

    ImageFilter(ComPtr<ID3D11Device> &device,
                ComPtr<ID3D11DeviceContext> &context,
                ComPtr<ID3D11PixelShader> &pixelShader, int width, int height);

    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    ComPtr<ID3D11PixelShader> &pixelShader, int width,
                    int height);

    void UpdateConstantBuffers(ComPtr<ID3D11Device> &device,
                               ComPtr<ID3D11DeviceContext> &context);

    void Render(ComPtr<ID3D11DeviceContext> &context) const;

    void SetShaderResources(
        const std::vector<ComPtr<ID3D11ShaderResourceView>> &resources);

    void SetRenderTargets(
        const std::vector<ComPtr<ID3D11RenderTargetView>> &targets);
    template <typename T_CONSTANT>
    static void CreateConstBuffer(ComPtr<ID3D11Device> &device,
                                  const T_CONSTANT &constantBufferData,
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

        ThrowIfFailed(device->CreateBuffer(&desc, &initData,
                                           constantBuffer.GetAddressOf()));
    }
    template <typename T_DATA>
    static void UpdateBuffer(ComPtr<ID3D11Device> &device,
                             ComPtr<ID3D11DeviceContext> &context,
                             const T_DATA &bufferData,
                             ComPtr<ID3D11Buffer> &buffer) {

        if (!buffer) {
            std::cout << "UpdateBuffer() buffer was not initialized."
                      << std::endl;
        }

        D3D11_MAPPED_SUBRESOURCE ms;
        context->Map(buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
        memcpy(ms.pData, &bufferData, sizeof(bufferData));
        context->Unmap(buffer.Get(), NULL);
    }

  public:
    struct ImageFilterConstData {
        float dx;
        float dy;
        float threshold;
        float strength;
        float option1;
        float option2;
        float option3;
        float option4;
    };

    ImageFilterConstData m_constData = {};

  protected:
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11Buffer> m_constBuffer;
    D3D11_VIEWPORT m_viewport = {};

    // Do not delete pointers
    std::vector<ID3D11ShaderResourceView *> m_shaderResources;
    std::vector<ID3D11RenderTargetView *> m_renderTargets;
};
} // namespace Engine
#endif
