#ifndef _PLAYER_NODE
#define _PLAYER_NODE

#include "animator.h"
#include "behavior_tree_builder.h"
#include "black_board.h"
#include "mesh_renderer.h"
#include "skinned_mesh_renderer.h"

namespace core {

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
    PlayerAnimator(ComPtr<ID3D11Device> &device, const AnimationData &aniData,
                   SkinnedMeshRenderer *renderer, Input *input);

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

        auto job_context = black_board->job_context;
        auto gui = black_board->gui;

        switch (job_context->stage_type) {
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

            auto renderer = std::make_shared<SkinnedMeshRenderer>(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context, meshes);

            renderer->material_consts.GetCpu().albedoFactor = Vector3(1.0f);
            renderer->material_consts.GetCpu().roughnessFactor = 0.8f;
            renderer->material_consts.GetCpu().metallicFactor = 0.0f;
            renderer->UpdateWorldRow(
                Matrix::CreateScale(1.0f) *
                Matrix::CreateTranslation(0.0f, 0.0f, 0.0f));

            auto animator = std::make_shared<PlayerAnimator>(
                GraphicsManager::Instance().device, aniData, renderer.get(),
                black_board->input.get());

            job_context->player = std::make_shared<Model>();
            job_context->player->AddComponent(EnumComponentType::eRenderer,
                                              renderer);
            job_context->player->AddComponent(EnumComponentType::eAnimator,
                                              animator);

            break;
        }
        case EnumStageType::eUpdate: {
            auto renderer =
                (SkinnedMeshRenderer *)job_context->player->GetComponent(
                    EnumComponentType::eRenderer);
            auto animator = (PlayerAnimator *)job_context->player->GetComponent(
                EnumComponentType::eAnimator);

            animator->Run(job_context->dt);

            renderer->UpdateConstantBuffers(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context);

            break;
        }
        case EnumStageType::eRender: {
            GraphicsManager::Instance().SetPipelineState(
                job_context->draw_wire ? graphics::skinnedWirePSO
                                       : graphics::skinnedSolidPSO);
            auto renderer =
                (SkinnedMeshRenderer *)job_context->player->GetComponent(
                    EnumComponentType::eRenderer);
            auto animator = (PlayerAnimator *)job_context->player->GetComponent(
                EnumComponentType::eAnimator);

            animator->UploadBoneData();
            renderer->Render(GraphicsManager::Instance().device_context);

            break;
        }
        default:
            break;
        }

        return EnumBehaviorTreeStatus::eSuccess;
    }
};
} // namespace engine

#endif