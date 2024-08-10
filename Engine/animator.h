#ifndef _ANIMATOR
#define _ANIMATOR

#include "animation_clip.h"
#include "component.h"
#include "graphics_manager.h"
#include "node.h"
#include "structured_buffer.h"
#include "mesh_renderer.h"
#include <filesystem>
#include <iostream>

namespace engine {
using Microsoft::WRL::ComPtr;

class Animator : public Component {
  public:
    Animator(){};
    Animator(ComPtr<ID3D11Device> &device, const AnimationData &aniData);
    void InitAnimationData(ComPtr<ID3D11Device> &device,
                           const AnimationData &aniData);
    void UpdateAnimation(ComPtr<ID3D11DeviceContext> &context, int clipId,
                         int frame);
    void UpdateAnimation(ComPtr<ID3D11DeviceContext> &context, int clipId,
                         float elapse_time);
    void UploadBoneBuffer();

    void Move(MeshRenderer *renderer, Vector3 direction, float speed);
    void Turn(MeshRenderer *renderer, Vector3 direction, float speed);

  public:
    StructuredBuffer<Matrix> bone_transforms;
    AnimationData animation_data;
};
} // namespace engine
#endif
