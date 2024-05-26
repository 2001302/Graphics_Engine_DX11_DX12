#pragma once

#define _USE_MATH_DEFINES
#define WM_MODEL_LOAD (WM_APP + 1)

#include <map>
#include <vector>
#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>		//for text file open
#include <cmath>		//for calculate coordination
#include <omp.h>		//for omp parallel
#include <commdlg.h>	//for file open
#include <wrl.h>

#include <eigen/dense>
#include <assimp/importer.hpp>
#include <assimp/scene.h>           
#include <assimp/postprocess.h>     
