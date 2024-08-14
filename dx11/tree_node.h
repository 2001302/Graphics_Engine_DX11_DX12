#ifndef _TREENODE
#define _TREENODE

#include "post_process_node.h"
#include "game_objects_node.h"
#include "gui_node.h"
#include "camera_node.h"
#include "light_node.h"
#include "skybox_node.h"
#include "mirror_effect_node.h"
#include "shadow_effect_node.h"
#include "shared_resource_node.h"
#include "player_node.h"
#include "gpu_node.h"
#include "shared_resource_node.h"

// clang-format off
namespace core {
class TreeNode {
  public:
    TreeNode() {
        light_node = std::make_shared<LightNodeInvoker>();
        camera_node = std::make_shared<CameraNodeInvoker>();
        skybox_node = std::make_shared<SkyboxNodeInvoker>();
        mirror_effect_node = std::make_shared<MirrorEffectNodeInvoker>();
        shared_resource_node = std::make_shared<SharedResourceNodeInvoker>();
        game_objects_node = std::make_shared<GameObjectNodeInvoker>();
        shadow_effect_node = std::make_shared<ShadowEffectNodeInvoker>();
        resolve_buffer = std::make_shared<ResolveBufferNode>();
        imgui_node = std::make_shared<GuiNodeInvoker>();
        post_processing = std::make_shared<PostProcessingNode>();
        present = std::make_shared<PresentNode>();
        player_node =  std::make_shared<PlayerNodeInvoker>();
    }

  protected:
    std::shared_ptr<LightNodeInvoker> light_node;
    std::shared_ptr<CameraNodeInvoker> camera_node;
    std::shared_ptr<SkyboxNodeInvoker> skybox_node;
    std::shared_ptr<MirrorEffectNodeInvoker> mirror_effect_node;
    std::shared_ptr<SharedResourceNodeInvoker> shared_resource_node;
    std::shared_ptr<GameObjectNodeInvoker> game_objects_node;
    std::shared_ptr<ShadowEffectNodeInvoker> shadow_effect_node;
    std::shared_ptr<ResolveBufferNode> resolve_buffer;
    std::shared_ptr<GuiNodeInvoker> imgui_node;
    std::shared_ptr<PostProcessingNode> post_processing;
    std::shared_ptr<PresentNode> present;
    std::shared_ptr<PlayerNodeInvoker> player_node;
};
} // namespace engine

// clang-format on
#endif