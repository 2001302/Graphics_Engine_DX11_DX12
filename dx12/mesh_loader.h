#pragma once

#include "animation_clip.h"
#include "mesh.h"
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
            if (m_aniData.boneNameToId.count(node->mName.C_Str())) {
                m_aniData.boneNameToId[node->mName.C_Str()] = *counter;
                *counter += 1;
            }
            for (UINT i = 0; i < node->mNumChildren; i++) {
                UpdateBoneIDs(node->mChildren[i], counter);
            }
        }
    }

  public:
    string m_basePath;
    vector<MeshData> m_meshes;

    AnimationData m_aniData;

    bool m_isGLTF = false; // gltf or fbx
    bool m_revertNormals = false;
};
} // namespace core
