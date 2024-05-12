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

	// Assimp ���̺귯���� ����Ͽ� �о�� 3D ���� ��Ʈ ������ �����Ͽ�
	// �� �����͸� ��ȸ�ϸ� �о���Դϴ�.
	ReadModelData(scene->mRootNode, -1, -1);

	// �޽ÿ� ����� ��Ų(��) �����͸� �о���Դϴ�.
	ReadSkinData();

	return true;
}

// ���� ���(��) �����͸� �а� ó���ϴ� �Լ�
void ModelClass::ReadModelData(aiNode* node, int index, int parent)
{
	// ���ο� �� ��ü�� �����ϰ� �⺻ ������ �����մϴ�.
	std::shared_ptr<Bone> bone = std::make_shared<Bone>();

	bone->index = index; // ���� ���� �ε���
	bone->parent = parent; // �θ� ���� �ε���
	bone->name = node->mName.C_Str(); // ���� �̸�

	// ���� ���� ��ȯ ����� ��������, ��ġ(transpose)�Ͽ� �����մϴ�.
	// Assimp�� �� ����(column-major) ����� ���������, DirectX�� OpenGL�� �� ����(row-major) ����� ����� �� �����Ƿ�, ��ġ�� �ʿ��� �� �ֽ��ϴ�.
	DirectX::XMMATRIX transform(node->mTransformation[0]);
	bone->transform = XMMatrixTranspose(transform);

	// ��Ʈ(Ȥ�� �θ�) �����κ��� ������� ��ȯ�� ����մϴ�.
	DirectX::XMMATRIX matParent = DirectX::XMMatrixIdentity(); // �⺻������ ���� ����� ���
	if (parent >= 0) {
		matParent = bones[parent]->transform; // �θ� ���� ��ȯ ����� �����ɴϴ�.
	}

	// ���������� ���� ��ȯ ����� ����մϴ�.
	// �� �ڽ��� ���� ��ȯ�� �θ��� ��ȯ ����� ���մϴ�.
	bone->transform = bone->transform * matParent;

	// ó���� �� ������ ���� ����Ʈ�� �߰��մϴ�.
	bones.push_back(bone);

	// ���� ���(��)�� ����� �޽� �����͸� �о���Դϴ�.
	ReadMeshData(node, index);

	// ���� ����� ��� �ڽ� ��带 ��������� Ž���Ͽ� ���� ó���� �ݺ��մϴ�.
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		ReadModelData(node->mChildren[i], bones.size(), index);
	}
}

// ��忡�� �޽� �����͸� �о�� ���� ����ü�� �����ϴ� �Լ�
void ModelClass::ReadMeshData(aiNode* node, int bone)
{
	if (node->mNumMeshes < 1)
		return; // �޽ð� ���� ���� ó������ ����

	std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>(); // �� �޽� ��ü ����

	mesh->name = node->mName.C_Str(); // �޽� �̸� ����
	mesh->boneIndex = bone; // �޽ÿ� ����� �� �ε��� ����

	// ��忡 ���Ե� ��� �޽ÿ� ���� �ݺ�
	for (int i = 0; i < node->mNumMeshes; i++) {
		int index = node->mMeshes[i];
		const aiMesh* srcMesh = scene->mMeshes[index]; // �ҽ� �޽� ����

		// �޽��� ���� �̸��� ������
		const aiMaterial* material = scene->mMaterials[srcMesh->mMaterialIndex];
		mesh->materialName = material->GetName().C_Str(); // ���� �̸� ����

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

	meshes.push_back(mesh); // �޽ø� ���� �޽� ����Ʈ�� �߰�
}

// ���� ���� ���� ����ġ ������ �о�� ó���ϴ� �Լ�
void ModelClass::ReadSkinData()
{
	// ��� �޽ÿ� ���� �ݺ�
	for (int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* srcMesh = scene->mMeshes[i];
		if (!srcMesh->HasBones()) continue; // ���� ������ �ǳʶ�

		std::shared_ptr<Mesh> mesh = meshes[i]; // ���� �޽� ����

		std::vector<BoneWeights> tempVertexBoneWeights;
		tempVertexBoneWeights.resize(mesh->vertices.size()); // ������ ����ġ ����Ʈ �ʱ�ȭ

		// ��� ���� ���� �ݺ��Ͽ� ����ġ ���� ����
		for (int b = 0; b < srcMesh->mNumBones; b++) {
			aiBone* srcMeshBone = srcMesh->mBones[b];
			int boneIndex = GetBoneIndex(srcMeshBone->mName.C_Str()); // �� �ε��� �˻�

			for (unsigned int w = 0; w < srcMeshBone->mNumWeights; w++) {
				unsigned int index = srcMeshBone->mWeights[w].mVertexId; // ���� �ε���
				float weight = srcMeshBone->mWeights[w].mWeight; // ����ġ

				tempVertexBoneWeights[index].AddWeights(boneIndex, weight); // ����ġ ���� �߰�
			}
		}

		// ���� ����ġ ������ ���� �����Ϳ� ����
		for (unsigned int v = 0; v < tempVertexBoneWeights.size(); v++)
		{
			tempVertexBoneWeights[v].Normalize(); // ����ġ ����ȭ

			BlendWeight blendWeight = tempVertexBoneWeights[v].GetBlendWeights();
			//mesh->vertices[v].blendIndices = blendWeight.indices; // �� �ε��� ����
			//mesh->vertices[v].blendWeights = blendWeight.weights; // �� ����ġ ����
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