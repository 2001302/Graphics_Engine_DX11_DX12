#ifndef _BehaviorLeaf
#define _BehaviorLeaf

#include "D3dclass.h"
#include "CommonModel.h"
#include "BehaviorTree.h"

namespace Engine
{
	class LoadTextureData : public ActionNode
	{
		EnumBehaviorTreeStatus Invoke() override;
	};

	class InitializeCamera : public ActionNode
	{
		EnumBehaviorTreeStatus Invoke() override;
	};

	class InitializeLight : public ActionNode
	{
		EnumBehaviorTreeStatus Invoke() override;
		HWND m_window;
	public :
		InitializeLight() { m_window = 0; };
		InitializeLight(HWND hwnd) { m_window = hwnd; };
	};

	class GetViewingPoint : public ActionNode
	{
		EnumBehaviorTreeStatus Invoke() override;
	};

	class RenderGameObjects : public ActionNode
	{
		EnumBehaviorTreeStatus Invoke() override;
	};
}
#endif