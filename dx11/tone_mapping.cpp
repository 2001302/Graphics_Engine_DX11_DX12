#include "tone_mapping.h"
#include "geometry_generator.h"
#include "graphics_common.h"
#include "graphics_util.h"

namespace core {

void ToneMapping::Initialize() {

    MeshData meshData = GeometryGenerator::MakeSquare();

    mesh = std::make_shared<Mesh>();
    dx11::Util::CreateVertexBuffer(meshData.vertices, mesh->vertexBuffer);
    mesh->indexCount = UINT(meshData.indices.size());
    dx11::Util::CreateIndexBuffer(meshData.indices, mesh->indexBuffer);
}

void ToneMapping::Render(ComPtr<ID3D11Buffer> const_buffer) {

    dx11::GpuCore::Instance().device_context->PSSetSamplers(
        0, 1, dx11::pso::linearClampSS.GetAddressOf());

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    dx11::GpuCore::Instance().device_context->IASetVertexBuffers(
        0, 1, mesh->vertexBuffer.GetAddressOf(), &stride, &offset);
    dx11::GpuCore::Instance().device_context->IASetIndexBuffer(
        mesh->indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

    dx11::GpuCore::Instance().device_context->RSSetViewports(
        1, &dx11::GpuCore::Instance().viewport);
    dx11::GpuCore::Instance().device_context->OMSetRenderTargets(
        1, dx11::GpuCore::Instance().back_buffer_RTV.GetAddressOf(),
        NULL);
    dx11::GpuCore::Instance().device_context->PSSetShaderResources(
        0, 1, dx11::GpuCore::Instance().resolved_SRV.GetAddressOf());
    dx11::GpuCore::Instance().device_context->PSSetShader(
        dx11::pso::combinePS.Get(), 0, 0);
    dx11::GpuCore::Instance().device_context->PSSetConstantBuffers(
        1, 1, const_buffer.GetAddressOf());
    dx11::GpuCore::Instance().device_context->DrawIndexed(
        mesh->indexCount, 0, 0);
}

} // namespace core