#include "panel.h"
#include "game_object_node.h"

using namespace Engine;
Panel::Panel(std::shared_ptr<PipelineManager> pipeline_manager) {
    pipeline_manager_ = pipeline_manager;
}

void Panel::OnStart() {
    ed::Config config;
    config.SettingsFile = "Widgets.json";
    context_ = ed::CreateEditor(&config);
}

void Panel::OnFrame(float deltaTime) {
    auto &io = ImGui::GetIO();

    {
        ImGuiStyle &style = ImGui::GetStyle();
        style.Alpha = 1.0f;
        style.WindowRounding = 5.3f;
        style.FrameRounding = 2.3f;
        style.ScrollbarRounding = 0;
    }

    {
        // FPS Counter Ribbon
        ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate,
                    io.Framerate ? 1000.0f / io.Framerate : 0.0f);
        ImGui::Separator();

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

    {
        // Node Editor Widget
        ed::SetCurrentEditor(context_);
        ed::Begin("My Editor", ImVec2(0.0, ImGui::GetWindowHeight() / 2.0f));
        
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
        
        ed::EditorContext *m_Context = nullptr;
    }

    {
        ImGui::BeginTabBar("TabBar");

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
        if (ImGui::BeginTabItem("Inspector")) {
            ImGui::Checkbox("Wire Frame", &draw_as_wire_);
            ImGui::Checkbox("Use Texture", &m_useTexture);
            ImGui::Checkbox("Use BlinnPhong", &m_useBlinnPhong);

            ImGui::Text("Transform");
            ImGui::SliderFloat3("Translation", &m_modelTranslation.x, -2.0f,
                                2.0f);
            ImGui::SliderFloat("Rotation", &m_modelRotation.y, -3.14f, 3.14f);
            ImGui::SliderFloat3("Scaling", &m_modelScaling.x, 0.1f, 4.0f);

            ImGui::Text("Material");
            ImGui::SliderFloat("Shininess", &shininess_, 1.0f, 256.0f);
            ImGui::SliderFloat("Diffuse", &m_materialDiffuse, 0.0f, 1.0f);
            ImGui::SliderFloat("Specular", &m_materialSpecular, 0.0f, 1.0f);

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Light")) {
            if (ImGui::RadioButton("Directional Light", m_lightType == 0)) {
                m_lightType = 0;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Point Light", m_lightType == 1)) {
                m_lightType = 1;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Spot Light", m_lightType == 2)) {
                m_lightType = 2;
            }

            ImGui::SliderFloat3("Position", &m_lightFromGUI.position.x, -5.0f,
                                5.0f);
            ImGui::SliderFloat("Fall Off Start", &m_lightFromGUI.fallOffStart,
                               0.0f, 5.0f);
            ImGui::SliderFloat("Fall Of fEnd", &m_lightFromGUI.fallOffEnd, 0.0f,
                               10.0f);
            ImGui::SliderFloat("Spot Power", &m_lightFromGUI.spotPower, 1.0f,
                               512.0f);

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}
