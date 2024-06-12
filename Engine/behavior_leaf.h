#ifndef _BehaviorLeaf
#define _BehaviorLeaf

#include "behavior_tree.h"

namespace Engine
{
	class InitializeCamera : public ActionNode
	{
		EnumBehaviorTreeStatus Invoke() override;
	};

	class InitializePhongShader : public ActionNode
	{
		EnumBehaviorTreeStatus Invoke() override;
		HWND m_window;
	public:
		InitializePhongShader() { m_window = 0; };
		InitializePhongShader(HWND hwnd) { m_window = hwnd; };
	};

	class RenderGameObjects : public ActionNode
	{
		EnumBehaviorTreeStatus Invoke() override;
	};
}
#endif