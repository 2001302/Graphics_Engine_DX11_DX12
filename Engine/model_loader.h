#pragma once

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>
#include <iostream>
#include <string>
#include <vector>

#include "common_struct.h"

namespace dx11 {

using namespace DirectX::SimpleMath;

class ModelLoader {
  public:
    void Load(std::string basePath, std::string filename);

    void ProcessNode(aiNode *node, const aiScene *scene,
                     DirectX::SimpleMath::Matrix tr);

    Mesh ProcessMesh(aiMesh *mesh, const aiScene *scene);

    void UpdateTangents();
    std::string ReadFilename(aiMaterial *material, aiTextureType type);
  public:
    std::string basePath;
    std::vector<Mesh> meshes;
};
} // namespace Engine
