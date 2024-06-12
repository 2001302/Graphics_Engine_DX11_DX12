#ifndef _PIPELINEMANAGER
#define _PIPELINEMANAGER

#include "dataBlock.h"
#include "input.h"
#include "camera.h"
#include "game_object.h"
#include "normal_shader.h"
#include "phong_shader.h"
#include "direct3D.h"

namespace Engine
{
	/// <summary>
	/// Rendering에 필요한 shared object의 집합.
	/// </summary>
	class PipelineManager : public IDataBlock
	{
	public:
		std::vector <GameObject*> models;
		std::unique_ptr<PhongShader> phongShader;
		std::unique_ptr<Camera> camera;
	};
}
#endif