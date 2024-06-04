#ifndef _BehaviorLeaf
#define _BehaviorLeaf

#include "BehaviorTree.h"

namespace Engine
{
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

	class RenderGameObjects : public ActionNode
	{
		EnumBehaviorTreeStatus Invoke() override;
	};
}
#endif