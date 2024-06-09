#pragma once

#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>
#include <iostream>
#include <string>
#include <vector>

#include "CommonStruct.h"

namespace Engine {

    using namespace DirectX::SimpleMath; 

    class ModelLoader {
    public:
        void Load(std::string basePath, std::string filename);

        void ProcessNode(aiNode* node, const aiScene* scene,
            DirectX::SimpleMath::Matrix tr);

        Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

    public:
        std::string basePath;
        std::vector<Mesh> meshes;
    };
}