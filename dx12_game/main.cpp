#pragma once

#include <engine.h>

namespace graphics {
class GameDx12 : public Engine {
  public:
    void LoadAsset(BlackBoard *black_board) override {
        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        // skybox
        {
            auto mesh = GeometryGenerator::MakeBox(40.0f);
            std::reverse(mesh.indices.begin(), mesh.indices.end());

            auto skybox_renderer = std::make_shared<SkyboxRenderer>();
            skybox_renderer->Initialize(
                std::vector{mesh},
                L"./Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds",
                L"./Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds",
                L"./Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds",
                L"./Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds");
            targets->world->TryAdd(skybox_renderer);
        }

        // sample object
        {
            MeshData mesh = GeometryGenerator::MakeBox(0.2f);
            Vector3 center(0.0f, 0.5f, 2.5f);

            auto renderer = std::make_shared<MeshRenderer>();
            renderer->Initialize(std::vector{mesh});
            renderer->UpdateWorldRow(Matrix::CreateTranslation(center));
            renderer->MaterialConsts().GetCpu().albedo_factor =
                Vector3(1.0f, 0.2f, 0.2f);
            renderer->MaterialConsts().GetCpu().roughness_factor = 0.5f;
            renderer->MaterialConsts().GetCpu().metallic_factor = 0.9f;
            renderer->MaterialConsts().GetCpu().emission_factor = Vector3(0.0f);
            renderer->UpdateConstantBuffers();

            auto obj = std::make_shared<common::Model>();
            obj->TryAdd(renderer);

            targets->objects.insert({obj->GetEntityId(), obj});
        }

        // sample object
        {
            MeshData mesh = GeometryGenerator::MakeSphere(0.2f, 200, 200);
            Vector3 center(0.5f, 0.5f, 2.0f);

            auto renderer = std::make_shared<MeshRenderer>();
            renderer->Initialize(std::vector{mesh});
            renderer->UpdateWorldRow(Matrix::CreateTranslation(center));
            renderer->MaterialConsts().GetCpu().albedo_factor =
                Vector3(0.1f, 0.1f, 1.0f);
            renderer->MaterialConsts().GetCpu().roughness_factor = 0.2f;
            renderer->MaterialConsts().GetCpu().metallic_factor = 0.6f;
            renderer->MaterialConsts().GetCpu().emission_factor = Vector3(0.0f);
            renderer->UpdateConstantBuffers();

            auto obj = std::make_shared<common::Model>();
            obj->TryAdd(renderer);

            targets->objects.insert({obj->GetEntityId(), obj});
        }

        // sample object
        {
            std::string base_path = "Assets/Characters/Mixamo/";
            std::string file_name = "character.fbx";

            auto component = std::make_shared<MeshRenderer>();
            component->Initialize(base_path, file_name);
            component->UpdateConstantBuffers();

            auto model = std::make_shared<common::Model>();
            model->TryAdd(component);

            targets->objects.insert({model->GetEntityId(), model});
        }

        // ground
        {
            auto mesh = GeometryGenerator::MakeSquare(5.0);

            auto component = std::make_shared<MirrorRenderer>();
            component->Initialize(std::vector{mesh});
            component->MaterialConsts().GetCpu().albedo_factor = Vector3(0.1f);
            component->MaterialConsts().GetCpu().emission_factor =
                Vector3(0.0f);
            component->MaterialConsts().GetCpu().metallic_factor = 0.5f;
            component->MaterialConsts().GetCpu().roughness_factor = 0.3f;

            Vector3 position = Vector3(0.0f, -0.5f, 2.0f);
            component->UpdateWorldRow(Matrix::CreateRotationX(PI * 0.5f) *
                                      Matrix::CreateTranslation(position));

            component->SetMirrorPlane(DirectX::SimpleMath::Plane(
                position, Vector3(0.0f, 1.0f, 0.0f)));
            component->SetMirrorAlpha(0.7f);

            targets->ground->TryAdd(component);
        }
    }
};
} // namespace graphics

void main() {
    std::unique_ptr<graphics::GameDx12> engine =
        std::make_unique<graphics::GameDx12>();

    if (engine->Start()) {
        engine->Run();
    }

    engine->Stop();
}
