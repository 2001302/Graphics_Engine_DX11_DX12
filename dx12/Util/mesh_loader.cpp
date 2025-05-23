#include "mesh_loader.h"
#include "../structure/mesh.h"

#include <DirectXMesh.h>
#include <filesystem>
#include <vector>

namespace graphics {

using namespace std;
using namespace DirectX::SimpleMath;

void UpdateNormals(vector<MeshData> &meshes) {

    for (auto &m : meshes) {

        vector<Vector3> normalsTemp(m.vertices.size(), Vector3(0.0f));
        vector<float> weightsTemp(m.vertices.size(), 0.0f);

        for (int i = 0; i < m.indices.size(); i += 3) {

            int idx0 = m.indices[i];
            int idx1 = m.indices[i + 1];
            int idx2 = m.indices[i + 2];

            auto v0 = m.vertices[idx0];
            auto v1 = m.vertices[idx1];
            auto v2 = m.vertices[idx2];

            auto faceNormal =
                (v1.position - v0.position).Cross(v2.position - v0.position);

            normalsTemp[idx0] += faceNormal;
            normalsTemp[idx1] += faceNormal;
            normalsTemp[idx2] += faceNormal;
            weightsTemp[idx0] += 1.0f;
            weightsTemp[idx1] += 1.0f;
            weightsTemp[idx2] += 1.0f;
        }

        for (int i = 0; i < m.vertices.size(); i++) {
            if (weightsTemp[i] > 0.0f) {
                m.vertices[i].normal = normalsTemp[i] / weightsTemp[i];
                m.vertices[i].normal.Normalize();
            }
        }
    }
}

string GetExtension(const string filename) {
    string ext(filesystem::path(filename).extension().string());
    transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

void MeshLoader::ReadAnimation(const aiScene *pScene) {

    animation_data.clips.resize(pScene->mNumAnimations);

    for (uint32_t i = 0; i < pScene->mNumAnimations; i++) {

        auto &clip = animation_data.clips[i];

        const aiAnimation *ani = pScene->mAnimations[i];

        clip.duration = ani->mDuration / 100.0f;
        clip.ticksPerSec = ani->mTicksPerSecond;
        clip.keys.resize(animation_data.boneNameToId.size());
        clip.numChannels = ani->mNumChannels;
        clip.numKeys = clip.keys.size();

        for (uint32_t c = 0; c < ani->mNumChannels; c++) {
            const aiNodeAnim *nodeAnim = ani->mChannels[c];
            const int boneId =
                animation_data.boneNameToId[nodeAnim->mNodeName.C_Str()];
            clip.keys[boneId].resize(nodeAnim->mNumPositionKeys);
            for (uint32_t k = 0; k < nodeAnim->mNumPositionKeys; k++) {
                const auto pos = nodeAnim->mPositionKeys[k].mValue;
                const auto rot = nodeAnim->mRotationKeys[k].mValue;
                const auto scale = nodeAnim->mScalingKeys[k].mValue;
                auto &key = clip.keys[boneId][k];
                key.pos = {pos.x, pos.y, pos.z};
                key.rot = Quaternion(rot.x, rot.y, rot.z, rot.w);
                key.scale = {scale.x, scale.y, scale.z};
            }
        }
    }
}

void MeshLoader::Load(std::string basePath, std::string filename,
                       bool revertNormals) {

    if (GetExtension(filename) == ".gltf") {
        is_GLTF = true;
        revert_normals = revertNormals;
    }

    base_path = basePath;

    Assimp::Importer importer;

    const aiScene *pScene = importer.ReadFile(
        base_path + filename,
        aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
    if (pScene) {

        FindDeformingBones(pScene);

        int counter = 0;
        UpdateBoneIDs(pScene->mRootNode, &counter);

        animation_data.boneIdToName.resize(animation_data.boneNameToId.size());
        for (auto &i : animation_data.boneNameToId)
            animation_data.boneIdToName[i.second] = i.first;

        animation_data.boneParents.resize(animation_data.boneNameToId.size(), -1);

        Matrix tr; // Initial transformation
        ProcessNode(pScene->mRootNode, pScene, tr);

        if (pScene->HasAnimations())
            ReadAnimation(pScene);

        UpdateTangents();
    } else {
        std::cout << "Failed to read file: " << base_path + filename
                  << std::endl;
        auto errorDescription = importer.GetErrorString();
        std::cout << "Assimp error: " << errorDescription << endl;
    }
}

void MeshLoader::LoadAnimation(string basePath, string filename) {

    base_path = basePath;

    Assimp::Importer importer;

    const aiScene *pScene = importer.ReadFile(
        base_path + filename,
        aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

    if (pScene && pScene->HasAnimations()) {
        ReadAnimation(pScene);
    } else {
        std::cout << "Failed to read animation from file: "
                  << base_path + filename << std::endl;
        auto errorDescription = importer.GetErrorString();
        std::cout << "Assimp error: " << errorDescription << endl;
    }
}

void MeshLoader::UpdateTangents() {

    using namespace std;
    using namespace DirectX;

    for (auto &m : this->meshes) {

        vector<XMFLOAT3> positions(m.vertices.size());
        vector<XMFLOAT3> normals(m.vertices.size());
        vector<XMFLOAT2> texcoords(m.vertices.size());
        vector<XMFLOAT3> tangents(m.vertices.size());
        vector<XMFLOAT3> bitangents(m.vertices.size());

        for (size_t i = 0; i < m.vertices.size(); i++) {
            auto &v = m.vertices[i];
            positions[i] = v.position;
            normals[i] = v.normal;
            texcoords[i] = v.texcoord;
        }

        ComputeTangentFrame(m.indices.data(), m.indices.size() / 3,
                            positions.data(), normals.data(), texcoords.data(),
                            m.vertices.size(), tangents.data(),
                            bitangents.data());

        for (size_t i = 0; i < m.vertices.size(); i++) {
            m.vertices[i].tangent = tangents[i];
        }

        if (m.skinned_vertices.size() > 0) {
            for (size_t i = 0; i < m.skinned_vertices.size(); i++) {
                m.skinned_vertices[i].tangentModel = tangents[i];
            }
        }
    }
}
void MeshLoader::FindDeformingBones(const aiScene *scene) {
    for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
        const auto *mesh = scene->mMeshes[i];
        if (mesh->HasBones()) {
            for (uint32_t i = 0; i < mesh->mNumBones; i++) {
                const aiBone *bone = mesh->mBones[i];

                animation_data.boneNameToId[bone->mName.C_Str()] = -1;
            }
        }
    }
}

const aiNode *MeshLoader::FindParent(const aiNode *node) {
    if (!node)
        return nullptr;
    if (animation_data.boneNameToId.count(node->mName.C_Str()) > 0)
        return node;
    return FindParent(node->mParent);
}

void MeshLoader::ProcessNode(aiNode *node, const aiScene *scene, Matrix tr) {

    if (node->mParent && animation_data.boneNameToId.count(node->mName.C_Str()) &&
        FindParent(node->mParent)) {
        const auto boneId = animation_data.boneNameToId[node->mName.C_Str()];
        animation_data.boneParents[boneId] =
            animation_data.boneNameToId[FindParent(node->mParent)->mName.C_Str()];
    }

    Matrix m(&node->mTransformation.a1);
    m = m.Transpose() * tr;

    for (UINT i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        auto newMesh = this->ProcessMesh(mesh, scene);
        for (auto &v : newMesh.vertices) {
            v.position = DirectX::SimpleMath::Vector3::Transform(v.position, m);
        }
        meshes.push_back(newMesh);
    }

    for (UINT i = 0; i < node->mNumChildren; i++) {
        this->ProcessNode(node->mChildren[i], scene, m);
    }
}

string MeshLoader::ReadTextureFilename(const aiScene *scene,
                                        aiMaterial *material,
                                        aiTextureType type) {

    if (material->GetTextureCount(type) > 0) {
        aiString filepath;
        material->GetTexture(type, 0, &filepath);

        string fullPath =
            base_path +
            string(filesystem::path(filepath.C_Str()).filename().string());

        if (!filesystem::exists(fullPath)) {
            const aiTexture *texture =
                scene->GetEmbeddedTexture(filepath.C_Str());
            if (texture) {
                if (string(texture->achFormatHint).find("png") !=
                    string::npos) {
                    std::ofstream fs(fullPath.c_str(), ios::binary | ios::out);
                    fs.write((char *)texture->pcData, texture->mWidth);
                    fs.close();
                }
            } else {
                cout << fullPath << " doesn't exists. Return empty filename."
                     << endl;
            }
        } else {
            return fullPath;
        }

        return fullPath;

    } else {
        return "";
    }
}

MeshData MeshLoader::ProcessMesh(aiMesh *mesh, const aiScene *scene) {

    MeshData newMesh;
    auto &vertices = newMesh.vertices;
    auto &indices = newMesh.indices;
    auto &skinnedVertices = newMesh.skinned_vertices;

    // Walk through each of the mesh's vertices
    for (UINT i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        vertex.normal.x = mesh->mNormals[i].x;
        if (is_GLTF) {
            vertex.normal.y = mesh->mNormals[i].z;
            vertex.normal.z = -mesh->mNormals[i].y;
        } else {
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }

        if (revert_normals) {
            vertex.normal *= -1.0f;
        }

        vertex.normal.Normalize();

        if (mesh->mTextureCoords[0]) {
            vertex.texcoord.x = (float)mesh->mTextureCoords[0][i].x;
            vertex.texcoord.y = (float)mesh->mTextureCoords[0][i].y;
        }

        vertices.push_back(vertex);
    }

    for (UINT i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (UINT j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    if (mesh->HasBones()) {

        vector<vector<float>> boneWeights(vertices.size());
        vector<vector<uint8_t>> boneIndices(vertices.size());

        animation_data.offsetMatrices.resize(animation_data.boneNameToId.size());
        animation_data.boneTransforms.resize(animation_data.boneNameToId.size());

        int count = 0;
        for (uint32_t i = 0; i < mesh->mNumBones; i++) {
            const aiBone *bone = mesh->mBones[i];

            // cout << "BoneMap " << count++ << " : " << bone->mName.C_Str()
            //     << " NumBoneWeights = " << bone->mNumWeights << endl;

            const uint32_t boneId = animation_data.boneNameToId[bone->mName.C_Str()];

            animation_data.offsetMatrices[boneId] =
                Matrix((float *)&bone->mOffsetMatrix).Transpose();

            for (uint32_t j = 0; j < bone->mNumWeights; j++) {
                aiVertexWeight weight = bone->mWeights[j];
                assert(weight.mVertexId < boneIndices.size());
                boneIndices[weight.mVertexId].push_back(boneId);
                boneWeights[weight.mVertexId].push_back(weight.mWeight);
            }
        }

        int maxBones = 0;
        for (int i = 0; i < boneWeights.size(); i++) {
            maxBones = (std::max)(maxBones, int(boneWeights[i].size()));
        }

        cout << "Max number of influencing bones per vertex = " << maxBones
             << endl;

        skinnedVertices.resize(vertices.size());
        for (int i = 0; i < vertices.size(); i++) {
            skinnedVertices[i].position = vertices[i].position;
            skinnedVertices[i].normalModel = vertices[i].normal;
            skinnedVertices[i].texcoord = vertices[i].texcoord;

            for (int j = 0; j < boneWeights[i].size(); j++) {
                skinnedVertices[i].blendWeights[j] = boneWeights[i][j];
                skinnedVertices[i].boneIndices[j] = boneIndices[i][j];
            }
        }

        // for (int i = 0; i < boneWeights.size(); i++) {
        //    cout << boneWeights[i].size() << " : ";
        //    for (int j = 0; j < boneWeights[i].size(); j++) {
        //        cout << boneWeights[i][j] << " ";
        //    }
        //    cout << " | ";
        //    for (int j = 0; j < boneIndices[i].size(); j++) {
        //        cout << int(boneIndices[i][j]) << " ";
        //    }
        //    cout << endl;
        //}
    }

    // http://assimp.sourceforge.net/lib_html/materials.html
    if (mesh->mMaterialIndex >= 0) {

        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        newMesh.albedo_name =
            ReadTextureFilename(scene, material, aiTextureType_BASE_COLOR);
        if (newMesh.albedo_name.empty()) {
            newMesh.albedo_name =
                ReadTextureFilename(scene, material, aiTextureType_DIFFUSE);
        }
        newMesh.emissive_name =
            ReadTextureFilename(scene, material, aiTextureType_EMISSIVE);
        newMesh.height_name =
            ReadTextureFilename(scene, material, aiTextureType_HEIGHT);
        newMesh.normal_name =
            ReadTextureFilename(scene, material, aiTextureType_NORMALS);
        newMesh.metallic_name =
            ReadTextureFilename(scene, material, aiTextureType_METALNESS);
        newMesh.roughness_name = ReadTextureFilename(
            scene, material, aiTextureType_DIFFUSE_ROUGHNESS);
        newMesh.ambient_occlusion_name = ReadTextureFilename(
            scene, material, aiTextureType_AMBIENT_OCCLUSION);
        if (newMesh.ambient_occlusion_name.empty()) {
            newMesh.ambient_occlusion_name =
                ReadTextureFilename(scene, material, aiTextureType_LIGHTMAP);
        }
        newMesh.opacity_name =
            ReadTextureFilename(scene, material, aiTextureType_OPACITY);

        if (!newMesh.opacity_name.empty()) {
            cout << newMesh.albedo_name << endl;
            cout << "Opacity " << newMesh.opacity_name << endl;
        }

        // for (size_t i = 0; i < 22; i++) {
        //    cout << i << " " << ReadTextureFilename(material,
        //    aiTextureType(i))
        //         << endl;
        //}
    }

    return newMesh;
}

} // namespace core
