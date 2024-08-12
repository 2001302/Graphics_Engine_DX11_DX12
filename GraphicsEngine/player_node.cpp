#include "player_node.h"

namespace engine {
PlayerAnimator::PlayerAnimator(ComPtr<ID3D11Device> &device,
                               const AnimationData &aniData,
                               SkinnedMeshRenderer *renderer,
                               common::Input *input)
    : Animator(device, aniData) {

    block.animator = this;
    block.renderer = renderer;
    block.input = input;

    read_input = std::make_shared<ReadInput>();
    check_walk = std::make_shared<CheckWalk>();
    idle_to_walk = std::make_shared<IdleToWalk>();
    walk = std::make_shared<Walk>();
    walk_to_idle = std::make_shared<WalkToIdle>();
    idle = std::make_shared<Idle>();
    stop_walk = std::make_shared<StopWalk>();

    Build();
};

void PlayerAnimator::Build() {
    // clang-format off
        behavior_tree.Build(&block)
            ->Excute(read_input)
            ->Selector()
                ->Conditional(check_walk)
                    ->Sequence()
                        ->Excute(idle_to_walk)
                        ->Excute(walk)
                        ->Excute(walk_to_idle)
                        ->Excute(stop_walk)
                    ->Close()
                ->Close()
            ->Excute(idle)
            ->Close();
    // clang-format on
};

common::EnumBehaviorTreeStatus PlayerAnimator::ReadInput::OnInvoke() {
    auto block = dynamic_cast<PlayerAnimator::AnimationBlock *>(data_block);
    assert(block != nullptr);

    if (block->input->KeyState(VK_UP)) {
        if (block->state != PlayerAnimator::EnumAnimationState::eWalk) {
            block->state = PlayerAnimator::EnumAnimationState::eWalk;
            GetParent()->Reset();
        }
    }

    if (block->input->KeyState(VK_LEFT)) {
        block->animator->Turn(block->renderer, -Vector3::UnitY, 1.0f);
    } else if (block->input->KeyState(VK_RIGHT)) {
        block->animator->Turn(block->renderer, Vector3::UnitY, 1.0f);
    }

    return common::EnumBehaviorTreeStatus::eSuccess;
}

common::EnumBehaviorTreeStatus PlayerAnimator::CheckWalk::OnInvoke() {
    auto block = dynamic_cast<PlayerAnimator::AnimationBlock *>(data_block);
    assert(block != nullptr);

    if (block->state != PlayerAnimator::EnumAnimationState::eWalk)
        return common::EnumBehaviorTreeStatus::eFail;

    return common::ConditionalNode::OnInvoke();
}

common::EnumBehaviorTreeStatus PlayerAnimator::IdleToWalk::OnInvoke() {
    auto block = dynamic_cast<PlayerAnimator::AnimationBlock *>(data_block);
    assert(block != nullptr);

    if (is_done)
        return common::EnumBehaviorTreeStatus::eSuccess;

    if (block->animator->animation_data.IsClipEnd(
            PlayerAnimator::EnumAnimationState::eIdleToWalk, elapsed_time)) {
        is_done = true;
        return common::EnumBehaviorTreeStatus::eSuccess;
    }
    elapsed_time += block->dt;

    block->animator->UpdateAnimation(
        GraphicsManager::Instance().device_context,
        PlayerAnimator::EnumAnimationState::eIdleToWalk, elapsed_time);
    block->animator->Move(block->renderer, block->renderer->world_row.Forward(),
                          0.2f);

    return common::EnumBehaviorTreeStatus::eRunning;
}

common::EnumBehaviorTreeStatus PlayerAnimator::Walk::OnInvoke() {
    auto block = dynamic_cast<PlayerAnimator::AnimationBlock *>(data_block);
    assert(block != nullptr);

    if (!block->input->KeyState(VK_UP))
        return common::EnumBehaviorTreeStatus::eSuccess;
    elapsed_time += block->dt;

    block->animator->UpdateAnimation(GraphicsManager::Instance().device_context,
                                     PlayerAnimator::EnumAnimationState::eWalk,
                                     elapsed_time);
    block->animator->Move(block->renderer, block->renderer->world_row.Forward(),
                          1.0f);

    return common::EnumBehaviorTreeStatus::eRunning;
}

common::EnumBehaviorTreeStatus PlayerAnimator::WalkToIdle::OnInvoke() {
    auto block = dynamic_cast<PlayerAnimator::AnimationBlock *>(data_block);
    assert(block != nullptr);

    if (is_done)
        return common::EnumBehaviorTreeStatus::eSuccess;

    if (block->animator->animation_data.IsClipEnd(
            PlayerAnimator::EnumAnimationState::eWalkToIdle, elapsed_time)) {
        is_done = true;
        return common::EnumBehaviorTreeStatus::eSuccess;
    }
    elapsed_time += block->dt;

    block->animator->UpdateAnimation(
        GraphicsManager::Instance().device_context,
        PlayerAnimator::EnumAnimationState::eWalkToIdle, elapsed_time);
    block->animator->Move(block->renderer, block->renderer->world_row.Forward(),
                          0.2f);

    return common::EnumBehaviorTreeStatus::eRunning;
}

common::EnumBehaviorTreeStatus PlayerAnimator::StopWalk::OnInvoke() {
    auto block = dynamic_cast<PlayerAnimator::AnimationBlock *>(data_block);
    assert(block != nullptr);

    block->state = PlayerAnimator::EnumAnimationState::eNone;

    return common::EnumBehaviorTreeStatus::eRunning;
}

common::EnumBehaviorTreeStatus PlayerAnimator::Idle::OnInvoke() {
    auto block = dynamic_cast<PlayerAnimator::AnimationBlock *>(data_block);
    assert(block != nullptr);

    block->state = PlayerAnimator::EnumAnimationState::eIdle;

    block->animator->UpdateAnimation(GraphicsManager::Instance().device_context,
                                     PlayerAnimator::EnumAnimationState::eIdle,
                                     elapsed_time);
    elapsed_time += block->dt;

    return common::EnumBehaviorTreeStatus::eRunning;
}

} // namespace engine