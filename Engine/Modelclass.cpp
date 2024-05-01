#include "modelclass.h"

ModelClass::ModelClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_Texture = 0;
	m_model = 0;
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
	return m_indexCount;
}

ID3D11ShaderResourceView* ModelClass::GetTexture()
{
	return m_Texture->GetTexture();
}

bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
    D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];

	// Create the index array.
	indices = new unsigned long[m_indexCount];

	// Load the vertex array and index array with data.
	for(i=0; i<m_vertexCount; i++)
	{
		vertices[i].position = XMFLOAT3(m_model[i].x, m_model[i].y, m_model[i].z);
		vertices[i].texture = XMFLOAT2(m_model[i].tu, m_model[i].tv);
		vertices[i].normal = XMFLOAT3(m_model[i].nx, m_model[i].ny, m_model[i].nz);

		indices[i] = i;
	}

	// Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
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
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if(FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete [] vertices;
	vertices = 0;

	delete [] indices;
	indices = 0;

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

	if (extension == "txt") 
	{
		ifstream fin;
		char input;
		int i;

		// Open the model file.
		fin.open(filename);

		// If it could not open the file then exit.
		if (fin.fail())
		{
			return false;
		}

		// Read up to the value of vertex count.
		fin.get(input);
		while (input != ':')
		{
			fin.get(input);
		}

		// Read in the vertex count.
		fin >> m_vertexCount;

		// Set the number of indices to be the same as the vertex count.
		m_indexCount = m_vertexCount;

		// Create the model using the vertex count that was read in.
		m_model = new ModelType[m_vertexCount];

		// Read up to the beginning of the data.
		fin.get(input);
		while (input != ':')
		{
			fin.get(input);
		}
		fin.get(input);
		fin.get(input);

		// Read in the vertex data.
		for (i = 0; i < m_vertexCount; i++)
		{
			fin >> m_model[i].x >> m_model[i].y >> m_model[i].z;
			fin >> m_model[i].tu >> m_model[i].tv;
			fin >> m_model[i].nx >> m_model[i].ny >> m_model[i].nz;
		}

		// Close the model file.
		fin.close();
	}
	else if (extension == "obj")
	{
		Maya maya;
		maya.LoadModelMaya(filename);
		ConvertFromMaya(&maya);
	}

	return true;
}

void ModelClass::ConvertFromMaya(Maya* maya)
{
	m_vertexCount = maya->faceCount * 3;
	// Set the number of indices to be the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Create the model using the vertex count that was read in.
	m_model = new ModelType[maya->faceCount*3];

	int index = 0;
	// Now loop through all the faces and output the three vertices for each face.
	for (int i = 0; i < maya->faceCount; i++)
	{
		int vIndex = maya->faces[i].vIndex1 - 1;
		int tIndex = maya->faces[i].tIndex1 - 1;
		int nIndex = maya->faces[i].nIndex1 - 1;

		m_model[index].x = maya->vertices[vIndex].x;
		m_model[index].y = maya->vertices[vIndex].y;
		m_model[index].z = maya->vertices[vIndex].z;

		m_model[index].tu = maya->texcoords[tIndex].x;
		m_model[index].tv = maya->texcoords[tIndex].y;

		m_model[index].nx = maya->normals[nIndex].x;
		m_model[index].ny = maya->normals[nIndex].y;
		m_model[index].nz = maya->normals[nIndex].z;

		index++;

		vIndex = maya->faces[i].vIndex2 - 1;
		tIndex = maya->faces[i].tIndex2 - 1;
		nIndex = maya->faces[i].nIndex2 - 1;

		m_model[index].x = maya->vertices[vIndex].x;
		m_model[index].y = maya->vertices[vIndex].y;
		m_model[index].z = maya->vertices[vIndex].z;

		m_model[index].tu = maya->texcoords[tIndex].x;
		m_model[index].tv = maya->texcoords[tIndex].y;

		m_model[index].nx = maya->normals[nIndex].x;
		m_model[index].ny = maya->normals[nIndex].y;
		m_model[index].nz = maya->normals[nIndex].z;

		index++;

		vIndex = maya->faces[i].vIndex3 - 1;
		tIndex = maya->faces[i].tIndex3 - 1;
		nIndex = maya->faces[i].nIndex3 - 1;

		m_model[index].x = maya->vertices[vIndex].x;
		m_model[index].y = maya->vertices[vIndex].y;
		m_model[index].z = maya->vertices[vIndex].z;

		m_model[index].tu = maya->texcoords[tIndex].x;
		m_model[index].tv = maya->texcoords[tIndex].y;

		m_model[index].nx = maya->normals[nIndex].x;
		m_model[index].ny = maya->normals[nIndex].y;
		m_model[index].nz = maya->normals[nIndex].z;

		index++;
	}
	index;
}

