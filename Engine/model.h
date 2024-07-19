#ifndef _MODEL
#define _MODEL

#include "node_ui.h"
#include "mesh.h"
#include "bone.h"
#include "phong_shader.h"
#include "physically_based_shader.h"

namespace dx11 {
using Microsoft::WRL::ComPtr;

enum EnumRenderMode { eLight = 0, ePhysicallyBasedRendering = 1 };

class Model : public common::INodeUi {
  public:
    Model();
    ~Model();

    int GetIndexCount();

    Vector3 translation = Vector3(0.0f);
    Vector3 rotation = Vector3(0.0f);
    Vector3 scaling = Vector3(1.0f);

    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<std::shared_ptr<Bone>> bones;
    // std::shared_ptr<Animation> animation;

    EnumShaderType shader_type;
    EnumRenderMode render_mode;
  private:
    void OnShow() override;
};

} // namespace dx11
#endif
