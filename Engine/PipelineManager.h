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
		std::vector <GameObject*> Models;
		std::vector <IShader*> Shaders;
		std::unique_ptr<NormalShader> NormalShader;
		std::unique_ptr<PhongShader> PhongShader;
		std::unique_ptr<Camera> Camera;
	};
}
#endif