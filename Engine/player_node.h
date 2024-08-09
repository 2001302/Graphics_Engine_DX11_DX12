#ifndef _PLAYER_NODE
#define _PLAYER_NODE

#include "behavior_tree_builder.h"
#include "black_board.h"
#include "mesh_renderer.h"
#include "skinned_mesh_renderer.h"

namespace engine {

enum EnumAnimationState {
    eIdle = 0,
    eIdleToWalk = 1,
    eWalk = 2,
    eWalkToIdle = 3,
    eDance = 4,
    eNone = 5,
};

struct AnimationBlock : public common::IDataBlock {
    AnimationData *aniData;
    SkinnedMeshRenderer *renderer;
    common::Input *input;
    EnumAnimationState state;
};

class AnimationState : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {
        auto animation_block = dynamic_cast<AnimationBlock *>(data_block);
        assert(animation_block != nullptr);

        if (animation_block->input->KeyState(VK_UP)) {
            if (animation_block->state != EnumAnimationState::eWalk) {
                animation_block->state = EnumAnimationState::eWalk;
                GetParent()->Reset();
            }
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class CheckWalk : public common::ConditionalNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {
        auto animation_block = dynamic_cast<AnimationBlock *>(data_block);
        assert(animation_block != nullptr);

        if (animation_block->state != EnumAnimationState::eWalk)
            return common::EnumBehaviorTreeStatus::eFail;

        return common::ConditionalNode::OnInvoke();
    }
};

class IdleToWalk : public common::AnimationNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {
        auto animation_block = dynamic_cast<AnimationBlock *>(data_block);
        assert(animation_block != nullptr);

        if (is_done)
            return common::EnumBehaviorTreeStatus::eSuccess;

        animation_block->renderer->UpdateAnimation(
            GraphicsManager::Instance().device_context,
            EnumAnimationState::eIdleToWalk, frame_count);
        frame_count += 1;

        if (animation_block->aniData->clips[EnumAnimationState::eIdleToWalk]
                .keys[0]
                .size() <= frame_count) {
            is_done = true;
            return common::EnumBehaviorTreeStatus::eSuccess;
        }
        return common::EnumBehaviorTreeStatus::eRunning;
    }
};

class Walk : public common::AnimationNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {
        auto animation_block = dynamic_cast<AnimationBlock *>(data_block);
        assert(animation_block != nullptr);

        if (!animation_block->input->KeyState(VK_UP))
            return common::EnumBehaviorTreeStatus::eSuccess;

        animation_block->renderer->UpdateAnimation(
            GraphicsManager::Instance().device_context,
            EnumAnimationState::eWalk, frame_count);
        frame_count += 1;

        auto forward =
            animation_block->renderer->mesh_consts.GetCpu().world.Forward();
        forward.Normalize();

        Vector3 translation =
            animation_block->renderer->mesh_consts.GetCpu().world.Translation();

        animation_block->renderer->mesh_consts.GetCpu().world.Translation(
            Vector3(0.0f));

        animation_block->renderer->UpdateWorldRow(
            Matrix::CreateTranslation(translation+(forward*0.01f)) *
            animation_block->renderer->mesh_consts.GetCpu().world);

        return common::EnumBehaviorTreeStatus::eRunning;
    }
};

class WalkToIdle : public common::AnimationNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {
        auto animation_block = dynamic_cast<AnimationBlock *>(data_block);
        assert(animation_block != nullptr);

        if (is_done)
            return common::EnumBehaviorTreeStatus::eSuccess;

        animation_block->renderer->UpdateAnimation(
            GraphicsManager::Instance().device_context,
            EnumAnimationState::eWalkToIdle, frame_count);
        frame_count += 1;

        if (animation_block->aniData->clips[EnumAnimationState::eWalkToIdle]
                .keys[0]
                .size() <= frame_count) {
            is_done = true;
            return common::EnumBehaviorTreeStatus::eSuccess;
        }
        return common::EnumBehaviorTreeStatus::eRunning;
    }
};

class FinishWalk : public common::AnimationNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {
        auto animation_block = dynamic_cast<AnimationBlock *>(data_block);
        assert(animation_block != nullptr);

        animation_block->state = EnumAnimationState::eNone;

        return common::EnumBehaviorTreeStatus::eRunning;
    }
};

class Idle : public common::AnimationNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {
        auto animation_block = dynamic_cast<AnimationBlock *>(data_block);
        assert(animation_block != nullptr);

