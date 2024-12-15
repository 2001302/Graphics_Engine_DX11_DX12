#include <engine.h>

namespace graphics {

class SimpleDx11 : public Engine {
  public:
    void LoadAsset(BlackBoard *black_board) override {

        auto targets = black_board->targets.get();

        //skybox
        {
            auto mesh_data = GeometryGenerator::MakeBox(40.0f);
            std::reverse(mesh_data.indices.begin(), mesh_data.indices.end());

            auto renderer =
                std::make_shared<SkyboxRenderer>(std::vector{mesh_data});
            renderer->CreateCubeMap(
                L"../Assets/Textures/Cubemaps/HDRI/SampleEnvHDR.dds",
                L"../Assets/Textures/Cubemaps/HDRI/SampleSpecularHDR.dds",
                L"../Assets/Textures/Cubemaps/HDRI/SampleDiffuseHDR.dds",
                L"../Assets/Textures/Cubemaps/HDRI/SampleBrdf.dds");

            targets->skybox->TryAdd(renderer);
        }

        //mirror
        {
            auto mesh = GeometryGenerator::MakeSquare(5.0);
            auto mirror = std::make_shared<MirrorRenderer>(std::vector{mesh});

            mirror->material_consts.GetCpu().albedoFactor = Vector3(0.1f);
            mirror->material_consts.GetCpu().emissionFactor = Vector3(0.0f);
            mirror->material_consts.GetCpu().metallicFactor = 0.5f;
            mirror->material_consts.GetCpu().roughnessFactor = 0.3f;

            Vector3 position = Vector3(0.0f, -0.5f, 2.0f);
            mirror->UpdateWorldRow(Matrix::CreateRotationX(3.141592f * 0.5f) *
                                   Matrix::CreateTranslation(position));

            mirror->SetMirrorPlane(DirectX::SimpleMath::Plane(
                position, Vector3(0.0f, 1.0f, 0.0f)));

            targets->ground->TryAdd(mirror);
        }
    }
};
} // namespace graphics
void main() {
    std::unique_ptr<graphics::SimpleDx11> engine =
        std::make_unique<graphics::SimpleDx11>();

    if (engine->Start()) {
        engine->Run();
    }

    engine->Stop();
}
