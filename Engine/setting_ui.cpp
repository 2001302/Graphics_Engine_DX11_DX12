#include "setting_ui.h"

using namespace common;

void SettingUi::OnStart() {
    ed::Config config;
    config.SettingsFile = "widgets.json";
    context_ = ed::CreateEditor(&config);
}
void SettingUi::OnFrame() {
    StyleSetting();
    MenuBar();
    NodeEditor();
    TabBar();
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

    // Start drawing nodes.
    if (selected_object_id_ != -99999) {
        auto node = node_map[selected_object_id_];
        node->Show();
    }

    ed::End();
    ed::SetCurrentEditor(nullptr);
}
void SettingUi::TabBar() { // Tab Bar
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
                if (ImGui::Selectable(
                        std::to_string(model.first)
                            .c_str(),
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

            ImGui::Checkbox("Wire Frame", &global_setting.draw_as_wire_);

            if (ImGui::RadioButton("Use Light", global_setting.render_mode ==
                                                    EnumRenderMode::eLight)) {
                global_setting.render_mode = EnumRenderMode::eLight;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Use Image Based Lighting",
                                   global_setting.render_mode ==
                                       EnumRenderMode::eImageBasedLighting)) {
                global_setting.render_mode =
                    EnumRenderMode::eImageBasedLighting;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton(
                    "Use Physically Based Lighting",
                    global_setting.render_mode ==
                        EnumRenderMode::ePhysicallyBasedRendering)) {
                global_setting.render_mode =
                    EnumRenderMode::ePhysicallyBasedRendering;
            }

            if (global_setting.render_mode == EnumRenderMode::eLight) {
                ImGui::Checkbox("Use BlinnPhong",
                                &global_setting.light_setting.use_blinn_phong);

                if (ImGui::RadioButton(
                        "Directional Light",
                        global_setting.light_setting.light_type == 0)) {
                    global_setting.light_setting.light_type = 0;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton(
                        "Point Light",
                        global_setting.light_setting.light_type == 1)) {
                    global_setting.light_setting.light_type = 1;
                }
                ImGui::SameLine();
                if (ImGui::RadioButton(
                        "Spot Light",
                        global_setting.light_setting.light_type == 2)) {
                    global_setting.light_setting.light_type = 2;
                }

                ImGui::SliderFloat3(
                    "Position",
                    &global_setting.light_setting.light_from_gui.position.x,
                    -5.0f, 5.0f);
                ImGui::SliderFloat(
                    "Fall Off Start",
                    &global_setting.light_setting.light_from_gui.fallOffStart,
                    0.0f, 5.0f);
                ImGui::SliderFloat(
                    "Fall Off End",
                    &global_setting.light_setting.light_from_gui.fallOffEnd,
                    0.0f, 10.0f);
                ImGui::SliderFloat(
                    "Spot Power",
                    &global_setting.light_setting.light_from_gui.spotPower,
                    1.0f, 512.0f);
            } else if (global_setting.render_mode ==
                       EnumRenderMode::eImageBasedLighting) {
                // ImGui::Checkbox(
                //     "Texture to draw",
                //     &global_setting.cube_map_setting.textureToDraw);
            } else if (global_setting.render_mode ==
                       EnumRenderMode::ePhysicallyBasedRendering) {
                ImGui::Checkbox("useAlbedoMap",
                                &global_setting.pbr_setting.useAlbedoMap);
                ImGui::Checkbox("useNormalMap",
                                &global_setting.pbr_setting.useNormalMap);
                ImGui::Checkbox("useAOMap",
                                &global_setting.pbr_setting.useAOMap);
                ImGui::Checkbox("invertNormalMapY",
                                &global_setting.pbr_setting.invertNormalMapY);
                ImGui::Checkbox("useMetallicMap",
                                &global_setting.pbr_setting.useMetallicMap);
                ImGui::Checkbox("useRoughnessMap",
                                &global_setting.pbr_setting.useRoughnessMap);
                ImGui::Checkbox("useEmissiveMap",
                                &global_setting.pbr_setting.useEmissiveMap);
                ImGui::SliderFloat("Metallic",
                                   &global_setting.pbr_setting.metallic, 0.0f,
                                   1.0f);
                ImGui::SliderFloat("Roughness",
                                   &global_setting.pbr_setting.roughness, 0.0f,
                                   1.0f);
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}
