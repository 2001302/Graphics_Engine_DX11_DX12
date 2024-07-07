#include "resource_helper.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <DirectXTexEXR.h> // EXR 형식 HDRI 읽기
#include <algorithm>
#include <directxtk/DDSTextureLoader.h> // 큐브맵 읽을 때 필요
#include <dxgi.h>                       // DXGIFactory
#include <dxgi1_4.h>                    // DXGIFactory4
#include <fp16.h>
#include <iostream>

using namespace Engine;
using namespace std;
using namespace DirectX;

void ReadEXRImage(const std::string filename, std::vector<uint8_t> &image,
                  int &width, int &height, DXGI_FORMAT &pixelFormat) {

    const std::wstring wFilename(filename.begin(), filename.end());

    TexMetadata metadata;
    ThrowIfFailed(GetMetadataFromEXRFile(wFilename.c_str(), metadata));

    ScratchImage scratchImage;
    ThrowIfFailed(LoadFromEXRFile(wFilename.c_str(), NULL, scratchImage));

    width = static_cast<int>(metadata.width);
    height = static_cast<int>(metadata.height);
    pixelFormat = metadata.format;

    cout << filename << " " << metadata.width << " " << metadata.height
         << metadata.format << endl;

    image.resize(scratchImage.GetPixelsSize());
    memcpy(image.data(), scratchImage.GetPixels(), image.size());

    // 데이터 범위 확인해보기
    vector<float> f32(image.size() / 2);
    uint16_t *f16 = (uint16_t *)image.data();
    for (int i = 0; i < image.size() / 2; i++) {
        f32[i] = fp16_ieee_to_fp32_value(f16[i]);
    }

    const float minValue = *std::min_element(f32.begin(), f32.end());
    const float maxValue = *std::max_element(f32.begin(), f32.end());

    cout << minValue << " " << maxValue << endl;

    // f16 = (uint16_t *)image.data();
    // for (int i = 0; i < image.size() / 2; i++) {
    //     f16[i] = fp16_ieee_from_fp32_value(f32[i] * 2.0f);
    // }
}

void ReadImage(const std::string filename, std::vector<uint8_t> &image,
               int &width, int &height) {

    int channels;

    unsigned char *img =
        stbi_load(filename.c_str(), &width, &height, &channels, 0);

    // assert(channels == 4);

    cout << filename << " " << width << " " << height << " " << channels
         << endl;

    // 4채널로 만들어서 복사
    image.resize(width * height * 4);

    if (channels == 1) {
        for (size_t i = 0; i < width * height; i++) {
            uint8_t g = img[i * channels + 0];
            for (size_t c = 0; c < 4; c++) {
                image[4 * i + c] = g;
            }
        }
    } else if (channels == 2) {
        for (size_t i = 0; i < width * height; i++) {
            for (size_t c = 0; c < 2; c++) {
                image[4 * i + c] = img[i * channels + c];
            }
            image[4 * i + 2] = 255;
            image[4 * i + 3] = 255;
        }
    } else if (channels == 3) {
        for (size_t i = 0; i < width * height; i++) {
            for (size_t c = 0; c < 3; c++) {
                image[4 * i + c] = img[i * channels + c];
            }
            image[4 * i + 3] = 255;
        }
    } else if (channels == 4) {
        for (size_t i = 0; i < width * height; i++) {
            for (size_t c = 0; c < 4; c++) {
                image[4 * i + c] = img[i * channels + c];
            }
        }
    } else {
        std::cout << "Cannot read " << channels << " channels" << endl;
    }
}

