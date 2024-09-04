#pragma once

#include "animation_clip.h"
#include "mesh.h"
#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\scene.h>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace core {

using std::map;
using std::string;
using std::vector;

class ModelLoader {
  public:
    void Load(string basePath, string filename, bool revertNormals);
    void LoadAnimation(string basePath, string filename);

    const aiNode *FindParent(const aiNode *node);

    void ProcessNode(aiNode *node, const aiScene *scene,
                     DirectX::SimpleMath::Matrix tr);

    dx12::MeshData ProcessMesh(aiMesh *mesh, const aiScene *scene);

    void ReadAnimation(const aiScene *scene);

    std::string ReadTextureFilename(const aiScene *scene, aiMaterial *material,
                                    aiTextureType type);

    void UpdateTangents();

    // 踰꾪뀓?ㅼ쓽 蹂?뺤뿉 吏곸젒?곸쑝濡?李몄뿬?섎뒗 堉덈뱾??紐⑸줉??留뚮벊?덈떎.
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
    vector<dx12::MeshData> m_meshes;

    AnimationData m_aniData;

    bool m_isGLTF = false; // gltf or fbx
    bool m_revertNormals = false;
};
} // namespace hlab
