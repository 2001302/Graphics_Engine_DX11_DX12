#ifndef _RENDERER_UPDATE_NODE
#define _RENDERER_UPDATE_NODE

#include "behavior_tree_builder.h"
#include "rendering_block.h"
#include "renderer.h"

namespace engine {

class UpdateCamera : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
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

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        float dt = ImGui::GetIO().DeltaTime;

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

                GraphicsUtil::UpdateBuffer(GraphicsManager::Instance().device,
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
            Renderer *renderer = nullptr;
            manager->light_spheres[i]->GetComponent(
                EnumComponentType::eRenderer, (Component **)(&renderer));

            renderer->UpdateWorldRow(
                Matrix::CreateScale((std::max)(
                    0.01f, manager->global_consts_CPU.lights[i].radius)) *
                Matrix::CreateTranslation(
                    manager->global_consts_CPU.lights[i].position));
        }
        return EnumBehaviorTreeStatus::eSuccess;
    }

  private:
    float dt;
};

class UpdateGlobalConstantBuffers : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

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

        GraphicsUtil::UpdateBuffer(GraphicsManager::Instance().device,
                                   GraphicsManager::Instance().device_context,
                                   manager->global_consts_CPU,
                                   manager->global_consts_GPU);

        GraphicsUtil::UpdateBuffer(GraphicsManager::Instance().device,
                                   GraphicsManager::Instance().device_context,
                                   manager->reflect_global_consts_CPU,
                                   manager->reflect_global_consts_GPU);

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class UpdateMirror : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        Renderer *renderer = nullptr;
        manager->mirror->GetComponent(EnumComponentType::eRenderer,
                                         (Component **)(&renderer));
        renderer->UpdateConstantBuffers(GraphicsManager::Instance().device,
            GraphicsManager::Instance().device_context);

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class ApplyMouseMovement : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
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

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        for (auto &i : manager->models) {

            Renderer *renderer = nullptr;
            i.second->GetComponent(EnumComponentType::eRenderer,
                            (Component **)(&renderer));
            renderer->UpdateConstantBuffers(GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context);
        }

        return EnumBehaviorTreeStatus::eSuccess;
    }
};

class UpdateLightSpheres : public BehaviorActionNode {
    EnumBehaviorTreeStatus OnInvoke() override {

        auto manager = dynamic_cast<RenderingBlock *>(
            data_block[EnumDataBlockType::eManager]);
        assert(manager != nullptr);

        for (auto &i : manager->light_spheres) {

            Renderer *renderer = nullptr;
            i->GetComponent(EnumComponentType::eRenderer,
                            (Component **)(&renderer));
            renderer->UpdateConstantBuffers(
                GraphicsManager::Instance().device,
                GraphicsManager::Instance().device_context);
        }
        return EnumBehaviorTreeStatus::eSuccess;
    }
};
}

#endif