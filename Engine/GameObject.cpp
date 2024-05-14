#pragma warning(disable:6385)
#pragma warning(disable:6386)

#include "GameObject.h"

using namespace Engine;

GameObject::GameObject()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	texture = 0;
}

GameObject::GameObject(const GameObject& other)
{
}

GameObject::~GameObject()
{	
}

bool GameObject::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	bool result;

	// Initialize the vertex and index buffers.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}

	return true;
}
void GameObject::Shutdown()
{
	// Release the texture object.
	if (texture)
	{
		texture->Shutdown();
		texture.reset();
	}

	// Shutdown the vertex and index buffers.
	ShutdownBuffers();
	return;
}

void GameObject::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);

	return;
}

int GameObject::GetIndexCount()
{
	int count = 0;
	for (auto mesh : meshes)
		count += mesh->indices.size();
	return count;
}

bool GameObject::InitializeBuffers(ID3D11Device* device)
{
	std::vector<VertexType> vertices;
	std::vector<int> indices;

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
	if (FAILED(result))
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
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void GameObject::ShutdownBuffers()
{
	// Release the index buffer.
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}

void GameObject::RenderBuffers(ID3D11DeviceContext* deviceContext)
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

GameObject* ResourceHelper::ImportModel(Engine::GameObject* gameObject, const char* filename)
{
	Assimp::Importer importer;

	auto scene = importer.ReadFile(std::string(filename),
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

	//Using the Assimp library, start from the root node of the imported 3D model and traverse through the model data to read it
	ReadModelData(gameObject, scene, scene->mRootNode, -1, -1);

	//Read the skin (bone) data to be applied to the mesh.
	ReadSkinData(gameObject, scene);

	ReadAnimationData(gameObject, scene);

	return gameObject;
}

void ResourceHelper::ReadModelData(Engine::GameObject* gameObject, const aiScene* scene, aiNode* node, int index, int parent)
{
	std::shared_ptr<Engine::Bone> bone = std::make_shared<Engine::Bone>();

	bone->index = index;
	bone->parent = parent;
	bone->name = node->mName.C_Str();

	//Retrieve the bone's local transformation matrix, transpose it, and store it
	//Assimp uses column-major matrices, but since DirectX and OpenGL can use row-major matrices, transposing may be necessary.
	DirectX::XMMATRIX transform(node->mTransformation[0]);
	bone->transform = XMMatrixTranspose(transform);

	//Calculate the transformation relative to the root (or parent) bone.
	DirectX::XMMATRIX matParent = DirectX::XMMatrixIdentity();
	if (parent >= 0) {
		matParent = gameObject->bones[parent]->transform;
	}

	//Finally, calculate the transformation matrix of the bone.
	bone->transform = bone->transform * matParent;

	gameObject->bones.push_back(bone);

	//Read the mesh data linked to the current node (bone).
	ReadMeshData(gameObject, scene, node, index);

	//Recursively traverse all child nodes of the current node, repeating the same process.
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		ReadModelData(gameObject, scene, node->mChildren[i], gameObject->bones.size(), index);
	}
}

void ResourceHelper::ReadMeshData(Engine::GameObject* gameObject, const aiScene* scene, aiNode* node, int bone)
{
	//Do not process nodes without a mesh.
	if (node->mNumMeshes < 1)
		return;

	std::shared_ptr<Engine::Mesh> mesh = std::make_shared<Engine::Mesh>();

	mesh->name = node->mName.C_Str();
	mesh->boneIndex = bone; //Set the bone indices associated with the mesh.

	for (int i = 0; i < node->mNumMeshes; i++) {
		int index = node->mMeshes[i];
		const aiMesh* srcMesh = scene->mMeshes[index];

		const aiMaterial* material = scene->mMaterials[srcMesh->mMaterialIndex];
		mesh->materialName = material->GetName().C_Str();

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

	gameObject->meshes.push_back(mesh);
}

void ResourceHelper::ReadSkinData(Engine::GameObject* gameObject, const aiScene* scene)
{
	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* srcMesh = scene->mMeshes[i];

		//Skip if there are no bones
		if (!srcMesh->HasBones()) continue;

		std::shared_ptr<Engine::Mesh> mesh = gameObject->meshes[i];

		std::vector<Engine::BoneWeights> tempVertexBoneWeights;
		tempVertexBoneWeights.resize(mesh->vertices.size());

		for (int b = 0; b < srcMesh->mNumBones; b++) {
			aiBone* srcMeshBone = srcMesh->mBones[b];
			int boneIndex = GetBoneIndex(gameObject, srcMeshBone->mName.C_Str());

			for (unsigned int w = 0; w < srcMeshBone->mNumWeights; w++) {
				unsigned int index = srcMeshBone->mWeights[w].mVertexId;
				float weight = srcMeshBone->mWeights[w].mWeight;

				tempVertexBoneWeights[index].AddWeights(boneIndex, weight);
			}
		}

		for (unsigned int v = 0; v < tempVertexBoneWeights.size(); v++)
		{
			tempVertexBoneWeights[v].Normalize();

			BlendWeight blendWeight = tempVertexBoneWeights[v].GetBlendWeights();
			mesh->vertices[v].blendIndices = blendWeight.indices;
			mesh->vertices[v].blendWeights = blendWeight.weights;
		}
	}
}

unsigned int ResourceHelper::GetBoneIndex(Engine::GameObject* gameObject, const std::string& name)
{
	for (std::shared_ptr<Bone>& bone : gameObject->bones)
	{
		if (bone->name == name)
			return bone->index;
	}

	assert(false);
	return 0;
}

void ResourceHelper::ReadAnimationData(Engine::GameObject* gameObject, const aiScene* scene)
{
	// Check if the scene has animations
	if (scene->HasAnimations()) {
		for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
			aiAnimation* anim = scene->mAnimations[i];
			std::cout << "Animation: " << anim->mName.C_Str() << "\n";
			std::cout << "Duration: " << anim->mDuration << "\n";
			std::cout << "Ticks per second: " << anim->mTicksPerSecond << "\n";

			// Process each channel (each channel corresponds to a bone)
			for (unsigned int j = 0; j < anim->mNumChannels; j++) {
				aiNodeAnim* channel = anim->mChannels[j];

				// Print bone name
				std::cout << "Bone: " << channel->mNodeName.C_Str() << "\n";

				// Position keys
				for (unsigned int k = 0; k < channel->mNumPositionKeys; k++) {
					aiVectorKey key = channel->mPositionKeys[k];
					std::cout << "Position Key: Time: " << key.mTime << " Value: " << key.mValue.x << ", " << key.mValue.y << ", " << key.mValue.z << "\n";
				}

				// Rotation keys
				for (unsigned int k = 0; k < channel->mNumRotationKeys; k++) {
					aiQuatKey key = channel->mRotationKeys[k];
					std::cout << "Rotation Key: Time: " << key.mTime << " Value: " << key.mValue.w << ", " << key.mValue.x << ", " << key.mValue.y << ", " << key.mValue.z << "\n";
				}

				// Scaling keys
				for (unsigned int k = 0; k < channel->mNumScalingKeys; k++) {
					aiVectorKey key = channel->mScalingKeys[k];
					std::cout << "Scale Key: Time: " << key.mTime << " Value: " << key.mValue.x << ", " << key.mValue.y << ", " << key.mValue.z << "\n";
				}
			}
		}
	}
}

GameObject* ResourceHelper::ImportTexture(GameObject* gameObject, ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* filename)
{
	gameObject->texture->Initialize(device, deviceContext, filename);
	return gameObject;
}
GameObject* ResourceHelper::ImportTexture(GameObject* gameObject, std::shared_ptr<TextureClass> texture)
{
	gameObject->texture = texture;
	return gameObject;
}