#include "engine.h"
#include "tree_node.h"

namespace graphics {
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

    GpuCore::Instance().Initialize();
    GpuBuffer::Instance().Initialize();

    OnPrepare(black_board.get());

    black_board->conditions->stage_type = EnumStageType::eInitialize;

    //initialize
    start_tree->Build(black_board.get())
    ->Sequence()
        ->Excute(node::global_resource_node)
        ->Excute(node::global_constant_node)
        ->Excute(node::camera_node)
        ->Excute(node::light_node)
        ->Excute(node::game_object_node)
        ->Excute(node::skybox_node)
        ->Excute(node::mirror_effect_node)
        ->Excute(node::tone_mapping)
        ->Excute(node::gui_node)
    ->Close()
    ->Run();
    
    //update
    update_tree->Build(black_board.get())
    ->Sequence()
        ->Excute(node::camera_node)
        ->Excute(node::light_node)
        ->Excute(node::global_resource_node)
        ->Excute(node::global_constant_node)
        ->Excute(node::game_object_node)
        ->Excute(node::mirror_effect_node)
    ->Close();
     
    //render
    render_tree->Build(black_board.get())
    ->Sequence()
        ->Excute(node::clear_buffer)
        ->Excute(node::game_object_node)
        ->Excute(node::skybox_node)
        ->Excute(node::mirror_effect_node)
        ->Excute(node::tone_mapping)
        ->Excute(node::gui_node)
        ->Excute(node::present)
    ->Close();

    Frame();

    // clang-format on
    return true;
}

bool Engine::Frame() {

    // update
    {
        common::ScopeStopWatch stop_watch("Update tree");
        black_board->conditions->delta_time =
            ImGui::GetIO().DeltaTime; // TODO: need to be fixed
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
void Engine::OnPrepare(BlackBoard* black_board) {

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
        component->MaterialConsts().GetCpu().emission_factor = Vector3(0.0f);
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
}
} // namespace graphics
