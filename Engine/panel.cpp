#include "panel.h"

using namespace Engine;

//void Panel::OnStart() 
//{
//
//}

void Panel::OnFrame(float deltaTime) 
{
    {
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

        // Framerate
        ImGui::Text("Average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::Separator();

        ImGui::BeginTabBar("TabBar");
        if (ImGui::BeginTabItem("Hierarchy")) {
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Inspector")) {
            ImGui::Checkbox("Wire Frame", &m_drawAsWire);
            ImGui::Checkbox("Use Texture", &m_useTexture);
            ImGui::Checkbox("Use BlinnPhong", &m_useBlinnPhong);

            ImGui::Text("Transform");
            ImGui::SliderFloat3("Translation", &m_modelTranslation.x, -2.0f,
                                2.0f);
            ImGui::SliderFloat("Rotation", &m_modelRotation.y, -3.14f, 3.14f);
            ImGui::SliderFloat3("Scaling", &m_modelScaling.x, 0.1f, 4.0f);

            ImGui::Text("Material");
            ImGui::SliderFloat("Shininess", &m_shininess, 1.0f, 256.0f);
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