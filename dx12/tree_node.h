#ifndef _TREENODE
#define _TREENODE

#include "camera_node.h"
#include "game_objects_node.h"
#include "gpu_node.h"
#include "gui_node.h"
#include "light_node.h"
#include "mirror_effect_node.h"
#include "skybox_node.h"
#include "tone_mapping_node.h"

namespace graphics {
namespace node {
class PrepareModelNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

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

            auto obj = std::make_shared<Model>();
            obj->AddComponent(EnumComponentType::eRenderer, renderer);

            targets->objects.insert({obj->GetEntityId(), obj});
        }

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

            auto obj = std::make_shared<Model>();
            obj->AddComponent(EnumComponentType::eRenderer, renderer);

            targets->objects.insert({obj->GetEntityId(), obj});
        }

        // sample object
        {
            std::string base_path = "Assets/Characters/Mixamo/";
            std::string file_name = "character.fbx";

            auto component = std::make_shared<MeshRenderer>();
            component->Initialize(base_path, file_name);
            component->UpdateConstantBuffers();

            auto model = std::make_shared<Model>();
            model->AddComponent(EnumComponentType::eRenderer, component);

            targets->objects.insert({model->GetEntityId(), model});
        }

        // ground
        {
            auto mesh = GeometryGenerator::MakeSquare(5.0);

            auto component = std::make_shared<MeshRenderer>();
            component->Initialize(std::vector{mesh});
            component->MaterialConsts().GetCpu().albedo_factor = Vector3(0.1f);
            component->MaterialConsts().GetCpu().emission_factor =
                Vector3(0.0f);
            component->MaterialConsts().GetCpu().metallic_factor = 0.5f;
            component->MaterialConsts().GetCpu().roughness_factor = 0.3f;

            Vector3 position = Vector3(0.0f, -0.5f, 2.0f);
            component->UpdateWorldRow(Matrix::CreateRotationX(PI * 0.5f) *
                                      Matrix::CreateTranslation(position));

            targets->ground = std::make_shared<ReflectableModel>();
            targets->ground->mirror = std::make_shared<Model>();
            targets->ground->mirror->AddComponent(EnumComponentType::eRenderer,
                                                  component);

            targets->ground->mirror_plane =
                DirectX::SimpleMath::Plane(position, Vector3(0.0f, 1.0f, 0.0f));
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

auto prepare_model = std::make_shared<PrepareModelNode>();
auto gpu_initialize = std::make_shared<GpuInitializeNode>();
auto clear_buffer = std::make_shared<ClearBufferNode>();
auto camera_node = std::make_shared<CameraNodeInvoker>();
auto global_resource_node = std::make_shared<GlobalResourceNode>();
auto global_constant_node = std::make_shared<GlobalConstantNode>();
auto game_object_node = std::make_shared<GameObjectNodeInvoker>();
auto gui_node = std::make_shared<GuiNodeInvoker>();
auto present = std::make_shared<PresentNode>();
auto light_node = std::make_shared<LightNodeInvoker>();
auto resolve_buffer = std::make_shared<ResolveBuffer>();
auto tone_mapping = std::make_shared<ToneMappingNodeInvoker>();
auto skybox_node = std::make_shared<SkyBoxNodeInvoker>();
auto mirror_effect_node = std::make_shared<MirrorEffectNodeInvoker>();

} // namespace node

} // namespace graphics
#endif