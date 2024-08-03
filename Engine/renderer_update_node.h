#ifndef _RENDERER_UPDATE_NODE
#define _RENDERER_UPDATE_NODE

#include "behavior_tree_builder.h"
#include "renderer.h"
#include "black_board.h"

namespace engine {

class UpdateCameraNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        manager->camera->Update();

        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

class UpdateLightsNode : public common::BehaviorActionNode {
  public:
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        float dt = manager->dt;

        // 회전하는 lights[1] 업데이트
        static Vector3 lightDev = Vector3(1.0f, 0.0f, 0.0f);
        if (manager->light_rotate) {
            lightDev = Vector3::Transform(
                lightDev, Matrix::CreateRotationY(dt * 3.141592f * 0.5f));
        }
        manager->global_consts_CPU.lights[1].position =
            Vector3(0.0f, 1.1f, 2.0f) + lightDev;
        Vector3 focusPosition = Vector3(0.0f, -0.5f, 1.7f);
        manager->global_consts_CPU.lights[1].direction =
            focusPosition - manager->global_consts_CPU.lights[1].position;
        manager->global_consts_CPU.lights[1].direction.Normalize();

        // 그림자맵을 만들기 위한 시점
        for (int i = 0; i < MAX_LIGHTS; i++) {
            const auto &light = manager->global_consts_CPU.lights[i];
            if (light.type & LIGHT_SHADOW) {

                Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
                if (abs(up.Dot(light.direction) + 1.0f) < 1e-5)
                    up = Vector3(1.0f, 0.0f, 0.0f);

                // 그림자맵을 만들 때 필요
                Matrix lightViewRow = DirectX::XMMatrixLookAtLH(
                    light.position, light.position + light.direction, up);

                Matrix lightProjRow = DirectX::XMMatrixPerspectiveFovLH(
                    DirectX::XMConvertToRadians(120.0f), 1.0f, 0.1f, 10.0f);

                manager->shadow_global_consts_CPU[i].eyeWorld = light.position;
                manager->shadow_global_consts_CPU[i].view =
                    lightViewRow.Transpose();
                manager->shadow_global_consts_CPU[i].proj =
                    lightProjRow.Transpose();
                manager->shadow_global_consts_CPU[i].invProj =
                    lightProjRow.Invert().Transpose();
                manager->shadow_global_consts_CPU[i].viewProj =
                    (lightViewRow * lightProjRow).Transpose();

                // LIGHT_FRUSTUM_WIDTH 확인
                // Vector4 eye(0.0f, 0.0f, 0.0f, 1.0f);
                // Vector4 xLeft(-1.0f, -1.0f, 0.0f, 1.0f);
                // Vector4 xRight(1.0f, 1.0f, 0.0f, 1.0f);
                // eye = Vector4::Transform(eye, lightProjRow);
                // xLeft = Vector4::Transform(xLeft, lightProjRow.Invert());
                // xRight = Vector4::Transform(xRight, lightProjRow.Invert());
                // xLeft /= xLeft.w;
                // xRight /= xRight.w;
                // cout << "LIGHT_FRUSTUM_WIDTH = " << xRight.x - xLeft.x <<
                // endl;

                GraphicsUtil::UpdateBuffer(
                    GraphicsManager::Instance().device_context,
                    manager->shadow_global_consts_CPU[i],
                    manager->shadow_global_consts_GPU[i]);

                // 그림자를 실제로 렌더링할 때 필요
                manager->global_consts_CPU.lights[i].viewProj =
                    manager->shadow_global_consts_CPU[i].viewProj;
                manager->global_consts_CPU.lights[i].invProj =
                    manager->shadow_global_consts_CPU[i].invProj;

                // 반사된 장면에서도 그림자를 그리고 싶다면 조명도 반사시켜서
                // 넣어주면 됩니다.
            }
        }

        // 조명의 위치 반영
        for (int i = 0; i < MAX_LIGHTS; i++) {
            auto renderer = (Renderer *)manager->light_spheres[i]->GetComponent(
                EnumComponentType::eRenderer);

            renderer->UpdateWorldRow(
                Matrix::CreateScale((std::max)(
                    0.01f, manager->global_consts_CPU.lights[i].radius)) *
                Matrix::CreateTranslation(
                    manager->global_consts_CPU.lights[i].position));
        }
        return common::EnumBehaviorTreeStatus::eSuccess;
    }
};

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

class UpdateMirrorNode : public common::BehaviorActionNode {
    common::EnumBehaviorTreeStatus OnInvoke() override {

        auto black_board = dynamic_cast<BlackBoard *>(data_block);
        assert(black_board != nullptr);

        auto manager = black_board->render_block;

        auto renderer = (Renderer *)manager->mirror->GetComponent(
            EnumComponentType::eRenderer);

        renderer->UpdateConstantBuffers(
            GraphicsManager::Instance().device,
            GraphicsManager::Instance().device_context);

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