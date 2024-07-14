#include "model_loader.h"
#include "mesh.h"

#include <DirectXMesh.h>
#include <filesystem>
#include <vector>

using namespace dx11;
using namespace DirectX::SimpleMath;

void ModelLoader::Load(std::string basePath, std::string filename) {
    this->basePath = basePath;

    Assimp::Importer importer;

    const aiScene *pScene = importer.ReadFile(
        this->basePath + filename,
        aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

    Matrix tr; // Initial transformation
    ProcessNode(pScene->mRootNode, pScene, tr);

    // 노멀 벡터가 없는 경우를 대비하여 다시 계산
    // 한 위치에는 한 버텍스만 있어야 연결 관계를 찾을 수 있음
    /* for (auto &m : this->meshes) {

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
    }*/

    UpdateTangents();
}

void ModelLoader::UpdateTangents() {

    using namespace std;
    using namespace DirectX;

    // https://github.com/microsoft/DirectXMesh/wiki/ComputeTangentFrame

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
    }
}

void ModelLoader::ProcessNode(aiNode *node, const aiScene *scene, Matrix tr) {

    // std::cout << node->mName.C_Str() << " : " << node->mNumMeshes << " "
    //           << node->mNumChildren << std::endl;

    Matrix m;
    ai_real *temp = &node->mTransformation.a1;
    float *mTemp = &m._11;
    for (int t = 0; t < 16; t++) {
        mTemp[t] = float(temp[t]);
    }
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

Mesh ModelLoader::ProcessMesh(aiMesh *mesh, const aiScene *scene) {
    // Data to fill
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    // Walk through each of the mesh's vertices
    for (UINT i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        vertex.normal.x = mesh->mNormals[i].x;
        vertex.normal.y = mesh->mNormals[i].y;
        vertex.normal.z = mesh->mNormals[i].z;
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

    Mesh newMesh;
    newMesh.vertices = vertices;
    newMesh.indices = indices;

    // http://assimp.sourceforge.net/lib_html/materials.html
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        newMesh.textureFilename = ReadFilename(material, aiTextureType_DIFFUSE);
        newMesh.albedoTextureFilename =
            ReadFilename(material, aiTextureType_BASE_COLOR);
        newMesh.emissiveTextureFilename =
            ReadFilename(material, aiTextureType_EMISSIVE);
        newMesh.heightTextureFilename =
            ReadFilename(material, aiTextureType_HEIGHT);
        newMesh.normalTextureFilename =
            ReadFilename(material, aiTextureType_NORMALS);
        newMesh.metallicTextureFilename =
            ReadFilename(material, aiTextureType_METALNESS);
        newMesh.roughnessTextureFilename =
            ReadFilename(material, aiTextureType_DIFFUSE_ROUGHNESS);
        newMesh.aoTextureFilename =
            ReadFilename(material, aiTextureType_AMBIENT_OCCLUSION);
    }

    return newMesh;
}

std::string ModelLoader::ReadFilename(aiMaterial *material,
                                      aiTextureType type) {

    if (material->GetTextureCount(type) > 0) {
        aiString filepath;
        material->GetTexture(type, 0, &filepath);

        std::string fullPath =
            this->basePath +
            std::string(
                std::filesystem::path(filepath.C_Str()).filename().string());

        return fullPath;
    } else {
        return "";
    }
}
