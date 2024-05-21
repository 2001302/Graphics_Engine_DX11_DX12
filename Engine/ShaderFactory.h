#ifndef _SHADERFACTORY
#define _SHADERFACTORY

#include "Header.h"

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

	class ShaderFactory
	{
	public:
		void Initialize() {};
		void Select() {};
		void Render() {};

		std::map<int, std::shared_ptr<IShader>> shaders;
	};
}
#endif