        animation_block->state = EnumAnimationState::eIdle;

        animation_block->renderer->UpdateAnimation(
            GraphicsManager::Instance().device_context,
            EnumAnimationState::eIdle, frame_count);
        frame_count += 1;

        return common::EnumBehaviorTreeStatus::eRunning;
    }
};

class Animator : public Component, public common::BehaviorTreeBuilder {
  public:
    void Initialize(AnimationData *aniData, SkinnedMeshRenderer *renderer,
                    common::Input *input) {
        block.aniData = aniData;
        block.renderer = renderer;
        block.input = input;

        animation_state = std::make_shared<AnimationState>();
        check_walk = std::make_shared<CheckWalk>();
        idle_to_walk = std::make_shared<IdleToWalk>();
        walk = std::make_shared<Walk>();
        walk_to_idle = std::make_shared<WalkToIdle>();
        idle = std::make_shared<Idle>();
        finish_walk = std::make_shared<FinishWalk>();

        // clang-format off
        Build(&block)
            ->Excute(animation_state)
            ->Selector()
                ->Conditional(check_walk)
                    ->Sequence()
                        ->Excute(idle_to_walk)
                        ->Excute(walk)
                        ->Excute(walk_to_idle)
                        ->Excute(finish_walk)
                    ->Close()
                ->Close()
                ->Excute(idle)
            ->Close();
        // clang-format on
    };
    void Updete() { Run(); };
    void Show(){};

  private:
    AnimationBlock block;

    std::shared_ptr<AnimationState> animation_state;
    std::shared_ptr<IdleToWalk> idle_to_walk;
    std::shared_ptr<Walk> walk;
    std::shared_ptr<WalkToIdle> walk_to_idle;
    std::shared_ptr<Idle> idle;
    std::shared_ptr<FinishWalk> finish_walk;
    std::shared_ptr<CheckWalk> check_walk;
};

class PlayerNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;
        auto gui = black_board->gui;

        switch (manager->stage_type) {
        case EnumStageType::eInitialize: {
            std::string path = "Assets/Characters/Mixamo/";
            std::vector<std::string> clipNames = {
                "CatwalkIdle.fbx", "CatwalkIdleToWalkForward.fbx",
                "CatwalkWalkForward.fbx", "CatwalkWalkStop.fbx",
                "BreakdanceFreezeVar2.fbx"};

            AnimationData aniData;
            auto [meshes, _] =
                GeometryGenerator::ReadAnimationFromFile(path, "character.fbx");

            for (auto &name : clipNames) {
                auto [_, ani] =
                    GeometryGenerator::ReadAnimationFromFile(path, name);

                if (aniData.clips.empty()) {
                    aniData = ani;
                } else {
                    aniData.clips.push_back(ani.clips.front());
                }
            }

            Vector3 center(0.0f, 0.0f, 0.0f);
            auto renderer = std::make_shared<SkinnedMeshRenderer>(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context, meshes, aniData);

            renderer->material_consts.GetCpu().albedoFactor = Vector3(1.0f);
            renderer->material_consts.GetCpu().roughnessFactor = 0.8f;
            renderer->material_consts.GetCpu().metallicFactor = 0.0f;
            renderer->UpdateWorldRow(Matrix::CreateScale(1.0f) *
                                     Matrix::CreateTranslation(center));

            manager->player = std::make_shared<Model>();
            manager->player->AddComponent(EnumComponentType::eRenderer,
                                          renderer);
            animator.Initialize(&renderer->m_aniData, renderer.get(),
                                black_board->input.get());
            break;
        }
        case EnumStageType::eUpdate: {
            auto renderer =
                (SkinnedMeshRenderer *)manager->player->GetComponent(
                    EnumComponentType::eRenderer);

            animator.Updete();

            renderer->UpdateConstantBuffers(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context);

            break;
        }
        case EnumStageType::eRender: {
            GraphicsManager::Instance().SetPipelineState(
                manager->draw_wire ? Graphics::skinnedWirePSO
                                   : Graphics::skinnedSolidPSO);
            auto renderer =
                (SkinnedMeshRenderer *)manager->player->GetComponent(
                    EnumComponentType::eRenderer);
            renderer->Render(GraphicsManager::Instance().device_context);

            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
    Animator animator;
};
} // namespace engine

#endif