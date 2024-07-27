#ifndef _RENDERER_INITIALIZE_NODE
#define _RENDERER_INITIALIZE_NODE

#include "behavior_tree_builder.h"
#include "geometry_generator.h"
#include "renderer.h"
#include "rendering_block.h"

namespace engine {
class InitializeLight : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        // 조명 설정
        {
            // 조명 0은 고정
            manager->m_globalConstsCPU.lights[0].radiance = Vector3(5.0f);
            manager->m_globalConstsCPU.lights[0].position =
                Vector3(0.0f, 1.5f, 1.1f);
            manager->m_globalConstsCPU.lights[0].direction =
                Vector3(0.0f, -1.0f, 0.0f);
            manager->m_globalConstsCPU.lights[0].spotPower = 3.0f;
            manager->m_globalConstsCPU.lights[0].radius = 0.02f;
            manager->m_globalConstsCPU.lights[0].type =
                LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

            // 조명 1의 위치와 방향은 Update()에서 설정
            manager->m_globalConstsCPU.lights[1].radiance = Vector3(5.0f);
            manager->m_globalConstsCPU.lights[1].spotPower = 3.0f;
            manager->m_globalConstsCPU.lights[1].fallOffEnd = 20.0f;
            manager->m_globalConstsCPU.lights[1].radius = 0.02f;
            manager->m_globalConstsCPU.lights[1].type =
                LIGHT_SPOT | LIGHT_SHADOW; // Point with shadow

            // 조명 2는 꺼놓음
            manager->m_globalConstsCPU.lights[2].type = LIGHT_OFF;
        }

