#pragma once

#include "../structure/animation_clip.h"
#include "../structure/mesh.h"
#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace graphics {

using std::map;
using std::string;
using std::vector;

class MeshLoader {
  public:
    void Load(string basePath, string filename, bool revertNormals);
    void LoadAnimation(string basePath, string filename);

    const aiNode *FindParent(const aiNode *node);

    void ProcessNode(aiNode *node, const aiScene *scene,
                     DirectX::SimpleMath::Matrix tr);

    MeshData ProcessMesh(aiMesh *mesh, const aiScene *scene);

    void ReadAnimation(const aiScene *scene);

    std::string ReadTextureFilename(const aiScene *scene, aiMaterial *material,
                                    aiTextureType type);

    void UpdateTangents();

    void FindDeformingBones(const aiScene *scene);
    void UpdateBoneIDs(aiNode *node, int *counter) {
        static int id = 0;
        if (node) {
            if (animation_data.boneNameToId.count(node->mName.C_Str())) {
                animation_data.boneNameToId[node->mName.C_Str()] = *counter;
                *counter += 1;
            }
            for (UINT i = 0; i < node->mNumChildren; i++) {
                UpdateBoneIDs(node->mChildren[i], counter);
            }
        }
    }

  public:
    string base_path;
    vector<MeshData> meshes;

    AnimationData animation_data;

    bool is_GLTF = false; // gltf or fbx
    bool revert_normals = false;
};
} // namespace core
