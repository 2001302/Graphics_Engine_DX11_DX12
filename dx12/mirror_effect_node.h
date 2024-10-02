#ifndef _MIRROR_EFFECT_NODE
#define _MIRROR_EFFECT_NODE

#include "black_board.h"
#include "mesh_pso.h"
#include "mesh_renderer.h"
#include <behavior_tree_builder.h>

namespace graphics {

class MirrorEffectNodeInvoker : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        switch (condition->stage_type) {
        case EnumStageType::eInitialize: {

            //1.pso 생성 (mesh,mirror)
            //2.model 생성 (mesh,mirror)
            //3.reflect global const buffer 생성  
            //  :기존 global const buffer를 복사해서 reflect row 만 업데이트해서 사용한다.

            //// pso
            //mesh_solid_PSO = std::make_shared<SolidMeshPSO>();
            //mesh_solid_PSO->Initialize();

            //// sample object
            //std::string base_path = "Assets/Characters/Mixamo/";
            //std::string file_name = "character.fbx";

            //auto component = std::make_shared<MeshRenderer>();
            //component->Initialize(base_path, file_name);
            //component->UpdateConstantBuffers();

            //auto model = std::make_shared<Model>();
            //model->AddComponent(EnumComponentType::eRenderer, component);

            //targets->objects.insert({model->GetEntityId(), model});

            break;
        }
        case EnumStageType::eUpdate: {
            //update reflect global const buffer
            
            //for (auto &i : targets->objects) {
            //    auto component = (MeshRenderer *)i.second->GetComponent(
            //        EnumComponentType::eRenderer);
            //    component->UpdateConstantBuffers();
            //}

            break;
        }
        case EnumStageType::eRender: {
            //1.stencilMaskPSO
            //2.reflectSolidPSO
            //3.reflectSkyboxSolidPSO
            //4.mirrorBlendSolidPSO


            //for (auto &i : targets->objects) {
            //    auto component = (MeshRenderer *)i.second->GetComponent(
            //        EnumComponentType::eRenderer);

            //    mesh_solid_PSO->Render(
            //        condition->shared_texture, condition->shared_sampler,
            //        condition->global_consts.Get(), component);
            //}

            break;
        }
        default:
            break;
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }

    //std::shared_ptr<SolidMeshPSO> mesh_solid_PSO;
    //std::shared_ptr<WireMeshPSO> mesh_wire_PSO;
};
} // namespace graphics

#endif
