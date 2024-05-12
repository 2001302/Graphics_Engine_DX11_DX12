#pragma warning(disable:6385)
#pragma warning(disable:6386)

#include "modelclass.h"

ModelClass::ModelClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_Texture = 0;

	vertices.clear();
	indices.clear();
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
	
	m_Texture = std::move(texture);

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
	return m_Texture->GetTexture();
}

bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	//VertexType* vertices;
	//unsigned long* indices;

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
	m_Texture = std::make_unique<TextureClass>();

	result = m_Texture->Initialize(device, deviceContext, filename);
	if(!result)
	{
		return false;
	}

	return true;
}


void ModelClass::ReleaseTexture()
{
	// Release the texture object.
	if(m_Texture)
	{
		m_Texture->Shutdown();
		m_Texture.reset();
	}

	return;
}


bool ModelClass::LoadModel(const char* filename)
{
	auto GetModelType = [](const char* filename) -> std::string 
		{
			// find last dot('.') in file path
			std::string name(filename);
			std::size_t lastDot = name.find_last_of(".");
		
			if (lastDot == std::string::npos) 
				return "";

			// extract last element
			return name.substr(lastDot + 1);
		};

	auto extension = GetModelType(filename);

	if (extension == "obj")
	{
		LoadObjectModel(filename);
	}
	else if (extension == "fbx")
	{
		LoadFBXModel(filename);
	}

	return true;
}

bool ModelClass::LoadObjectModel(const char* filename)
{
	Assimp::Importer importer;

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

	const aiScene* scene = importer.ReadFile(std::string(filename), 
	aiProcess_Triangulate |						
	aiProcess_ConvertToLeftHanded);            

	for (unsigned int i = 0; i < scene->mNumMeshes; i++) 
	{
		aiMesh* mesh = scene->mMeshes[i];

		for (unsigned int k = 0; k < mesh->mNumVertices; k++)
		{
			VertexType vertice;

			XMFLOAT3 pos(&mesh->mVertices[k].x);
			vertice.position = pos;

			XMFLOAT3 normal(&mesh->mNormals[k].x);
			vertice.normal = normal;

			if (mesh->mTextureCoords[0])
			{
				XMFLOAT2 tex = XMFLOAT2(&mesh->mTextureCoords[0][k].x);
				vertice.texture = tex;
			}

			vertices.push_back(vertice);
		}

		for (UINT i = 0; i < mesh->mNumFaces; ++i) 
		{
			const aiFace& face = mesh->mFaces[i];

			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}
	}

	return true;
}

bool ModelClass::LoadFBXModel(const char* filename)
{
	Assimp::Importer importer;

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

	const aiScene* scene = importer.ReadFile(std::string(filename),
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded);

	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[i];

		for (unsigned int k = 0; k < mesh->mNumVertices; k++)
		{
			VertexType vertice;

			XMFLOAT3 pos(&mesh->mVertices[k].x);
			vertice.position = pos;

			XMFLOAT3 normal(&mesh->mNormals[k].x);
			vertice.normal = normal;

			if (mesh->mTextureCoords[0])
			{
				XMFLOAT2 tex = XMFLOAT2(&mesh->mTextureCoords[0][k].x);
				vertice.texture = tex;
			}

			vertices.push_back(vertice);
		}

		for (UINT i = 0; i < mesh->mNumFaces; ++i)
		{
			const aiFace& face = mesh->mFaces[i];

			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}
	}

	return true;
}

void ModelClass::ReleaseModel()
{
	vertices.clear();
	indices.clear();

	return;
}
