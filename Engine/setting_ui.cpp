#include "setting_ui.h"

using namespace common;
void SetWindowLocation(float x, float y, float width, float height) {

    ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
}

void FrameRate() {
    auto &io = ImGui::GetIO();
    ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate,
                io.Framerate ? 1000.0f / io.Framerate : 0.0f);
}

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

    ImGui::Begin("TOP BAR", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoScrollbar);
    FrameRate();

    ImGui::SameLine(left_panel_width, 0.0f);
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

    for (auto node : panel_items) {
        node->ShowPanel();
    }
    Hierarchy();
}

void SettingUi::NodeEditor() {
    ed::SetCurrentEditor(context_);
    ed::Begin("My Editor");
    auto &io = ImGui::GetIO();

    for (auto node : node_items) {
        node->ShowNode();
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

    for (auto &model : node_items) {

        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        if (ImGui::Selectable(model->GetName().c_str(),
                              model->GetEntityId() == selected_object_id_,
                              ImGuiSelectableFlags_SpanAllColumns)) {
            selected_object_id_ = model->GetEntityId();
        }

        ImGui::TableSetColumnIndex(1);
        ImGui::Text("Node");

        if (selected_object_id_ == model->GetEntityId()) {
            ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
                                   ImGui::GetColorU32(ImGuiCol_Header));
        }
    }

    ImGui::EndTable();
}

// int SettingUi::SelectedId() {
//     if (selected_object_id_ == -99999)
//         return 0;
//     else {
//         return selected_object_id_;
//     }
// }

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

void SettingUi::PushPanelItem(INode *node) { panel_items.push_back(node); };

void SettingUi::ClearPanelItem() { panel_items.clear(); };