#ifndef _IMAGEFILTER
#define _IMAGEFILTER

#include "graphics_pso.h"
#include "tone_mapping.h"

namespace core {

class ImageFilter : public Component {
  public:
    void Render(ComPtr<ID3D11ComputeShader> shader,
                ComPtr<ID3D11Buffer> const_buffer,
                const std::vector<ComPtr<ID3D11ShaderResourceView>> &srv,
                ComPtr<ID3D11UnorderedAccessView> uav) {

        graphics::GraphicsCore::Instance().device_context->CSSetShader(
            shader.Get(), 0, 0);
        graphics::GraphicsCore::Instance().device_context->CSSetConstantBuffers(
            0, 1, const_buffer.GetAddressOf());
        graphics::GraphicsCore::Instance().device_context->CSSetShaderResources(
            0, UINT(srv.size()), srv.data()->GetAddressOf());
        graphics::GraphicsCore::Instance()
            .device_context->CSSetUnorderedAccessViews(0, 1, uav.GetAddressOf(),
                                                       NULL);

        graphics::GraphicsCore::Instance().device_context->Dispatch(
            UINT(ceil(common::Env::Instance().screen_width / 16.0f)),
            UINT(ceil(common::Env::Instance().screen_height / 16.0f)), 1);

        // Let the computer shader finish its task and then release the
        // resources.
        graphics::Util::ComputeShaderBarrier();
    }
};
} // namespace core
#endif