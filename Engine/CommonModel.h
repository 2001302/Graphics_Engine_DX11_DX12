#ifndef _CommonModel
#define _CommonModel

#include "inputclass.h"
#include "cameraclass.h"
#include "gameobject.h"
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

	/// <summary>
	/// Disposer Pattern
	/// </summary>
	class IDisposable
	{
	public:
		~IDisposable() 
		{ 
			Dispose(); 
		};
		virtual void Dispose() {};
	};

	/// <summary>
	/// Rendering에 필요한 shared object의 집합.
	/// </summary>
	class Manager : public IDataBlock
	{
	public:
		Manager() 
		{
		};
		~Manager() {};

		std::vector <GameObject*> Models;
		std::unique_ptr<LightClass> Light;
		std::unique_ptr<LightShaderClass> LightShader;
		std::unique_ptr<CameraClass> Camera;
		std::unique_ptr<TextureClass> Texture; //default
	};

	class ViewingPoint : public IDataBlock
	{
	public:
		ViewingPoint()
		{

		};
		~ViewingPoint() {};

		XMMATRIX WorldMatrix;
		XMMATRIX ViewMatrix;
		XMMATRIX ProjectionMatrix;
	};
}
#endif