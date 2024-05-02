#pragma once

#define _USE_MATH_DEFINES
#define WM_MODEL_LOAD (WM_APP + 1)

#include <map>
#include <vector>
#include <deque>
#include <windows.h>
#include <windowsx.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <Eigen/Dense>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/algorithm/string.hpp>
#include <omp.h>