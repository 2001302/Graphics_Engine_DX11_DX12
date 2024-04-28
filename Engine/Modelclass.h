#include <d3d11.h>
#include <directxmath.h>
#include <fstream>

#include "textureclass.h"

using namespace DirectX;
using namespace std;

class Maya
{
private:
	struct VertexTypeMaya
	{
		float x, y, z;
	};
	struct FaceType
	{
		int vIndex1, vIndex2, vIndex3;
		int tIndex1, tIndex2, tIndex3;
		int nIndex1, nIndex2, nIndex3;
	};
public:
	int vertexCount, textureCount, normalCount, faceCount;
	VertexTypeMaya* vertices, * texcoords, * normals;
	FaceType* faces;

	bool LoadModelMaya(const char* filename);
	bool ReadFileCounts(const char* filename);
	bool LoadDataStructures(const char* filename);
};

class ModelClass
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};
	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};
public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* modelFilename, const char* textureFilename);
	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* modelFilename, TextureClass* texture);
	void Shutdown();
	void Render(ID3D11DeviceContext* deviceContext);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();
	void ConvertFromMaya(Maya* maya);

	XMMATRIX Transform;
private:
	bool InitializeBuffers(ID3D11Device* device);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* deviceContext);

	bool LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* filename);
	void ReleaseTexture();

	bool LoadModel(const char* filename);
	void ReleaseModel();

	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
	int m_vertexCount, m_indexCount;
	TextureClass* m_Texture;
	ModelType* m_model;
};