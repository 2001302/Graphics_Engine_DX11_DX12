#include "tree_node.h"

namespace graphics {
namespace node {

std::shared_ptr<LightNodeInvoker> light_node =
    std::make_shared<LightNodeInvoker>();
std::shared_ptr<CameraNodeInvoker> camera_node =
    std::make_shared<CameraNodeInvoker>();
std::shared_ptr<SkyboxNodeInvoker> skybox_node =
    std::make_shared<SkyboxNodeInvoker>();
std::shared_ptr<MirrorObjectNodeInvoker> mirror_object_node =
    std::make_shared<MirrorObjectNodeInvoker>();
std::shared_ptr<SharedResourceNodeInvoker> shared_resource_node =
    std::make_shared<SharedResourceNodeInvoker>();
std::shared_ptr<MeshObjectNodeInvoker> game_objects_node =
    std::make_shared<MeshObjectNodeInvoker>();
std::shared_ptr<ShadowEffectNodeInvoker> shadow_effect_node =
    std::make_shared<ShadowEffectNodeInvoker>();
std::shared_ptr<ResolveBufferNode> resolve_buffer =
    std::make_shared<ResolveBufferNode>();
std::shared_ptr<GuiNodeInvoker> imgui_node = std::make_shared<GuiNodeInvoker>();
std::shared_ptr<PostProcessingNode> post_processing =
    std::make_shared<PostProcessingNode>();
std::shared_ptr<PresentNode> present = std::make_shared<PresentNode>();

} // namespace node
} // namespace graphics
