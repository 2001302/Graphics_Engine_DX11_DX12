#ifndef CAMERA
#define CAMERA

#include "common_struct.h"

class Camera
{
public:
	void Render();

	DirectX::SimpleMath::Matrix view;
	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector3 rotation;
};

#endif