#ifndef _TREENODE
#define _TREENODE

#include "camera_node.h"
#include "game_objects_node.h"
#include "gpu_node.h"
#include "gui_node.h"
#include "light_node.h"
#include "mirror_effect_node.h"
#include "player_node.h"
#include "post_process_node.h"
#include "shadow_effect_node.h"
#include "shared_resource_node.h"
#include "skybox_node.h"

namespace graphics {
namespace node {

auto light_node = std::make_shared<LightNodeInvoker>();
auto camera_node = std::make_shared<CameraNodeInvoker>();
auto skybox_node = std::make_shared<SkyboxNodeInvoker>();
auto mirror_object_node = std::make_shared<MirrorObjectNodeInvoker>();
auto shared_resource_node = std::make_shared<SharedResourceNodeInvoker>();
auto game_objects_node = std::make_shared<GameObjectNodeInvoker>();
auto shadow_effect_node = std::make_shared<ShadowEffectNodeInvoker>();
auto resolve_buffer = std::make_shared<ResolveBufferNode>();
auto imgui_node = std::make_shared<GuiNodeInvoker>();
auto post_processing = std::make_shared<PostProcessingNode>();
auto present = std::make_shared<PresentNode>();
auto player_node = std::make_shared<PlayerNodeInvoker>();

} // namespace node
} // namespace graphics

#endif