        // 조명 위치 표시
        {
            for (int i = 0; i < MAX_LIGHTS; i++) {
                MeshData sphere = GeometryGenerator::MakeSphere(1.0f, 20, 20);

                Renderer *renderer =
                    new Renderer(GraphicsManager::Instance().device,
                                 GraphicsManager::Instance().device_context,
                                 std::vector{sphere});

                renderer->UpdateWorldRow(Matrix::CreateTranslation(
                    manager->m_globalConstsCPU.lights[i].position));
                renderer->m_materialConstsCPU.albedoFactor = Vector3(0.0f);
                renderer->m_materialConstsCPU.emissionFactor =
                    Vector3(1.0f, 1.0f, 0.0f);
                renderer->m_castShadow = false; // 조명 표시 물체들은 그림자 X

                if (manager->m_globalConstsCPU.lights[i].type == 0)
                    renderer->m_isVisible = false;

                manager->light_spheres[i] = std::make_shared<Model>();
                manager->light_spheres[i]->AddComponent(
                    EnumComponentType::eRenderer, renderer);

                manager->models.insert(
                    {manager->light_spheres[i]->GetEntityId(),
                     manager->light_spheres[i]});
            }
        }

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class InitializeCamera : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        manager->camera = std::make_unique<Camera>();
        manager->camera->Update();

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class InitializeSkybox : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        auto mesh_data = GeometryGenerator::MakeBox(40.0f);
        std::reverse(mesh_data.indices.begin(), mesh_data.indices.end());
        Renderer *renderer = new Renderer(
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
                                       envFilename, true, manager->m_envSRV);
        GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                       specularFilename, true,
                                       manager->m_specularSRV);
        GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                       irradianceFilename, true,
                                       manager->m_irradianceSRV);
        GraphicsUtil::CreateDDSTexture(GraphicsManager::Instance().device,
                                       brdfFilename, true, manager->m_brdfSRV);

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class InitializeMirrorGround : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        auto mesh = GeometryGenerator::MakeSquare(5.0);
        Renderer *renderer = new Renderer(
            GraphicsManager::Instance().device,
            GraphicsManager::Instance().device_context, std::vector{mesh});

        // mesh.albedoTextureFilename =
        //     "../Assets/Textures/blender_uv_grid_2k.png";
        renderer->m_materialConstsCPU.albedoFactor = Vector3(0.1f);
        renderer->m_materialConstsCPU.emissionFactor = Vector3(0.0f);
        renderer->m_materialConstsCPU.metallicFactor = 0.5f;
        renderer->m_materialConstsCPU.roughnessFactor = 0.3f;

        Vector3 position = Vector3(0.0f, -0.5f, 2.0f);
        renderer->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
                                 Matrix::CreateTranslation(position));

        manager->ground = std::make_shared<Model>();
        manager->ground->AddComponent(EnumComponentType::eRenderer, renderer);

        manager->m_mirrorPlane =
            DirectX::SimpleMath::Plane(position, Vector3(0.0f, 1.0f, 0.0f));
        manager->m_mirror = manager->ground; // 바닥에 거울처럼 반사 구현

        // m_basicList.push_back(m_ground); // 거울은 리스트에 등록 X

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class CreateGlobalConstantBuffer : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                        manager->m_globalConstsCPU,
                                        manager->m_globalConstsGPU);

        GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                        manager->m_reflectGlobalConstsCPU,
                                        manager->m_reflectGlobalConstsGPU);

        // 그림자맵 렌더링할 때 사용할 GlobalConsts들 별도 생성
        for (int i = 0; i < MAX_LIGHTS; i++) {
            GraphicsUtil::CreateConstBuffer(
                GraphicsManager::Instance().device,
                manager->m_shadowGlobalConstsCPU[i],
                manager->m_shadowGlobalConstsGPU[i]);
        }

        // 후처리 효과용 ConstBuffer
        GraphicsUtil::CreateConstBuffer(GraphicsManager::Instance().device,
                                        manager->m_postEffectsConstsCPU,
                                        manager->m_postEffectsConstsGPU);

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class InitializePostEffect : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        MeshData meshData = GeometryGenerator::MakeSquare();
        Renderer *renderer = new Renderer(
            GraphicsManager::Instance().device,
            GraphicsManager::Instance().device_context, std::vector{meshData});

        manager->screen_square = std::make_shared<Model>();
        manager->screen_square->AddComponent(EnumComponentType::eRenderer,
                                              renderer);

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class InitializePostProcessing : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        manager->m_postProcess.Initialize(
            GraphicsManager::Instance().device,
            GraphicsManager::Instance().device_context,
            {GraphicsManager::Instance().postEffectsSRV},
            {GraphicsManager::Instance().back_buffer_RTV},
            common::Env::Instance().screen_width,
            common::Env::Instance().screen_height, 4);

        return EnumBehaviorTreeStatus::eSuccess;
    }
};
class InitializeBasicModels : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);
        // 추가 물체1
        {
            MeshData mesh = GeometryGenerator::MakeSphere(0.2f, 200, 200);
            Vector3 center(0.5f, 0.5f, 2.0f);
            Renderer *renderer = new Renderer(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context, std::vector{mesh});
            renderer->UpdateWorldRow(Matrix::CreateTranslation(center));
            renderer->m_materialConstsCPU.albedoFactor =
                Vector3(0.1f, 0.1f, 1.0f);
            renderer->m_materialConstsCPU.roughnessFactor = 0.2f;
            renderer->m_materialConstsCPU.metallicFactor = 0.6f;
            renderer->m_materialConstsCPU.emissionFactor = Vector3(0.0f);
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
            Renderer *renderer = new Renderer(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context, std::vector{mesh});
            renderer->UpdateWorldRow(Matrix::CreateTranslation(center));
            renderer->m_materialConstsCPU.albedoFactor =
                Vector3(1.0f, 0.2f, 0.2f);
            renderer->m_materialConstsCPU.roughnessFactor = 0.5f;
            renderer->m_materialConstsCPU.metallicFactor = 0.9f;
            renderer->m_materialConstsCPU.emissionFactor = Vector3(0.0f);
            renderer->UpdateConstantBuffers(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context);

            auto obj = std::make_shared<Model>();
            obj->AddComponent(EnumComponentType::eRenderer, renderer);

            manager->models.insert({obj->GetEntityId(), obj});
        }

        return EnumBehaviorTreeStatus::eSuccess;
    }
};
} // namespace engine

#endif