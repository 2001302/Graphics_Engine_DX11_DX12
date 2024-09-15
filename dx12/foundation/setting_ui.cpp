#include "setting_ui.h"

namespace common {
void SetWindowLocation(float x, float y, float width, float height) {

    ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
}

void FrameRate() {
    auto &io = ImGui::GetIO();
    ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate,
                io.Framerate ? 1000.0f / io.Framerate : 0.0f);
}

void SettingUi::PushNodeItem(INode *node) {
    node->uniqueId = unique_id;
    node->position = ImVec2((float)unique_pos_x, 0);

    node_items.push_back(node);
    unique_id = unique_id + 10;
    unique_pos_x = unique_pos_x + 500;
};

void SettingUi::ClearNodeItem() {
    unique_id = 1;
    unique_pos_x = 0;
    node_items.clear();
};

void SettingUi::PushInfoItem(IInfo *node) { info_items.push_back(node); };

void SettingUi::ClearInfoItem() { info_items.clear(); };

void SettingUi::OnStart() {

    ed::Config config;
    config.SettingsFile = "widgets.json";
    context_ = ed::CreateEditor(&config);
}

void SettingUi::OnFrame() {

    screen_width = (float)Env::Instance().screen_width;
    screen_hieght = (float)Env::Instance().screen_height;

    TopBar();
    MainView();
}

void SettingUi::TopBar() {
    SetWindowLocation(0.0f, 0.0f, screen_width, offset_top);
    const float button_width = 150.0f;

    ImGui::Begin("Top Bar", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoScrollbar);
    FrameRate();

    ImGui::SameLine(left_panel_width + 3.0f, 0.0f);
    if (ImGui::Selectable("3D Viewport", view_type == EnumViewType::e3dViewport,
                          ImGuiSelectableFlags_None,
                          ImVec2(button_width, 0.0f))) {
        view_type = EnumViewType::e3dViewport;
    }

    ImGui::SameLine();
    if (ImGui::Selectable("Node Editor", view_type == EnumViewType::eNodeEditor,
                          ImGuiSelectableFlags_None,
                          ImVec2(button_width, 0.0f))) {
        view_type = EnumViewType::eNodeEditor;
    }

    ImGui::End();
}

void SettingUi::MainView() {
    if (view_type == EnumViewType::e3dViewport) {
        SetWindowLocation(0.0f, offset_top, left_panel_width,
                          screen_hieght - offset_top);

    } else if (view_type == EnumViewType::eNodeEditor) {
        SetWindowLocation(0.0f, offset_top, screen_width,
                          screen_hieght - offset_top);
    }

    ImGui::Begin("main ui", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoResize);
    gui_size = ImGui::GetWindowSize();

    ImGui::BeginChild("left panel", ImVec2(left_panel_width, 0.0f));
    LeftPanel();
    ImGui::EndChild();

    if (view_type == EnumViewType::eNodeEditor) {
        ImGui::SameLine(left_panel_width, 0.0f);
        NodeEditor();
    }

    ImGui::End();
}

void SettingUi::LeftPanel() {
    ImGui::PushItemWidth(left_panel_width / 2.0f);

    ImGui::SetNextItemOpen(false, ImGuiCond_Once);
    if (ImGui::TreeNode("Assets")) {

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
        ImGui::TreePop();
    }

    for (auto node : info_items) {
        node->Show();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Hierarchy")) {
        Hierarchy();
        ImGui::TreePop();
    }

    ImGui::PopItemWidth();
}

void SettingUi::NodeEditor() {
    ed::SetCurrentEditor(context_);
    ed::Begin("My Editor");
    auto &io = ImGui::GetIO();

    for (auto node : node_items) {
        node->Show();
    }

    ed::End();
    ed::SetCurrentEditor(nullptr);
}

void SettingUi::Hierarchy() {

    ImGui::BeginTable("MyTable", 2,
                      ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable,
                      ImVec2(300.0f, 0.0f), 300.0f);
    // headers
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("ID");
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("Name");

    // for (auto model : node_items) {

    //    ImGui::TableNextRow();

    //    ImGui::TableSetColumnIndex(0);
    //    if (ImGui::Selectable(model->GetName().c_str(),
    //            model->GetEntityId() == selected_object.GetEntityId(),
    //                          ImGuiSelectableFlags_None,
    //                          ImVec2(left_panel_width, 0.0f))) {
    //        selected_object = *model;
    //    }

    //    ImGui::TableSetColumnIndex(1);
    //    ImGui::Text("Node");

    //    if (selected_object.GetEntityId() == model->GetEntityId()) {
    //        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
    //                               ImGui::GetColorU32(ImGuiCol_Header));
    //    }
    //}

    ImGui::EndTable();
}
} // namespace foundation
