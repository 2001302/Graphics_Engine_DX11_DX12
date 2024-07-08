#include "cube_map_shader.h"

using namespace Engine;

void CubeMapShader::CreateDDSTexture(
    const wchar_t *filename,
    ComPtr<ID3D11ShaderResourceView> &textureResourceView) {

    ComPtr<ID3D11Texture2D> texture;

    UINT miscFlags = 0;
    if (true) {
        miscFlags |= D3D11_RESOURCE_MISC_TEXTURECUBE;
    }

    auto hr = CreateDDSTextureFromFileEx(
        Direct3D::GetInstance().device().Get(), filename, 0,
        D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, miscFlags,
        DDS_LOADER_FLAGS(false), (ID3D11Resource **)texture.GetAddressOf(),
        textureResourceView.GetAddressOf(), NULL);

    if (FAILED(hr)) {
        std::cout << "CreateDDSTextureFromFileEx() failed" << std::endl;
    }
}