ComPtr<ID3D11Texture2D>
CreateStagingTexture(ComPtr<ID3D11Device> &device,
                     ComPtr<ID3D11DeviceContext> &context, const int width,
                     const int height, const std::vector<uint8_t> &image,
                     const DXGI_FORMAT pixelFormat = DXGI_FORMAT_R8G8B8A8_UNORM,
                     const int mipLevels = 1, const int arraySize = 1) {

    // 스테이징 텍스춰 만들기
    D3D11_TEXTURE2D_DESC txtDesc;
    ZeroMemory(&txtDesc, sizeof(txtDesc));
    txtDesc.Width = width;
    txtDesc.Height = height;
    txtDesc.MipLevels = mipLevels;
    txtDesc.ArraySize = arraySize;
    txtDesc.Format = pixelFormat;
    txtDesc.SampleDesc.Count = 1;
    txtDesc.Usage = D3D11_USAGE_STAGING;
    txtDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;

    ComPtr<ID3D11Texture2D> stagingTexture;
    if (FAILED(device->CreateTexture2D(&txtDesc, NULL,
                                       stagingTexture.GetAddressOf()))) {
        cout << "Failed()" << endl;
    }

    // CPU에서 이미지 데이터 복사
    size_t pixelSize = sizeof(uint8_t) * 4;
    if (pixelFormat == DXGI_FORMAT_R16G16B16A16_FLOAT) {
        pixelSize = sizeof(uint16_t) * 4;
    }

    D3D11_MAPPED_SUBRESOURCE ms;
    context->Map(stagingTexture.Get(), NULL, D3D11_MAP_WRITE, NULL, &ms);
    uint8_t *pData = (uint8_t *)ms.pData;
    for (UINT h = 0; h < UINT(height); h++) { // 가로줄 한 줄씩 복사
        memcpy(&pData[h * ms.RowPitch], &image[h * width * pixelSize],
               width * pixelSize);
    }
    context->Unmap(stagingTexture.Get(), NULL);

    return stagingTexture;
}

void ResourceHelper::CreateTexture(
    const std::string filename, ComPtr<ID3D11Texture2D> &texture,
    ComPtr<ID3D11ShaderResourceView> &textureResourceView,
    bool usSRGB) {

    int width = 0, height = 0;
    std::vector<uint8_t> image;
    DXGI_FORMAT pixelFormat =
        usSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;

    std::string ext(filename.end() - 3, filename.end());
    std::transform(ext.begin(), ext.end(), ext.begin(), std::tolower);

    if (ext == "exr") {
        ReadEXRImage(filename, image, width, height, pixelFormat);
    } else {
        ReadImage(filename, image, width, height);
    }

    // 스테이징 텍스춰 만들고 CPU에서 이미지를 복사합니다.
    auto device = Direct3D::GetInstance().device();
    auto context = Direct3D::GetInstance().device_context();

    ComPtr<ID3D11Texture2D> stagingTexture = CreateStagingTexture(
        device, context, width, height, image, pixelFormat);

    // 실제로 사용할 텍스춰 설정
    D3D11_TEXTURE2D_DESC txtDesc;
    ZeroMemory(&txtDesc, sizeof(txtDesc));
    txtDesc.Width = width;
    txtDesc.Height = height;
    txtDesc.MipLevels = 0; // 밉맵 레벨 최대
    txtDesc.ArraySize = 1;
    txtDesc.Format = pixelFormat;
    txtDesc.SampleDesc.Count = 1;
    txtDesc.Usage = D3D11_USAGE_DEFAULT; // 스테이징 텍스춰로부터 복사 가능
    txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    txtDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS; // 밉맵 사용
    txtDesc.CPUAccessFlags = 0;

    // 초기 데이터 없이 텍스춰 생성 (전부 검은색)
    Direct3D::GetInstance().device()->CreateTexture2D(&txtDesc, NULL,
                                                      texture.GetAddressOf());

    // 실제로 생성된 MipLevels를 확인해보고 싶을 경우
    // texture->GetDesc(&txtDesc);
    // cout << txtDesc.MipLevels << endl;

    // 스테이징 텍스춰로부터 가장 해상도가 높은 이미지 복사
    Direct3D::GetInstance().device_context()->CopySubresourceRegion(
        texture.Get(), 0, 0, 0, 0, stagingTexture.Get(), 0, NULL);

    // ResourceView 만들기
    Direct3D::GetInstance().device()->CreateShaderResourceView(
        texture.Get(), 0, textureResourceView.GetAddressOf());

    // 해상도를 낮춰가며 밉맵 생성
    Direct3D::GetInstance().device_context()->GenerateMips(
        textureResourceView.Get());

    // HLSL 쉐이더 안에서는 SampleLevel() 사용
}

