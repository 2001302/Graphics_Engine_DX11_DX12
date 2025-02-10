#ifndef _ANIMATOR
#define _ANIMATOR

#include "animation_clip.h"
#include "mesh_renderer.h"
#include "structured_buffer.h"
#include <behaviortree/behavior_tree_builder.h>
#include <structure/component.h>
#include <filesystem>
#include <structure/input.h>
#include <iostream>
#include <util/logger.h>
#include <structure/node.h>

namespace graphics {
using Microsoft::WRL::ComPtr;

class Animator : public common::Component {
  public:
    struct Block : public common::IDataBlock {
        Block() : clip_id(0), input(0), elapsed_time(0.0f){};

        int clip_id;
        float elapsed_time;
        common::Input *input;
    };

    Animator() : block(0), behavior_tree(0){};
    Animator(const AnimationData &aniData);

    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() {
        return bone_transforms.GetHandle();
    }

    void SetBuilder(common::BehaviorTreeBuilder *behavior_tree) {
        this->behavior_tree = behavior_tree;
    };
    void SetBlock(Block *block) { this->block = block; };

    void Run(float delta_time) {
        behavior_tree->Run();
        UpdateAnimation(block->clip_id, block->elapsed_time);
        block->elapsed_time += delta_time;
    };

    void Move(MeshRenderer *renderer, Vector3 direction, float speed);
    void Turn(MeshRenderer *renderer, Vector3 direction, float speed);

  private:
    void InitAnimationData(const AnimationData &aniData);
    void UpdateAnimation(int clipId, float elapse_time);

    StructuredBuffer<Matrix> bone_transforms;
    AnimationData animation_data;
    Block *block;
    common::BehaviorTreeBuilder *behavior_tree;
};
} // namespace graphics
#endif
