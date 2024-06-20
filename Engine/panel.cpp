#include "panel.h"
#include "game_object_node.h"

using namespace Engine;
Panel::Panel(std::shared_ptr<PipelineManager> pipeline_manager) {
    pipeline_manager_ = pipeline_manager;
}
void Panel::OnStart() {
    ed::Config config;
    config.SettingsFile = "widgets.json";
    context_ = ed::CreateEditor(&config);
}
void Panel::OnFrame(float deltaTime) {
    StyleSetting();
    MenuBar();
    NodeEditor();
    TabBar();
}
void Panel::StyleSetting() {
    ImGuiStyle &style = ImGui::GetStyle();
    style.Alpha = 1.0f;
    style.WindowRounding = 5.3f;
    style.FrameRounding = 2.3f;
    style.ScrollbarRounding = 0;
}
void Panel::FrameRate() {
    auto &io = ImGui::GetIO();

    // FPS Counter Ribbon
    ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate,
                io.Framerate ? 1000.0f / io.Framerate : 0.0f);
    ImGui::Separator();
}
void Panel::MenuBar() {
    // Add Object
    if (ImGui::Button("Sphere")) {
        SendMessage(main_window_, WM_SPHERE_LOAD, 0, 0);
    }
    ImGui::SameLine();
    if (ImGui::Button("Box")) {
        SendMessage(main_window_, WM_BOX_LOAD, 0, 0);
    }
    ImGui::SameLine();
    if (ImGui::Button("Cylinder")) {
        SendMessage(main_window_, WM_CYLINDER_LOAD, 0, 0);
    }
    ImGui::SameLine();
    if (ImGui::Button("Search")) {
        SendMessage(main_window_, WM_MODEL_LOAD, 0, 0);
    }
    ImGui::Separator();
}
void Panel::NodeEditor() {
    // Node Editor Widget
    ed::SetCurrentEditor(context_);
    ed::Begin("My Editor", ImVec2(0.0, ImGui::GetWindowHeight() / 1.5f));

    // Start drawing nodes.
    if (selected_object_id_ == -99999) {
        auto graph = std::make_shared<DefaultGraphNode>();
        graph->OnShow();
    } else {
        auto graph = pipeline_manager_->behaviors[selected_object_id_];
        graph->Show();
    }

    ed::End();
    ed::SetCurrentEditor(nullptr);
}
void Panel::TabBar() { // Tab Bar
    if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_FittingPolicyScroll)) {
        if (ImGui::BeginTabItem("Hierarchy")) {
            ImGui::BeginTable("MyTable", ImGuiTableFlags_Resizable |
                                             ImGuiTableFlags_Reorderable);
            // headers
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("ID");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("Name");

            for (auto &model : pipeline_manager_->models) {

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                if (ImGui::Selectable(
                        std::to_string(model.second.get()->GetEntityId())
                            .c_str(),
                        model.second->GetEntityId() == selected_object_id_,
                        ImGuiSelectableFlags_SpanAllColumns)) {
                    selected_object_id_ = model.second.get()->GetEntityId();
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::Text(model.second.get()->GetName().c_str());

                if (selected_object_id_ == model.second->GetEntityId()) {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
                                           ImGui::GetColorU32(ImGuiCol_Header));
                }
            }

            ImGui::EndTable();

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Light")) {

            ImGui::Checkbox("Wire Frame", &light_tab_.draw_as_wire_);

            if (ImGui::RadioButton("Directional Light",
                                   light_tab_.light_type == 0)) {
                light_tab_.light_type = 0;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Point Light", light_tab_.light_type == 1)) {
                light_tab_.light_type = 1;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Spot Light", light_tab_.light_type == 2)) {
                light_tab_.light_type = 2;
            }

            ImGui::SliderFloat3(
                "Position", &light_tab_.light_from_gui.position.x, -5.0f,
                                5.0f);
            ImGui::SliderFloat("Fall Off Start",
                               &light_tab_.light_from_gui.fallOffStart,
                               0.0f, 5.0f);
            ImGui::SliderFloat("Fall Of fEnd",
                               &light_tab_.light_from_gui.fallOffEnd, 0.0f,
                               10.0f);
            ImGui::SliderFloat("Spot Power",
                               &light_tab_.light_from_gui.spotPower, 1.0f,
                               512.0f);

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}