void ResourceHelper::CreateTextureArray(
    const std::vector<std::string> filenames, ComPtr<ID3D11Texture2D> &texture,
    ComPtr<ID3D11ShaderResourceView> &textureResourceView) {

    using namespace std;

    if (filenames.empty())
        return;

    // 모든 이미지의 width와 height가 같다고 가정합니다.

    // 파일로부터 이미지 여러 개를 읽어들입니다.
    int width = 0, height = 0;
    vector<vector<uint8_t>> imageArray;
    for (const auto &f : filenames) {

        cout << f << endl;

        std::vector<uint8_t> image;

        ReadImage(f, image, width, height);

        imageArray.push_back(image);
    }

    UINT size = UINT(filenames.size());

    // Texture2DArray를 만듭니다. 이때 데이터를 CPU로부터 복사하지 않습니다.
    D3D11_TEXTURE2D_DESC txtDesc;
    ZeroMemory(&txtDesc, sizeof(txtDesc));
    txtDesc.Width = UINT(width);
    txtDesc.Height = UINT(height);
    txtDesc.MipLevels = 0; // 밉맵 레벨 최대
    txtDesc.ArraySize = size;
    txtDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    txtDesc.SampleDesc.Count = 1;
    txtDesc.SampleDesc.Quality = 0;
    txtDesc.Usage = D3D11_USAGE_DEFAULT; // 스테이징 텍스춰로부터 복사 가능
    txtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    txtDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS; // 밉맵 사용

    // 초기 데이터 없이 텍스춰를 만듭니다.
    Direct3D::GetInstance().device()->CreateTexture2D(&txtDesc, NULL,
                                                      texture.GetAddressOf());

    // 실제로 만들어진 MipLevels를 확인
    texture->GetDesc(&txtDesc);
    // cout << txtDesc.MipLevels << endl;

    // StagingTexture를 만들어서 하나씩 복사합니다.
    for (size_t i = 0; i < imageArray.size(); i++) {

        auto &image = imageArray[i];

        auto device = Direct3D::GetInstance().device();
        auto context = Direct3D::GetInstance().device_context();
        // StagingTexture는 Texture2DArray가 아니라 Texture2D 입니다.
        ComPtr<ID3D11Texture2D> stagingTexture = CreateStagingTexture(
            device, context, width, height, image, txtDesc.Format, 1, 1);

        // 스테이징 텍스춰를 텍스춰 배열의 해당 위치에 복사합니다.
        UINT subresourceIndex =
            D3D11CalcSubresource(0, UINT(i), txtDesc.MipLevels);

        context->CopySubresourceRegion(texture.Get(), subresourceIndex, 0, 0, 0,
                                       stagingTexture.Get(), 0, NULL);
    }

    Direct3D::GetInstance().device()->CreateShaderResourceView(
        texture.Get(), NULL,
                                     textureResourceView.GetAddressOf());

    Direct3D::GetInstance().device_context()->GenerateMips(textureResourceView.Get());
}

GameObject *ResourceHelper::ImportModel(Engine::GameObject *gameObject,
                                        const char *filename) {
    Assimp::Importer importer;

    auto scene = importer.ReadFile(std::string(filename),
                                   aiProcess_Triangulate |
                                       aiProcess_ConvertToLeftHanded);

    /*
        aiProcess_JoinIdenticalVertices |       //Combine identical vertices,
       optimize indexing aiProcess_ValidateDataStructure |       //Verify the
       output of the loader aiProcess_ImproveCacheLocality |        //Improve
       the cache position of the output vertices
        aiProcess_RemoveRedundantMaterials |    //Remove duplicate materials
        aiProcess_GenUVCoords |					//Convert spherical,
       cylindrical, box, and planar mapping to appropriate UVs
        aiProcess_TransformUVCoords |           //UV transformation processor
       (scaling, transforming...) aiProcess_FindInstances | //Search for
       instanced meshes and remove them as references to a single master
        aiProcess_LimitBoneWeights |            //Limit bone weights to a
       maximum of four per vertex aiProcess_OptimizeMeshes |              //Join
       small meshes where possible aiProcess_GenSmoothNormals | //Generate
       smooth normal vectors aiProcess_SplitLargeMeshes |            //Split a
       large single mesh into sub-meshes aiProcess_Triangulate | //Convert
       polygonal faces with more than three edges into triangles
        aiProcess_ConvertToLeftHanded |         //Convert to D3D's left-handed
       coordinate system aiProcess_SortByPType);                 //Create a
       'clean' mesh composed of a single type of primitive
    */

    // Using the Assimp library, start from the root node of the imported 3D
    // model and traverse through the model data to read it
    ReadModelData(gameObject, scene, scene->mRootNode, -1, -1);

    // Read the skin (bone) data to be applied to the mesh.
    ReadSkinData(gameObject, scene);

    if (scene->HasAnimations()) {
        gameObject->animation = ReadAnimationData(scene);
    }

    return gameObject;
}

