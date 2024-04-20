#pragma once
#include "D3dclass.h"
#include "CommonModel.h"
#include "BehaviorTree.h"

namespace Engine
{
	class LoadModelData : public ActionNode 
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

	class RenderModels : public ActionNode
	{
		EnumBehaviorTreeStatus Invoke() override;
	};
}