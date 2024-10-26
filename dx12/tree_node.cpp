#include "tree_node.h"

namespace graphics {
namespace node {

std::shared_ptr<ClearBufferNode> clear_buffer =
    std::make_shared<ClearBufferNode>();

std::shared_ptr<CameraNodeInvoker> camera_node =
    std::make_shared<CameraNodeInvoker>();

std::shared_ptr<GlobalConstantNode> global_constant_node =
    std::make_shared<GlobalConstantNode>();

std::shared_ptr<MeshObjectNodeInvoker> mesh_object_node =
    std::make_shared<MeshObjectNodeInvoker>();

std::shared_ptr<GuiNodeInvoker> gui_node = std::make_shared<GuiNodeInvoker>();

std::shared_ptr<PresentNode> present = std::make_shared<PresentNode>();

std::shared_ptr<LightNodeInvoker> light_node =
    std::make_shared<LightNodeInvoker>();

std::shared_ptr<ToneMappingNodeInvoker> tone_mapping =
    std::make_shared<ToneMappingNodeInvoker>();

std::shared_ptr<SkyBoxNodeInvoker> skybox_node =
    std::make_shared<SkyBoxNodeInvoker>();

std::shared_ptr<MirrorObjectNodeInvoker> mirror_object_node =
    std::make_shared<MirrorObjectNodeInvoker>();

std::shared_ptr<ShadowEffectNode> shadow_effect_node =
    std::make_shared<ShadowEffectNode>();

} // namespace node

} // namespace graphics