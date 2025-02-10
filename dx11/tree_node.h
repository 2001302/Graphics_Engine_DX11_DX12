#ifndef _TREENODE
#define _TREENODE

#include "behaviortreenode/camera_node.h"
#include "behaviortreenode/mesh_objects_node.h"
#include "behaviortreenode/gpu_node.h"
#include "behaviortreenode/gui_node.h"
#include "behaviortreenode/light_node.h"
#include "behaviortreenode/mirror_effect_node.h"
#include "behaviortreenode/post_process_node.h"
#include "behaviortreenode/shadow_effect_node.h"
#include "behaviortreenode/shared_resource_node.h"
#include "behaviortreenode/skybox_node.h"

namespace graphics {
namespace node {

extern std::shared_ptr<LightNodeInvoker> light_node;
extern std::shared_ptr<CameraNodeInvoker> camera_node;
extern std::shared_ptr<SkyboxNodeInvoker> skybox_node;
extern std::shared_ptr<MirrorObjectNodeInvoker> mirror_object_node;
extern std::shared_ptr<SharedResourceNodeInvoker> shared_resource_node;
extern std::shared_ptr<MeshObjectNodeInvoker> game_objects_node;
extern std::shared_ptr<ShadowEffectNodeInvoker> shadow_effect_node;
extern std::shared_ptr<ResolveBufferNode> resolve_buffer;
extern std::shared_ptr<GuiNodeInvoker> imgui_node;
extern std::shared_ptr<PostProcessingNode> post_processing;
extern std::shared_ptr<PresentNode> present;

} // namespace node
} // namespace graphics

#endif