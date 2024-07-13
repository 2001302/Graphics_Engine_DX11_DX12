#ifndef _COMMONSTRUCT
#define _COMMONSTRUCT

#define _USE_MATH_DEFINES
#define WM_MODEL_LOAD (WM_APP + 1)
#define WM_BOX_LOAD (WM_APP + 2)
#define WM_CYLINDER_LOAD (WM_APP + 3)
#define WM_SPHERE_LOAD (WM_APP + 4)

// clang-format off
#include <map>
#include <vector>
#include <algorithm>
#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>		//for text file open
#include <cmath>		//for calculate coordination
#include <omp.h>		//for omp parallel
#include <commdlg.h>	//for file open
#include <wrl.h>
#include <memory>
#include <shellscalingapi.h>

#include <d3d11.h>
#include <directxmath.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>
#include <directxtk/DDSTextureLoader.h>

#include <assimp/importer.hpp>
#include <assimp/scene.h>           
#include <assimp/postprocess.h>

// clang-format on

#pragma comment(lib, "shcore.lib")


namespace dx11 {
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;
using Microsoft::WRL::ComPtr;

inline void ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr)) {
        throw std::exception();
    }
}

struct Vertex {
    Vector3 position;
    Vector3 normal;
    Vector2 texcoord;
    Vector3 tangent;
};

struct Material {
    Vector3 ambient = Vector3(0.1f);  // 12
    float shininess = 1.0f;           // 4
    Vector3 diffuse = Vector3(0.5f);  // 12
    float dummy1;                     // 4
    Vector3 specular = Vector3(0.5f); // 12
    float dummy2;                     // 4
};

struct Light {
    Vector3 strength = Vector3(1.0f);              // 12
    float fallOffStart = 0.0f;                     // 4
    Vector3 direction = Vector3(0.0f, 0.0f, 1.0f); // 12
    float fallOffEnd = 10.0f;                      // 4
    Vector3 position = Vector3(0.0f, 0.0f, -2.0f); // 12
    float spotPower = 1.0f;                        // 4
};

} // namespace Engine
#endif
