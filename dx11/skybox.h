#ifndef _SKYBOX
#define _SKYBOX

#include <model.h>

namespace graphics {
struct Skybox {
    std::shared_ptr<common::Model> model;
    ComPtr<ID3D11ShaderResourceView> env_SRV;
    ComPtr<ID3D11ShaderResourceView> irradiance_SRV;
    ComPtr<ID3D11ShaderResourceView> specular_SRV;
    ComPtr<ID3D11ShaderResourceView> brdf_SRV;
};
} // namespace engine
#endif
