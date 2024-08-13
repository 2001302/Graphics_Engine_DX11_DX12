#include "tone_mapping.h"
#include "geometry_generator.h"
#include "graphics_common.h"
#include "graphics_util.h"

namespace core {

void ToneMapping::Initialize() {

    MeshData meshData = GeometryGenerator::MakeSquare();

    mesh = std::make_shared<Mesh>();
    GraphicsUtil::CreateVertexBuffer(meshData.vertices, mesh->vertexBuffer);
    mesh->indexCount = UINT(meshData.indices.size());
    GraphicsUtil::CreateIndexBuffer(meshData.indices, mesh->indexBuffer);
}

void ToneMapping::Render(ComPtr<ID3D11Buffer> const_buffer) {

    GraphicsCore::Instance().device_context->PSSetSamplers(
        0, 1, graphics::linearClampSS.GetAddressOf());

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    GraphicsCore::Instance().device_context->IASetVertexBuffers(
        0, 1, mesh->vertexBuffer.GetAddressOf(), &stride, &offset);
    GraphicsCore::Instance().device_context->IASetIndexBuffer(
        mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    GraphicsCore::Instance().device_context->RSSetViewports(
        1, &GraphicsCore::Instance().viewport);
    GraphicsCore::Instance().device_context->OMSetRenderTargets(
        1, GraphicsCore::Instance().back_buffer_RTV.GetAddressOf(), NULL);
    GraphicsCore::Instance().device_context->PSSetShaderResources(
        0, 1, GraphicsCore::Instance().resolved_SRV.GetAddressOf());
    GraphicsCore::Instance().device_context->PSSetShader(
        graphics::combinePS.Get(), 0, 0);
    GraphicsCore::Instance().device_context->PSSetConstantBuffers(
        1, 1, const_buffer.GetAddressOf());
    GraphicsCore::Instance().device_context->DrawIndexed(mesh->indexCount, 0,
                                                         0);
}

} // namespace core