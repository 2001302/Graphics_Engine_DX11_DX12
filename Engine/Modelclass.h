#ifndef _MODEL
#define _MODEL

#include <d3d11.h>
#include <directxmath.h>

#include "TextureClass.h"
#include "Header.h"

using namespace DirectX;
using namespace std;

class ModelClass
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};
public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* modelFilename, const char* textureFilename);
	bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* modelFilename, std::unique_ptr<TextureClass>& texture);
	void Shutdown();
	void Render(ID3D11DeviceContext* deviceContext);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

	XMMATRIX Transform;
private:
	bool InitializeBuffers(ID3D11Device* device);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* deviceContext);

	bool LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* filename);
	void ReleaseTexture();

	bool LoadModel(const char* filename);
	bool LoadObjectModel(const char* filename);
	bool LoadFBXModel(const char* filename);
	
	void ReleaseModel();

	ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
	std::unique_ptr<TextureClass> m_Texture;

	std::vector <VertexType> vertices;
	std::vector<unsigned long> indices;
};
#endif