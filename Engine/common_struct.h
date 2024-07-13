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

#endif
