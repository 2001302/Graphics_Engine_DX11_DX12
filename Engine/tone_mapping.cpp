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
    GraphicsUtil::UpdateBuffer(device, context, const_data, const_buffer);
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
    context->PSSetConstantBuffers(0, 1, const_buffer.GetAddressOf());
    context->DrawIndexed(mesh->indexCount, 0, 0);
}

// create constant buffer
// GraphicsUtil::CreateComputeShader(device, L"CombineCS.hlsl", combine_CS);
// compute_PSO.m_computeShader = combine_CS;

// const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
// context->ClearRenderTargetView(
//     GraphicsManager::Instance().back_buffer_RTV.Get(), clearColor);

//// update buffer
// GraphicsUtil::UpdateBuffer(device, context, constsCPU, constsGPU);
// GraphicsManager::Instance().SetPipelineState(Graphics::postProcessingPSO);

// context->CSSetConstantBuffers(0, 1, constsGPU.GetAddressOf());
// context->CSSetUnorderedAccessViews(
//     0, 1, GraphicsManager::Instance().resolved_UAV.GetAddressOf(), NULL);
// context->OMSetRenderTargets(
//     1, GraphicsManager::Instance().back_buffer_RTV.GetAddressOf(), NULL);

//// TODO: ThreadGroupCount를 쉐이더의 numthreads에 따라 잘 바꿔주기
//// TODO: ceil() 사용하는 이유 이해하기
// context->Dispatch(UINT(ceil(common::Env::Instance().screen_width /
// 256.0f)),
//                   common::Env::Instance().screen_height, 1);

//// 컴퓨터 쉐이더가 하던 일을 끝내게 만들고 Resources 해제
// ComputeShaderBarrier(context);

//void SetPipelineState(ComPtr<ID3D11DeviceContext> &context,
//                      const ComputePSO &pso) {
//    context->VSSetShader(NULL, 0, 0);
//    context->PSSetShader(NULL, 0, 0);
//    context->HSSetShader(NULL, 0, 0);
//    context->DSSetShader(NULL, 0, 0);
//    context->GSSetShader(NULL, 0, 0);
//    context->CSSetShader(pso.m_computeShader.Get(), 0, 0);
//}
//
//void ComputeShaderBarrier(ComPtr<ID3D11DeviceContext> &context) {
//
//    // 예제들에서 최대 사용하는 SRV, UAV 갯수가 6개
//    ID3D11ShaderResourceView *nullSRV[6] = {
//        0,
//    };
//    context->CSSetShaderResources(0, 6, nullSRV);
//    ID3D11UnorderedAccessView *nullUAV[6] = {
//        0,
//    };
//    context->CSSetUnorderedAccessViews(0, 6, nullUAV, NULL);
//}

} // namespace engine