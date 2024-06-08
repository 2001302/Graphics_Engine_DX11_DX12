#ifndef _BehaviorLeaf
#define _BehaviorLeaf

#include "BehaviorTree.h"

namespace Engine
{
	class InitializeCamera : public ActionNode
	{
		EnumBehaviorTreeStatus Invoke() override;
	};

	class InitializeLightShader : public ActionNode
	{
		EnumBehaviorTreeStatus Invoke() override;
		HWND m_window;
	public:
		InitializeLightShader() { m_window = 0; };
		InitializeLightShader(HWND hwnd) { m_window = hwnd; };
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

	class RenderGameObjectsWithPhong : public ActionNode
	{
		EnumBehaviorTreeStatus Invoke() override;
	};
}
#endif