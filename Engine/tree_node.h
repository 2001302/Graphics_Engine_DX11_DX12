#ifndef _TREENODE
#define _TREENODE

#include "post_process_node.h"
#include "renderer_draw_node.h"
#include "renderer_initialize_node.h"
#include "renderer_update_node.h"
#include "gui_node.h"
#include "camera_node.h"
#include "light_node.h"
#include "skybox_node.h"
#include "mirror_effect_node.h"

// clang-format off
namespace engine {
class TreeNode {
  public:
    TreeNode() {
        light_node = std::make_shared<LightNodeInvoker>();
        camera_node = std::make_shared<CameraNodeInvoker>();
        skybox_node = std::make_shared<SkyboxNodeInvoker>();
        mirror_effect_node = std::make_shared<MirrorEffectNodeInvoker>();
        create_global_constant_buffer = std::make_shared<CreateGlobalConstantBufferNode>();
        initialize_basic_models = std::make_shared<InitializeBasicModelsNode>();
        update_global_constant_buffers = std::make_shared<UpdateGlobalConstantBuffersNode>();
        update_basic_objects = std::make_shared<UpdateBasicObjectsNode>();
        update_light_spheres = std::make_shared<UpdateLightSpheresNode>();
        set_sampler_states = std::make_shared<SetSamplerStatesNode>();
        draw_only_depth = std::make_shared<DrawOnlyDepthNode>();
        set_shadow_viewport = std::make_shared<SetShadowViewportNode>();
        draw_shadow_map = std::make_shared<DrawShadowMapNode>();
        set_main_render_target = std::make_shared<SetMainRenderTargetNode>();
        draw_objects = std::make_shared<DrawObjectsNode>();
        draw_light_spheres = std::make_shared<DrawLightSpheresNode>();
        resolve_buffer = std::make_shared<ResolveBufferNode>();
        imgui = std::make_shared<GuiNodeInvoker>();
        post_processing = std::make_shared<PostProcessingNode>();
        present = std::make_shared<PresentNode>();
    }

    std::shared_ptr<LightNodeInvoker> LightNode(){return light_node;}
    std::shared_ptr<CameraNodeInvoker> CameraNode(){return camera_node;}
    std::shared_ptr<SkyboxNodeInvoker> SkyboxNode(){return skybox_node;}
    std::shared_ptr<MirrorEffectNodeInvoker> MirrorEffectNode(){return mirror_effect_node;}
    std::shared_ptr<CreateGlobalConstantBufferNode> CreateGlobalConstantBuffer(){return create_global_constant_buffer;}
    std::shared_ptr<InitializeBasicModelsNode> InitializeBasicModels(){return initialize_basic_models;}
    std::shared_ptr<UpdateGlobalConstantBuffersNode> UpdateGlobalConstantBuffers(){return update_global_constant_buffers;}
    std::shared_ptr<UpdateBasicObjectsNode> UpdateBasicObjects(){return update_basic_objects;}
    std::shared_ptr<UpdateLightSpheresNode> UpdateLightSpheres(){return update_light_spheres;}
    std::shared_ptr<SetSamplerStatesNode> SetSamplerStates(){return set_sampler_states;}
    std::shared_ptr<DrawOnlyDepthNode> DrawOnlyDepth(){return draw_only_depth;}
    std::shared_ptr<SetShadowViewportNode> SetShadowViewport(){return set_shadow_viewport;}
    std::shared_ptr<DrawShadowMapNode> DrawShadowMap(){return draw_shadow_map;}
    std::shared_ptr<SetMainRenderTargetNode> SetMainRenderTarget(){return set_main_render_target;}
    std::shared_ptr<DrawObjectsNode> DrawObjects(){return draw_objects;}
    std::shared_ptr<DrawLightSpheresNode> DrawLightSpheres(){return draw_light_spheres;}
    std::shared_ptr<ResolveBufferNode> ResolveBuffer(){return resolve_buffer;}
    std::shared_ptr<GuiNodeInvoker> ImGuiNode(){return imgui;}
    std::shared_ptr<PostProcessingNode> PostProcessing() {return post_processing;}
    std::shared_ptr<PresentNode> Present(){return present;}


  private:
    std::shared_ptr<LightNodeInvoker> light_node;
    std::shared_ptr<CameraNodeInvoker> camera_node;
    std::shared_ptr<SkyboxNodeInvoker> skybox_node;
    std::shared_ptr<MirrorEffectNodeInvoker> mirror_effect_node;
    std::shared_ptr<CreateGlobalConstantBufferNode> create_global_constant_buffer;
    std::shared_ptr<InitializeBasicModelsNode> initialize_basic_models;
    std::shared_ptr<UpdateGlobalConstantBuffersNode> update_global_constant_buffers;
    std::shared_ptr<UpdateBasicObjectsNode> update_basic_objects;
    std::shared_ptr<UpdateLightSpheresNode> update_light_spheres;
    std::shared_ptr<SetSamplerStatesNode> set_sampler_states;
    std::shared_ptr<DrawOnlyDepthNode> draw_only_depth;
    std::shared_ptr<SetShadowViewportNode> set_shadow_viewport;
    std::shared_ptr<DrawShadowMapNode> draw_shadow_map;
    std::shared_ptr<SetMainRenderTargetNode> set_main_render_target;
    std::shared_ptr<DrawObjectsNode> draw_objects;
    std::shared_ptr<DrawLightSpheresNode> draw_light_spheres;
    std::shared_ptr<ResolveBufferNode> resolve_buffer;
    std::shared_ptr<GuiNodeInvoker> imgui;
    std::shared_ptr<PostProcessingNode> post_processing;
    std::shared_ptr<PresentNode> present;
};
} // namespace engine

// clang-format on
#endif