void ModelClass::ReleaseModel()
{
	if(m_model)
	{
		delete [] m_model;
		m_model = 0;
	}

	return;
}

bool Maya::LoadModelMaya(const char* filename)
{
	int vertexCount = 0;
	int	textureCount = 0;
	int normalCount = 0;

	boost::iostreams::mapped_file mmap(filename, boost::iostreams::mapped_file::readonly);

	{
		const char* start = mmap.const_data();
		const char* end = start + mmap.size();
		const char* current = start;

		while (current && current != end) {
			const char* next = static_cast<const char*>(memchr(current, '\n', end - current));

			// If 'next' is nullptr, then we are at the last line (which might not end with a newline character)
			if (!next) next = end;

			// Extract the line between 'current' and 'next'
			std::string line(current, next);

			if (line.substr(0, 2) == "v ") { vertexCount++; }
			if (line.substr(0, 2) == "vt") { textureCount++; }
			if (line.substr(0, 2) == "vn") { normalCount++; }
			if (line.substr(0, 2) == "f ") { faceCount++; }

			// Move 'current' to the character after the newline
			current = next + 1;
		}
	}
	{
		// Now read the data from the file into the data structures and then output it in our model format.
		int vertexIndex, texcoordIndex, normalIndex, faceIndex, vIndex, tIndex, nIndex;
		char input, input2;

		// Initialize the four data structures.
		vertices = new VertexTypeMaya[vertexCount];
		texcoords = new VertexTypeMaya[textureCount];
		normals = new VertexTypeMaya[normalCount];
		faces = new FaceType[faceCount];

		// Initialize the indexes.
		vertexIndex = 0;
		texcoordIndex = 0;
		normalIndex = 0;
		faceIndex = 0;

		const char* start = mmap.const_data();
		const char* end = start + mmap.size();
		const char* current = start;

		while (current && current != end) {
			const char* next = static_cast<const char*>(memchr(current, '\n', end - current));

			// If 'next' is nullptr, then we are at the last line (which might not end with a newline character)
			if (!next) next = end;

			// Extract the line between 'current' and 'next'
			std::string line(current, next);

			istringstream iss(line);

			std::string input;
			iss >> input;  //read first string on the line

			if (input == "v")
			{
				iss >> vertices[vertexIndex].x >> vertices[vertexIndex].y >> vertices[vertexIndex].z;
				// Invert the Z vertex to change to left hand system.
				vertices[vertexIndex].z = vertices[vertexIndex].z * -1.0f;
				vertexIndex++;
			}
			if (input == "vt")
			{
				iss >> texcoords[texcoordIndex].x >> texcoords[texcoordIndex].y;
				// Invert the V texture coordinates to left hand system.
				texcoords[texcoordIndex].y = 1.0f - texcoords[texcoordIndex].y;
				texcoordIndex++;
			}
			if (input == "vn")
			{
				iss >> normals[normalIndex].x >> normals[normalIndex].y >> normals[normalIndex].z;
				// Invert the Z normal to change to left hand system.
				normals[normalIndex].z = normals[normalIndex].z * -1.0f;
				normalIndex++;
			}

			// Read in the faces.
			if (input == "f")
			{
				// Read the face data in backwards to convert it to a left hand system from right hand system.
				iss >> faces[faceIndex].vIndex3 >> input2 >> faces[faceIndex].tIndex3 >> input2 >> faces[faceIndex].nIndex3
					>> faces[faceIndex].vIndex2 >> input2 >> faces[faceIndex].tIndex2 >> input2 >> faces[faceIndex].nIndex2
					>> faces[faceIndex].vIndex1 >> input2 >> faces[faceIndex].tIndex1 >> input2 >> faces[faceIndex].nIndex1;
				faceIndex++;
			}
			// Move 'current' to the character after the newline
			current = next + 1;
		}
	}
	return true;
}