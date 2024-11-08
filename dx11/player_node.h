#ifndef _PLAYER_NODE
#define _PLAYER_NODE

#include "animator.h"
#include "black_board.h"
#include "mesh_renderer.h"
#include "skinned_mesh_renderer.h"
#include <behavior_tree_builder.h>

namespace graphics {

using namespace common;

class PlayerAnimator : public Animator {
  public:
    enum EnumAnimationState {
        eIdle = 0,
        eIdleToWalk = 1,
        eWalk = 2,
        eWalkToIdle = 3,
        eDance = 4,
        eNone = 5,
    };

    struct AnimationBlock : public IDataBlock {
        AnimationBlock()
            : state(EnumAnimationState::eIdle), animator(0), renderer(0),
              input(0), dt(0.0f){};

        EnumAnimationState state;
        PlayerAnimator *animator;
        SkinnedMeshRenderer *renderer;
        Input *input;
        float dt;
    };

    PlayerAnimator(){};
    PlayerAnimator(const AnimationData &aniData, SkinnedMeshRenderer *renderer,
                   Input *input);

    void Build();
    void Run(float dt) {
        block.dt = dt;
        behavior_tree.Run();
    }

  private:
    AnimationBlock block;
    BehaviorTreeBuilder behavior_tree;

    struct ReadInput : public BehaviorActionNode {
        EnumBehaviorTreeStatus OnInvoke() override;
    };

    struct CheckWalk : public ConditionalNode {
        EnumBehaviorTreeStatus OnInvoke() override;
    };

    struct IdleToWalk : public AnimationNode {
        EnumBehaviorTreeStatus OnInvoke() override;
    };

    struct Walk : public AnimationNode {
        EnumBehaviorTreeStatus OnInvoke() override;
    };

    struct WalkToIdle : public AnimationNode {
        EnumBehaviorTreeStatus OnInvoke() override;
    };

    struct StopWalk : public AnimationNode {
        EnumBehaviorTreeStatus OnInvoke() override;
    };

    struct Idle : public AnimationNode {
        EnumBehaviorTreeStatus OnInvoke() override;
    };
    std::shared_ptr<ReadInput> read_input;
    std::shared_ptr<IdleToWalk> idle_to_walk;
    std::shared_ptr<Walk> walk;
    std::shared_ptr<WalkToIdle> walk_to_idle;
    std::shared_ptr<Idle> idle;
    std::shared_ptr<StopWalk> stop_walk;
    std::shared_ptr<CheckWalk> check_walk;
};

class PlayerNodeInvoker : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto job_context = black_board->targets;
        auto gui = black_board->gui;

        switch (job_context->stage_type) {
        case EnumStageType::eInitialize: {
            std::string path = "../Assets/Characters/Mixamo/";
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

            auto renderer = std::make_shared<SkinnedMeshRenderer>(meshes);

            renderer->material_consts.GetCpu().albedoFactor = Vector3(1.0f);
            renderer->material_consts.GetCpu().roughnessFactor = 0.8f;
            renderer->material_consts.GetCpu().metallicFactor = 0.0f;
            renderer->UpdateWorldRow(
                Matrix::CreateScale(1.0f) *
                Matrix::CreateTranslation(0.0f, 0.0f, 0.0f));

            auto animator = std::make_shared<PlayerAnimator>(
                aniData, renderer.get(), black_board->input.get());

            job_context->player = std::make_shared<Model>();
            job_context->player->TryAdd(renderer);
            job_context->player->TryAdd(animator);

            break;
        }
        case EnumStageType::eUpdate: {
            SkinnedMeshRenderer *renderer = nullptr;
            PlayerAnimator *animator = nullptr;
            if (job_context->player->TryGet(renderer) &&
                job_context->player->TryGet(animator)) {
                animator->Run(job_context->delta_time);
                renderer->UpdateConstantBuffers();
            }

            break;
        }
        case EnumStageType::eRender: {
            graphics::Util::SetPipelineState(
                job_context->draw_wire ? graphics::pipeline::skinnedWirePSO
                                       : graphics::pipeline::skinnedSolidPSO);

            SkinnedMeshRenderer *renderer = nullptr;
            PlayerAnimator *animator = nullptr;
            if (job_context->player->TryGet(renderer) &&
                job_context->player->TryGet(animator)) {
                animator->UploadBoneData();
                renderer->Render();
            }

            break;
        }
        default:
            break;
        }

        return EnumBehaviorTreeStatus::eSuccess;
    }
};
} // namespace graphics

#endif