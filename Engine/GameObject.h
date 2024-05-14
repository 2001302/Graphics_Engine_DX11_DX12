#ifndef _GAMEOBJECT
#define _GAMEOBJECT

#include <d3d11.h>
#include <directxmath.h>

#include "TextureClass.h"
#include "Header.h"

namespace Engine
{
	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT4 blendIndices;
		DirectX::XMFLOAT4 blendWeights;
	};
	struct Bone
	{
		int index;
		std::string name;
		int parent;
		DirectX::XMMATRIX transform;
	};
	struct Mesh
	{
		std::string name;
		int boneIndex;
		std::string materialName;
		std::vector<VertexType> vertices;
		std::vector<int> indices;
	};

	class GameObject
	{
	public:
		GameObject();
		GameObject(const GameObject&);
		~GameObject();

		bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
		void Shutdown();
		void Render(ID3D11DeviceContext* deviceContext);

		int GetIndexCount();

		DirectX::XMMATRIX transform;
		std::vector<std::shared_ptr<Bone>> bones;
		std::vector<std::shared_ptr<Mesh>> meshes;
		std::shared_ptr<TextureClass> texture;

	private:
		bool InitializeBuffers(ID3D11Device* device);
		void ShutdownBuffers();
		void RenderBuffers(ID3D11DeviceContext* deviceContext);

		ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
		
	};

}
#endif