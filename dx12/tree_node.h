#ifndef _TREENODE
#define _TREENODE

#include "behaviortreenode/camera_node.h"
#include "behaviortreenode/mesh_object_node.h"
#include "behaviortreenode/gpu_node.h"
#include "behaviortreenode/gui_node.h"
#include "behaviortreenode/light_node.h"
#include "behaviortreenode/mirror_object_node.h"
#include "behaviortreenode/skybox_node.h"
#include "behaviortreenode/tone_mapping_node.h"
#include "behaviortreenode/shadow_effect_node.h"

namespace graphics {
namespace node {

extern std::shared_ptr<ClearBufferNode> clear_buffer;
extern std::shared_ptr<CameraNodeInvoker> camera_node;
extern std::shared_ptr<GlobalConstantNode> global_constant_node;
extern std::shared_ptr<MeshObjectNodeInvoker> mesh_object_node;
extern std::shared_ptr<GuiNodeInvoker> gui_node;
extern std::shared_ptr<PresentNode> present;
extern std::shared_ptr<LightNodeInvoker> light_node;
extern std::shared_ptr<ToneMappingNodeInvoker> tone_mapping;
extern std::shared_ptr<SkyBoxNodeInvoker> skybox_node;
extern std::shared_ptr<MirrorObjectNodeInvoker> mirror_object_node;
extern std::shared_ptr<ShadowEffectNode> shadow_effect_node;

} // namespace node

} // namespace graphics
#endif