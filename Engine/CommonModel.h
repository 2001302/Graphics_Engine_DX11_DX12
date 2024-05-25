#ifndef _CommonModel
#define _CommonModel

#include "inputclass.h"
#include "cameraclass.h"
#include "gameobject.h"
#include "light.h"
#include "lightshader.h"
#include "D3dmanager.h"

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
	/// Rendering�� �ʿ��� shared object�� ����.
	/// </summary>
	class Manager : public IDataBlock
	{
	public:
		std::vector <GameObject*> Models;
		std::unique_ptr<Light> Light;
		std::unique_ptr<LightShader> LightShader;
		std::unique_ptr<CameraClass> Camera;
		std::shared_ptr<TextureClass> Texture; //default
	};

	class ViewingPoint : public IDataBlock
	{
	public:
		XMMATRIX WorldMatrix;
		XMMATRIX ViewMatrix;
		XMMATRIX ProjectionMatrix;
	};
}
#endif