void ResourceHelper::ReadModelData(Engine::GameObject *gameObject,
                                   const aiScene *scene, aiNode *node,
                                   int index, int parent) {
    std::shared_ptr<Engine::Bone> bone = std::make_shared<Engine::Bone>();

    bone->index = index;
    bone->parent = parent;
    bone->name = node->mName.C_Str();

    // Retrieve the bone's local transformation matrix, transpose it, and store
    // it Assimp uses column-major matrices, but since DirectX and OpenGL can
    // use row-major matrices, transposing may be necessary.
    DirectX::XMMATRIX transform(node->mTransformation[0]);
    bone->transform = XMMatrixTranspose(transform);

    // Calculate the transformation relative to the root (or parent) bone.
    DirectX::XMMATRIX matParent = DirectX::XMMatrixIdentity();
    if (parent >= 0) {
        matParent = gameObject->bones[parent]->transform;
    }

    // Finally, calculate the transformation matrix of the bone.
    bone->transform = bone->transform * matParent;

    gameObject->bones.push_back(bone);

    // Read the mesh data linked to the current node (bone).
    ReadMeshData(gameObject, scene, node, index);

    // Recursively traverse all child nodes of the current node, repeating the
    // same process.
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ReadModelData(gameObject, scene, node->mChildren[i],
                      gameObject->bones.size(), index);
    }
}

void ResourceHelper::ReadMeshData(Engine::GameObject *gameObject,
                                  const aiScene *scene, aiNode *node,
                                  int bone) {
    // Do not process nodes without a mesh.
    if (node->mNumMeshes < 1)
        return;

    std::shared_ptr<Engine::Mesh> mesh = std::make_shared<Engine::Mesh>();

    mesh->name = node->mName.C_Str();
    mesh->boneIndex = bone; // Set the bone indices associated with the mesh.

    for (int i = 0; i < node->mNumMeshes; i++) {
        int index = node->mMeshes[i];
        const aiMesh *srcMesh = scene->mMeshes[index];

        const aiMaterial *material = scene->mMaterials[srcMesh->mMaterialIndex];
        mesh->materialName = material->GetName().C_Str();

        for (unsigned int k = 0; k < srcMesh->mNumVertices; k++) {
            Vertex vertice;

            DirectX::XMFLOAT3 pos(&srcMesh->mVertices[k].x);
            vertice.position = pos;

            DirectX::XMFLOAT3 normal(&srcMesh->mNormals[k].x);
            vertice.normal = normal;

            if (srcMesh->mTextureCoords[0]) {
                DirectX::XMFLOAT2 tex =
                    DirectX::XMFLOAT2(&srcMesh->mTextureCoords[0][k].x);
                vertice.texcoord = tex;
            }

            mesh->vertices.push_back(vertice);
        }

        for (UINT i = 0; i < srcMesh->mNumFaces; ++i) {
            const aiFace &face = srcMesh->mFaces[i];

            mesh->indices.push_back(face.mIndices[0]);
            mesh->indices.push_back(face.mIndices[1]);
            mesh->indices.push_back(face.mIndices[2]);
        }
    }

    gameObject->meshes.push_back(mesh);
}

