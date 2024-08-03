#include "setting_ui.h"

using namespace common;

void SettingUi::OnStart() {
    ed::Config config;
    config.SettingsFile = "widgets.json";
    context_ = ed::CreateEditor(&config);
}

void SettingUi::OnFrame(INode *node) {
     FrameRate();
     StyleSetting();
     MenuBar();
     NodeEditor();
     TabBar(node);
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

void SettingUi::TabBar(INode *node) { 

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

            //for (auto &model : node_map) {

            //    ImGui::TableNextRow();

            //    ImGui::TableSetColumnIndex(0);
            //    if (ImGui::Selectable(std::to_string(model.first).c_str(),
            //                          model.first == selected_object_id_,
            //                          ImGuiSelectableFlags_SpanAllColumns)) {
            //        selected_object_id_ = model.first;
            //    }

            //    ImGui::TableSetColumnIndex(1);
            //    ImGui::Text("Node");

            //    if (selected_object_id_ == model.first) {
            //        ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0,
            //                               ImGui::GetColorU32(ImGuiCol_Header));
            //    }
            //}

            ImGui::EndTable();

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

void SettingUi::PushNode(INode *node) {
    node->uniqueId = unique_id;
    node->position = ImVec2((float)unique_pos_x, 0);

    nodes.push_back(node);
    unique_id = unique_id + 10;
    unique_pos_x = unique_pos_x + 500;
};

void SettingUi::ClearNode() {
    unique_id = 1;
    unique_pos_x = 0;
    nodes.clear();
};
