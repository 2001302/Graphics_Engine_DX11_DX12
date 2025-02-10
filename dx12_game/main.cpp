#pragma once

#include "animator.h"
#include <engine.h>
#include <util/path.h>

namespace graphics {
struct Idle : public common::AnimationNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {
        auto block = dynamic_cast<Animator::Block *>(data_block);
        assert(block != nullptr);

        block->clip_id = 0;

        return common::EnumBehaviorTreeStatus::eRunning;
    }
};

class GameDx12 : public Engine {
  public:
    void LoadAsset(BlackBoard *black_board) override {
        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        // skybox
        {
            auto mesh = GeometryGenerator::MakeBox(40.0f);
            std::reverse(mesh.indices.begin(), mesh.indices.end());

            auto env_path = common::Path::Combine(
                common::env::env_root,
                "Assets/Textures/Cubemaps/HDRI/DaySky/DaySkyEnvHDR.dds");
            auto specular_path = common::Path::Combine(
                common::env::env_root,
                "Assets/Textures/Cubemaps/HDRI/DaySky/DaySkySpecularHDR.dds");
            auto diffuse_path = common::Path::Combine(
                common::env::env_root,
                "Assets/Textures/Cubemaps/HDRI/DaySky/DaySkyDiffuseHDR.dds");
            auto brdf_path = common::Path::Combine(
                common::env::env_root,
                "Assets/Textures/Cubemaps/HDRI/DaySky/DaySkyBrdf.dds");

            auto skybox_renderer = std::make_shared<SkyboxRenderer>();
            skybox_renderer->Initialize(
                std::vector{mesh},
                std::wstring(env_path.begin(), env_path.end()).c_str(),
                std::wstring(specular_path.begin(), specular_path.end())
                    .c_str(),
                std::wstring(diffuse_path.begin(), diffuse_path.end()).c_str(),
                std::wstring(brdf_path.begin(), brdf_path.end()).c_str());

            targets->world->TryAdd(skybox_renderer);
        }

        //// sample object 1
        //{
        //    MeshData mesh = GeometryGenerator::MakeBox(0.2f);

        //    auto renderer = std::make_shared<MeshRenderer>();
        //    renderer->Initialize(std::vector{mesh});
        //    renderer->UpdateWorldRow(
        //        Matrix::CreateTranslation(Vector3(0.0f, 0.5f, 2.5f)));
        //    renderer->MaterialConsts().GetCpu().SetMaterialConstants(
        //        Vector3(1.0f, 0.2f, 0.2f), 0.5f, 0.9f, Vector3(0.0f));
        //    renderer->UpdateConstantBuffers();

        //    auto obj = std::make_shared<common::Model>();
        //    obj->TryAdd(renderer);

        //    targets->objects.insert({obj->GetEntityId(), obj});
        //}

        //// sample object 2
        //{
        //    MeshData mesh = GeometryGenerator::MakeSphere(0.2f, 200, 200);

        //    auto renderer = std::make_shared<MeshRenderer>();
        //    renderer->Initialize(std::vector{mesh});
        //    renderer->UpdateWorldRow(
        //        Matrix::CreateTranslation(Vector3(0.5f, 0.5f, 2.0f)));
        //    renderer->MaterialConsts().GetCpu().SetMaterialConstants(
        //        Vector3(0.1f, 0.1f, 1.0f), 0.2f, 0.6f, Vector3(0.0f));
        //    renderer->UpdateConstantBuffers();

        //    auto obj = std::make_shared<common::Model>();
        //    obj->TryAdd(renderer);

        //    targets->objects.insert({obj->GetEntityId(), obj});
        //}

        //// sample object 3
        //{
        //    std::string base_path = "../Assets/Characters/Mixamo/";
        //    std::vector<std::string> clipNames = {"Shuffling.fbx"};

        //    AnimationData aniData;
        //    auto [meshes, _] = GeometryGenerator::ReadAnimationFromFile(
        //        base_path, "character.fbx");

        //    for (auto &name : clipNames) {
        //        auto [_, ani] =
        //            GeometryGenerator::ReadAnimationFromFile(base_path, name);

        //        if (aniData.clips.empty()) {
        //            aniData = ani;
        //        } else {
        //            aniData.clips.push_back(ani.clips.front());
        //        }
        //    }

        //    auto behavior_tree = new common::BehaviorTreeBuilder();
        //    auto block = new Animator::Block();
        //    behavior_tree->Build(block)
        //        ->Excute(std::make_shared<Idle>())
        //        ->Close();

        //    auto renderer = std::make_shared<MeshRenderer>();
        //    renderer->Initialize(meshes, true);
        //    renderer->UpdateConstantBuffers();

        //    auto animator = std::make_shared<Animator>(aniData);
        //    animator->SetBuilder(std::move(behavior_tree));
        //    animator->SetBlock(std::move(block));
        //
        //    auto model = std::make_shared<common::Model>();
        //    model->TryAdd(renderer);
        //    model->TryAdd(animator);

        //    targets->objects.insert({model->GetEntityId(), model});
        //}

        // sample object 3
        {
            std::string base_path = common::Path::Combine(
                common::env::env_root, "Assets/Characters/Temp/");

            auto meshes = GeometryGenerator::ReadFromFile(
                base_path, "The_Hulk_Full_Body_C_0206115946_refine.fbx");

            auto renderer = std::make_shared<MeshRenderer>();
            renderer->Initialize(meshes);
            renderer->UpdateConstantBuffers();

            auto model = std::make_shared<common::Model>();
            model->TryAdd(renderer);

            targets->objects.insert({model->GetEntityId(), model});
        }

        // ground
        {
            auto mesh = GeometryGenerator::MakeSquare(50.0);

            auto component = std::make_shared<MirrorRenderer>();
            component->Initialize(std::vector{mesh});
            component->MaterialConsts().GetCpu().SetMaterialConstants(
                Vector3(0.1f), 0.5f, 0.3f, Vector3(0.0f));

            Vector3 position = Vector3(0.0f, -0.5f, 2.0f);
            component->UpdateWorldRow(Matrix::CreateRotationX(PI * 0.5f) *
                                      Matrix::CreateTranslation(position));

            component->SetMirrorPlane(DirectX::SimpleMath::Plane(
                position, Vector3(0.0f, 1.0f, 0.0f)));
            component->SetMirrorAlpha(0.7f);

            targets->world->TryAdd(component);
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
