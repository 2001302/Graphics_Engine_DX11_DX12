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

extern std::shared_ptr<LightNodeInvoker> light_node;
extern std::shared_ptr<CameraNodeInvoker> camera_node;
extern std::shared_ptr<SkyboxNodeInvoker> skybox_node;
extern std::shared_ptr<MirrorObjectNodeInvoker> mirror_object_node;
extern std::shared_ptr<SharedResourceNodeInvoker> shared_resource_node;
extern std::shared_ptr<GameObjectNodeInvoker> game_objects_node;
extern std::shared_ptr<ShadowEffectNodeInvoker> shadow_effect_node;
extern std::shared_ptr<ResolveBufferNode> resolve_buffer;
extern std::shared_ptr<GuiNodeInvoker> imgui_node;
extern std::shared_ptr<PostProcessingNode> post_processing;
extern std::shared_ptr<PresentNode> present;
extern std::shared_ptr<PlayerNodeInvoker> player_node;

} // namespace node
} // namespace graphics

#endif