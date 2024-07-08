#ifndef _NORMALGEOMETRYSHADER
#define _NORMALGEOMETRYSHADER

#include "shader.h"

using namespace DirectX;

namespace Engine {

struct NormalGeometryVertexConstantBuffer {

};

struct NormalGeometryPixelConstantBuffer {

};

class NormalGeometryShader : public IShader {
  public:
    void CreateGeometryShader(const std::wstring &filename,
                              ComPtr<ID3D11GeometryShader> &geometryShader) {

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
            "gs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

        // CheckResult(hr, errorBlob.Get());

        Direct3D::GetInstance().device()->CreateGeometryShader(
            shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL,
            &geometryShader);
    }
    ComPtr<ID3D11GeometryShader> normalGeometryShader;
};

class NormalGeometryShaderSource : public IShaderSource {
  public:
    ComPtr<ID3D11Buffer> vertex_constant_buffer;
    ComPtr<ID3D11Buffer> pixel_constant_buffer;
    NormalGeometryVertexConstantBuffer vertex_constant_buffer_data;
    NormalGeometryPixelConstantBuffer pixel_constant_buffer_data;

  private:
    void InitializeThis() override {
        shader_->CreateConstantBuffer(vertex_constant_buffer_data,
                                      vertex_constant_buffer);
        shader_->CreateConstantBuffer(pixel_constant_buffer_data,
                                      pixel_constant_buffer);
    }
};
} // namespace Engine
#endif