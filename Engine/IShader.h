#ifndef _IShader
#define _IShader

#include "CommonStruct.h"
#include "direct3D.h"

namespace Engine
{
	class IShader
	{
	public:
        void CreatePixelShader(const std::wstring& filename,
            ComPtr<ID3D11PixelShader>& pixelShader) {
            ComPtr<ID3DBlob> shaderBlob;
            ComPtr<ID3DBlob> errorBlob;

            UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
            compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

            // 쉐이더의 시작점의 이름이 "main"인 함수로 지정
            // D3D_COMPILE_STANDARD_FILE_INCLUDE 추가: 쉐이더에서 include 사용
            HRESULT hr = D3DCompileFromFile(
                filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
                "ps_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

            Direct3D::GetInstance().GetDevice()->CreatePixelShader(shaderBlob->GetBufferPointer(),
                shaderBlob->GetBufferSize(), NULL,
                &pixelShader);
        }

        void CreateVertexShaderAndInputLayout(
            const std::wstring& filename,
            const std::vector<D3D11_INPUT_ELEMENT_DESC>& inputElements,
            ComPtr<ID3D11VertexShader>& vertexShader,
            ComPtr<ID3D11InputLayout>& inputLayout)
        {
            ComPtr<ID3DBlob> shaderBlob;
            ComPtr<ID3DBlob> errorBlob;

            UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
            compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

            // 쉐이더의 시작점의 이름이 "main"인 함수로 지정
            // D3D_COMPILE_STANDARD_FILE_INCLUDE 추가: 쉐이더에서 include 사용
            HRESULT hr = D3DCompileFromFile(
                filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
                "vs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

            Direct3D::GetInstance().GetDevice()->CreateVertexShader(shaderBlob->GetBufferPointer(),
                shaderBlob->GetBufferSize(), NULL,
                &vertexShader);

            Direct3D::GetInstance().GetDevice()->CreateInputLayout(inputElements.data(),
                UINT(inputElements.size()),
                shaderBlob->GetBufferPointer(),
                shaderBlob->GetBufferSize(), &inputLayout);
        }

        template <typename T_CONSTANT>
        void CreateConstantBuffer(const T_CONSTANT& constantBufferData,
            ComPtr<ID3D11Buffer>& constantBuffer) {
            // 주의:
            // For a constant buffer (BindFlags of D3D11_BUFFER_DESC set to
            // D3D11_BIND_CONSTANT_BUFFER), you must set the ByteWidth value of
            // D3D11_BUFFER_DESC in multiples of 16, and less than or equal to
            // D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT.

            D3D11_BUFFER_DESC cbDesc;
            cbDesc.ByteWidth = sizeof(constantBufferData);
            cbDesc.Usage = D3D11_USAGE_DYNAMIC;
            cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            cbDesc.MiscFlags = 0;
            cbDesc.StructureByteStride = 0;

            // Fill in the subresource data.
            D3D11_SUBRESOURCE_DATA initData;
            initData.pSysMem = &constantBufferData;
            initData.SysMemPitch = 0;
            initData.SysMemSlicePitch = 0;

            auto hr = Direct3D::GetInstance().GetDevice()->CreateBuffer(&cbDesc, &initData,
                constantBuffer.GetAddressOf());
            if (FAILED(hr)) {
                std::cout << "CreateConstantBuffer() CreateBuffer failed()."
                    << std::endl;
            }
        }
        template <typename T_DATA>
        void UpdateBuffer(const T_DATA& bufferData, ComPtr<ID3D11Buffer>& buffer) {

            if (!buffer) {
                std::cout << "UpdateBuffer() buffer was not initialized."
                    << std::endl;
            }

            D3D11_MAPPED_SUBRESOURCE ms;
            Direct3D::GetInstance().GetDeviceContext()->Map(buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
            memcpy(ms.pData, &bufferData, sizeof(bufferData));
            Direct3D::GetInstance().GetDeviceContext()->Unmap(buffer.Get(), NULL);
        }

        ComPtr<ID3D11VertexShader> vertexShader;
        ComPtr<ID3D11PixelShader> pixelShader;
        ComPtr<ID3D11InputLayout> layout;
        ComPtr<ID3D11SamplerState> sampleState;

        ComPtr<ID3D11Buffer> vertexConstantBuffer;
        ComPtr<ID3D11Buffer> pixelShaderConstantBuffer;
	};
}
#endif