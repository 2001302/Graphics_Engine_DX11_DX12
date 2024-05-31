#ifndef _GAMEOBJECT
#define _GAMEOBJECT

#include <d3d11.h>
#include <directxmath.h>

#include "Header.h"
#include "Texture.h"
#include "CommonStruct.h"

namespace Engine
{
	class GameObject
	{
	public:
		GameObject();
		GameObject(const GameObject&);
		~GameObject();

		int GetIndexCount();

		DirectX::XMMATRIX transform;

		std::vector<std::shared_ptr<Bone>> bones;
		std::vector<std::shared_ptr<Mesh>> meshes;
		std::shared_ptr<Texture> texture;
		std::shared_ptr<Animation> animation;

		ID3D11Buffer* vertexBuffer;
		ID3D11Buffer* indexBuffer;
	};
}
#endif