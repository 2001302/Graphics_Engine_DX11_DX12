#ifndef _RENDERER_UPDATE_NODE
#define _RENDERER_UPDATE_NODE

#include "behavior_tree_builder.h"
#include "renderer.h"
#include "black_board.h"

namespace engine {

class UpdateGlobalConstantBuffersNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        const Vector3 eyeWorld = manager->camera->GetPosition();
        const Matrix reflectRow =
            Matrix::CreateReflection(manager->mirror_plane);
        const Matrix viewRow = manager->camera->GetView();
        const Matrix projRow = manager->camera->GetProjection();

        manager->global_consts_CPU.eyeWorld = eyeWorld;
        manager->global_consts_CPU.view = viewRow.Transpose();
        manager->global_consts_CPU.proj = projRow.Transpose();
        manager->global_consts_CPU.invProj = projRow.Invert().Transpose();
        manager->global_consts_CPU.viewProj = (viewRow * projRow).Transpose();

        // 그림자 렌더링에 사용
        manager->global_consts_CPU.invViewProj =
            manager->global_consts_CPU.viewProj.Invert();

        manager->reflect_global_consts_CPU = manager->global_consts_CPU;
        memcpy(&manager->reflect_global_consts_CPU, &manager->global_consts_CPU,
               sizeof(manager->global_consts_CPU));
        manager->reflect_global_consts_CPU.view =
            (reflectRow * viewRow).Transpose();
        manager->reflect_global_consts_CPU.viewProj =
            (reflectRow * viewRow * projRow).Transpose();

        // 그림자 렌더링에 사용 (TODO: 광원의 위치도 반사시킨 후에 계산해야 함)
        manager->reflect_global_consts_CPU.invViewProj =
            manager->reflect_global_consts_CPU.viewProj.Invert();

        GraphicsUtil::UpdateBuffer(GraphicsManager::Instance().device_context,
                                   manager->global_consts_CPU,
                                   manager->global_consts_GPU);

        GraphicsUtil::UpdateBuffer(GraphicsManager::Instance().device_context,
                                   manager->reflect_global_consts_CPU,
                                   manager->reflect_global_consts_GPU);

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class ApplyMouseMovementNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        //// 마우스 이동/회전 반영
        // if (m_leftButton || m_rightButton) {
        //     Quaternion q;
        //     Vector3 dragTranslation;
        //     Vector3 pickPoint;
        //     if (UpdateMouseControl(m_mainBoundingSphere, q, dragTranslation,
        //                            pickPoint)) {
        //         Vector3 translation = m_mainObj->m_worldRow.Translation();
        //         m_mainObj->m_worldRow.Translation(Vector3(0.0f));
        //         m_mainObj->UpdateWorldRow(
        //             m_mainObj->m_worldRow * Matrix::CreateFromQuaternion(q) *
        //             Matrix::CreateTranslation(dragTranslation +
        //             translation));
        //         m_mainBoundingSphere.Center =
        //         m_mainObj->m_worldRow.Translation();

        //        // 충돌 지점에 작은 구 그리기
        //        m_cursorSphere->m_isVisible = true;
        //        m_cursorSphere->UpdateWorldRow(
        //            Matrix::CreateTranslation(pickPoint));
        //    } else {
        //        m_cursorSphere->m_isVisible = false;
        //    }
        //} else {
        //    m_cursorSphere->m_isVisible = false;
        //}
        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class UpdateBasicObjectsNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        for (auto &i : manager->models) {
            auto renderer = (Renderer *)i.second->GetComponent(
                EnumComponentType::eRenderer);
            renderer->UpdateConstantBuffers(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context);
        }

        {
            auto renderer = (SkinnedMeshRenderer *)manager->m_character->GetComponent(
                EnumComponentType::eRenderer);
            static int frameCount = 0;
            static int state = 0;
            renderer->UpdateAnimation(
                GraphicsManager::Instance().device_context, state, frameCount);
            frameCount += 1;

            renderer->UpdateConstantBuffers(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context);
        }

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class UpdateLightSpheresNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        for (auto &i : manager->light_spheres) {
            auto renderer =
                (Renderer *)i->GetComponent(EnumComponentType::eRenderer);
            renderer->UpdateConstantBuffers(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context);
        }
        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};
} // namespace engine

#endif