#ifndef _ANIMATOR
#define _ANIMATOR

#include "animation_clip.h"
#include "component.h"
#include "node.h"
#include "structured_buffer.h"
#include "graphics_manager.h"
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
    void UploadBoneBuffer();

  public:
    StructuredBuffer<Matrix> bone_transforms;
    AnimationData animation_data;
};
} // namespace engine
#endif