void ResourceHelper::ReadSkinData(Engine::GameObject *gameObject,
                                  const aiScene *scene) {
    for (int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh *srcMesh = scene->mMeshes[i];

        // Skip if there are no bones
        if (!srcMesh->HasBones())
            continue;

        std::shared_ptr<Engine::Mesh> mesh = gameObject->meshes[i];

        std::vector<Engine::BoneWeights> tempVertexBoneWeights;
        tempVertexBoneWeights.resize(mesh->vertices.size());

        for (int b = 0; b < srcMesh->mNumBones; b++) {
            aiBone *srcMeshBone = srcMesh->mBones[b];
            int boneIndex =
                GetBoneIndex(gameObject, srcMeshBone->mName.C_Str());

            for (unsigned int w = 0; w < srcMeshBone->mNumWeights; w++) {
                unsigned int index = srcMeshBone->mWeights[w].mVertexId;
                float weight = srcMeshBone->mWeights[w].mWeight;

                tempVertexBoneWeights[index].AddWeights(boneIndex, weight);
            }
        }

        for (unsigned int v = 0; v < tempVertexBoneWeights.size(); v++) {
            tempVertexBoneWeights[v].Normalize();

            BlendWeight blendWeight =
                tempVertexBoneWeights[v].GetBlendWeights();
            // mesh->vertices[v].blendIndices = blendWeight.indices;
            // mesh->vertices[v].blendWeights = blendWeight.weights;
        }
    }
}

unsigned int ResourceHelper::GetBoneIndex(Engine::GameObject *gameObject,
                                          const std::string &name) {
    for (std::shared_ptr<Bone> &bone : gameObject->bones) {
        if (bone->name == name)
            return bone->index;
    }

    assert(false);
    return 0;
}

std::shared_ptr<Animation>
ResourceHelper::ReadAnimationData(const aiScene *scene) {
    auto srcAnimation = *scene->mAnimations;
    // 새로운 애니메이션 객체를 생성합니다.
    std::shared_ptr<Animation> animation = std::make_shared<Animation>();
    // 애니메이션 이름을 설정합니다.
    animation->name = srcAnimation->mName.C_Str();
    // 애니메이션의 프레임 속도를 설정합니다. (초당 틱 수)
    animation->frameRate = (float)srcAnimation->mTicksPerSecond;
    // 애니메이션의 프레임 수를 설정합니다. (지속 시간 기반)
    animation->frameCount = (int)srcAnimation->mDuration + 1;

    // 애니메이션 노드를 캐싱하기 위한 맵을 선언합니다.
    std::map<std::string, std::shared_ptr<AnimationNode>> cacheAnimNodes;
    // Assimp 애니메이션 채널을 순회하며 노드별 애니메이션 데이터를 처리합니다.
    for (int i = 0; i < srcAnimation->mNumChannels; i++) {
        aiNodeAnim *srcNode = srcAnimation->mChannels[i];

        // 애니메이션 노드를 파싱합니다.
        std::shared_ptr<AnimationNode> node =
            ParseAnimationNode(animation, srcNode);

        // 애니메이션의 최대 지속 시간을 업데이트합니다.
        animation->duration =
            max(animation->duration, node->keyframe.back().time);

        // 파싱된 노드를 캐시에 추가합니다.
        cacheAnimNodes[srcNode->mNodeName.C_Str()] = node;
    }
    // 애니메이션 키프레임 데이터를 처리합니다.
    ReadKeyframeData(animation, scene->mRootNode, cacheAnimNodes);

    return animation;
}

