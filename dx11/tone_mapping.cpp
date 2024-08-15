#include "tone_mapping.h"
#include "geometry_generator.h"
#include "graphics_common.h"
#include "graphics_util.h"

namespace core {

void ToneMapping::Initialize() {

    MeshData meshData = GeometryGenerator::MakeSquare();

    mesh = std::make_shared<Mesh>();
    graphics::Util::CreateVertexBuffer(meshData.vertices, mesh->vertexBuffer);
    mesh->indexCount = UINT(meshData.indices.size());
    graphics::Util::CreateIndexBuffer(meshData.indices, mesh->indexBuffer);
}

void ToneMapping::Render(ComPtr<ID3D11Buffer> const_buffer) {

    graphics::GpuCore::Instance().device_context->PSSetSamplers(
        0, 1, graphics::pso::linearClampSS.GetAddressOf());

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    graphics::GpuCore::Instance().device_context->IASetVertexBuffers(
        0, 1, mesh->vertexBuffer.GetAddressOf(), &stride, &offset);
    graphics::GpuCore::Instance().device_context->IASetIndexBuffer(
        mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    graphics::GpuCore::Instance().device_context->RSSetViewports(
        1, &graphics::GpuCore::Instance().viewport);
    graphics::GpuCore::Instance().device_context->OMSetRenderTargets(
        1, graphics::GpuCore::Instance().back_buffer_RTV.GetAddressOf(),
        NULL);
    graphics::GpuCore::Instance().device_context->PSSetShaderResources(
        0, 1, graphics::GpuCore::Instance().resolved_SRV.GetAddressOf());
    graphics::GpuCore::Instance().device_context->PSSetShader(
        graphics::pso::combinePS.Get(), 0, 0);
    graphics::GpuCore::Instance().device_context->PSSetConstantBuffers(
        1, 1, const_buffer.GetAddressOf());
    graphics::GpuCore::Instance().device_context->DrawIndexed(
        mesh->indexCount, 0, 0);
}

} // namespace core