#include "tone_mapping.h"
#include "geometry_generator.h"
#include "graphics_common.h"
#include "graphics_util.h"

namespace engine {

void ToneMapping::Initialize(ComPtr<ID3D11Device> &device,
                             ComPtr<ID3D11DeviceContext> &context) {

    MeshData meshData = GeometryGenerator::MakeSquare();

    mesh = std::make_shared<Mesh>();
    GraphicsUtil::CreateVertexBuffer(device, meshData.vertices,
                                     mesh->vertexBuffer);
    mesh->indexCount = UINT(meshData.indices.size());
    GraphicsUtil::CreateIndexBuffer(device, meshData.indices,
                                    mesh->indexBuffer);

    // Combine + ToneMapping
    const_data.dx = 1.0f / common::Env::Instance().screen_width;
    const_data.dy = 1.0f / common::Env::Instance().screen_height;
    const_data.strength = 0.0f; // Bloom strength
    const_data.option1 = 1.0f;  // Exposure
    const_data.option2 = 2.2f;  // Gamma
    GraphicsUtil::CreateConstBuffer(device, const_data, const_buffer);
    GraphicsUtil::UpdateBuffer(context, const_data, const_buffer);
}

void ToneMapping::Render(ComPtr<ID3D11Device> &device,
                         ComPtr<ID3D11DeviceContext> &context) {

    context->PSSetSamplers(0, 1, Graphics::linearClampSS.GetAddressOf());

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
    context->PSSetShader(Graphics::combinePS.Get(), 0, 0);
    context->PSSetConstantBuffers(1, 1, const_buffer.GetAddressOf());
    context->DrawIndexed(mesh->indexCount, 0, 0);
}

} // namespace engine