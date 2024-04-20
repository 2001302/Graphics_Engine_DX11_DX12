#pragma once

#include "inputclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "lightclass.h"
#include "rendertextureclass.h"
#include "lightshaderclass.h"
#include "refractionshaderclass.h"
#include "watershaderclass.h"
#include "D3dclass.h"

namespace Engine
{
	/// <summary>
	/// Abstract class
	/// BehaviorTree traversal 수행 시 사용되는 DataBlock 단위.
	/// </summary>
	class IDataBlock
	{
	public:
		virtual ~IDataBlock() {};
	};

	class Manager : public IDataBlock
	{
	public:
		Manager() {};
		~Manager() {};

		std::vector <ModelClass*> Models;
		LightClass* Light;
		LightShaderClass* LightShader;
		CameraClass* Camera;
	};
}