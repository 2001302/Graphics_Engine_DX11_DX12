#ifndef _GAMEOBJECT
#define _GAMEOBJECT

#include "CommonStruct.h"
#include "PhongShader.h"

namespace Engine
{
	using Microsoft::WRL::ComPtr;

	class GameObject
	{
	public:
		GameObject();
		GameObject(const GameObject&);
		~GameObject();

		int GetIndexCount();

		DirectX::SimpleMath::Matrix transform;

		std::vector<std::shared_ptr<Bone>> bones;
		std::vector<std::shared_ptr<Mesh>> meshes;
		std::shared_ptr<Animation> animation;

		ComPtr<ID3D11Texture2D> texture;
		ComPtr<ID3D11ShaderResourceView> textureResourceView;
		ComPtr<ID3D11Buffer> vertexBuffer;
		ComPtr<ID3D11Buffer> indexBuffer;

		std::shared_ptr<PhongShaderSource> phongShader;
	};
}
#endif