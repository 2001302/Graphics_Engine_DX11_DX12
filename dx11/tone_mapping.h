#ifndef _TONEMAPPING
#define _TONEMAPPING

#include "constant_buffer.h"
#include "mesh.h"
#include <structure/component.h>
#include <structure/env.h>

namespace graphics {
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

class ToneMapping : public common::Component {
  public:
    void Initialize();
    void Render(ComPtr<ID3D11Buffer> const_buffer);

  private:
    std::shared_ptr<Mesh> mesh;
};
} // namespace graphics
#endif