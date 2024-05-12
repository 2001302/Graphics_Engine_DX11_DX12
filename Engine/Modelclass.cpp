#pragma warning(disable:6385)
#pragma warning(disable:6386)

#include "modelclass.h"

ModelClass::ModelClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_texture = 0;

	vertices.clear();
	indices.clear();

	meshes.clear();
	bones.clear();
}

ModelClass::ModelClass(const ModelClass& other)
{
}

ModelClass::~ModelClass()
{
}

bool ModelClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* modelFilename, const char* textureFilename)
{
	bool result;

	// Load in the model data.
	result = LoadModel(modelFilename);
	if (!result)
	{
		return false;
	}

	// Initialize the vertex and index buffers.
	result = InitializeBuffers(device);
	if(!result)
	{
		return false;
	}

	// Load the texture for this model.
	result = LoadTexture(device, deviceContext, textureFilename);
	if(!result)
	{
		return false;
	}

	return true;
}
bool ModelClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* modelFilename, std::unique_ptr<TextureClass>& texture)
{
	bool result;

	// Load in the model data.
	result = LoadModel(modelFilename);
	if (!result)
	{
		return false;
	}

	// Initialize the vertex and index buffers.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}
	
	m_texture = std::move(texture);

	return true;
}
void ModelClass::Shutdown()
{
	// Release the model texture.
	ReleaseTexture();

	// Shutdown the vertex and index buffers.
	ShutdownBuffers();

	// Release the model data.
	ReleaseModel();

	return;
}

void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);

	return;
}

int ModelClass::GetIndexCount()
{
	return indices.size();
}

ID3D11ShaderResourceView* ModelClass::GetTexture()
{
	return m_texture->GetTexture();
}

bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	for (auto mesh : meshes)
	{
		vertices.insert(vertices.end(), mesh->vertices.begin(), mesh->vertices.end());
		indices.insert(indices.end(), mesh->indices.begin(), mesh->indices.end());
	}

	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	// Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertices.size();
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = &vertices[0];
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
    result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * indices.size();
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    indexData.pSysMem = &indices[0];
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}


void ModelClass::ShutdownBuffers()
{
	// Release the index buffer.
	if(m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if(m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}


void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType); 
	offset = 0;
    
	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

    // Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    // Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}


bool ModelClass::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const  char* filename)
{
	bool result;


	// Create and initialize the texture object.
	m_texture = std::make_unique<TextureClass>();

	result = m_texture->Initialize(device, deviceContext, filename);
	if(!result)
	{
		return false;
	}

	return true;
}


void ModelClass::ReleaseTexture()
{
	// Release the texture object.
	if(m_texture)
	{
		m_texture->Shutdown();
		m_texture.reset();
	}

	return;
}

void ModelClass::ReleaseModel()
{
	vertices.clear();
	indices.clear();

	meshes.clear();
	bones.clear();
	return;
}

bool ModelClass::LoadModel(const char* filename)
{
	Assimp::Importer importer;

	scene = importer.ReadFile(std::string(filename),
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded);

	/*
		aiProcess_JoinIdenticalVertices |       //Combine identical vertices, optimize indexing
		aiProcess_ValidateDataStructure |       //Verify the output of the loader
		aiProcess_ImproveCacheLocality |        //Improve the cache position of the output vertices
		aiProcess_RemoveRedundantMaterials |    //Remove duplicate materials
		aiProcess_GenUVCoords |					//Convert spherical, cylindrical, box, and planar mapping to appropriate UVs
		aiProcess_TransformUVCoords |           //UV transformation processor (scaling, transforming...)
		aiProcess_FindInstances |               //Search for instanced meshes and remove them as references to a single master
		aiProcess_LimitBoneWeights |            //Limit bone weights to a maximum of four per vertex
		aiProcess_OptimizeMeshes |              //Join small meshes where possible
		aiProcess_GenSmoothNormals |            //Generate smooth normal vectors
		aiProcess_SplitLargeMeshes |            //Split a large single mesh into sub-meshes
		aiProcess_Triangulate |                 //Convert polygonal faces with more than three edges into triangles
		aiProcess_ConvertToLeftHanded |         //Convert to D3D's left-handed coordinate system
		aiProcess_SortByPType);                 //Create a 'clean' mesh composed of a single type of primitive
	*/

	// Assimp 라이브러리를 사용하여 읽어온 3D 모델의 루트 노드부터 시작하여
	// 모델 데이터를 순회하며 읽어들입니다.
	ReadModelData(scene->mRootNode, -1, -1);

	// 메시에 적용될 스킨(본) 데이터를 읽어들입니다.
	ReadSkinData();

	return true;
}

