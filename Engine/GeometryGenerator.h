#ifndef _GEOMETRYGENERATOR
#define _GEOMETRYGENERATOR	

#include "GameObject.h"
#include "Direct3D.h"
#include "ModelLoader.h"

namespace Engine
{
	class GeometryGenerator
	{

	public:
		GeometryGenerator() {};
		~GeometryGenerator() {};

		static GameObject* MakeSphere(GameObject* gameObject, const float radius, const int numSlices, const int numStacks);
		static GameObject* MakeBox(GameObject* gameObject);
		static GameObject* MakeCylinder(GameObject* gameObject, const float bottomRadius,const float topRadius, float height,int numSlices);
		static GameObject* ReadFromFile(GameObject* gameObject, std::string basePath, std::string filename);
	};
}
#endif