#ifndef _IMAGEFILTER
#define _IMAGEFILTER

#include "compute_pso.h"
#include "tone_mapping.h"

namespace engine {

class ImageFilter : public Component {
  public:
    void Render(
                ComPtr<ID3D11DeviceContext> &context,
                ComPtr<ID3D11Buffer> const_buffer,
                ComPtr<ID3D11ShaderResourceView> srv,
                ComPtr<ID3D11UnorderedAccessView> uav) {
        context->CSSetConstantBuffers(0, 1, const_buffer.GetAddressOf());
        context->CSSetShaderResources(0, 1, srv.GetAddressOf());
        context->CSSetUnorderedAccessViews(0, 1, uav.GetAddressOf(), NULL);

        context->Dispatch(
            UINT(ceil(common::Env::Instance().screen_width / 16.0f)),
            UINT(ceil(common::Env::Instance().screen_height / 16.0f)), 1);

        // 컴퓨터 쉐이더가 하던 일을 끝내게 만들고 Resources 해제
        GraphicsUtil::ComputeShaderBarrier(context);
    }
};
} // namespace engine
#endif