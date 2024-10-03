#include "engine.h"

#include "camera_node.h"
#include "game_objects_node.h"
#include "gpu_node.h"
#include "gui_node.h"
#include "light_node.h"
#include "skybox_node.h"
#include "tone_mapping_node.h"
#include "mirror_effect_node.h"

namespace graphics {
class PrepareModelNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

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

            targets->objects.insert({targets->ground->mirror->GetEntityId(),
                                     targets->ground->mirror});
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

Engine::Engine() {
    black_board = std::make_shared<BlackBoard>();
    message_receiver = std::make_unique<MessageReceiver>();
    start_tree = std::make_shared<common::BehaviorTreeBuilder>();
    update_tree = std::make_shared<common::BehaviorTreeBuilder>();
    render_tree = std::make_shared<common::BehaviorTreeBuilder>();
};

bool Engine::Start() {
    // clang-format off
    Platform::Start();
    
    black_board->conditions->stage_type = EnumStageType::eInitialize;

    //initialize
    start_tree->Build(black_board.get())
    ->Sequence()
        ->Excute(gpu_initialize)
        ->Excute(prepare_model)
        ->Excute(global_resource_node)
        ->Excute(global_constant_node)
        ->Excute(camera_node)
        ->Excute(light_node)
        ->Excute(game_object_node)
        ->Excute(skybox_node)
        ->Excute(mirror_effect_node)
        ->Excute(tone_mapping)
        ->Excute(gui_node)
    ->Close()
    ->Run();
    
    //update
    update_tree->Build(black_board.get())
    ->Sequence()
        ->Excute(camera_node)
        ->Excute(light_node)
        ->Excute(global_resource_node)
        ->Excute(global_constant_node)
        ->Excute(game_object_node)
        ->Excute(mirror_effect_node)
    ->Close();
     
    //render
    render_tree->Build(black_board.get())
    ->Sequence()
        ->Excute(clear_buffer)
        ->Excute(skybox_node)
        ->Excute(mirror_effect_node)
        ->Excute(game_object_node)
        ->Excute(tone_mapping)
        ->Excute(gui_node)
        ->Excute(present)
    ->Close();

    Frame();

    // clang-format on
    return true;
}

bool Engine::Frame() {

    // update
    {
        common::ScopeStopWatch stop_watch("Update tree");
        black_board->conditions->delta_time = ImGui::GetIO().DeltaTime;//TODO: need to be fixed
        black_board->conditions->stage_type = EnumStageType::eUpdate;
        update_tree->Run();
    }

    // render
    {
        common::ScopeStopWatch stop_watch("Render tree");
        black_board->conditions->stage_type = EnumStageType::eRender;
        render_tree->Run();
    }

    return true;
}

bool Engine::Stop() {
    Platform::Stop();

    if (black_board) {
        if (black_board->targets) {
            for (auto &model : black_board->targets->objects) {
                model.second.reset();
            }
            black_board->targets->camera.reset();
        }
        if (black_board->gui) {
            GuiNodeInvoker::Shutdown();
            black_board->gui.reset();
        }
        if (black_board->input) {
            black_board->input.reset();
        }
    }

    GpuCore::Instance().Shutdown();

    return true;
}

} // namespace graphics
