#include "animator.h"

namespace engine {
Animator::Animator(ComPtr<ID3D11Device> &device, const AnimationData &aniData) {
    InitAnimationData(device, aniData);
}

void Animator::InitAnimationData(ComPtr<ID3D11Device> &device,
                       const AnimationData &aniData) {
    if (!aniData.clips.empty()) {
        animation_data = aniData;

        bone_transforms.m_cpu.resize(aniData.clips.front().keys.size());

        for (int i = 0; i < aniData.clips.front().keys.size(); i++)
            bone_transforms.m_cpu[i] = Matrix();
        bone_transforms.Initialize(device);
    }
}

void Animator::UpdateAnimation(ComPtr<ID3D11DeviceContext> &context, int clipId,
                     int frame) {

    animation_data.Update(clipId, frame);

    for (int i = 0; i < bone_transforms.m_cpu.size(); i++) {
        bone_transforms.m_cpu[i] =
            animation_data.Get(clipId, i, frame).Transpose();
    }

    bone_transforms.Upload(context);
}
void Animator::UploadBoneBuffer() {
    GraphicsManager::Instance().device_context->VSSetShaderResources(
        9, 1, bone_transforms.GetAddressOfSRV());
}
} // namespace engine