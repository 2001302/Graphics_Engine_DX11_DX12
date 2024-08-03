#ifndef _RENDERER_DRAW_NODE
#define _RENDERER_DRAW_NODE

#include "behavior_tree_builder.h"
#include "renderer.h"
#include "skinned_mesh_renderer.h"
#include "black_board.h"

namespace engine {
    
class GameObjectNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;
        auto gui = black_board->gui;

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {

            // 추가 물체1
            {
                MeshData mesh = GeometryGenerator::MakeSphere(0.2f, 200, 200);
                Vector3 center(0.5f, 0.5f, 2.0f);

                auto renderer = std::make_shared<Renderer>(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context,
                    std::vector{mesh});

                renderer->UpdateWorldRow(Matrix::CreateTranslation(center));
                renderer->m_materialConsts.GetCpu().albedoFactor =
                    Vector3(0.1f, 0.1f, 1.0f);
                renderer->m_materialConsts.GetCpu().roughnessFactor = 0.2f;
                renderer->m_materialConsts.GetCpu().metallicFactor = 0.6f;
                renderer->m_materialConsts.GetCpu().emissionFactor =
                    Vector3(0.0f);
                renderer->UpdateConstantBuffers(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context);

                auto obj = std::make_shared<Model>();
                obj->AddComponent(EnumComponentType::eRenderer, renderer);

                manager->models.insert({obj->GetEntityId(), obj});
            }

            // 추가 물체2
            {
                MeshData mesh = GeometryGenerator::MakeBox(0.2f);
                Vector3 center(0.0f, 0.5f, 2.5f);

                auto renderer = std::make_shared<Renderer>(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context,
                    std::vector{mesh});

                renderer->UpdateWorldRow(Matrix::CreateTranslation(center));
                renderer->m_materialConsts.GetCpu().albedoFactor =
                    Vector3(1.0f, 0.2f, 0.2f);
                renderer->m_materialConsts.GetCpu().roughnessFactor = 0.5f;
                renderer->m_materialConsts.GetCpu().metallicFactor = 0.9f;
                renderer->m_materialConsts.GetCpu().emissionFactor =
                    Vector3(0.0f);
                renderer->UpdateConstantBuffers(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context);

                auto obj = std::make_shared<Model>();
                obj->AddComponent(EnumComponentType::eRenderer, renderer);

                manager->models.insert({obj->GetEntityId(), obj});
            }

            // main object
            {
                std::string path = "Assets/Characters/Mixamo/";
                std::vector<std::string> clipNames = {
                    "CatwalkIdle.fbx", "CatwalkIdleToWalkForward.fbx",
                    "CatwalkWalkForward.fbx", "CatwalkWalkStop.fbx",
                    "BreakdanceFreezeVar2.fbx"};

                AnimationData aniData;

                auto [meshes, _] = GeometryGenerator::ReadAnimationFromFile(
                    path, "character.fbx");

                for (auto &name : clipNames) {
                    auto [_, ani] =
                        GeometryGenerator::ReadAnimationFromFile(path, name);

                    if (aniData.clips.empty()) {
                        aniData = ani;
                    } else {
                        aniData.clips.push_back(ani.clips.front());
                    }
                }

                Vector3 center(0.0f, 0.5f, 2.0f);
                auto renderer = std::make_shared<SkinnedMeshRenderer>(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context, meshes,
                    aniData);

                renderer->m_materialConsts.GetCpu().albedoFactor =
                    Vector3(1.0f);
                renderer->m_materialConsts.GetCpu().roughnessFactor = 0.8f;
                renderer->m_materialConsts.GetCpu().metallicFactor = 0.0f;
                renderer->UpdateWorldRow(Matrix::CreateScale(1.0f) *
                                         Matrix::CreateTranslation(center));

                manager->m_character = std::make_shared<Model>();
                manager->m_character->AddComponent(EnumComponentType::eRenderer,
                                                   renderer);
            }

            break;
        }
        case EnumStageType::eUpdate: {

            for (auto &i : manager->models) {
                auto renderer = (Renderer *)i.second->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->UpdateConstantBuffers(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context);
            }

            {
                auto renderer =
                    (SkinnedMeshRenderer *)manager->m_character->GetComponent(
                        EnumComponentType::eRenderer);
                static int frameCount = 0;
                static int state = 0;
                renderer->UpdateAnimation(
                    GraphicsManager::Instance().device_context, state,
                    frameCount);
                frameCount += 1;

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
                auto renderer = (Renderer *)i.second->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->Render(GraphicsManager::Instance().device_context);
            }

            // If there is no need to draw mirror reflections, draw only the
            // opaque mirror
            if (manager->mirror_alpha == 1.0f) {
                auto renderer = (Renderer *)manager->mirror->GetComponent(
                    EnumComponentType::eRenderer);
                renderer->Render(GraphicsManager::Instance().device_context);
            }

            {
                GraphicsManager::Instance().SetPipelineState(
                    manager->draw_wire ? Graphics::skinnedWirePSO
                                       : Graphics::skinnedSolidPSO);
                auto renderer =
                    (SkinnedMeshRenderer *)manager->m_character->GetComponent(
                        EnumComponentType::eRenderer);
                renderer->Render(GraphicsManager::Instance().device_context);
            }

            GraphicsManager::Instance().SetPipelineState(Graphics::normalsPSO);
            for (auto &i : manager->models) {
                auto renderer = (Renderer *)i.second->GetComponent(
                    EnumComponentType::eRenderer);
                if (renderer->m_drawNormals)
                    renderer->RenderNormals(
                        GraphicsManager::Instance().device_context);
            }
            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class UpdateLightSpheresNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        for (auto &i : manager->light_spheres) {
            auto renderer =
                (Renderer *)i->GetComponent(EnumComponentType::eRenderer);
            renderer->UpdateConstantBuffers(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context);
        }
        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};
class DrawOnlyDepthNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        // Depth Only Pass (no RTS)
        GraphicsManager::Instance().device_context->OMSetRenderTargets(
            0, NULL, GraphicsManager::Instance().m_depthOnlyDSV.Get());
        GraphicsManager::Instance().device_context->ClearDepthStencilView(
            GraphicsManager::Instance().m_depthOnlyDSV.Get(), D3D11_CLEAR_DEPTH,
            1.0f, 0);

        GraphicsManager::Instance().SetPipelineState(Graphics::depthOnlyPSO);
        GraphicsManager::Instance().SetGlobalConsts(manager->global_consts_GPU);

        for (auto &i : manager->models) {
            auto renderer = (Renderer *)i.second->GetComponent(
                EnumComponentType::eRenderer);
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        if (true) {
            auto renderer = (Renderer *)manager->skybox->GetComponent(
                EnumComponentType::eRenderer);
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        if (true) {
            auto renderer = (Renderer *)manager->mirror->GetComponent(
                EnumComponentType::eRenderer);
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class DrawLightSpheresNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        GraphicsManager::Instance().SetPipelineState(
            manager->draw_wire ? Graphics::defaultWirePSO
                               : Graphics::defaultSolidPSO);
        GraphicsManager::Instance().SetGlobalConsts(manager->global_consts_GPU);

        for (auto &i : manager->light_spheres) {
            auto renderer =
                (Renderer *)i->GetComponent(EnumComponentType::eRenderer);
            renderer->Render(GraphicsManager::Instance().device_context);
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
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