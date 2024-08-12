#ifndef _SKYBOX
#define _SKYBOX

#include "model.h"

namespace engine {
struct Skybox {
    std::shared_ptr<Model> model;
    ComPtr<ID3D11ShaderResourceView> env_SRV;
    ComPtr<ID3D11ShaderResourceView> irradiance_SRV;
    ComPtr<ID3D11ShaderResourceView> specular_SRV;
    ComPtr<ID3D11ShaderResourceView> brdf_SRV;
};
} // namespace engine
#endif
