#ifndef _SKYBOX_NODE
#define _SKYBOX_NODE

#include "foundation/behavior_tree_builder.h"
#include "graphics/command_pool.h"
#include "graphics/skybox_pso.h"
#include "black_board.h"
#include "mesh_renderer.h"

namespace core {

class SkyBoxNodeInvoker : public foundation::BehaviorActionNode {
    foundation::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        auto command_list = condition->command_pool->Get(0);
        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            skyboxPSO = std::make_shared<dx12::SkyboxPSO>();
            skyboxPSO->Initialize();

            auto mesh_data = GeometryGenerator::MakeBox(40.0f);
            std::reverse(mesh_data.indices.begin(), mesh_data.indices.end());

            auto renderer = std::make_shared<MeshRenderer>();
            renderer->Initialize(std::vector{mesh_data}, command_list, false);

            skybox = std::make_shared<Model>();
            skybox->AddComponent(EnumComponentType::eRenderer, renderer);

            condition->shared_texture = std::make_shared<dx12::GpuBufferList>();
            condition->shared_texture->Add(std::make_shared<dx12::TextureCube>(
                L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds",
                command_list, true));
            condition->shared_texture->Add(std::make_shared<dx12::TextureCube>(
                L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds",
                command_list, true));
            condition->shared_texture->Add(std::make_shared<dx12::TextureCube>(
                L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds",
                command_list, true));
            condition->shared_texture->Add(std::make_shared<dx12::TextureCube>(
                L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds", command_list,
                true, true));

            for (int i = 0; i < MAX_LIGHTS; i++) {
                condition->shared_texture->Add(
                    std::make_shared<dx12::Texture2D>(
                        1024, 1024, DXGI_FORMAT_R8G8B8A8_UNORM));
            }
            condition->shared_texture->Allocate(condition->gpu_heap.get());

            break;
        }
        case EnumStageType::eRender: {

            auto renderer = (MeshRenderer *)skybox->GetComponent(
                EnumComponentType::eRenderer);
            auto mesh = renderer->meshes.front();

            skyboxPSO->Render(
                command_list, dx12::GpuCore::Instance().GetHandleHDR(),
                dx12::GpuCore::Instance().GetHandleDSV(),
                condition->gpu_heap.get(), condition->shared_texture.get(),
                condition->sampler_heap, condition->global_consts.Get(),
                renderer->mesh_consts.Get(), renderer->material_consts.Get(),
                mesh->vertex_buffer_view, mesh->index_buffer_view,
                mesh->index_count);
            break;
        }
        default:
            break;
        }

        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }

    std::shared_ptr<dx12::SkyboxPSO> skyboxPSO;
    std::shared_ptr<Model> skybox;
};
} // namespace core

#endif
