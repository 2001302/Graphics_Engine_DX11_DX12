#pragma once

#define MAX_LIGHTS 3
#define LIGHT_OFF 0x00
#define LIGHT_DIRECTIONAL 0x01
#define LIGHT_POINT 0x02
#define LIGHT_SPOT 0x04
#define LIGHT_SHADOW 0x10

#define ASSERT_FAILED(hr)                                                      \
    if (FAILED(hr)) {                                                          \
        throw std::exception();                                                \
    }

#include <d3d12.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <iostream>
#include <map>
#include <memory>
#include <stdint.h>
#include <string>
#include <vector>
#include <windows.h>
#include <wrl/client.h>

#include "d3dx12.h"
#include <env.h>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using Microsoft::WRL::ComPtr;