// 애니메이션 노드를 파싱하는 함수
std::shared_ptr<AnimationNode>
ResourceHelper::ParseAnimationNode(std::shared_ptr<Animation> animation,
                                   aiNodeAnim *srcNode) {
    // 새로운 애니메이션 노드 객체를 생성합니다.
    std::shared_ptr<AnimationNode> node = std::make_shared<AnimationNode>();
    // 노드 이름을 설정합니다.
    node->name = srcNode->mNodeName.C_Str();

    // 위치, 회전, 스케일 중 가장 많은 키프레임을 가진 것을 기준으로 총 키프레임
    // 수를 결정합니다.
    int keyCount = max(max(srcNode->mNumPositionKeys, srcNode->mNumScalingKeys),
                       srcNode->mNumRotationKeys);

    // 각 키프레임에 대해 반복합니다.
    for (int k = 0; k < keyCount; k++) {

        KeyframeData frameData; // 키프레임 데이터 객체

        bool found = false; // 키프레임이 발견되었는지 여부
        int t = node->keyframe.size(); // 현재 키프레임의 인덱스

        // 위치, 회전, 스케일 키프레임을 처리합니다.각 키프레임의 시간과
        // 데이터를 추출하여 frameData에 저장합니다. Position
        if (::fabsf((float)srcNode->mPositionKeys[k].mTime - (float)t) <=
            0.0001f) {
            aiVectorKey key = srcNode->mPositionKeys[k];
            frameData.time = (float)key.mTime;
            ::memcpy_s(&frameData.translation, sizeof(DirectX::XMFLOAT3),
                       &key.mValue, sizeof(aiVector3D));

            found = true;
        }

        // Rotation
        if (::fabsf((float)srcNode->mRotationKeys[k].mTime - (float)t) <=
            0.0001f) {
            aiQuatKey key = srcNode->mRotationKeys[k];
            frameData.time = (float)key.mTime;
            frameData.rotation = {key.mValue.x, key.mValue.y, key.mValue.z,
                                  key.mValue.w};

            found = true;
        }

        // Scale
        if (::fabsf((float)srcNode->mScalingKeys[k].mTime - (float)t) <=
            0.0001f) {
            aiVectorKey key = srcNode->mScalingKeys[k];
            frameData.time = (float)key.mTime;
            ::memcpy_s(&frameData.scale, sizeof(DirectX::XMFLOAT3), &key.mValue,
                       sizeof(aiVector3D));

            found = true;
        }

        if (found == true)
            node->keyframe.push_back(
                frameData); // 처리된 키프레임을 노드에 추가합니다.
    }

    // 애니메이션의 키프레임 수보다 노드의 키프레임 수가 적은 경우, 마지막
    // 키프레임을 복제하여 채웁니다.
    if (node->keyframe.size() < animation->frameCount) {
        int count = animation->frameCount -
                    node->keyframe.size(); // 채워야 할 키프레임 수
        KeyframeData keyFrame = node->keyframe.back(); // 마지막 키프레임

        for (int n = 0; n < count; n++) {
            node->keyframe.push_back(keyFrame); // 키프레임 복제하여 추가
        }
    }

    return node;
}

// 애니메이션 데이터에서 특정 노드의 키프레임 데이터를 읽어 내부 데이터 구조에
// 저장하는 함수
void ResourceHelper::ReadKeyframeData(
    std::shared_ptr<Animation> animation, aiNode *node,
    std::map<std::string, std::shared_ptr<AnimationNode>> &cache) {
    // 새로운 키프레임 객체를 생성합니다.
    std::shared_ptr<Keyframe> keyframe = std::make_shared<Keyframe>();
    // 현재 노드(본)의 이름을 키프레임의 본 이름으로 설정합니다.
    keyframe->boneName = node->mName.C_Str();

    // 현재 노드에 해당하는 애니메이션 노드를 찾습니다.
    std::shared_ptr<AnimationNode> findNode = cache[node->mName.C_Str()];

    // 애니메이션의 모든 프레임에 대해 반복합니다.
    for (int i = 0; i < animation->frameCount; i++) {
        KeyframeData frameData; // 키프레임 데이터 객체를 생성합니다.

        // 만약 현재 노드에 대한 애니메이션 노드가 캐시에서 찾아지지 않는 경우
        if (findNode == nullptr) {
            // 노드의 변환 행렬을 가져와 전치한 뒤, 이를 기반으로 위치, 회전,
            // 스케일 데이터를 추출합니다.
            DirectX::XMMATRIX transform(node->mTransformation[0]);
            transform = XMMatrixTranspose(transform);
            frameData.time = (float)i; // 프레임 시간을 설정합니다.
            // transform.Decompose(OUT frameData.scale, OUT frameData.rotation,
            // OUT frameData.translation);
            XMMatrixDecompose(&frameData.scale, &frameData.rotation,
                              &frameData.translation, transform);
        } else {
            // 캐시에서 찾아진 애니메이션 노드에 이미 키프레임 데이터가 있으면,
            // 해당 데이터를 사용합니다.
            frameData = findNode->keyframe[i];
        }

        // 처리된 키프레임 데이터를 키프레임 객체에 추가합니다.
        keyframe->transforms.push_back(frameData);
    }

    // 처리된 키프레임 객체를 애니메이션의 키프레임 목록에 추가합니다.
    animation->keyframes.push_back(*keyframe);

    // 현재 노드의 모든 자식 노드에 대해 재귀적으로 동일한 처리를 수행합니다.
    for (int i = 0; i < node->mNumChildren; i++)
        ReadKeyframeData(animation, node->mChildren[i], cache);
}