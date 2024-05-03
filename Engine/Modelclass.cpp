#pragma warning(disable:6385)
#pragma warning(disable:6386)

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
		LoadTextModel(filename);
	}
	else if (extension == "obj")
	{
		LoadMayaModel(filename);
	}

	return true;
}

bool ModelClass::LoadTextModel(const char* filename)
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
	return true;
}

bool ModelClass::LoadMayaModel(const char* filename)
{
	boost::iostreams::mapped_file mmap(filename, boost::iostreams::mapped_file::readonly);

	std::vector<XMFLOAT3> vertices;
	std::vector<XMFLOAT2> texcoords;
	std::vector<XMFLOAT3> normals;
	std::vector<std::pair<const char*, const char*>> faceStarts;

	{
		//Now read the data from the file into the data structures and then output it in our model format.
		const char* start = mmap.const_data();
		const char* end = start + mmap.size();
		const char* current = start;

		while (current && current != end) 
		{
			const char* next = static_cast<const char*>(memchr(current, '\n', end - current));
			
			//extract the line
			std::string line(current, next);

			std::deque<std::string> words;
			boost::split(words, line, boost::is_any_of(" "), boost::token_compress_on);

			if (words.front() == "v")
			{
				words.pop_front();
				assert(words.size() == 3);

				//invert the Z vertex to change to left hand system.
				XMFLOAT3 vertex;
				vertex.x = std::stof(words.front());
				words.pop_front();
				vertex.y = std::stof(words.front());
				words.pop_front();
				vertex.z = std::stof(words.front()) * -1.0f;
				words.pop_front();

				vertices.push_back(vertex);
			}
			else if (words.front() == "vt")
			{
				words.pop_front();
				assert(words.size() == 2);

				XMFLOAT2 texcoord;
				texcoord.x = std::stof(words.front());
				words.pop_front();
				texcoord.y = 1.0f - std::stof(words.front());
				words.pop_front();

				texcoords.push_back(texcoord);
			}
			else if (words.front() == "vn")
			{
				words.pop_front();
				assert(words.size() == 3);

				//invert the Z normal to change to left hand system.
				XMFLOAT3 normal;
				normal.x = std::stof(words.front());
				words.pop_front();
				normal.y = std::stof(words.front());
				words.pop_front();
				normal.z = std::stof(words.front()) * -1.0f;
				words.pop_front();

				normals.push_back(normal);
			}
			else if (words.front() == "f")
			{
				faceStarts.push_back(make_pair(current,next));
			}
			//move 'current' to the character after the newline
			current = next + 1;
		}

		{
			// Set the number of indices to be the same as the vertex count.
			m_vertexCount = faceStarts.size() * 3;
			m_indexCount = m_vertexCount;
			m_model = new ModelType[m_vertexCount];

			for (int i =0 ;i< faceStarts.size();i++ )
			{
				int index = i * 3;
				//extract the line
				std::string line(faceStarts[i].first, faceStarts[i].second);

				std::deque<std::string> words;
				boost::split(words, line, boost::is_any_of(" /"), boost::token_compress_on);

				words.pop_front();
				assert(words.size() == 9);

				{
					auto nIndex1 = std::stof(words.back()) - 1;
					m_model[index].nx = normals[nIndex1].x;
					m_model[index].ny = normals[nIndex1].y;
					m_model[index].nz = normals[nIndex1].z;
					words.pop_back();

					auto tIndex1 = std::stof(words.back()) - 1;
					m_model[index].tu = texcoords[tIndex1].x;
					m_model[index].tv = texcoords[tIndex1].y;
					words.pop_back();

					auto vIndex1 = std::stof(words.back()) - 1;
					m_model[index].x = vertices[vIndex1].x;
					m_model[index].y = vertices[vIndex1].y;
					m_model[index].z = vertices[vIndex1].z;
					words.pop_back();
				}
				{
					auto nIndex1 = std::stof(words.back()) - 1;
					m_model[index+1].nx = normals[nIndex1].x;
					m_model[index+1].ny = normals[nIndex1].y;
					m_model[index+1].nz = normals[nIndex1].z;
					words.pop_back();

					auto tIndex1 = std::stof(words.back()) - 1;
					m_model[index+1].tu = texcoords[tIndex1].x;
					m_model[index+1].tv = texcoords[tIndex1].y;
					words.pop_back();

					auto vIndex1 = std::stof(words.back()) - 1;
					m_model[index+1].x = vertices[vIndex1].x;
					m_model[index+1].y = vertices[vIndex1].y;
					m_model[index+1].z = vertices[vIndex1].z;
					words.pop_back();
				} 
				{
					auto nIndex1 = std::stof(words.back()) - 1;
					m_model[index+2].nx = normals[nIndex1].x;
					m_model[index+2].ny = normals[nIndex1].y;
					m_model[index+2].nz = normals[nIndex1].z;
					words.pop_back();

					auto tIndex1 = std::stof(words.back()) - 1;
					m_model[index+2].tu = texcoords[tIndex1].x;
					m_model[index+2].tv = texcoords[tIndex1].y;
					words.pop_back();

					auto vIndex1 = std::stof(words.back()) - 1;
					m_model[index+2].x = vertices[vIndex1].x;
					m_model[index+2].y = vertices[vIndex1].y;
					m_model[index+2].z = vertices[vIndex1].z;
					words.pop_back();
				}
			}
		}
	}
	return true;
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
