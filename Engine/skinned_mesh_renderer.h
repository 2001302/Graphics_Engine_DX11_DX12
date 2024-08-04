#ifndef _SKINNED_MESH_RENDERER
#define _SKINNED_MESH_RENDERER

#include "bone.h"
#include "component.h"
#include "constant_buffer.h"
#include "geometry_generator.h"
#include "mesh.h"
#include "node.h"
#include "mesh_renderer.h"
#include "animation_clip.h"
#include "structured_buffer.h"
#include <filesystem>
#include <iostream>

namespace engine {
using Microsoft::WRL::ComPtr;

class SkinnedMeshRenderer : public MeshRenderer {
  public:
    SkinnedMeshRenderer(ComPtr<ID3D11Device> &device,
                     ComPtr<ID3D11DeviceContext> &context,
                     const vector<MeshData> &meshes,
                     const AnimationData &aniData) {
        Initialize(device, context, meshes, aniData);
    }

    void Initialize(ComPtr<ID3D11Device> &device,
                    ComPtr<ID3D11DeviceContext> &context,
                    const vector<MeshData> &meshes,
                    const AnimationData &aniData) {
        InitAnimationData(device, aniData);
        MeshRenderer::Initialize(device, context, meshes);
    }

    GraphicsPSO &GetPSO(const bool wired) override {
        return wired ? Graphics::skinnedWirePSO : Graphics::skinnedSolidPSO;
    }

    GraphicsPSO &GetReflectPSO(const bool wired) override {
        return wired ? Graphics::reflectSkinnedWirePSO
                     : Graphics::reflectSkinnedSolidPSO;
    }

    GraphicsPSO &GetDepthOnlyPSO() override {
        return Graphics::depthOnlySkinnedPSO;
    }

    void InitMeshBuffers(ComPtr<ID3D11Device> &device, const MeshData &meshData,
                         std::shared_ptr<Mesh> &newMesh) override {
        GraphicsUtil::CreateVertexBuffer(device, meshData.skinnedVertices,
                                       newMesh->vertexBuffer);
        newMesh->indexCount = UINT(meshData.indices.size());
        newMesh->vertexCount = UINT(meshData.skinnedVertices.size());
        newMesh->stride = UINT(sizeof(SkinnedVertex));
        GraphicsUtil::CreateIndexBuffer(device, meshData.indices,
                                      newMesh->indexBuffer);
    }

    void InitAnimationData(ComPtr<ID3D11Device> &device,
                           const AnimationData &aniData) {
        if (!aniData.clips.empty()) {
            m_aniData = aniData;

            // 여기서는 AnimationClip이 SkinnedMesh라고 가정하겠습니다.
            // 일반적으로는 모든 Animation이 SkinnedMesh Animation은 아닙니다.
            m_boneTransforms.m_cpu.resize(aniData.clips.front().keys.size());

            // 주의: 모든 keys() 개수가 동일하지 않을 수도 있습니다.
            for (int i = 0; i < aniData.clips.front().keys.size(); i++)
                m_boneTransforms.m_cpu[i] = Matrix();
            m_boneTransforms.Initialize(device);
        }
    }

    void UpdateAnimation(ComPtr<ID3D11DeviceContext> &context, int clipId,
                         int frame) override {

        m_aniData.Update(clipId, frame);

        for (int i = 0; i < m_boneTransforms.m_cpu.size(); i++) {
            m_boneTransforms.m_cpu[i] =
                m_aniData.Get(clipId, i, frame).Transpose();
        }

        m_boneTransforms.Upload(context);
    }

    void Render(ComPtr<ID3D11DeviceContext> &context) override {

        // ConstBuffer 대신 StructuredBuffer 사용
        // context->VSSetConstantBuffers(3, 1, m_skinnedConsts.GetAddressOf());

        context->VSSetShaderResources(
            9, 1, m_boneTransforms.GetAddressOfSRV()); // 항상 slot index 주의

        // Skinned VS/PS는 GetPSO()를 통해서 지정되기 때문에
        // SkinnedMeshRenderer::Render(.)를 같이 사용 가능

        MeshRenderer::Render(context);
    };

    // SkinnedMesh는 BoundingBox를 그릴 때 Root의 Transform을 반영해야 합니다.
    // virtual void RenderWireBoundingBox(ComPtr<ID3D11DeviceContext> &context);
    // virtual void RenderWireBoundingSphere(ComPtr<ID3D11DeviceContext>
    // &context);

  public:
    // ConstantBuffer<SkinnedConsts> m_skinnedConsts;
    StructuredBuffer<Matrix> m_boneTransforms;

    AnimationData m_aniData;
};
} // namespace engine
#endif
