#ifndef _ANIMATIONCLIP
#define _ANIMATIONCLIP

#include <directxtk/SimpleMath.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace graphics {

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Quaternion;
using DirectX::SimpleMath::Vector3;
using std::map;
using std::string;
using std::vector;

struct AnimationClip {

    struct Key {
        Vector3 pos = Vector3(0.0f);
        Vector3 scale = Vector3(1.0f);
        Quaternion rot = Quaternion();

        Matrix GetTransform() {
            return Matrix::CreateScale(scale) *
                   Matrix::CreateFromQuaternion(rot) *
                   Matrix::CreateTranslation(pos);
        }
    };

    string name;              // Name of this animation clip
    vector<vector<Key>> keys; // m_key[boneIdx][frameIdx]
    int numChannels;          // Number of bones
    int numKeys;              // Number of frames of this animation clip
    double duration;          // Duration of animation in ticks
    double ticksPerSec;       // Frames per second
};

struct AnimationData {

    map<string, int32_t> boneNameToId;
    vector<string> boneIdToName;
    vector<int32_t> boneParents;
    vector<Matrix> offsetMatrices;
    vector<Matrix> boneTransforms;
    vector<AnimationClip> clips;
    Matrix defaultTransform;
    Matrix rootTransform = Matrix();
    Matrix accumulatedRootTransform = Matrix();
    Vector3 prevPos = Vector3(0.0f);

    bool IsClipEnd(int clipId, float elapse_time) {
        if (clips[clipId].duration <= elapse_time)
            return true;
        return false;
    }

    Matrix Get(int clipId, int boneId, int frame) {

        return defaultTransform.Invert() * offsetMatrices[boneId] *
               boneTransforms[boneId] * defaultTransform;
    }

    void Update(int clipId, int frame) {

        auto &clip = clips[clipId];

        for (int boneId = 0; boneId < boneTransforms.size(); boneId++) {
            auto &keys = clip.keys[boneId];

            const int parentIdx = boneParents[boneId];
            const Matrix parentMatrix = parentIdx >= 0
                                            ? boneTransforms[parentIdx]
                                            : accumulatedRootTransform;

            auto key = keys.size() > 0 ? keys[frame % keys.size()]
                                       : AnimationClip::Key();

            // Root
            if (parentIdx < 0) {
                if (frame != 0) {
                    // accumulatedRootTransform =
                    //     Matrix::CreateTranslation(key.pos - prevPos) *
                    //     accumulatedRootTransform;
                } else {
                    auto temp = accumulatedRootTransform.Translation();
                    temp.y = key.pos.y;
                    accumulatedRootTransform.Translation(temp);
                }

                prevPos = key.pos;
                key.pos = Vector3(0.0f);
            }

            boneTransforms[boneId] = Matrix::CreateFromQuaternion(key.rot) *
                                     Matrix::CreateTranslation(key.pos) *
                                     parentMatrix;
        }
    }
};

} // namespace graphics
#endif
