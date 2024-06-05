#ifndef _PipelineManager
#define _PipelineManager

#include "input.h"
#include "camera.h"
#include "gameobject.h"
#include "light.h"
#include "lightshader.h"
#include "direct3D.h"

namespace Engine
{
	/// <summary>
	/// Abstract class
	/// BehaviorTree traversal ���� �� ���Ǵ� DataBlock ����.
	/// </summary>
	class IDataBlock
	{
	public:
		virtual ~IDataBlock() {};
	};

	class Env : public IDataBlock
	{
	public:
		const float screenDepth = 1000.0f;
		const float screenNear = 0.3f;
		float fieldOfView = M_PI / 4.0f;

		float aspect;
	};

	/// <summary>
	/// Rendering�� �ʿ��� shared object�� ����.
	/// </summary>
	class PipelineManager : public IDataBlock
	{
	public:
		std::vector <GameObject*> Models;
		std::vector <IShader*> Shaders;
		std::unique_ptr<Light> Light;
		std::unique_ptr<LightShader> LightShader;
		std::unique_ptr<Camera> Camera;
	};
}
#endif