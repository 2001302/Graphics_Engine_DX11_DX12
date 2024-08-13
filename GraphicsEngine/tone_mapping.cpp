#include "tone_mapping.h"
#include "geometry_generator.h"
#include "graphics_common.h"
#include "graphics_util.h"

namespace core {

void ToneMapping::Initialize(ComPtr<ID3D11Device> &device,
                             ComPtr<ID3D11DeviceContext> &context) {

    MeshData meshData = GeometryGenerator::MakeSquare();

    mesh = std::make_shared<Mesh>();
    GraphicsUtil::CreateVertexBuffer(device, meshData.vertices,
                                     mesh->vertexBuffer);
    mesh->indexCount = UINT(meshData.indices.size());
    GraphicsUtil::CreateIndexBuffer(device, meshData.indices,
                                    mesh->indexBuffer);
}

void ToneMapping::Render(ComPtr<ID3D11Device> &device,
                         ComPtr<ID3D11DeviceContext> &context,
                         ComPtr<ID3D11Buffer> const_buffer) {

    context->PSSetSamplers(0, 1, graphics::linearClampSS.GetAddressOf());

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    context->IASetVertexBuffers(0, 1, mesh->vertexBuffer.GetAddressOf(),
                                &stride, &offset);
    context->IASetIndexBuffer(mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT,
                              0);

    context->RSSetViewports(1, &GraphicsManager::Instance().viewport);
    context->OMSetRenderTargets(
        1, GraphicsManager::Instance().back_buffer_RTV.GetAddressOf(), NULL);
    context->PSSetShaderResources(
        0, 1, GraphicsManager::Instance().resolved_SRV.GetAddressOf());
    context->PSSetShader(graphics::combinePS.Get(), 0, 0);
    context->PSSetConstantBuffers(1, 1, const_buffer.GetAddressOf());
    context->DrawIndexed(mesh->indexCount, 0, 0);
}

} // namespace engine