#ifndef _GAMEOBJECT
#define _GAMEOBJECT

#include <d3d11.h>
#include <directxmath.h>

#include "Header.h"
#include "TextureClass.h"
#include "CommonStruct.h"

namespace Engine
{
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
		std::shared_ptr<Animation> animation;

	private:
		bool InitializeBuffers(ID3D11Device* device);
		void ShutdownBuffers();
		void RenderBuffers(ID3D11DeviceContext* deviceContext);

		ID3D11Buffer* m_vertexBuffer, * m_indexBuffer;
		
	};

}
#endif