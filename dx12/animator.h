#ifndef _ANIMATOR
#define _ANIMATOR

#include "animation_clip.h"
#include "mesh_renderer.h"
#include "structured_buffer.h"
#include <component.h>
#include <filesystem>
#include <iostream>
#include <node.h>
#include <logger.h>

namespace graphics {
using Microsoft::WRL::ComPtr;

class Animator : public common::Component {
  public:
    Animator(){};
    Animator(const AnimationData &aniData);
    void InitAnimationData(const AnimationData &aniData);
    void UpdateAnimation(int clipId, int frame);
    void UpdateAnimation(int clipId, float elapse_time);

    D3D12_GPU_DESCRIPTOR_HANDLE GetBone() {
        return bone_transforms.GetHandle();
	}

    void Move(MeshRenderer *renderer, Vector3 direction, float speed);
    void Turn(MeshRenderer *renderer, Vector3 direction, float speed);

  public:
    StructuredBuffer<Matrix> bone_transforms;
    AnimationData animation_data;
};
} // namespace graphics
#endif
