#pragma once

#include "compute_pso.h"
#include "constant_buffer.h"
#include "env.h"
#include "mesh.h"

namespace engine {
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

class PostProcess {
  public:
    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context);

    void Render(ComPtr<ID3D11Device> &device,
                ComPtr<ID3D11DeviceContext> &context,
                GlobalConstants *constsCPU, ComPtr<ID3D11Buffer> constsGPU);

  private:
    ImageFilterConstData m_constData = {};
    ComPtr<ID3D11Buffer> m_constBuffer;
    std::shared_ptr<Mesh> m_mesh;

    // ComPtr<ID3D11ComputeShader> combine_CS;
    // ComputePSO compute_PSO;    template <typename T_DATA>

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

        device->CreateBuffer(&desc, &initData, constantBuffer.GetAddressOf());
    }
};
} // namespace engine