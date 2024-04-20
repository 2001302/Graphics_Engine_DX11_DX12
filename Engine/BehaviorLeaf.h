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
		InitializeLight() {};
		InitializeLight(HWND hwnd) { m_window = hwnd; };
	};
}