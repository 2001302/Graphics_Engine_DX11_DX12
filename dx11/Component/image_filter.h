#ifndef _IMAGEFILTER
#define _IMAGEFILTER

#include "../gpu/graphics_pso.h"
#include "tone_mapping.h"

namespace graphics {

class ImageFilter : public common::Component {
  public:
    void Render(ComPtr<ID3D11ComputeShader> shader,
                ComPtr<ID3D11Buffer> const_buffer,
                const std::vector<ComPtr<ID3D11ShaderResourceView>> &srv,
                ComPtr<ID3D11UnorderedAccessView> uav) {

        graphics::GpuCore::Instance().device_context->CSSetShader(shader.Get(),
                                                                  0, 0);
        graphics::GpuCore::Instance().device_context->CSSetConstantBuffers(
            0, 1, const_buffer.GetAddressOf());
        graphics::GpuCore::Instance().device_context->CSSetShaderResources(
            0, UINT(srv.size()), srv.data()->GetAddressOf());
        graphics::GpuCore::Instance().device_context->CSSetUnorderedAccessViews(
            0, 1, uav.GetAddressOf(), NULL);

        graphics::GpuCore::Instance().device_context->Dispatch(
            UINT(ceil(common::env::screen_width / 16.0f)),
            UINT(ceil(common::env::screen_height / 16.0f)), 1);

        // Let the computer shader finish its task and then release the
        // resources.
        graphics::Util::ComputeShaderBarrier();
    }
};
} // namespace graphics
#endif