// 모델의 노드(본) 데이터를 읽고 처리하는 함수
void ModelClass::ReadModelData(aiNode* node, int index, int parent)
{
	// 새로운 본 객체를 생성하고 기본 정보를 설정합니다.
	std::shared_ptr<Bone> bone = std::make_shared<Bone>();

	bone->index = index; // 본의 고유 인덱스
	bone->parent = parent; // 부모 본의 인덱스
	bone->name = node->mName.C_Str(); // 본의 이름

	// 본의 로컬 변환 행렬을 가져오고, 전치(transpose)하여 저장합니다.
	// Assimp는 열 기준(column-major) 행렬을 사용하지만, DirectX나 OpenGL은 행 기준(row-major) 행렬을 사용할 수 있으므로, 전치가 필요할 수 있습니다.
	DirectX::XMMATRIX transform(node->mTransformation[0]);
	bone->transform = XMMatrixTranspose(transform);

	// 루트(혹은 부모) 본으로부터 상대적인 변환을 계산합니다.
	DirectX::XMMATRIX matParent = DirectX::XMMatrixIdentity(); // 기본값으로 단위 행렬을 사용
	if (parent >= 0) {
		matParent = bones[parent]->transform; // 부모 본의 변환 행렬을 가져옵니다.
	}

	// 최종적으로 본의 변환 행렬을 계산합니다.
	// 본 자신의 로컬 변환에 부모의 변환 행렬을 곱합니다.
	bone->transform = bone->transform * matParent;

	// 처리된 본 정보를 내부 리스트에 추가합니다.
	bones.push_back(bone);

	// 현재 노드(본)에 연결된 메시 데이터를 읽어들입니다.
	ReadMeshData(node, index);

	// 현재 노드의 모든 자식 노드를 재귀적으로 탐색하여 같은 처리를 반복합니다.
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		ReadModelData(node->mChildren[i], bones.size(), index);
	}
}

// 노드에서 메시 데이터를 읽어와 내부 구조체에 저장하는 함수
void ModelClass::ReadMeshData(aiNode* node, int bone)
{
	if (node->mNumMeshes < 1)
		return; // 메시가 없는 노드는 처리하지 않음

	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(); // 새 메시 객체 생성

	mesh->name = node->mName.C_Str(); // 메시 이름 설정
	mesh->boneIndex = bone; // 메시와 연결된 본 인덱스 설정

	// 노드에 포함된 모든 메시에 대해 반복
	for (int i = 0; i < node->mNumMeshes; i++) {
		int index = node->mMeshes[i];
		const aiMesh* srcMesh = scene->mMeshes[index]; // 소스 메시 참조

		// 메시의 재질 이름을 가져옴
		const aiMaterial* material = scene->mMaterials[srcMesh->mMaterialIndex];
		mesh->materialName = material->GetName().C_Str(); // 재질 이름 설정

		for (unsigned int k = 0; k < srcMesh->mNumVertices; k++)
		{
			VertexType vertice;

			DirectX::XMFLOAT3 pos(&srcMesh->mVertices[k].x);
			vertice.position = pos;

			DirectX::XMFLOAT3 normal(&srcMesh->mNormals[k].x);
			vertice.normal = normal;

			if (srcMesh->mTextureCoords[0])
			{
				DirectX::XMFLOAT2 tex = DirectX::XMFLOAT2(&srcMesh->mTextureCoords[0][k].x);
				vertice.texture = tex;
			}

			mesh->vertices.push_back(vertice);
		}

		for (UINT i = 0; i < srcMesh->mNumFaces; ++i)
		{
			const aiFace& face = srcMesh->mFaces[i];

			mesh->indices.push_back(face.mIndices[0]);
			mesh->indices.push_back(face.mIndices[1]);
			mesh->indices.push_back(face.mIndices[2]);
		}
	}

	meshes.push_back(mesh); // 메시를 내부 메시 리스트에 추가
}

// 모델의 본과 정점 가중치 정보를 읽어와 처리하는 함수
void ModelClass::ReadSkinData()
{
	// 모든 메시에 대해 반복
	for (int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* srcMesh = scene->mMeshes[i];
		if (!srcMesh->HasBones()) continue; // 본이 없으면 건너뜀

		std::shared_ptr<Mesh> mesh = meshes[i]; // 현재 메시 참조

		std::vector<BoneWeights> tempVertexBoneWeights;
		tempVertexBoneWeights.resize(mesh->vertices.size()); // 정점별 가중치 리스트 초기화

		// 모든 본에 대해 반복하여 가중치 정보 추출
		for (int b = 0; b < srcMesh->mNumBones; b++) {
			aiBone* srcMeshBone = srcMesh->mBones[b];
			int boneIndex = GetBoneIndex(srcMeshBone->mName.C_Str()); // 본 인덱스 검색

			for (unsigned int w = 0; w < srcMeshBone->mNumWeights; w++) {
				unsigned int index = srcMeshBone->mWeights[w].mVertexId; // 정점 인덱스
				float weight = srcMeshBone->mWeights[w].mWeight; // 가중치

				tempVertexBoneWeights[index].AddWeights(boneIndex, weight); // 가중치 정보 추가
			}
		}

		// 최종 가중치 정보를 정점 데이터에 적용
		for (unsigned int v = 0; v < tempVertexBoneWeights.size(); v++)
		{
			tempVertexBoneWeights[v].Normalize(); // 가중치 정규화

			BlendWeight blendWeight = tempVertexBoneWeights[v].GetBlendWeights();
			//mesh->vertices[v].blendIndices = blendWeight.indices; // 본 인덱스 설정
			//mesh->vertices[v].blendWeights = blendWeight.weights; // 본 가중치 설정
		}
	}
}

unsigned int ModelClass::GetBoneIndex(const std::string& name)
{
	for (std::shared_ptr<Bone>& bone : bones)
	{
		if (bone->name == name)
			return bone->index;
	}

	assert(false);
	return 0;
}