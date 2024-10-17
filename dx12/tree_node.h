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

auto clear_buffer = std::make_shared<ClearBufferNode>();
auto camera_node = std::make_shared<CameraNodeInvoker>();
auto global_constant_node = std::make_shared<GlobalConstantNode>();
auto game_object_node = std::make_shared<MeshObjectNodeInvoker>();
auto gui_node = std::make_shared<GuiNodeInvoker>();
auto present = std::make_shared<PresentNode>();
auto light_node = std::make_shared<LightNodeInvoker>();
auto resolve_buffer = std::make_shared<ResolveBuffer>();
auto tone_mapping = std::make_shared<ToneMappingNodeInvoker>();
auto skybox_node = std::make_shared<SkyBoxNodeInvoker>();
auto mirror_effect_node = std::make_shared<MirrorEffectNodeInvoker>();
auto shadow_effect_node = std::make_shared<ShadowEffectNode>();

} // namespace node

} // namespace graphics
#endif