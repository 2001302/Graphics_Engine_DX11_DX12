#include "player_node.h"

namespace core {
PlayerAnimator::PlayerAnimator(const AnimationData &aniData,
                               SkinnedMeshRenderer *renderer,
                               foundation::Input *input)
    : Animator(aniData) {

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

foundation::EnumBehaviorTreeStatus PlayerAnimator::ReadInput::OnInvoke() {
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

    return foundation::EnumBehaviorTreeStatus::eSuccess;
}

foundation::EnumBehaviorTreeStatus PlayerAnimator::CheckWalk::OnInvoke() {
    auto block = dynamic_cast<PlayerAnimator::AnimationBlock *>(data_block);
    assert(block != nullptr);

    if (block->state != PlayerAnimator::EnumAnimationState::eWalk)
        return foundation::EnumBehaviorTreeStatus::eFail;

    return foundation::ConditionalNode::OnInvoke();
}

foundation::EnumBehaviorTreeStatus PlayerAnimator::IdleToWalk::OnInvoke() {
    auto block = dynamic_cast<PlayerAnimator::AnimationBlock *>(data_block);
    assert(block != nullptr);

    if (is_done)
        return foundation::EnumBehaviorTreeStatus::eSuccess;

    if (block->animator->animation_data.IsClipEnd(
            PlayerAnimator::EnumAnimationState::eIdleToWalk, elapsed_time)) {
        is_done = true;
        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
    elapsed_time += block->dt;

    block->animator->UpdateAnimation(
        PlayerAnimator::EnumAnimationState::eIdleToWalk, elapsed_time);
    block->animator->Move(block->renderer, block->renderer->world_row.Forward(),
                          0.2f);

    return foundation::EnumBehaviorTreeStatus::eRunning;
}

foundation::EnumBehaviorTreeStatus PlayerAnimator::Walk::OnInvoke() {
    auto block = dynamic_cast<PlayerAnimator::AnimationBlock *>(data_block);
    assert(block != nullptr);

    if (!block->input->KeyState(VK_UP))
        return foundation::EnumBehaviorTreeStatus::eSuccess;
    elapsed_time += block->dt;

    block->animator->UpdateAnimation(PlayerAnimator::EnumAnimationState::eWalk,
                                     elapsed_time);
    block->animator->Move(block->renderer, block->renderer->world_row.Forward(),
                          1.0f);

    return foundation::EnumBehaviorTreeStatus::eRunning;
}

foundation::EnumBehaviorTreeStatus PlayerAnimator::WalkToIdle::OnInvoke() {
    auto block = dynamic_cast<PlayerAnimator::AnimationBlock *>(data_block);
    assert(block != nullptr);

    if (is_done)
        return foundation::EnumBehaviorTreeStatus::eSuccess;

    if (block->animator->animation_data.IsClipEnd(
            PlayerAnimator::EnumAnimationState::eWalkToIdle, elapsed_time)) {
        is_done = true;
        return foundation::EnumBehaviorTreeStatus::eSuccess;
    }
    elapsed_time += block->dt;

    block->animator->UpdateAnimation(
        PlayerAnimator::EnumAnimationState::eWalkToIdle, elapsed_time);
    block->animator->Move(block->renderer, block->renderer->world_row.Forward(),
                          0.2f);

    return foundation::EnumBehaviorTreeStatus::eRunning;
}

foundation::EnumBehaviorTreeStatus PlayerAnimator::StopWalk::OnInvoke() {
    auto block = dynamic_cast<PlayerAnimator::AnimationBlock *>(data_block);
    assert(block != nullptr);

    block->state = PlayerAnimator::EnumAnimationState::eNone;

    return foundation::EnumBehaviorTreeStatus::eRunning;
}

foundation::EnumBehaviorTreeStatus PlayerAnimator::Idle::OnInvoke() {
    auto block = dynamic_cast<PlayerAnimator::AnimationBlock *>(data_block);
    assert(block != nullptr);

    block->state = PlayerAnimator::EnumAnimationState::eIdle;

    block->animator->UpdateAnimation(PlayerAnimator::EnumAnimationState::eIdle,
                                     elapsed_time);
    elapsed_time += block->dt;

    return foundation::EnumBehaviorTreeStatus::eRunning;
}

} // namespace core