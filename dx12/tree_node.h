#ifndef _TREENODE
#define _TREENODE

#include "camera_node.h"
#include "mesh_object_node.h"
#include "gpu_node.h"
#include "gui_node.h"
#include "light_node.h"
#include "mirror_object_node.h"
#include "skybox_node.h"
#include "tone_mapping_node.h"
#include "shadow_effect_node.h"

namespace graphics {
namespace node {

extern std::shared_ptr<ClearBufferNode> clear_buffer;
extern std::shared_ptr<CameraNodeInvoker> camera_node;
extern std::shared_ptr<GlobalConstantNode> global_constant_node;
extern std::shared_ptr<MeshObjectNodeInvoker> game_object_node;
extern std::shared_ptr<GuiNodeInvoker> gui_node;
extern std::shared_ptr<PresentNode> present;
extern std::shared_ptr<LightNodeInvoker> light_node;
extern std::shared_ptr<ToneMappingNodeInvoker> tone_mapping;
extern std::shared_ptr<SkyBoxNodeInvoker> skybox_node;
extern std::shared_ptr<MirrorEffectNodeInvoker> mirror_effect_node;
extern std::shared_ptr<ShadowEffectNode> shadow_effect_node;

} // namespace node

} // namespace graphics
#endif