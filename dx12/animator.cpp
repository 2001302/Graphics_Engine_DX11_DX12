#include "animator.h"

namespace graphics {
Animator::Animator(const AnimationData &aniData) { InitAnimationData(aniData); }

void Animator::InitAnimationData(const AnimationData &aniData) {
    if (!aniData.clips.empty()) {
        animation_data = aniData;

        bone_transforms.GetCpu().resize(aniData.clips.front().keys.size());

        for (int i = 0; i < aniData.clips.front().keys.size(); i++)
            bone_transforms.GetCpu()[i] = Matrix();
        bone_transforms.Initialize();
    }
}

void Animator::UpdateAnimation(int clipId, int frame) {

    animation_data.Update(clipId, frame);

    for (int i = 0; i < bone_transforms.GetCpu().size(); i++) {
        bone_transforms.GetCpu()[i] =
            animation_data.Get(clipId, i, frame).Transpose();
    }

    bone_transforms.Upload();
}

void Animator::UpdateAnimation(int clipId, float elapse_time) {
    auto frame = animation_data.clips[clipId].keys[0].size() *
                 (elapse_time / (animation_data.clips[clipId].duration));

    animation_data.Update(clipId, frame);

    for (int i = 0; i < bone_transforms.GetCpu().size(); i++) {
        bone_transforms.GetCpu()[i] =
            animation_data.Get(clipId, i, frame).Transpose();
    }

    bone_transforms.Upload();
}

void Animator::Move(MeshRenderer *renderer, Vector3 direction, float speed) {
    Vector3 translation = renderer->GetWorldRow().Translation();
    Quaternion rotation =
        Quaternion::CreateFromRotationMatrix(renderer->GetWorldRow());

    renderer->UpdateWorldRow(
        Matrix::CreateFromQuaternion(rotation) *
        Matrix::CreateTranslation(translation + (direction * speed * 0.01f)));
}

void Animator::Turn(MeshRenderer *renderer, Vector3 direction, float speed) {
    Quaternion additional =
        Quaternion::CreateFromAxisAngle(direction, speed * 0.01f);

    Vector3 translation = renderer->GetWorldRow().Translation();
    Quaternion rotation = additional * Quaternion::CreateFromRotationMatrix(
                                           renderer->GetWorldRow());

    renderer->UpdateWorldRow(Matrix::CreateFromQuaternion(rotation) *
                             Matrix::CreateTranslation(translation));
}
} // namespace graphics