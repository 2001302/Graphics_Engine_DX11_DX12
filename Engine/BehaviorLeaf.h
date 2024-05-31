#ifndef _BehaviorLeaf
#define _BehaviorLeaf

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
	};

	class InitializeShader : public ActionNode
	{
		EnumBehaviorTreeStatus Invoke() override;
		HWND m_window;
	public:
		InitializeShader() { m_window = 0; };
		InitializeShader(HWND hwnd) { m_window = hwnd; };
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