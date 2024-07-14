#include "setting_ui.h"

using namespace common;

void SettingUi::OnStart() {
    ed::Config config;
    config.SettingsFile = "widgets.json";
    context_ = ed::CreateEditor(&config);
}
void SettingUi::OnFrame() {
    // FrameRate();
    // StyleSetting();
    // MenuBar();
    // NodeEditor();
    // TabBar();
}
void SettingUi::StyleSetting() {
    ImGuiStyle &style = ImGui::GetStyle();
    style.Alpha = 1.0f;
    style.WindowRounding = 5.3f;
    style.FrameRounding = 2.3f;
    style.ScrollbarRounding = 0;
}
void SettingUi::FrameRate() {
    auto &io = ImGui::GetIO();

    // FPS Counter Ribbon
    ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate,
                io.Framerate ? 1000.0f / io.Framerate : 0.0f);
    ImGui::Separator();
}
void SettingUi::MenuBar() {
    // Add Object
    if (ImGui::Button("Sphere")) {
        SendMessage(Env::Instance().main_window, WM_SPHERE_LOAD, 0, 0);
    }
    ImGui::SameLine();
    if (ImGui::Button("Box")) {
        SendMessage(Env::Instance().main_window, WM_BOX_LOAD, 0, 0);
    }
    ImGui::SameLine();
    if (ImGui::Button("Cylinder")) {
        SendMessage(Env::Instance().main_window, WM_CYLINDER_LOAD, 0, 0);
    }
    ImGui::SameLine();
    if (ImGui::Button("Search")) {
        SendMessage(Env::Instance().main_window, WM_MODEL_LOAD, 0, 0);
    }
    ImGui::Separator();
}
void SettingUi::NodeEditor() {
    // Node Editor Widget
    ed::SetCurrentEditor(context_);
    ed::Begin("My Editor", ImVec2(0.0, ImGui::GetWindowHeight() / 1.5f));
    auto &io = ImGui::GetIO();

    for (auto node : nodes) {
        node->Show();
    }

    ed::End();
    ed::SetCurrentEditor(nullptr);
}
void SettingUi::TabBar(
    std::unordered_map<int, std::shared_ptr<INodeUi>> node_map) { // Tab Bar

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

            for (auto &model : node_map) {

                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                if (ImGui::Selectable(std::to_string(model.first).c_str(),
                                      model.first == selected_object_id_,
                                      ImGuiSelectableFlags_SpanAllColumns)) {
                    selected_object_id_ = model.first;
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::Text("Node");

                if (selected_object_id_ == model.first) {
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
                                           ImGui::GetColorU32(ImGuiCol_Header));
                }
            }

            ImGui::EndTable();

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Setting")) {

            ImGui::Checkbox("Wire Frame", &tab.common.draw_as_wire_);

            if (ImGui::RadioButton("Use Light", tab.common.render_mode ==
                                                    EnumRenderMode::eLight)) {
                tab.common.render_mode = EnumRenderMode::eLight;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Use Image Based Lighting",
                                   tab.common.render_mode ==
                                       EnumRenderMode::eImageBasedLighting)) {
                tab.common.render_mode = EnumRenderMode::eImageBasedLighting;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton(
                    "Use Physically Based Lighting",
                    tab.common.render_mode ==
                        EnumRenderMode::ePhysicallyBasedRendering)) {
                tab.common.render_mode =
                    EnumRenderMode::ePhysicallyBasedRendering;
            }

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Cube Map")) {

            ImGui::SliderFloat("MipLevel", &tab.cube_map.mipLevel, 0.0f, 1.0f);
            ImGui::SliderInt("textureToDraw", &tab.cube_map.textureToDraw, 0,
                             2);

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Ground")) {

            ImGui::Checkbox("useHeightMap", &tab.ground.useHeightMap);

            ImGui::SliderFloat("heightScale", &tab.ground.heightScale, 0.0f,
                               1.0f);
            ImGui::SliderFloat("ambient", &tab.ground.ambient, 0.0f, 1.0f);
            ImGui::SliderFloat("shininess", &tab.ground.shininess, 0.0f, 1.0f);
            ImGui::SliderFloat("diffuse", &tab.ground.diffuse, 0.0f, 1.0f);
            ImGui::SliderFloat("specular", &tab.ground.specular, 0.0f, 1.0f);
            ImGui::SliderFloat("fresnelR0", &tab.ground.fresnelR0, 0.0f, 1.0f);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Light")) {
            if (ImGui::RadioButton("Directional Light",
                                   tab.light.light_type == 0)) {
                tab.light.light_type = 0;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Point Light", tab.light.light_type == 1)) {
                tab.light.light_type = 1;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Spot Light", tab.light.light_type == 2)) {
                tab.light.light_type = 2;
            }

            ImGui::SliderFloat3(
                "Position", &tab.light.light_from_gui.position.x, -5.0f, 5.0f);
            ImGui::SliderFloat("Fall Off Start",
                               &tab.light.light_from_gui.fallOffStart, 0.0f,
                               5.0f);
            ImGui::SliderFloat("Fall Off End",
                               &tab.light.light_from_gui.fallOffEnd, 0.0f,
                               10.0f);
            ImGui::SliderFloat("Spot Power",
                               &tab.light.light_from_gui.spotPower, 1.0f,
                               512.0f);

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Filter")) {

            ImGui::SliderFloat("bloom_strength", &tab.filter.bloom_strength,
                               0.0f, 1.0f);
            ImGui::SliderFloat("bloom_exposure", &tab.filter.bloom_exposure,
                               0.0f, 1.0f);
            ImGui::SliderFloat("bloom_gamma", &tab.filter.bloom_gamma, 0.0f,
                               1.0f);

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}
int SettingUi::SelectedId() {
    if (selected_object_id_ == -99999)
        return 0;
    else {
        return selected_object_id_;
    }
}
void SettingUi::PushNode(INodeUi *node) {
    node->uniqueId = unique_id;
    node->position = ImVec2(unique_pos_x, 0);

    nodes.push_back(node);
    unique_id = unique_id + 10;
    unique_pos_x = unique_pos_x + 500;
};
void SettingUi::ClearNode() {
    unique_id = 1;
    unique_pos_x = 0;
    nodes.clear();
};
