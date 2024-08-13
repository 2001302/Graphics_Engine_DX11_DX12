#ifndef _TONEMAPPING
#define _TONEMAPPING

#include "constant_buffer.h"
#include "env.h"
#include "mesh.h"
#include "component.h"

namespace core {
struct ImageFilterConstData {
    float dx;
    float dy;
    float threshold;
    float strength;
    float option1;
    float option2;
    float option3;
    float option4;
};

class ToneMapping : public Component {
  public:
    void Initialize();

    void Render(ComPtr<ID3D11Device> &device,
                ComPtr<ID3D11DeviceContext> &context,
                ComPtr<ID3D11Buffer> const_buffer);

  private:
    std::shared_ptr<Mesh> mesh;
};
} // namespace engine
#endif