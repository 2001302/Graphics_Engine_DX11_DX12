#ifndef _PIPELINEMANAGER
#define _PIPELINEMANAGER

#include "IDataBlock.h"
#include "Input.h"
#include "Camera.h"
#include "Gameobject.h"
#include "NormalShader.h"
#include "PhongShader.h"
#include "Direct3D.h"

namespace Engine
{
	/// <summary>
	/// Rendering�� �ʿ��� shared object�� ����.
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