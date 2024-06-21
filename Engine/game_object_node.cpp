#include "pipeline_manager.h"
#include "game_object_node.h"

using namespace Engine;

EnumBehaviorTreeStatus GameObjectDetailNode::OnInvoke() {

    return EnumBehaviorTreeStatus::eSuccess;
}
EnumBehaviorTreeStatus GameObjectDetailNode::OnShow() {

    static bool firstframe = true; // Used to position the nodes on startup
    auto &io = ImGui::GetIO();

    int uniqueId = 1;

    //Pin
    auto basic_id = uniqueId++;
    ed::BeginNode(basic_id);
    ImGui::Text("Detail");
    ed::BeginPin(uniqueId++, ed::PinKind::Input);
    ImGui::Text("-> In");
    ed::EndPin();
    ImGui::SameLine();
    ImGui::Dummy(
        ImVec2(250, 0)); // Hacky magic number to space out the output pin.
    ImGui::SameLine();
    ed::BeginPin(uniqueId++, ed::PinKind::Output);
    ImGui::Text("Out ->");
    ed::EndPin();

    // Normal Button
    static int clicked = 0;
    if (ImGui::Button("Button"))
        clicked++;
    if (clicked & 1) {
        ImGui::SameLine();
        ImGui::Text("Thanks for clicking me!");
    }

    ImGui::Checkbox("Use Texture", &use_texture);

    ImGui::Text("Transform");
    ImGui::SliderFloat3("Translation", &translation.x, -2.0f, 2.0f);
    ImGui::SliderFloat("Rotation", &rotation.y, -3.14f, 3.14f);
    ImGui::SliderFloat3("Scaling", &scaling.x, 0.1f, 4.0f);

    ImGui::Text("Material");
    ImGui::SliderFloat("Shininess", &shininess, 0.01f, 1.0f);
    ImGui::SliderFloat("Diffuse", &diffuse, 0.0f, 1.0f);
    ImGui::SliderFloat("Specular", &specular, 0.0f, 1.0f);

    // Checkbox
    static bool check = true;
    ImGui::Checkbox("checkbox", &check);

    // Radio buttons
    static int e = 0;
    ImGui::RadioButton("radio a", &e, 0);
    ImGui::SameLine();
    ImGui::RadioButton("radio b", &e, 1);
    ImGui::SameLine();
    ImGui::RadioButton("radio c", &e, 2);

    // Color buttons, demonstrate using PushID() to add unique identifier in
    // the ID stack, and changing style.
    for (int i = 0; i < 7; i++) {
        if (i > 0)
            ImGui::SameLine();
        ImGui::PushID(i);
        ImGui::PushStyleColor(ImGuiCol_Button,
                              (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                              (ImVec4)ImColor::HSV(i / 7.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                              (ImVec4)ImColor::HSV(i / 7.0f, 0.8f, 0.8f));
        ImGui::Button("Click");
        ImGui::PopStyleColor(3);
        ImGui::PopID();
    }

    // Use AlignTextToFramePadding() to align text baseline to the baseline
    // of framed elements (otherwise a Text+SameLine+Button sequence will
    // have the text a little too high by default)
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Hold to repeat:");
    ImGui::SameLine();

    // Arrow buttons with Repeater
    static int counter = 0;
    float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
    ImGui::PushButtonRepeat(true);
    if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
        counter--;
    }
    ImGui::SameLine(0.0f, spacing);
    if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
        counter++;
    }
    ImGui::PopButtonRepeat();
    ImGui::SameLine();
    ImGui::Text("%d", counter);

    // The input widgets also require you to manually disable the editor
    // shortcuts so the view doesn't fly around. (note that this is a
    // per-frame setting, so it disables it for all text boxes.  I left it
    // here so you could find it!)
    ed::EnableShortcuts(!io.WantTextInput);
    // The input widgets require some guidance on their widths, or else
    // they're very large. (note matching pop at the end).
    ImGui::PushItemWidth(200);
    static char str1[128] = "";
    ImGui::InputTextWithHint("input text (w/ hint)", "enter text here", str1,
                             IM_ARRAYSIZE(str1));

    static float f0 = 0.001f;
    ImGui::InputFloat("input float", &f0, 0.01f, 1.0f, "%.3f");

    static float f1 = 1.00f, f2 = 0.0067f;
    ImGui::DragFloat("drag float", &f1, 0.005f);
    ImGui::DragFloat("drag small float", &f2, 0.0001f, 0.0f, 0.0f, "%.06f ns");
    ImGui::PopItemWidth();

    ed::EndNode();
    if (firstframe) {
        ed::SetNodePosition(basic_id, ImVec2(20, 20));
    }

    // ==================================================================================================
    // Link Drawing Section

    for (auto &linkInfo : links_)
        ed::Link(linkInfo.Id, linkInfo.InputId, linkInfo.OutputId);

    // ==================================================================================================
    // Interaction Handling Section
    // This was coppied from BasicInteration.cpp. See that file for
    // commented code.

    // Handle creation action
    // ---------------------------------------------------------------------------
    if (ed::BeginCreate()) {
        ed::PinId inputPinId, outputPinId;
        if (ed::QueryNewLink(&inputPinId, &outputPinId)) {
            if (inputPinId && outputPinId) {
                if (ed::AcceptNewItem()) {
                    links_.push_back(
                        {ed::LinkId(next_link_Id++), inputPinId, outputPinId});
                    ed::Link(links_.back().Id, links_.back().InputId,
                             links_.back().OutputId);
                }
            }
        }
    }
    ed::EndCreate();

    // Handle deletion action
    // ---------------------------------------------------------------------------
    if (ed::BeginDelete()) {
        ed::LinkId deletedLinkId;
        while (ed::QueryDeletedLink(&deletedLinkId)) {
            if (ed::AcceptDeletedItem()) {
                for (auto &link : links_) {
                    if (link.Id == deletedLinkId) {
                        links_.erase(&link);
                        break;
                    }
                }
            }
        }
    }
    ed::EndDelete();

    firstframe = false;

    return EnumBehaviorTreeStatus::eSuccess;
}
