#ifndef _POSTPROCESSNODE
#define _POSTPROCESSNODE

#include "behavior_tree_builder.h"
#include "black_board.h"
#include "image_filter.h"
#include "tone_mapping.h"

namespace core {
/// <summary>
/// Bloom + ToneMapping
/// </summary>
class PostProcessingNode : public foundation::BehaviorActionNode,
                           public foundation::IInfo {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto job_context = black_board->job_context;

        switch (job_context->stage_type) {
        case EnumStageType::eInitialize: {
            const_data.dx = 1.0f / foundation::Env::Instance().screen_width;
            const_data.dy = 1.0f / foundation::Env::Instance().screen_height;
            const_data.strength = 0.5f; // Bloom strength
            const_data.option1 = 1.0f;  // Exposure
            const_data.option2 = 2.2f;  // Gamma

            graphics::Util::CreateConstBuffer(const_data, const_buffer);
            graphics::Util::UpdateBuffer(const_data, const_buffer);

            // bloom
            graphics::Util::CreateUATexture(
                foundation::Env::Instance().screen_width,
                foundation::Env::Instance().screen_height,
                DXGI_FORMAT_R16G16B16A16_FLOAT, bright_pass_buffer,
                bright_pass_RTV, bright_pass_SRV, bright_pass_UAV);

            graphics::Util::CreateUATexture(
                foundation::Env::Instance().screen_width,
                foundation::Env::Instance().screen_height,
                DXGI_FORMAT_R16G16B16A16_FLOAT, blur_vertical_buffer,
                blur_vertical_RTV, blur_vertical_SRV, blur_vertical_UAV);

            graphics::Util::CreateUATexture(
                foundation::Env::Instance().screen_width,
                foundation::Env::Instance().screen_height,
                DXGI_FORMAT_R16G16B16A16_FLOAT, blur_horizontal_buffer,
                blur_horizontal_RTV, blur_horizontal_SRV, blur_horizontal_UAV);

            // tone mapping
            tone_mapping.Initialize();

            black_board->gui->PushInfoItem(this);
            break;
        }
        case EnumStageType::eRender: {

            if (use_bloom) {
                // bright pass->blur vertical->blur horizontal->composite
                bright_pass.Render(
                    graphics::pipeline::brightPassCS, const_buffer,
                    {graphics::GpuCore::Instance().resolved_SRV},
                    bright_pass_UAV);

                blur_vertical.Render(graphics::pipeline::blurVerticalCS,
                                     const_buffer, {bright_pass_SRV},
                                     blur_vertical_UAV);

                blur_horizontal.Render(graphics::pipeline::blurHorizontalCS,
                                       const_buffer, {blur_vertical_SRV},
                                       blur_horizontal_UAV);

                bloom_composite.Render(
                    graphics::pipeline::bloomComposite, const_buffer,
                    {graphics::GpuCore::Instance().resolved_SRV,
                     blur_horizontal_SRV},
                    bright_pass_UAV);

                graphics::GpuCore::Instance().device_context->CopyResource(
                    graphics::GpuCore::Instance().resolved_buffer.Get(),
                    bright_pass_buffer.Get());
            }

            // tone mapping
            graphics::Util::SetPipelineState(graphics::pipeline::postProcessingPSO);
            tone_mapping.Render(const_buffer);
            break;
        }
        default:
            break;
        }
        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }

    void OnShow() override {
        if (ImGui::TreeNode("Post Processing")) {
            ImGui::Checkbox("Use Bloom Filter", &use_bloom);
            ImGui::TreePop();
        }
    }

    ImageFilterConstData const_data = {};
    ComPtr<ID3D11Buffer> const_buffer;

    bool use_bloom = false;
    ImageFilter bright_pass;
    ImageFilter blur_vertical;
    ImageFilter blur_horizontal;
    ImageFilter bloom_composite;
    ToneMapping tone_mapping;

    ComPtr<ID3D11Texture2D> bright_pass_buffer;
    ComPtr<ID3D11RenderTargetView> bright_pass_RTV;
    ComPtr<ID3D11ShaderResourceView> bright_pass_SRV;
    ComPtr<ID3D11UnorderedAccessView> bright_pass_UAV;

    ComPtr<ID3D11Texture2D> blur_vertical_buffer;
    ComPtr<ID3D11RenderTargetView> blur_vertical_RTV;
    ComPtr<ID3D11ShaderResourceView> blur_vertical_SRV;
    ComPtr<ID3D11UnorderedAccessView> blur_vertical_UAV;

    ComPtr<ID3D11Texture2D> blur_horizontal_buffer;
    ComPtr<ID3D11RenderTargetView> blur_horizontal_RTV;
    ComPtr<ID3D11ShaderResourceView> blur_horizontal_SRV;
    ComPtr<ID3D11UnorderedAccessView> blur_horizontal_UAV;
};
} // namespace core

#endif