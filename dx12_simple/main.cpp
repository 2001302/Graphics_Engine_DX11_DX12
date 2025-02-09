#pragma once

#include <engine.h>

namespace graphics {
class SimpleDx12 : public Engine {
  public:
    void LoadAsset(BlackBoard *black_board) override {

        auto condition = black_board->conditions.get();
        auto targets = black_board->targets.get();

        // sample object
        {
            MeshData mesh = GeometryGenerator::MakeSphere(0.2f, 200, 200);

            auto renderer = std::make_shared<MeshRenderer>();
            renderer->Initialize(std::vector{mesh});
            renderer->UpdateWorldRow(
                Matrix::CreateTranslation(Vector3(0.5f, 0.5f, 2.0f)));
            renderer->MaterialConsts().GetCpu().SetMaterialConstants(
                Vector3(0.1f, 0.1f, 1.0f), 0.2f, 0.6f, Vector3(0.0f));
            renderer->UpdateConstantBuffers();

            auto obj = std::make_shared<common::Model>();
            obj->TryAdd(renderer);

            targets->objects.insert({obj->GetEntityId(), obj});
        }

        // ground
        {
            auto mesh = GeometryGenerator::MakeSquare(5.0);

            auto component = std::make_shared<MirrorRenderer>();
            component->Initialize(std::vector{mesh});
            component->MaterialConsts().GetCpu().SetMaterialConstants(
                Vector3(0.1f), 0.5f, 0.3f, Vector3(0.0f));

            Vector3 position = Vector3(0.0f, -0.5f, 2.0f);
            component->UpdateWorldRow(Matrix::CreateRotationX(PI * 0.5f) *
                                      Matrix::CreateTranslation(position));

            component->SetMirrorPlane(DirectX::SimpleMath::Plane(
                position, Vector3(0.0f, 1.0f, 0.0f)));
            component->SetMirrorAlpha(0.1f);

            targets->world->TryAdd(component);
        }
    }
};
} // namespace graphics

void main() {
    std::unique_ptr<graphics::SimpleDx12> engine =
        std::make_unique<graphics::SimpleDx12>();

    if (engine->Start()) {
        engine->Run();
    }

    engine->Stop();
}
