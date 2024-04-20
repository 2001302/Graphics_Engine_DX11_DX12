#pragma once
#include "D3dclass.h"
#include "Manager.h"
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
	};
}