#ifndef _TREENODE
#define _TREENODE

#include "post_process_node.h"
#include "renderer_draw_node.h"
#include "renderer_initialize_node.h"
#include "renderer_update_node.h"

// clang-format off
namespace engine {
class TreeNode {
  public:
    TreeNode() {
        initialize_post_processing =
            std::make_shared<InitializePostProcessingNode>();
        draw_post_processing = std::make_shared<DrawPostProcessingNode>();
        initialize_input = std::make_shared<InitializeInputNode>();
        initialize_imgui = std::make_shared<InitializeImguiNode>();
        initialize_light = std::make_shared<InitializeLightNode>();
        initialize_camera = std::make_shared<InitializeCameraNode>();
        initialize_skybox = std::make_shared<InitializeSkyboxNode>();
        initialize_mirror_ground = std::make_shared<InitializeMirrorGroundNode>();
        create_global_constant_buffer = std::make_shared<CreateGlobalConstantBufferNode>();
        initialize_post_effect = std::make_shared<InitializePostEffectNode>();
        initialize_basic_models = std::make_shared<InitializeBasicModelsNode>();
        read_input = std::make_shared<ReadInputNode>();
        update_camera = std::make_shared<UpdateCameraNode>();
        update_lights = std::make_shared<UpdateLightsNode>();
        update_global_constant_buffers = std::make_shared<UpdateGlobalConstantBuffersNode>();
        update_mirror = std::make_shared<UpdateMirrorNode>();
        update_basic_objects = std::make_shared<UpdateBasicObjectsNode>();
        update_light_spheres = std::make_shared<UpdateLightSpheresNode>();
        set_sampler_states = std::make_shared<SetSamplerStatesNode>();
        draw_only_depth = std::make_shared<DrawOnlyDepthNode>();
        set_shadow_viewport = std::make_shared<SetShadowViewportNode>();
        draw_shadow_map = std::make_shared<DrawShadowMapNode>();
        set_main_render_target = std::make_shared<SetMainRenderTargetNode>();
        draw_objects = std::make_shared<DrawObjectsNode>();
        draw_light_spheres = std::make_shared<DrawLightSpheresNode>();
        draw_related_with_camera = std::make_shared<DrawRelatedWithCameraNode>();
        draw_skybox = std::make_shared<DrawSkyboxNode>();
        draw_mirror_surface = std::make_shared<DrawMirrorSurfaceNode>();
        resolve_buffer = std::make_shared<ResolveBufferNode>();
        draw_setting_ui = std::make_shared<DrawSettingUiNode>();
        present = std::make_shared<PresentNode>();
    }

