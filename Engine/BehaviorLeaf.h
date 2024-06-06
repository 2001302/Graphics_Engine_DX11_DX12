#ifndef _BehaviorLeaf
#define _BehaviorLeaf

#include "BehaviorTree.h"
#include "PipelineManager.h"

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

	class InitializeLightShader : public ActionNode
	{
		EnumBehaviorTreeStatus Invoke() override;
		HWND m_window;
	public:
		InitializeLightShader() { m_window = 0; };
		InitializeLightShader(HWND hwnd) { m_window = hwnd; };
	};

	class RenderGameObjects : public ActionNode
	{
		EnumBehaviorTreeStatus Invoke() override;
	};
}
#endif