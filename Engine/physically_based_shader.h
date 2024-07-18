#ifndef _PHYSICALLYBASEDSHADER
#define _PHYSICALLYBASEDSHADER

#include "behavior_tree_builder.h"
#include "node_ui.h"
#include "shader.h"

using namespace DirectX;

namespace dx11 {
//
//struct PhsicallyBasedShader : public IShader {
//    struct PhysicallyMatrial {
//        Vector3 albedo = Vector3(1.0f); // 12
//        float roughness = 0.0f;
//        float metallic = 0.0f;
//        Vector3 dummy;
//    };
//
//    struct PhsicallyBasedVertexConstantBuffer {
//        Matrix modelWorld;
//        Matrix invTranspose;
//        Matrix view;
//        Matrix projection;
//        int useHeightMap = 0;
//        float heightScale = 0.0f;
//        Vector2 dummy;
//    };
//
//    struct PhsicallyPixelConstantBuffer {
//        Vector3 eyeWorld;         // 12
//        float mipmapLevel = 0.0f; // 4
//
//        PhysicallyMatrial material; // 48
//        Light lights[MAX_LIGHTS];   // 48 * MAX_LIGHTS
//
//        int useAlbedoMap = 1;
//        int useNormalMap = 1;
//        int useAOMap = 0;         // Ambient Occlusion
//        int invertNormalMapY = 0; // 16
//        int useMetallicMap = 0;
//        int useRoughnessMap = 0;
//        int useEmissiveMap = 0;
//        float expose = 1.0f; // 16
//        float gamma = 1.0f;
//        Vector3 dummy; // 16
//    };
//
//    struct PhsicallyBasedConstantBufferData : public IConstantBufferData,
//                                              common::INodeUi {
//      public:
//        ComPtr<ID3D11Buffer> vertex_constant_buffer;
//        ComPtr<ID3D11Buffer> pixel_constant_buffer;
//        PhsicallyBasedVertexConstantBuffer
//            vertex_constant;
//        PhsicallyPixelConstantBuffer pixel_constant;
//
//      private:
//        void InitializeThis() override;
//        void OnShow() override;
//    };
//
//    ComPtr<ID3D11SamplerState> clampSamplerState;
//};
//
//class InitializePhysicallyBasedShader : public BehaviorActionNode {
//    EnumBehaviorTreeStatus OnInvoke() override;
//};
//
//class CheckPhysicallyBasedShader : public BehaviorActionNode {
//    EnumBehaviorTreeStatus OnInvoke() override;
//};
//
//class UpdateGameObjectsUsingPhysicallyBasedShader : public BehaviorActionNode {
//    EnumBehaviorTreeStatus OnInvoke() override;
//};
//
//class RenderGameObjectsUsingPhysicallyBasedShader : public BehaviorActionNode {
//    EnumBehaviorTreeStatus OnInvoke() override;
//};

} // namespace dx11
#endif
