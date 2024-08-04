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

            // 추가 물체1
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

            // 추가 물체2
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

                renderer->material_consts.GetCpu().albedoFactor =
                    Vector3(1.0f);
                renderer->material_consts.GetCpu().roughnessFactor = 0.8f;
                renderer->material_consts.GetCpu().metallicFactor = 0.0f;
                renderer->UpdateWorldRow(Matrix::CreateScale(1.0f) *
                                         Matrix::CreateTranslation(center));

                manager->character = std::make_shared<Model>();
                manager->character->AddComponent(EnumComponentType::eRenderer,
                                                   renderer);
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

            {
                auto renderer =
                    (SkinnedMeshRenderer *)manager->character->GetComponent(
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

            {
                for (auto &i : manager->light_spheres) {
                    auto renderer = (MeshRenderer *)i->GetComponent(
                        EnumComponentType::eRenderer);
                    renderer->UpdateConstantBuffers(
                        GraphicsManager::Instance().device,
                        GraphicsManager::Instance().device_context);
                }
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

            {
                GraphicsManager::Instance().SetPipelineState(
                    manager->draw_wire ? Graphics::skinnedWirePSO
                                       : Graphics::skinnedSolidPSO);
                auto renderer =
                    (SkinnedMeshRenderer *)manager->character->GetComponent(
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