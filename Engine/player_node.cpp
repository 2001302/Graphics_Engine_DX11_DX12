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

    behavior_tree = std::make_shared<common::BehaviorTreeBuilder>();
    set_state = std::make_shared<SetAnimationState>();
    check_walk = std::make_shared<CheckWalk>();
    idle_to_walk = std::make_shared<IdleToWalk>();
    walk = std::make_shared<Walk>();
    walk_to_idle = std::make_shared<WalkToIdle>();
    idle = std::make_shared<Idle>();
    stop_walk = std::make_shared<StopWalk>();
};

void PlayerAnimator::Build() {
    // clang-format off
        behavior_tree->Build(&block)
            ->Excute(set_state)
            ->Selector()
                ->Conditional(check_walk)
                    ->Sequence()
                        //move forward
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

common::EnumBehaviorTreeStatus PlayerAnimator::SetAnimationState::OnInvoke() {
    auto block = dynamic_cast<PlayerAnimator::AnimationBlock *>(data_block);
    assert(block != nullptr);

    if (block->input->KeyState(VK_UP)) {
        if (block->state != PlayerAnimator::EnumAnimationState::eWalk) {
            block->state = PlayerAnimator::EnumAnimationState::eWalk;
            GetParent()->Reset();
        }
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

    block->animator->UpdateAnimation(
        GraphicsManager::Instance().device_context,
        PlayerAnimator::EnumAnimationState::eIdleToWalk, elapsed_time);
    elapsed_time += block->dt;

    if (block->animator->animation_data.IsClipEnd(
            PlayerAnimator::EnumAnimationState::eIdleToWalk, elapsed_time)) {
        is_done = true;
        return common::EnumBehaviorTreeStatus::eSuccess;
    }
    return common::EnumBehaviorTreeStatus::eRunning;
}

common::EnumBehaviorTreeStatus PlayerAnimator::Walk::OnInvoke() {
    auto block = dynamic_cast<PlayerAnimator::AnimationBlock *>(data_block);
    assert(block != nullptr);

    if (!block->input->KeyState(VK_UP))
        return common::EnumBehaviorTreeStatus::eSuccess;

    block->animator->UpdateAnimation(GraphicsManager::Instance().device_context,
                                     PlayerAnimator::EnumAnimationState::eWalk,
                                     elapsed_time);
    elapsed_time += block->dt;

    auto forward = block->renderer->mesh_consts.GetCpu().world.Forward();
    forward.Normalize();

    Vector3 translation =
        block->renderer->mesh_consts.GetCpu().world.Translation();

    block->renderer->mesh_consts.GetCpu().world.Translation(Vector3(0.0f));

    block->renderer->UpdateWorldRow(
        Matrix::CreateTranslation(translation + (forward * 0.01f)) *
        block->renderer->mesh_consts.GetCpu().world);

    return common::EnumBehaviorTreeStatus::eRunning;
}

common::EnumBehaviorTreeStatus PlayerAnimator::WalkToIdle::OnInvoke() {
    auto block = dynamic_cast<PlayerAnimator::AnimationBlock *>(data_block);
    assert(block != nullptr);

    if (is_done)
        return common::EnumBehaviorTreeStatus::eSuccess;

    block->animator->UpdateAnimation(
        GraphicsManager::Instance().device_context,
        PlayerAnimator::EnumAnimationState::eWalkToIdle, elapsed_time);
    elapsed_time += block->dt;

    if (block->animator->animation_data.IsClipEnd(
            PlayerAnimator::EnumAnimationState::eWalkToIdle, elapsed_time)) {
        is_done = true;
        return common::EnumBehaviorTreeStatus::eSuccess;
    }
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