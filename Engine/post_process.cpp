#include "post_process.h"
#include "geometry_generator.h"
#include "graphics_common.h"
#include "graphics_util.h"

namespace engine {

void PostProcess::Initialize(ComPtr<ID3D11Device> &device,
                             ComPtr<ID3D11DeviceContext> &context) {

    GraphicsUtil::CreateComputeShader(device, L"CombineCS.hlsl", combine_CS);
    compute_PSO.m_computeShader = combine_CS;

    // create constant buffer
}

void SetPipelineState(ComPtr<ID3D11DeviceContext> &context,
                      const ComputePSO &pso) {
    context->VSSetShader(NULL, 0, 0);
    context->PSSetShader(NULL, 0, 0);
    context->HSSetShader(NULL, 0, 0);
    context->DSSetShader(NULL, 0, 0);
    context->GSSetShader(NULL, 0, 0);
    context->CSSetShader(pso.m_computeShader.Get(), 0, 0);
}

void ComputeShaderBarrier(ComPtr<ID3D11DeviceContext> &context) {

    // 예제들에서 최대 사용하는 SRV, UAV 갯수가 6개
    ID3D11ShaderResourceView *nullSRV[6] = {
        0,
    };
    context->CSSetShaderResources(0, 6, nullSRV);
    ID3D11UnorderedAccessView *nullUAV[6] = {
        0,
    };
    context->CSSetUnorderedAccessViews(0, 6, nullUAV, NULL);
}

void PostProcess::Render(ComPtr<ID3D11Device> &device,
                         ComPtr<ID3D11DeviceContext> &context,
                         GlobalConstants* constsCPU,
                         ComPtr<ID3D11Buffer> constsGPU) {

    // update buffer
    GraphicsUtil::UpdateBuffer(device,context, constsCPU, constsGPU);

    SetPipelineState(context, compute_PSO);

    context->CSSetConstantBuffers(0, 1, constsGPU.GetAddressOf());
    context->CSSetUnorderedAccessViews(
        0, 1, GraphicsManager::Instance().resolved_UAV.GetAddressOf(), NULL);

    context->OMSetRenderTargets(
        1, GraphicsManager::Instance().back_buffer_RTV.GetAddressOf(), NULL);

    // TODO: ThreadGroupCount를 쉐이더의 numthreads에 따라 잘 바꿔주기
    // TODO: ceil() 사용하는 이유 이해하기
    context->Dispatch(UINT(ceil(common::Env::Instance().screen_width / 256.0f)),
                      common::Env::Instance().screen_height, 1);

    // 컴퓨터 쉐이더가 하던 일을 끝내게 만들고 Resources 해제
    ComputeShaderBarrier(context);
}

} // namespace engine