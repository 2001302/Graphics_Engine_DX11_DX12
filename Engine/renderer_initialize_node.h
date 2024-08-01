#ifndef _RENDERER_INITIALIZE_NODE
#define _RENDERER_INITIALIZE_NODE

#include "behavior_tree_builder.h"
#include "geometry_generator.h"
#include "renderer.h"
#include "rendering_block.h"

namespace engine {
class InitializeLightNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[common::EnumDataBlockType::eRenderBlock]);
        assert(manager != nullptr);

        // 조명 설정
        {
            // 조명 0은 고정
            manager->global_consts_CPU.lights[0].radiance = Vector3(5.0f);
            manager->global_consts_CPU.lights[0].position =
                Vector3(0.0f, 1.5f, 1.1f);
            manager->global_consts_CPU.lights[0].direction =
                Vector3(0.0f, -1.0f, 0.0f);
            manager->global_consts_CPU.lights[0].spotPower = 3.0f;
            manager->global_consts_CPU.lights[0].radius = 0.02f;
            manager->global_consts_CPU.lights[0].type =
                LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

            // 조명 1의 위치와 방향은 Update()에서 설정
            manager->global_consts_CPU.lights[1].radiance = Vector3(5.0f);
            manager->global_consts_CPU.lights[1].spotPower = 3.0f;
            manager->global_consts_CPU.lights[1].fallOffEnd = 20.0f;
            manager->global_consts_CPU.lights[1].radius = 0.02f;
            manager->global_consts_CPU.lights[1].type =
                LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

            // 조명 2는 꺼놓음
            manager->global_consts_CPU.lights[2].type = LIGHT_OFF;
        }

        // 조명 위치 표시
        {
            for (int i = 0; i < MAX_LIGHTS; i++) {
                MeshData sphere = GeometryGenerator::MakeSphere(1.0f, 20, 20);

                auto renderer = std::make_shared<Renderer>(
                    GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context,
                    std::vector{sphere});

                renderer->UpdateWorldRow(Matrix::CreateTranslation(
                    manager->global_consts_CPU.lights[i].position));
                renderer->m_materialConsts.GetCpu().albedoFactor =
                    Vector3(0.0f);
                renderer->m_materialConsts.GetCpu().emissionFactor =
                    Vector3(1.0f, 1.0f, 0.0f);
                renderer->m_castShadow = false; // 조명 표시 물체들은 그림자 X

                if (manager->global_consts_CPU.lights[i].type == 0)
                    renderer->m_isVisible = false;

                manager->light_spheres[i] = std::make_shared<Model>();
                manager->light_spheres[i]->AddComponent(
                    EnumComponentType::eRenderer, renderer);
            }
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class InitializeCameraNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[common::EnumDataBlockType::eRenderBlock]);
        assert(manager != nullptr);

        manager->camera = std::make_unique<Camera>();
        manager->camera->Initialize();
        manager->camera->Update();

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class InitializeSkyboxNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[common::EnumDataBlockType::eRenderBlock]);
        assert(manager != nullptr);

        auto mesh_data = GeometryGenerator::MakeBox(40.0f);
        std::reverse(mesh_data.indices.begin(), mesh_data.indices.end());

        auto renderer = std::make_shared<Renderer>(
            GraphicsManager::Instance().device,
            GraphicsManager::Instance().device_context, std::vector{mesh_data});

        manager->skybox = std::make_shared<Model>();
        manager->skybox->AddComponent(EnumComponentType::eRenderer, renderer);

        auto envFilename = L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds";
        auto specularFilename =
            L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds";
        auto irradianceFilename =
            L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds";
        auto brdfFilename = L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds";

        GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                       envFilename, true, manager->env_SRV);
        GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                       specularFilename, true,
                                       manager->specular_SRV);
        GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                       irradianceFilename, true,
                                       manager->irradiance_SRV);
        GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                       brdfFilename, true, manager->brdf_SRV);

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class InitializeMirrorGroundNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[common::EnumDataBlockType::eRenderBlock]);
        assert(manager != nullptr);

        auto mesh = GeometryGenerator::MakeSquare(5.0);

        auto renderer = std::make_shared<Renderer>(
            GraphicsManager::Instance().device,
            GraphicsManager::Instance().device_context, std::vector{mesh});

        // mesh.albedoTextureFilename =
        //     "../Assets/Textures/blender_uv_grid_2k.png";
        renderer->m_materialConsts.GetCpu().albedoFactor = Vector3(0.1f);
        renderer->m_materialConsts.GetCpu().emissionFactor = Vector3(0.0f);
        renderer->m_materialConsts.GetCpu().metallicFactor = 0.5f;
        renderer->m_materialConsts.GetCpu().roughnessFactor = 0.3f;

        Vector3 position = Vector3(0.0f, -0.5f, 2.0f);
        renderer->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
                                 Matrix::CreateTranslation(position));

        manager->ground = std::make_shared<Model>();
        manager->ground->AddComponent(EnumComponentType::eRenderer, renderer);

        manager->mirror_plane =
            DirectX::SimpleMath::Plane(position, Vector3(0.0f, 1.0f, 0.0f));
        manager->mirror = manager->ground; // 바닥에 거울처럼 반사 구현

        // m_basicList.push_back(m_ground); // 거울은 리스트에 등록 X

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class CreateGlobalConstantBufferNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[common::EnumDataBlockType::eRenderBlock]);
        assert(manager != nullptr);

        GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                        manager->global_consts_CPU,
                                        manager->global_consts_GPU);

        GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                        manager->reflect_global_consts_CPU,
                                        manager->reflect_global_consts_GPU);

        // 그림자맵 렌더링할 때 사용할 GlobalConsts들 별도 생성
        for (int i = 0; i < MAX_LIGHTS; i++) {
            GraphicsUtil::CreateConstBuffer(
                GraphicsManager::Instance().device,
                manager->shadow_global_consts_CPU[i],
                manager->shadow_global_consts_GPU[i]);
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class InitializePostEffectNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[common::EnumDataBlockType::eRenderBlock]);
        assert(manager != nullptr);

        MeshData meshData = GeometryGenerator::MakeSquare();

        auto renderer = std::make_shared<Renderer>(
            GraphicsManager::Instance().device,
            GraphicsManager::Instance().device_context, std::vector{meshData});

        manager->screen_square = std::make_shared<Model>();
        manager->screen_square->AddComponent(EnumComponentType::eRenderer,
                                             renderer);

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class InitializeBasicModelsNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[common::EnumDataBlockType::eRenderBlock]);
        assert(manager != nullptr);
        // 추가 물체1
        {
            MeshData mesh = GeometryGenerator::MakeSphere(0.2f, 200, 200);
            Vector3 center(0.5f, 0.5f, 2.0f);

            auto renderer = std::make_shared<Renderer>(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context, std::vector{mesh});

            renderer->UpdateWorldRow(Matrix::CreateTranslation(center));
            renderer->m_materialConsts.GetCpu().albedoFactor =
                Vector3(0.1f, 0.1f, 1.0f);
            renderer->m_materialConsts.GetCpu().roughnessFactor = 0.2f;
            renderer->m_materialConsts.GetCpu().metallicFactor = 0.6f;
            renderer->m_materialConsts.GetCpu().emissionFactor = Vector3(0.0f);
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
                GraphicsManager::Instance().device_context, std::vector{mesh});

            renderer->UpdateWorldRow(Matrix::CreateTranslation(center));
            renderer->m_materialConsts.GetCpu().albedoFactor =
                Vector3(1.0f, 0.2f, 0.2f);
            renderer->m_materialConsts.GetCpu().roughnessFactor = 0.5f;
            renderer->m_materialConsts.GetCpu().metallicFactor = 0.9f;
            renderer->m_materialConsts.GetCpu().emissionFactor = Vector3(0.0f);
            renderer->UpdateConstantBuffers(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context);

            auto obj = std::make_shared<Model>();
            obj->AddComponent(EnumComponentType::eRenderer, renderer);

            manager->models.insert({obj->GetEntityId(), obj});
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};
} // namespace engine

#endif