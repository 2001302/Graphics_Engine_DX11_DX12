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

		std::shared_ptr<PhongShaderSource> phongShader;
	};
}
#endif