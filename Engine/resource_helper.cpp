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
    auto device = Direct3D::Instance().device();
    auto context = Direct3D::Instance().device_context();

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
    Direct3D::Instance().device()->CreateTexture2D(&txtDesc, NULL,
                                                      texture.GetAddressOf());

    // 실제로 생성된 MipLevels를 확인해보고 싶을 경우
    // texture->GetDesc(&txtDesc);
    // cout << txtDesc.MipLevels << endl;

    // 스테이징 텍스춰로부터 가장 해상도가 높은 이미지 복사
    Direct3D::Instance().device_context()->CopySubresourceRegion(
        texture.Get(), 0, 0, 0, 0, stagingTexture.Get(), 0, NULL);

    // ResourceView 만들기
    Direct3D::Instance().device()->CreateShaderResourceView(
        texture.Get(), 0, textureResourceView.GetAddressOf());

    // 해상도를 낮춰가며 밉맵 생성
    Direct3D::Instance().device_context()->GenerateMips(
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
    Direct3D::Instance().device()->CreateTexture2D(&txtDesc, NULL,
                                                      texture.GetAddressOf());

    // 실제로 만들어진 MipLevels를 확인
    texture->GetDesc(&txtDesc);
    // cout << txtDesc.MipLevels << endl;

    // StagingTexture를 만들어서 하나씩 복사합니다.
    for (size_t i = 0; i < imageArray.size(); i++) {

        auto &image = imageArray[i];

        auto device = Direct3D::Instance().device();
        auto context = Direct3D::Instance().device_context();
        // StagingTexture는 Texture2DArray가 아니라 Texture2D 입니다.
        ComPtr<ID3D11Texture2D> stagingTexture = CreateStagingTexture(
            device, context, width, height, image, txtDesc.Format, 1, 1);

        // 스테이징 텍스춰를 텍스춰 배열의 해당 위치에 복사합니다.
        UINT subresourceIndex =
            D3D11CalcSubresource(0, UINT(i), txtDesc.MipLevels);

        context->CopySubresourceRegion(texture.Get(), subresourceIndex, 0, 0, 0,
                                       stagingTexture.Get(), 0, NULL);
    }

    Direct3D::Instance().device()->CreateShaderResourceView(
        texture.Get(), NULL,
                                     textureResourceView.GetAddressOf());

    Direct3D::Instance().device_context()->GenerateMips(textureResourceView.Get());
}

Model *ResourceHelper::ImportModel(Engine::Model *gameObject,
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

    //// Read the skin (bone) data to be applied to the mesh.
    //ReadSkinData(gameObject, scene);

    //if (scene->HasAnimations()) {
    //    gameObject->animation = ReadAnimationData(scene);
    //}

    return gameObject;
}

void ResourceHelper::ReadModelData(Engine::Model *gameObject,
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

void ResourceHelper::ReadMeshData(Engine::Model *gameObject,
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

unsigned int ResourceHelper::GetBoneIndex(Engine::Model *gameObject,
                                          const std::string &name) {
    for (std::shared_ptr<Bone> &bone : gameObject->bones) {
        if (bone->name == name)
            return bone->index;
    }

    assert(false);
    return 0;
}
