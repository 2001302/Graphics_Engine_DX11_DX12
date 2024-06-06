#ifndef _PIPELINEMANAGER
#define _PIPELINEMANAGER

#include "IDataBlock.h"
#include "Input.h"
#include "Camera.h"
#include "Gameobject.h"
#include "Light.h"
#include "Lightshader.h"
#include "Direct3D.h"

namespace Engine
{
	/// <summary>
	/// Rendering에 필요한 shared object의 집합.
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