#ifndef _RENDERER_DRAW_NODE
#define _RENDERER_DRAW_NODE

#include "behavior_tree_builder.h"
#include "black_board.h"
#include "mesh_renderer.h"
#include "skinned_mesh_renderer.h"

namespace engine {

class GameObjectNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;
        auto gui = black_board->gui;

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {

            // additional object 1
            {
                MeshData mesh = GeometryGenerator::MakeSphere(0.2f, 200, 200);
                Vector3 center(0.5f, 0.5f, 2.0f);

                auto renderer = std::make_shared<MeshRenderer>(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context,
                    std::vector{mesh});

                renderer->UpdateWorldRow(Matrix::CreateTranslation(center));
                renderer->material_consts.GetCpu().albedoFactor =
                    Vector3(0.1f, 0.1f, 1.0f);
                renderer->material_consts.GetCpu().roughnessFactor = 0.2f;
                renderer->material_consts.GetCpu().metallicFactor = 0.6f;
                renderer->material_consts.GetCpu().emissionFactor =
                    Vector3(0.0f);
                renderer->UpdateConstantBuffers(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context);

                auto obj = std::make_shared<Model>();
                obj->AddComponent(EnumComponentType::eRenderer, renderer);

                manager->models.insert({obj->GetEntityId(), obj});
            }

            // additional object 2
            {
                MeshData mesh = GeometryGenerator::MakeBox(0.2f);
                Vector3 center(0.0f, 0.5f, 2.5f);

                auto renderer = std::make_shared<MeshRenderer>(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context,
                    std::vector{mesh});

                renderer->UpdateWorldRow(Matrix::CreateTranslation(center));
                renderer->material_consts.GetCpu().albedoFactor =
                    Vector3(1.0f, 0.2f, 0.2f);
                renderer->material_consts.GetCpu().roughnessFactor = 0.5f;
                renderer->material_consts.GetCpu().metallicFactor = 0.9f;
                renderer->material_consts.GetCpu().emissionFactor =
                    Vector3(0.0f);
                renderer->UpdateConstantBuffers(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context);

                auto obj = std::make_shared<Model>();
                obj->AddComponent(EnumComponentType::eRenderer, renderer);

                manager->models.insert({obj->GetEntityId(), obj});
            }

            break;
        }
        case EnumStageType::eUpdate: {

            for (auto &i : manager->models) {
                auto renderer = (MeshRenderer *)i.second->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->UpdateConstantBuffers(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context);
            }

            for (auto &i : manager->light_spheres) {
                auto renderer = (MeshRenderer *)i->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->UpdateConstantBuffers(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context);
            }
            break;
        }
        case EnumStageType::eRender: {

            GraphicsManager::Instance().SetPipelineState(
                manager->draw_wire ? Graphics::defaultWirePSO
                                   : Graphics::defaultSolidPSO);
            GraphicsManager::Instance().SetGlobalConsts(
                manager->global_consts_GPU);

            for (auto &i : manager->models) {
                auto renderer = (MeshRenderer *)i.second->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->Render(GraphicsManager::Instance().device_context);
            }

            // If there is no need to draw mirror reflections, draw only the
            // opaque mirror
            if (manager->mirror_alpha == 1.0f) {
                auto renderer = (MeshRenderer *)manager->mirror->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->Render(GraphicsManager::Instance().device_context);
            }

            GraphicsManager::Instance().SetPipelineState(Graphics::normalsPSO);
            for (auto &i : manager->models) {
                auto renderer = (MeshRenderer *)i.second->GetComponent(
                    EnumComponentType::eRenderer);
                if (renderer->draw_normals)
                    renderer->RenderNormals(
                        GraphicsManager::Instance().device_context);
            }

            {
                GraphicsManager::Instance().SetPipelineState(
                    manager->draw_wire ? Graphics::defaultWirePSO
                                       : Graphics::defaultSolidPSO);
                GraphicsManager::Instance().SetGlobalConsts(
                    manager->global_consts_GPU);
                for (auto &i : manager->light_spheres) {
                    auto renderer = (MeshRenderer *)i->GetComponent(
                        EnumComponentType::eRenderer);
                    renderer->Render(
                        GraphicsManager::Instance().device_context);
                }
            }
            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

enum EnumAnimationState {
    eIdle = 0,
    eIdleToWalk = 1,
    eWalk = 2,
    eWalkToIdle = 3,
    eDance = 4,
};

struct AnimationBlock : public common::IDataBlock {
    AnimationData *aniData;
    SkinnedMeshRenderer *renderer;
    common::Input *input;
    EnumAnimationState state;
};

class AnimationState : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {
        auto animation_block = dynamic_cast<AnimationBlock *>(data_block);
        assert(animation_block != nullptr);

        auto key_flag = animation_block->input->KeyState(87);

        if (key_flag) {
            if (animation_block->state != EnumAnimationState::eWalk) {
                animation_block->state = EnumAnimationState::eWalk;
                GetParent()->Reset();
            }
        } else {
            if (animation_block->state != EnumAnimationState::eIdle) {
                animation_block->state = EnumAnimationState::eIdle;
                GetParent()->Reset();
            }
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class CheckWalk : public common::ConditionalNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {
        auto animation_block = dynamic_cast<AnimationBlock *>(data_block);
        assert(animation_block != nullptr);

        if ((animation_block->state != EnumAnimationState::eIdleToWalk) &&
            (animation_block->state != EnumAnimationState::eWalk) &&
            (animation_block->state != EnumAnimationState::eWalkToIdle))
            return common::EnumBehaviorTreeStatus::eSuccess;

        common::ConditionalNode::OnInvoke();

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class CheckIdle : public common::ConditionalNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {
        auto animation_block = dynamic_cast<AnimationBlock *>(data_block);
        assert(animation_block != nullptr);

        if (animation_block->state != EnumAnimationState::eIdle)
            return common::EnumBehaviorTreeStatus::eSuccess;

        common::ConditionalNode::OnInvoke();

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class IdleToWalk : public common::AnimationNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {
        auto animation_block = dynamic_cast<AnimationBlock *>(data_block);
        assert(animation_block != nullptr);

        if (is_done)
            return common::EnumBehaviorTreeStatus::eSuccess;

        animation_block->renderer->UpdateAnimation(
            GraphicsManager::Instance().device_context,
            EnumAnimationState::eIdleToWalk, frame_count);
        frame_count += 1;

        if (animation_block->aniData->clips[EnumAnimationState::eIdleToWalk]
                .keys.size() <= frame_count) {
            is_done = true;
            return common::EnumBehaviorTreeStatus::eSuccess;
        }
        return common::EnumBehaviorTreeStatus::eRunning;
    }
};

class Walk : public common::AnimationNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {
        auto animation_block = dynamic_cast<AnimationBlock *>(data_block);
        assert(animation_block != nullptr);

        if (is_done)
            return common::EnumBehaviorTreeStatus::eSuccess;

        animation_block->renderer->UpdateAnimation(
            GraphicsManager::Instance().device_context,
            EnumAnimationState::eWalk, frame_count);
        frame_count += 1;

        if (animation_block->state != EnumAnimationState::eWalk) {
            is_done = true;
            return common::EnumBehaviorTreeStatus::eSuccess;
        }
        return common::EnumBehaviorTreeStatus::eRunning;
    }
};

class WalkToIdle : public common::AnimationNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {
        auto animation_block = dynamic_cast<AnimationBlock *>(data_block);
        assert(animation_block != nullptr);

        if (is_done)
            return common::EnumBehaviorTreeStatus::eSuccess;

        animation_block->renderer->UpdateAnimation(
            GraphicsManager::Instance().device_context,
            EnumAnimationState::eWalkToIdle, frame_count);
        frame_count += 1;

        if (animation_block->aniData->clips[EnumAnimationState::eWalkToIdle]
                .keys.size() <= frame_count) {
            is_done = true;
            return common::EnumBehaviorTreeStatus::eSuccess;
        }
        return common::EnumBehaviorTreeStatus::eRunning;
    }
};

class Idle : public common::AnimationNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {
        auto animation_block = dynamic_cast<AnimationBlock *>(data_block);
        assert(animation_block != nullptr);

        if (is_done)
            return common::EnumBehaviorTreeStatus::eSuccess;

        animation_block->renderer->UpdateAnimation(
            GraphicsManager::Instance().device_context,
            EnumAnimationState::eIdle, frame_count);
        frame_count += 1;

        if (animation_block->state != EnumAnimationState::eIdle) {
            is_done = true;
            return common::EnumBehaviorTreeStatus::eSuccess;
        }

        return common::EnumBehaviorTreeStatus::eRunning;
    }
}; // must be success

class Animator : public Component, public common::BehaviorTreeBuilder {
  public:
    void Initialize(AnimationData *aniData, SkinnedMeshRenderer *renderer,
                    common::Input *input) {
        block.aniData = aniData;
        block.renderer = renderer;
        block.input = input;

        animation_state = std::make_shared<AnimationState>();
        check_walk = std::make_shared<CheckWalk>();
        check_idle = std::make_shared<CheckIdle>();
        idle_to_walk = std::make_shared<IdleToWalk>();
        walk = std::make_shared<Walk>();
        walk_to_idle = std::make_shared<WalkToIdle>();
        idle = std::make_shared<Idle>();
    };
    void Updete() {
        // clang-format off
        Build(&block)
            ->Sequence()
                ->Excute(animation_state)
                ->Conditional(check_walk)
                    ->Sequence()
                        ->Excute(idle_to_walk)
                        ->Excute(walk)
                        ->Excute(walk_to_idle)
                    ->Close()
                ->Close()
                ->Conditional(check_idle)
                    ->Excute(idle)
                ->Close()
            ->Close()
        ->Run();
        // clang-format on
    };
    void Show(){};

  private:
    AnimationBlock block;

    std::shared_ptr<AnimationState> animation_state;
    std::shared_ptr<CheckWalk> check_walk;
    std::shared_ptr<CheckIdle> check_idle;
    std::shared_ptr<IdleToWalk> idle_to_walk;
    std::shared_ptr<Walk> walk;
    std::shared_ptr<WalkToIdle> walk_to_idle;
    std::shared_ptr<Idle> idle;
};

class PlayerNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;
        auto gui = black_board->gui;

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {
            std::string path = "Assets/Characters/Mixamo/";
            std::vector<std::string> clipNames = {
                "CatwalkIdle.fbx", "CatwalkIdleToWalkForward.fbx",
                "CatwalkWalkForward.fbx", "CatwalkWalkStop.fbx",
                "BreakdanceFreezeVar2.fbx"};

            AnimationData aniData;
            auto [meshes, _] =
                GeometryGenerator::ReadAnimationFromFile(path, "character.fbx");

            for (auto &name : clipNames) {
                auto [_, ani] =
                    GeometryGenerator::ReadAnimationFromFile(path, name);

                if (aniData.clips.empty()) {
                    aniData = ani;
                } else {
                    aniData.clips.push_back(ani.clips.front());
                }
            }

            Vector3 center(0.0f, 0.0f, 2.0f);
            auto renderer = std::make_shared<SkinnedMeshRenderer>(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context, meshes, aniData);

            renderer->material_consts.GetCpu().albedoFactor = Vector3(1.0f);
            renderer->material_consts.GetCpu().roughnessFactor = 0.8f;
            renderer->material_consts.GetCpu().metallicFactor = 0.0f;
            renderer->UpdateWorldRow(Matrix::CreateScale(1.0f) *
                                     Matrix::CreateTranslation(center));

            manager->player = std::make_shared<Model>();
            manager->player->AddComponent(EnumComponentType::eRenderer,
                                          renderer);
            animator.Initialize(&renderer->m_aniData, renderer.get(),
                                black_board->input.get());
            break;
        }
        case EnumStageType::eUpdate: {
            auto renderer =
                (SkinnedMeshRenderer *)manager->player->GetComponent(
                    EnumComponentType::eRenderer);

            animator.Updete();

            renderer->UpdateConstantBuffers(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context);

            break;
        }
        case EnumStageType::eRender: {
            GraphicsManager::Instance().SetPipelineState(
                manager->draw_wire ? Graphics::skinnedWirePSO
                                   : Graphics::skinnedSolidPSO);
            auto renderer =
                (SkinnedMeshRenderer *)manager->player->GetComponent(
                    EnumComponentType::eRenderer);
            renderer->Render(GraphicsManager::Instance().device_context);

            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
    Animator animator;
};

class OnlyDepthNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {

            D3D11_TEXTURE2D_DESC desc;
            desc.Width = common::Env::Instance().screen_width;
            desc.Height = common::Env::Instance().screen_height;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;

            // Depth 전용
            desc.Format = DXGI_FORMAT_R32_TYPELESS;
            desc.BindFlags =
                D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
            desc.SampleDesc.Count = 1;
            desc.SampleDesc.Quality = 0;
            ThrowIfFailed(GraphicsManager::Instance().device->CreateTexture2D(
                &desc, NULL, depthOnlyBuffer.GetAddressOf()));
            break;
        }
        case EnumStageType::eRender: {
            // Depth Only Pass (no RTS)
            GraphicsManager::Instance().device_context->OMSetRenderTargets(
                0, NULL, depthOnlyDSV.Get());
            GraphicsManager::Instance().device_context->ClearDepthStencilView(
                depthOnlyDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

            GraphicsManager::Instance().SetPipelineState(
                Graphics::depthOnlyPSO);
            GraphicsManager::Instance().SetGlobalConsts(
                manager->global_consts_GPU);

            for (auto &i : manager->models) {
                auto renderer = (MeshRenderer *)i.second->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->Render(GraphicsManager::Instance().device_context);
            }

            if (true) {
                auto renderer = (MeshRenderer *)manager->skybox->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->Render(GraphicsManager::Instance().device_context);
            }

            if (true) {
                auto renderer = (MeshRenderer *)manager->mirror->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->Render(GraphicsManager::Instance().device_context);
            }
            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }

    ComPtr<ID3D11Texture2D> depthOnlyBuffer; // No MSAA
    ComPtr<ID3D11DepthStencilView> depthOnlyDSV;
    ComPtr<ID3D11ShaderResourceView> depthOnlySRV;
};

class ResolveBufferNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        GraphicsManager::Instance().device_context->ResolveSubresource(
            GraphicsManager::Instance().resolved_buffer.Get(), 0,
            GraphicsManager::Instance().float_buffer.Get(), 0,
            DXGI_FORMAT_R16G16B16A16_FLOAT);

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class PresentNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        // Present the rendered scene to the screen.
        if (common::Env::Instance().vsync_enabled) {
            GraphicsManager::Instance().swap_chain->Present(1, 0);
        } else {
            GraphicsManager::Instance().swap_chain->Present(0, 0);
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

} // namespace engine

#endif