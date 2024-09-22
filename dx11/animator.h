#ifndef _ANIMATOR
#define _ANIMATOR

#include "animation_clip.h"
#include "component.h"
#include "graphics_util.h"
#include "mesh_renderer.h"
#include "node.h"
#include "structured_buffer.h"
#include <filesystem>
#include <iostream>

namespace graphics {
using Microsoft::WRL::ComPtr;

class Animator : public Component {
  public:
    Animator(){};
    Animator(const AnimationData &aniData);
    void InitAnimationData(const AnimationData &aniData);
    void UpdateAnimation(int clipId, int frame);
    void UpdateAnimation(int clipId, float elapse_time);
    void UploadBoneData();

    void Move(MeshRenderer *renderer, Vector3 direction, float speed);
    void Turn(MeshRenderer *renderer, Vector3 direction, float speed);

  public:
    StructuredBuffer<Matrix> bone_transforms;
    AnimationData animation_data;
};
} // namespace core
#endif
