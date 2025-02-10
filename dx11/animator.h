#ifndef _ANIMATOR
#define _ANIMATOR

#include "animation_clip.h"
#include "graphics_util.h"
#include "mesh_renderer.h"
#include "structured_buffer.h"
#include <structure/component.h>
#include <filesystem>
#include <iostream>
#include <structure/node.h>

namespace graphics {
using Microsoft::WRL::ComPtr;

class Animator : public common::Component {
  public:
    Animator(){};
    Animator(const AnimationData &aniData);
    void InitAnimationData(const AnimationData &aniData);
    void UpdateAnimation(int clipId, int frame);
    void UpdateAnimation(int clipId, float elapse_time);
    void UploadBoneData();

    void Move(MeshRenderer *renderer, Vector3 direction, float speed);
    void Turn(MeshRenderer *renderer, Vector3 direction, float speed);
    void Run(float time){};

  public:
    StructuredBuffer<Matrix> bone_transforms;
    AnimationData animation_data;
};
} // namespace graphics
#endif