    std::shared_ptr<InitializePostProcessingNode> InitializePostProcessing() {return initialize_post_processing;}
    std::shared_ptr<DrawPostProcessingNode> DrawPostProcessing() {return draw_post_processing;}
    std::shared_ptr<InitializeInputNode> InitializeInput(){return initialize_input;}
    std::shared_ptr<InitializeImguiNode> InitializeImgui(){return initialize_imgui;}
    std::shared_ptr<InitializeLightNode> InitializeLight(){return initialize_light;}
    std::shared_ptr<InitializeCameraNode> InitializeCamera(){return initialize_camera;}
    std::shared_ptr<InitializeSkyboxNode> InitializeSkybox(){return initialize_skybox;}
    std::shared_ptr<InitializeMirrorGroundNode> InitializeMirrorGround(){return initialize_mirror_ground;}
    std::shared_ptr<CreateGlobalConstantBufferNode> CreateGlobalConstantBuffer(){return create_global_constant_buffer;}
    std::shared_ptr<InitializePostEffectNode> InitializePostEffect(){return initialize_post_effect;}
    std::shared_ptr<InitializeBasicModelsNode> InitializeBasicModels(){return initialize_basic_models;}
    std::shared_ptr<ReadInputNode> ReadInput(){return read_input;}
    std::shared_ptr<UpdateCameraNode> UpdateCamera(){return update_camera;}
    std::shared_ptr<UpdateLightsNode> UpdateLights(){return update_lights;}
    std::shared_ptr<UpdateGlobalConstantBuffersNode> UpdateGlobalConstantBuffers(){return update_global_constant_buffers;}
    std::shared_ptr<UpdateMirrorNode> UpdateMirror(){return update_mirror;}
    std::shared_ptr<UpdateBasicObjectsNode> UpdateBasicObjects(){return update_basic_objects;}
    std::shared_ptr<UpdateLightSpheresNode> UpdateLightSpheres(){return update_light_spheres;}
    std::shared_ptr<SetSamplerStatesNode> SetSamplerStates(){return set_sampler_states;}
    std::shared_ptr<DrawOnlyDepthNode> DrawOnlyDepth(){return draw_only_depth;}
    std::shared_ptr<SetShadowViewportNode> SetShadowViewport(){return set_shadow_viewport;}
    std::shared_ptr<DrawShadowMapNode> DrawShadowMap(){return draw_shadow_map;}
    std::shared_ptr<SetMainRenderTargetNode> SetMainRenderTarget(){return set_main_render_target;}
    std::shared_ptr<DrawObjectsNode> DrawObjects(){return draw_objects;}
    std::shared_ptr<DrawLightSpheresNode> DrawLightSpheres(){return draw_light_spheres;}
    std::shared_ptr<DrawRelatedWithCameraNode> DrawRelatedWithCamera(){return draw_related_with_camera;}
    std::shared_ptr<DrawSkyboxNode> DrawSkybox(){return draw_skybox;}
    std::shared_ptr<DrawMirrorSurfaceNode> DrawMirrorSurface(){return draw_mirror_surface;}
    std::shared_ptr<ResolveBufferNode> ResolveBuffer(){return resolve_buffer;}
    std::shared_ptr<DrawSettingUiNode> DrawSettingUi(){return draw_setting_ui;}
    std::shared_ptr<PresentNode> Present(){return present;}


  private:
    std::shared_ptr<InitializeInputNode> initialize_input;
    std::shared_ptr<InitializeImguiNode> initialize_imgui;
    std::shared_ptr<InitializeLightNode> initialize_light;
    std::shared_ptr<InitializeCameraNode> initialize_camera;
    std::shared_ptr<InitializeSkyboxNode> initialize_skybox;
    std::shared_ptr<InitializeMirrorGroundNode> initialize_mirror_ground;
    std::shared_ptr<CreateGlobalConstantBufferNode> create_global_constant_buffer;
    std::shared_ptr<InitializePostEffectNode> initialize_post_effect;
    std::shared_ptr<InitializeBasicModelsNode> initialize_basic_models;
    std::shared_ptr<ReadInputNode> read_input;
    std::shared_ptr<UpdateCameraNode> update_camera;
    std::shared_ptr<UpdateLightsNode> update_lights;
    std::shared_ptr<UpdateGlobalConstantBuffersNode> update_global_constant_buffers;
    std::shared_ptr<UpdateMirrorNode> update_mirror;
    std::shared_ptr<UpdateBasicObjectsNode> update_basic_objects;
    std::shared_ptr<UpdateLightSpheresNode> update_light_spheres;
    std::shared_ptr<SetSamplerStatesNode> set_sampler_states;
    std::shared_ptr<DrawOnlyDepthNode> draw_only_depth;
    std::shared_ptr<SetShadowViewportNode> set_shadow_viewport;
    std::shared_ptr<DrawShadowMapNode> draw_shadow_map;
    std::shared_ptr<SetMainRenderTargetNode> set_main_render_target;
    std::shared_ptr<DrawObjectsNode> draw_objects;
    std::shared_ptr<DrawLightSpheresNode> draw_light_spheres;
    std::shared_ptr<DrawRelatedWithCameraNode> draw_related_with_camera;
    std::shared_ptr<DrawSkyboxNode> draw_skybox;
    std::shared_ptr<DrawMirrorSurfaceNode> draw_mirror_surface;
    std::shared_ptr<ResolveBufferNode> resolve_buffer;
    std::shared_ptr<DrawSettingUiNode> draw_setting_ui;
    std::shared_ptr<PresentNode> present;
    std::shared_ptr<InitializePostProcessingNode> initialize_post_processing;
    std::shared_ptr<DrawPostProcessingNode> draw_post_processing;
};
} // namespace engine

// clang-format on
#endif