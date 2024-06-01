#ifndef _IShader
#define _IShader

#include "CommonStruct.h"

namespace Engine
{
	class IShader
	{
	public:
		virtual void SetParam() {};
		virtual void Initialize() {};
		virtual void Execute() {};
		virtual void Dispose() {};
	};
}
#endif