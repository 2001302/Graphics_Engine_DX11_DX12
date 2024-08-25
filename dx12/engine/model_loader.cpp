﻿#include "model_loader.h"
#include "mesh.h"

#include <DirectXMesh.h>
#include <filesystem>
#include <vector>

namespace core {

using namespace std;
using namespace DirectX::SimpleMath;

void UpdateNormals(vector<MeshData> &meshes) {

    // ?몃? 踰≫꽣媛 ?녿뒗 寃쎌슦瑜??鍮꾪븯???ㅼ떆 怨꾩궛
    // ???꾩튂?먮뒗 ??踰꾪뀓?ㅻ쭔 ?덉뼱???곌껐 愿怨꾨? 李얠쓣 ???덉쓬

    // DirectXMesh??ComputeNormals()怨?鍮꾩듂?⑸땲??
    // https://github.com/microsoft/DirectXMesh/wiki/ComputeNormals

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

void ModelLoader::ReadAnimation(const aiScene *pScene) {

    m_aniData.clips.resize(pScene->mNumAnimations);

    for (uint32_t i = 0; i < pScene->mNumAnimations; i++) {

        auto &clip = m_aniData.clips[i];

        const aiAnimation *ani = pScene->mAnimations[i];

        clip.duration = ani->mDuration / 100.0f;
        clip.ticksPerSec = ani->mTicksPerSecond;
        clip.keys.resize(m_aniData.boneNameToId.size());
        clip.numChannels = ani->mNumChannels;
        clip.numKeys = clip.keys.size();

        for (uint32_t c = 0; c < ani->mNumChannels; c++) {
            const aiNodeAnim *nodeAnim = ani->mChannels[c];
            const int boneId =
                m_aniData.boneNameToId[nodeAnim->mNodeName.C_Str()];
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

/*
 * ?щ윭媛쒖쓽 堉덈뱾???덇퀬 ?몃━ 援ъ“??
 * 洹?以묒뿉??Vertex???곹뼢??二쇰뒗 寃껊뱾? ?쇰???
 * Vertex???곹뼢??二쇰뒗 堉덈뱾怨?遺紐⑤뱾源뚯? ?ы븿?댁꽌
 * ?몃옒踰꾩뒪 ?쒖꽌濡????
 */
void ModelLoader::Load(std::string basePath, std::string filename,
                       bool revertNormals) {

    if (GetExtension(filename) == ".gltf") {
        m_isGLTF = true;
        m_revertNormals = revertNormals;
    }

    m_basePath = basePath; // ?띿뒪異??쎌뼱?ㅼ씪 ???꾩슂

    Assimp::Importer importer;

    const aiScene *pScene = importer.ReadFile(
        m_basePath + filename,
        aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);
    // ReadFile()?먯꽌 寃쎌슦???곕씪???щ윭媛吏 ?듭뀡???ㅼ젙 媛??
    // aiProcess_JoinIdenticalVertices | aiProcess_PopulateArmatureData |
    // aiProcess_SplitByBoneCount |
    // aiProcess_Debone); // aiProcess_LimitBoneWeights

    if (pScene) {

        // 1. 紐⑤뱺 硫붿돩????댁꽌 踰꾪뀓?ㅼ뿉 ?곹뼢??二쇰뒗 堉덈뱾??紐⑸줉??留뚮뱺??
        FindDeformingBones(pScene);

        // 2. ?몃━ 援ъ“瑜??곕씪 ?낅뜲?댄듃 ?쒖꽌?濡?堉덈뱾???몃뜳?ㅻ? 寃곗젙?쒕떎
        int counter = 0;
        UpdateBoneIDs(pScene->mRootNode, &counter);

        // 3. ?낅뜲?댄듃 ?쒖꽌?濡?堉??대쫫 ???(boneIdToName)
        m_aniData.boneIdToName.resize(m_aniData.boneNameToId.size());
        for (auto &i : m_aniData.boneNameToId)
            m_aniData.boneIdToName[i.second] = i.first;

        // ?붾쾭源낆슜
        // cout << "Num boneNameToId : " << m_aniData.boneNameToId.size() <<
        // endl; for (auto &i : m_aniData.boneNameToId) {
        //    cout << "NameId pair : " << i.first << " " << i.second << endl;
        //}
        // cout << "Num boneIdToName : " << m_aniData.boneIdToName.size() <<
        // endl; for (size_t i = 0; i < m_aniData.boneIdToName.size(); i++) {
        //    cout << "BoneId: " << i << " " << m_aniData.boneIdToName[i] <<
        //    endl;
        //}
        // exit(-1);

        // 媛?堉덉쓽 遺紐??몃뜳?ㅻ? ??ν븷 以鍮?
        m_aniData.boneParents.resize(m_aniData.boneNameToId.size(), -1);

        Matrix tr; // Initial transformation
        ProcessNode(pScene->mRootNode, pScene, tr);

        // ?붾쾭源낆슜
        // cout << "Num boneIdToName : " << m_aniData.boneIdToName.size() <<
        // endl; for (size_t i = 0; i < m_aniData.boneIdToName.size(); i++) {
        //    cout << "BoneId: " << i << " " << m_aniData.boneIdToName[i]
        //         << " , Parent: "
        //         << (m_aniData.boneParents[i] == -1
        //                 ? "NONE"
        //                 : m_aniData.boneIdToName[m_aniData.boneParents[i]])
        //         << endl;
        //}

        // ?좊땲硫붿씠???뺣낫 ?쎄린
        if (pScene->HasAnimations())
            ReadAnimation(pScene);

        // UpdateNormals(this->meshes); // Vertex Normal??吏곸젒 怨꾩궛 (李멸퀬??

        UpdateTangents();
    } else {
        std::cout << "Failed to read file: " << m_basePath + filename
                  << std::endl;
        auto errorDescription = importer.GetErrorString();
        std::cout << "Assimp error: " << errorDescription << endl;
    }
}

void ModelLoader::LoadAnimation(string basePath, string filename) {

    m_basePath = basePath;

    Assimp::Importer importer;

    const aiScene *pScene = importer.ReadFile(
        m_basePath + filename,
        aiProcess_Triangulate | aiProcess_ConvertToLeftHanded);

    if (pScene && pScene->HasAnimations()) {
        ReadAnimation(pScene);
    } else {
        std::cout << "Failed to read animation from file: "
                  << m_basePath + filename << std::endl;
        auto errorDescription = importer.GetErrorString();
        std::cout << "Assimp error: " << errorDescription << endl;
    }
}

void ModelLoader::UpdateTangents() {

    using namespace std;
    using namespace DirectX;

    // https://github.com/microsoft/DirectXMesh/wiki/ComputeTangentFrame

    for (auto &m : this->m_meshes) {

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

// 踰꾪뀓?ㅼ쓽 蹂?뺤뿉 吏곸젒?곸쑝濡?李몄뿬?섎뒗 堉덈뱾??紐⑸줉??留뚮벊?덈떎.

void ModelLoader::FindDeformingBones(const aiScene *scene) {
    for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
        const auto *mesh = scene->mMeshes[i];
        if (mesh->HasBones()) {
            for (uint32_t i = 0; i < mesh->mNumBones; i++) {
                const aiBone *bone = mesh->mBones[i];

                // bone怨???묐릺??node???대쫫? ?숈씪
                // ?ㅼ뿉??node ?대쫫?쇰줈 遺紐⑤? 李얠쓣 ???덉쓬
                m_aniData.boneNameToId[bone->mName.C_Str()] = -1;

                // 二쇱쓽: 堉덉쓽 ?쒖꽌媛 ?낅뜲?댄듃 ?쒖꽌???꾨떂

                // 湲고?: bone->mWeights == 0??寃쎌슦?먮룄 ?ы븿?쒖섟??
                // 湲고?: bone->mNode = 0?대씪???ъ슜 遺덇?
            }
        }
    }
}

// ?붿옄?몄쓣 ?꾪븳 ?몃뱶?ㅼ쓣 嫄대꼫?곌퀬 遺紐??몃뱶 李얘린
const aiNode *ModelLoader::FindParent(const aiNode *node) {
    if (!node)
        return nullptr;
    if (m_aniData.boneNameToId.count(node->mName.C_Str()) > 0)
        return node;
    return FindParent(node->mParent);
}

void ModelLoader::ProcessNode(aiNode *node, const aiScene *scene, Matrix tr) {

    // https://ogldev.org/www/tutorial38/tutorial38.html
    // If a node represents a bone in the hierarchy then the node name must
    // match the bone name.

    // ?ъ슜?섎뒗 遺紐?堉덈? 李얠븘??遺紐⑥쓽 ?몃뜳?????
    if (node->mParent && m_aniData.boneNameToId.count(node->mName.C_Str()) &&
        FindParent(node->mParent)) {
        const auto boneId = m_aniData.boneNameToId[node->mName.C_Str()];
        m_aniData.boneParents[boneId] =
            m_aniData.boneNameToId[FindParent(node->mParent)->mName.C_Str()];
    }

    Matrix m(&node->mTransformation.a1);
    m = m.Transpose() * tr;

    for (UINT i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        auto newMesh = this->ProcessMesh(mesh, scene);
        for (auto &v : newMesh.vertices) {
            v.position = DirectX::SimpleMath::Vector3::Transform(v.position, m);
        }
        m_meshes.push_back(newMesh);
    }

    for (UINT i = 0; i < node->mNumChildren; i++) {
        this->ProcessNode(node->mChildren[i], scene, m);
    }
}

string ModelLoader::ReadTextureFilename(const aiScene *scene,
                                        aiMaterial *material,
                                        aiTextureType type) {

    if (material->GetTextureCount(type) > 0) {
        aiString filepath;
        material->GetTexture(type, 0, &filepath);

        string fullPath =
            m_basePath +
            string(filesystem::path(filepath.C_Str()).filename().string());

        // 1. ?ㅼ젣濡??뚯씪??議댁옱?섎뒗吏 ?뺤씤
        if (!filesystem::exists(fullPath)) {
            // 2. ?뚯씪???놁쓣 寃쎌슦 ?뱀떆 fbx ?먯껜??Embedded?몄? ?뺤씤
            const aiTexture *texture =
                scene->GetEmbeddedTexture(filepath.C_Str());
            if (texture) {
                // 3. Embedded texture媛 議댁옱?섍퀬 png??寃쎌슦 ???
                if (string(texture->achFormatHint).find("png") !=
                    string::npos) {
                    std::ofstream fs(fullPath.c_str(), ios::binary | ios::out);
                    fs.write((char *)texture->pcData, texture->mWidth);
                    fs.close();
                    // 李멸퀬: compressed format??寃쎌슦 texture->mHeight媛 0
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

MeshData ModelLoader::ProcessMesh(aiMesh *mesh, const aiScene *scene) {

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
        if (m_isGLTF) {
            vertex.normal.y = mesh->mNormals[i].z;
            vertex.normal.z = -mesh->mNormals[i].y;
        } else {
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }

        if (m_revertNormals) {
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

        m_aniData.offsetMatrices.resize(m_aniData.boneNameToId.size());
        m_aniData.boneTransforms.resize(m_aniData.boneNameToId.size());

        int count = 0;
        for (uint32_t i = 0; i < mesh->mNumBones; i++) {
            const aiBone *bone = mesh->mBones[i];

            // ?붾쾭源?
            // cout << "BoneMap " << count++ << " : " << bone->mName.C_Str()
            //     << " NumBoneWeights = " << bone->mNumWeights << endl;

            const uint32_t boneId = m_aniData.boneNameToId[bone->mName.C_Str()];

            m_aniData.offsetMatrices[boneId] =
                Matrix((float *)&bone->mOffsetMatrix).Transpose();

            // ??堉덇? ?곹뼢??二쇰뒗 Vertex??媛쒖닔
            for (uint32_t j = 0; j < bone->mNumWeights; j++) {
                aiVertexWeight weight = bone->mWeights[j];
                assert(weight.mVertexId < boneIndices.size());
                boneIndices[weight.mVertexId].push_back(boneId);
                boneWeights[weight.mVertexId].push_back(weight.mWeight);
            }
        }

        // ?덉쟾?먮뒗 Vertex ?섎굹???곹뼢??二쇰뒗 Bone? 理쒕? 4媛?
        // ?붿쬁? ??留롮쓣 ?섎룄 ?덈뒗??紐⑤뜽留??뚰봽?몄썾?댁뿉??議곗젙?섍굅??
        // ?쎌뼱?ㅼ씠硫댁꽌 weight媛 ?덈Т ?묒? 寃껊뱾? 類??섎룄 ?덉쓬

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

        // ?붾쾭源낆슜 異쒕젰 (boneWeights)
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

        newMesh.albedoTextureFilename =
            ReadTextureFilename(scene, material, aiTextureType_BASE_COLOR);
        if (newMesh.albedoTextureFilename.empty()) {
            newMesh.albedoTextureFilename =
                ReadTextureFilename(scene, material, aiTextureType_DIFFUSE);
        }
        newMesh.emissiveTextureFilename =
            ReadTextureFilename(scene, material, aiTextureType_EMISSIVE);
        newMesh.heightTextureFilename =
            ReadTextureFilename(scene, material, aiTextureType_HEIGHT);
        newMesh.normalTextureFilename =
            ReadTextureFilename(scene, material, aiTextureType_NORMALS);
        newMesh.metallicTextureFilename =
            ReadTextureFilename(scene, material, aiTextureType_METALNESS);
        newMesh.roughnessTextureFilename = ReadTextureFilename(
            scene, material, aiTextureType_DIFFUSE_ROUGHNESS);
        newMesh.aoTextureFilename = ReadTextureFilename(
            scene, material, aiTextureType_AMBIENT_OCCLUSION);
        if (newMesh.aoTextureFilename.empty()) {
            newMesh.aoTextureFilename =
                ReadTextureFilename(scene, material, aiTextureType_LIGHTMAP);
        }
        newMesh.opacityTextureFilename =
            ReadTextureFilename(scene, material, aiTextureType_OPACITY);

        if (!newMesh.opacityTextureFilename.empty()) {
            cout << newMesh.albedoTextureFilename << endl;
            cout << "Opacity " << newMesh.opacityTextureFilename << endl;
        }

        // ?붾쾭源낆슜
        // for (size_t i = 0; i < 22; i++) {
        //    cout << i << " " << ReadTextureFilename(material,
        //    aiTextureType(i))
        //         << endl;
        //}
    }

    return newMesh;
}

} // namespace engine
