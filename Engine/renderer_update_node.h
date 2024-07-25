#ifndef _RENDERER_UPDATE_NODE
#define _RENDERER_UPDATE_NODE

#include "behavior_tree_builder.h"
#include "pipeline_manager.h"
#include "renderer.h"

namespace engine {

class UpdateCamera : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<PipelineManager *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        manager->camera->Update();

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class UpdateLights : public BehaviorActionNode {
  public:
    UpdateLights(float dt) { this->dt = dt; }
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<PipelineManager *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        float dt = ImGui::GetIO().DeltaTime;

        // 회전하는 lights[1] 업데이트
        static Vector3 lightDev = Vector3(1.0f, 0.0f, 0.0f);
        if (manager->m_lightRotate) {
            lightDev = Vector3::Transform(
                lightDev, Matrix::CreateRotationY(dt * 3.141592f * 0.5f));
        }
        manager->m_globalConstsCPU.lights[1].position =
            Vector3(0.0f, 1.1f, 2.0f) + lightDev;
        Vector3 focusPosition = Vector3(0.0f, -0.5f, 1.7f);
        manager->m_globalConstsCPU.lights[1].direction =
            focusPosition - manager->m_globalConstsCPU.lights[1].position;
        manager->m_globalConstsCPU.lights[1].direction.Normalize();

        // 그림자맵을 만들기 위한 시점
        for (int i = 0; i < MAX_LIGHTS; i++) {
            const auto &light = manager->m_globalConstsCPU.lights[i];
            if (light.type & LIGHT_SHADOW) {

                Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
                if (abs(up.Dot(light.direction) + 1.0f) < 1e-5)
                    up = Vector3(1.0f, 0.0f, 0.0f);

                // 그림자맵을 만들 때 필요
                Matrix lightViewRow = DirectX::XMMatrixLookAtLH(
                    light.position, light.position + light.direction, up);

                Matrix lightProjRow = DirectX::XMMatrixPerspectiveFovLH(
                    DirectX::XMConvertToRadians(120.0f), 1.0f, 0.1f, 10.0f);

                manager->m_shadowGlobalConstsCPU[i].eyeWorld = light.position;
                manager->m_shadowGlobalConstsCPU[i].view =
                    lightViewRow.Transpose();
                manager->m_shadowGlobalConstsCPU[i].proj =
                    lightProjRow.Transpose();
                manager->m_shadowGlobalConstsCPU[i].invProj =
                    lightProjRow.Invert().Transpose();
                manager->m_shadowGlobalConstsCPU[i].viewProj =
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

                GraphicsUtil::UpdateBuffer(GraphicsManager::Instance().device,
                    GraphicsManager::Instance().device_context,
                                           manager->m_shadowGlobalConstsCPU[i],
                                           manager->m_shadowGlobalConstsGPU[i]);

                // 그림자를 실제로 렌더링할 때 필요
                manager->m_globalConstsCPU.lights[i].viewProj =
                    manager->m_shadowGlobalConstsCPU[i].viewProj;
                manager->m_globalConstsCPU.lights[i].invProj =
                    manager->m_shadowGlobalConstsCPU[i].invProj;

                // 반사된 장면에서도 그림자를 그리고 싶다면 조명도 반사시켜서
                // 넣어주면 됩니다.
            }
        }

        // 조명의 위치 반영
        for (int i = 0; i < MAX_LIGHTS; i++) {
            Renderer *renderer = nullptr;
            manager->m_lightSphere[i]->GetComponent(
                EnumComponentType::eRenderer, (Component **)(&renderer));

            renderer->UpdateWorldRow(
                Matrix::CreateScale((std::max)(
                    0.01f, manager->m_globalConstsCPU.lights[i].radius)) *
                Matrix::CreateTranslation(
                    manager->m_globalConstsCPU.lights[i].position));
        }
        return EnumBehaviorTreeStatus::eSuccess;
    }

  private:
    float dt;
};

class UpdateGlobalConstantBuffers : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<PipelineManager *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        const Vector3 eyeWorld = manager->camera->GetPosition();
        const Matrix reflectRow =
            Matrix::CreateReflection(manager->m_mirrorPlane);
        const Matrix viewRow = manager->camera->GetView();
        const Matrix projRow = manager->camera->GetProjection();

        manager->m_globalConstsCPU.eyeWorld = eyeWorld;
        manager->m_globalConstsCPU.view = viewRow.Transpose();
        manager->m_globalConstsCPU.proj = projRow.Transpose();
        manager->m_globalConstsCPU.invProj = projRow.Invert().Transpose();
        manager->m_globalConstsCPU.viewProj = (viewRow * projRow).Transpose();

        // 그림자 렌더링에 사용
        manager->m_globalConstsCPU.invViewProj =
            manager->m_globalConstsCPU.viewProj.Invert();

        manager->m_reflectGlobalConstsCPU = manager->m_globalConstsCPU;
        memcpy(&manager->m_reflectGlobalConstsCPU, &manager->m_globalConstsCPU,
               sizeof(manager->m_globalConstsCPU));
        manager->m_reflectGlobalConstsCPU.view =
            (reflectRow * viewRow).Transpose();
        manager->m_reflectGlobalConstsCPU.viewProj =
            (reflectRow * viewRow * projRow).Transpose();

        // 그림자 렌더링에 사용 (TODO: 광원의 위치도 반사시킨 후에 계산해야 함)
        manager->m_reflectGlobalConstsCPU.invViewProj =
            manager->m_reflectGlobalConstsCPU.viewProj.Invert();

        GraphicsUtil::UpdateBuffer(GraphicsManager::Instance().device,
                                   GraphicsManager::Instance().device_context,
                                   manager->m_globalConstsCPU,
                                   manager->m_globalConstsGPU);

        GraphicsUtil::UpdateBuffer(GraphicsManager::Instance().device,
                                   GraphicsManager::Instance().device_context,
                                   manager->m_reflectGlobalConstsCPU,
                                   manager->m_reflectGlobalConstsGPU);

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class UpdateMirror : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<PipelineManager *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        Renderer *renderer = nullptr;
        manager->m_mirror->GetComponent(EnumComponentType::eRenderer,
                                         (Component **)(&renderer));
        renderer->UpdateConstantBuffers(GraphicsManager::Instance().device,
            GraphicsManager::Instance().device_context);

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class ApplyMouseMovement : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<PipelineManager *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

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
        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class UpdateBasicObjects : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<PipelineManager *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        for (auto &i : manager->m_basicList) {

            Renderer *renderer = nullptr;
            i->GetComponent(EnumComponentType::eRenderer,
                            (Component **)(&renderer));
            renderer->UpdateConstantBuffers(GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context);
        }

        return EnumBehaviorTreeStatus::eSuccess;
    }
};
}

#endif