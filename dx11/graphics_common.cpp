#include "graphics_common.h"

namespace dx11 {

namespace pso {

// Sampler States
ComPtr<ID3D11SamplerState> linearWrapSS;
ComPtr<ID3D11SamplerState> linearClampSS;
ComPtr<ID3D11SamplerState> shadowPointSS;
ComPtr<ID3D11SamplerState> shadowCompareSS;
std::vector<ID3D11SamplerState *> sampleStates;

// Rasterizer States
ComPtr<ID3D11RasterizerState> solidRS;
ComPtr<ID3D11RasterizerState> solidCCWRS;
ComPtr<ID3D11RasterizerState> wireRS;
ComPtr<ID3D11RasterizerState> wireCCWRS;
ComPtr<ID3D11RasterizerState> postProcessingRS;

// Depth Stencil States
ComPtr<ID3D11DepthStencilState> drawDSS;       // 일반적으로 그리기
ComPtr<ID3D11DepthStencilState> maskDSS;       // 스텐실버퍼에 표시
ComPtr<ID3D11DepthStencilState> drawMaskedDSS; // 스텐실 표시된 곳만

// Blend States
ComPtr<ID3D11BlendState> mirrorBS;

// Shaders
ComPtr<ID3D11VertexShader> basicVS;
ComPtr<ID3D11VertexShader> skinnedVS;
ComPtr<ID3D11VertexShader> skyboxVS;
ComPtr<ID3D11VertexShader> samplingVS;
ComPtr<ID3D11VertexShader> normalVS;
ComPtr<ID3D11VertexShader> depthOnlyVS;
ComPtr<ID3D11VertexShader> depthOnlySkinnedVS;

ComPtr<ID3D11PixelShader> basicPS;
ComPtr<ID3D11PixelShader> skyboxPS;
ComPtr<ID3D11PixelShader> combinePS;
ComPtr<ID3D11PixelShader> bloomDownPS;
ComPtr<ID3D11PixelShader> bloomUpPS;
ComPtr<ID3D11PixelShader> normalPS;
ComPtr<ID3D11PixelShader> depthOnlyPS;
ComPtr<ID3D11PixelShader> postEffectsPS;

ComPtr<ID3D11GeometryShader> normalGS;

ComPtr<ID3D11ComputeShader> brightPassCS;
ComPtr<ID3D11ComputeShader> blurVerticalCS;
ComPtr<ID3D11ComputeShader> blurHorizontalCS;
ComPtr<ID3D11ComputeShader> bloomComposite;

// Input Layouts
ComPtr<ID3D11InputLayout> basicIL;
ComPtr<ID3D11InputLayout> skinnedIL;
ComPtr<ID3D11InputLayout> samplingIL;
ComPtr<ID3D11InputLayout> skyboxIL;
ComPtr<ID3D11InputLayout> postProcessingIL;

// Graphics Pipeline States
PipelineState defaultSolidPSO;
PipelineState skinnedSolidPSO;
PipelineState defaultWirePSO;
PipelineState skinnedWirePSO;
PipelineState stencilMaskPSO;
PipelineState reflectSolidPSO;
PipelineState reflectSkinnedSolidPSO;
PipelineState reflectWirePSO;
PipelineState reflectSkinnedWirePSO;
PipelineState mirrorBlendSolidPSO;
PipelineState mirrorBlendWirePSO;
PipelineState skyboxSolidPSO;
PipelineState skyboxWirePSO;
PipelineState reflectSkyboxSolidPSO;
PipelineState reflectSkyboxWirePSO;
PipelineState normalsPSO;
PipelineState depthOnlyPSO;
PipelineState depthOnlySkinnedPSO;
PipelineState postEffectsPSO;
PipelineState postProcessingPSO;

} // namespace graphics

void CheckResult(HRESULT hr, ID3DBlob *errorBlob) {
    if (FAILED(hr)) {
        if ((hr & D3D11_ERROR_FILE_NOT_FOUND) != 0) {
            std::cout << "File not found." << std::endl;
        }
        if (errorBlob) {
            std::cout << "Shader compile error\n"
                      << (char *)errorBlob->GetBufferPointer() << std::endl;
        }
    }
}

void pso::CreateVertexShaderAndInputLayout(
    ComPtr<ID3D11Device> &device, std::wstring filename,
    const std::vector<D3D11_INPUT_ELEMENT_DESC> &inputElements,
    ComPtr<ID3D11VertexShader> &m_vertexShader,
    ComPtr<ID3D11InputLayout> &m_inputLayout,
    const std::vector<D3D_SHADER_MACRO> shaderMacros) {

    ComPtr<ID3DBlob> shaderBlob;

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), shaderMacros.empty() ? NULL : shaderMacros.data(),
        D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", compileFlags, 0,
        &shaderBlob, &errorBlob);
    CheckResult(hr, errorBlob.Get());

    /*
        // .cso 파일을 읽어들이는 방식, D3DReadFileToBlob() 사용
        wstring path = L"x64/Release/";
    #if defined(DEBUG) || defined(_DEBUG)
        path = L"x64/Debug/";
    #endif
        filename.erase(filename.end() - 4, filename.end()); // 확장자 hlsl 삭제
        filename = path + filename + L"cso";
        HRESULT hr = D3DReadFileToBlob(filename.c_str(),
    shaderBlob.GetAddressOf());
    */

    device->CreateVertexShader(shaderBlob->GetBufferPointer(),
                               shaderBlob->GetBufferSize(), NULL,
                               &m_vertexShader);

    device->CreateInputLayout(inputElements.data(), UINT(inputElements.size()),
                              shaderBlob->GetBufferPointer(),
                              shaderBlob->GetBufferSize(), &m_inputLayout);
}

void pso::CreateHullShader(ComPtr<ID3D11Device> &device,
                                const std::wstring &filename,
                                ComPtr<ID3D11HullShader> &m_hullShader) {
    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "hs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

    CheckResult(hr, errorBlob.Get());

    device->CreateHullShader(shaderBlob->GetBufferPointer(),
                             shaderBlob->GetBufferSize(), NULL, &m_hullShader);
}

void pso::CreateDomainShader(ComPtr<ID3D11Device> &device,
                                  const std::wstring &filename,
                                  ComPtr<ID3D11DomainShader> &m_domainShader) {

    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "ds_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

    CheckResult(hr, errorBlob.Get());

    device->CreateDomainShader(shaderBlob->GetBufferPointer(),
                               shaderBlob->GetBufferSize(), NULL,
                               &m_domainShader);
}

void pso::CreatePixelShader(ComPtr<ID3D11Device> &device,
                                 const std::wstring &filename,
                                 ComPtr<ID3D11PixelShader> &m_pixelShader) {
    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "ps_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

    CheckResult(hr, errorBlob.Get());

    device->CreatePixelShader(shaderBlob->GetBufferPointer(),
                              shaderBlob->GetBufferSize(), NULL,
                              &m_pixelShader);
}

void pso::CreateGeometryShader(
    ComPtr<ID3D11Device> &device, const std::wstring &filename,
    ComPtr<ID3D11GeometryShader> &geometryShader) {

    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "gs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

    device->CreateGeometryShader(shaderBlob->GetBufferPointer(),
                                 shaderBlob->GetBufferSize(), NULL,
                                 &geometryShader);
}

void pso::CreateComputeShader(ComPtr<ID3D11Device> &device,
                                   const std::wstring &filename,
                                   ComPtr<ID3D11ComputeShader> &computeShader) {

    ComPtr<ID3DBlob> shaderBlob;
    ComPtr<ID3DBlob> errorBlob;

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
        "cs_5_0", compileFlags, 0, &shaderBlob, &errorBlob);

    ThrowIfFailed(device->CreateComputeShader(shaderBlob->GetBufferPointer(),
                                              shaderBlob->GetBufferSize(), NULL,
                                              &computeShader));
}

void pso::InitCommonStates(ComPtr<ID3D11Device> &device) {

    InitShaders(device);
    InitSamplers(device);
    InitRasterizerStates(device);
    InitBlendStates(device);
    InitDepthStencilStates(device);
    InitPipelineStates(device);
}

void pso::InitSamplers(ComPtr<ID3D11Device> &device) {

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    device->CreateSamplerState(&sampDesc, linearWrapSS.GetAddressOf());
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    device->CreateSamplerState(&sampDesc, linearClampSS.GetAddressOf());

    // shadowPointSS
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.BorderColor[0] = 1.0f; // 큰 Z값
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    device->CreateSamplerState(&sampDesc, shadowPointSS.GetAddressOf());

    // shadowCompareSS, 쉐이더 안에서는 SamplerComparisonState
    // Filter = "_COMPARISON_" 주의
    // https://www.gamedev.net/forums/topic/670575-uploading-samplercomparisonstate-in-hlsl/
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.BorderColor[0] = 100.0f; // 큰 Z값
    sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    device->CreateSamplerState(&sampDesc, shadowCompareSS.GetAddressOf());

    // 샘플러 순서가 "Common.hlsli"에서와 일관성 있어야 함
    sampleStates.push_back(linearWrapSS.Get());
    sampleStates.push_back(linearClampSS.Get());
    sampleStates.push_back(shadowPointSS.Get());
    sampleStates.push_back(shadowCompareSS.Get());
}

void pso::InitRasterizerStates(ComPtr<ID3D11Device> &device) {

    // Rasterizer States
    D3D11_RASTERIZER_DESC rastDesc;
    ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
    rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.DepthClipEnable = true;
    rastDesc.MultisampleEnable = true;
    ThrowIfFailed(
        device->CreateRasterizerState(&rastDesc, solidRS.GetAddressOf()));

    // 거울에 반사되면 삼각형의 Winding이 바뀌기 때문에 CCW로 그려야함
    rastDesc.FrontCounterClockwise = true;
    ThrowIfFailed(
        device->CreateRasterizerState(&rastDesc, solidCCWRS.GetAddressOf()));

    rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
    ThrowIfFailed(
        device->CreateRasterizerState(&rastDesc, wireCCWRS.GetAddressOf()));

    rastDesc.FrontCounterClockwise = false;
    ThrowIfFailed(
        device->CreateRasterizerState(&rastDesc, wireRS.GetAddressOf()));

    ZeroMemory(&rastDesc, sizeof(D3D11_RASTERIZER_DESC));
    rastDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    rastDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.DepthClipEnable = false;
    ThrowIfFailed(device->CreateRasterizerState(
        &rastDesc, postProcessingRS.GetAddressOf()));
}

void pso::InitBlendStates(ComPtr<ID3D11Device> &device) {

    // "이미 그려져있는 화면"과 어떻게 섞을지를 결정
    // Dest: 이미 그려져 있는 값들을 의미
    // Src: 픽셀 쉐이더가 계산한 값들을 의미 (여기서는 마지막 거울)

    D3D11_BLEND_DESC mirrorBlendDesc;
    ZeroMemory(&mirrorBlendDesc, sizeof(mirrorBlendDesc));
    mirrorBlendDesc.AlphaToCoverageEnable = true; // MSAA
    mirrorBlendDesc.IndependentBlendEnable = false;
    // 개별 RenderTarget에 대해서 설정 (최대 8개)
    mirrorBlendDesc.RenderTarget[0].BlendEnable = true;
    mirrorBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_BLEND_FACTOR;
    mirrorBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_BLEND_FACTOR;
    mirrorBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

    mirrorBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    mirrorBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    mirrorBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

    // 필요하면 RGBA 각각에 대해서도 조절 가능
    mirrorBlendDesc.RenderTarget[0].RenderTargetWriteMask =
        D3D11_COLOR_WRITE_ENABLE_ALL;

    ThrowIfFailed(
        device->CreateBlendState(&mirrorBlendDesc, mirrorBS.GetAddressOf()));
}

void pso::InitDepthStencilStates(ComPtr<ID3D11Device> &device) {

    // D3D11_DEPTH_STENCIL_DESC 옵션 정리
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_depth_stencil_desc
    // StencilRead/WriteMask: 예) uint8 중 어떤 비트를 사용할지

    // D3D11_DEPTH_STENCILOP_DESC 옵션 정리
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_depth_stencilop_desc
    // StencilPassOp : 둘 다 pass일 때 할 일
    // StencilDepthFailOp : Stencil pass, Depth fail 일 때 할 일
    // StencilFailOp : 둘 다 fail 일 때 할 일

    // m_drawDSS: 기본 DSS
    D3D11_DEPTH_STENCIL_DESC dsDesc;
    ZeroMemory(&dsDesc, sizeof(dsDesc));
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    dsDesc.StencilEnable = false; // Stencil 불필요
    dsDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    dsDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    // 앞면에 대해서 어떻게 작동할지 설정
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    // 뒷면에 대해 어떻게 작동할지 설정 (뒷면도 그릴 경우)
    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    ThrowIfFailed(
        device->CreateDepthStencilState(&dsDesc, drawDSS.GetAddressOf()));

    // Stencil에 1로 표기해주는 DSS
    dsDesc.DepthEnable = true; // 이미 그려진 물체 유지
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    dsDesc.StencilEnable = true;    // Stencil 필수
    dsDesc.StencilReadMask = 0xFF;  // 모든 비트 다 사용
    dsDesc.StencilWriteMask = 0xFF; // 모든 비트 다 사용
    // 앞면에 대해서 어떻게 작동할지 설정
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    ThrowIfFailed(
        device->CreateDepthStencilState(&dsDesc, maskDSS.GetAddressOf()));

    // Stencil에 1로 표기된 경우에"만" 그리는 DSS
    // DepthBuffer는 초기화된 상태로 가정
    // D3D11_COMPARISON_EQUAL 이미 1로 표기된 경우에만 그리기
    // OMSetDepthStencilState(..., 1); <- 여기의 1
    dsDesc.DepthEnable = true;   // 거울 속을 다시 그릴때 필요
    dsDesc.StencilEnable = true; // Stencil 사용
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // <- 주의
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

    ThrowIfFailed(
        device->CreateDepthStencilState(&dsDesc, drawMaskedDSS.GetAddressOf()));
}

void pso::InitShaders(ComPtr<ID3D11Device> &device) {

    // Shaders, InputLayouts

    std::vector<D3D11_INPUT_ELEMENT_DESC> basicIEs = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    std::vector<D3D11_INPUT_ELEMENT_DESC> skinnedIEs = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BLENDWEIGHT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 60,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 76,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BLENDINDICES", 1, DXGI_FORMAT_R8G8B8A8_UINT, 0, 80,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    std::vector<D3D11_INPUT_ELEMENT_DESC> samplingIED = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    std::vector<D3D11_INPUT_ELEMENT_DESC> skyboxIE = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
         D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    CreateVertexShaderAndInputLayout(device, L"BasicVS.hlsl", basicIEs, basicVS,
                                     basicIL);
    CreateVertexShaderAndInputLayout(
        device, L"BasicVS.hlsl", skinnedIEs, skinnedVS, skinnedIL,
        std::vector<D3D_SHADER_MACRO>{{"SKINNED", "1"}, {NULL, NULL}});
    CreateVertexShaderAndInputLayout(device, L"NormalVS.hlsl", basicIEs,
                                     normalVS, basicIL);
    CreateVertexShaderAndInputLayout(device, L"SamplingVS.hlsl", samplingIED,
                                     samplingVS, samplingIL);

    CreateVertexShaderAndInputLayout(device, L"SkyboxVS.hlsl", skyboxIE,
                                     skyboxVS, skyboxIL);

    CreateVertexShaderAndInputLayout(device, L"DepthOnlyVS.hlsl", basicIEs,
                                     depthOnlyVS, skyboxIL);
    CreateVertexShaderAndInputLayout(
        device, L"DepthOnlyVS.hlsl", skinnedIEs, depthOnlySkinnedVS, skinnedIL,
        std::vector<D3D_SHADER_MACRO>{{"SKINNED", "1"}, {NULL, NULL}});

    CreatePixelShader(device, L"BasicPS.hlsl", basicPS);
    CreatePixelShader(device, L"NormalPS.hlsl", normalPS);

    CreatePixelShader(device, L"SkyboxPS.hlsl", skyboxPS);

    CreatePixelShader(device, L"CombinePS.hlsl", combinePS);
    CreatePixelShader(device, L"BloomDownPS.hlsl", bloomDownPS);
    CreatePixelShader(device, L"BloomUpPS.hlsl", bloomUpPS);
    CreatePixelShader(device, L"DepthOnlyPS.hlsl", depthOnlyPS);
    CreatePixelShader(device, L"PostEffectsPS.hlsl", postEffectsPS);

    CreateGeometryShader(device, L"NormalGS.hlsl", normalGS);

    CreateComputeShader(device, L"BrightPassCS.hlsl", brightPassCS);
    CreateComputeShader(device, L"BlurVertical.hlsl", blurVerticalCS);
    CreateComputeShader(device, L"BlurHorizontal.hlsl", blurHorizontalCS);
    CreateComputeShader(device, L"BloomCompositeCS.hlsl", bloomComposite);
}

void pso::InitPipelineStates(ComPtr<ID3D11Device> &device) {

    // defaultSolidPSO;
    defaultSolidPSO.vertex_shader = basicVS;
    defaultSolidPSO.input_layout = basicIL;
    defaultSolidPSO.pixel_shader = basicPS;
    defaultSolidPSO.rasterizer_state = solidRS;
    defaultSolidPSO.primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // Skinned mesh solid
    skinnedSolidPSO = defaultSolidPSO;
    skinnedSolidPSO.vertex_shader = skinnedVS;
    skinnedSolidPSO.input_layout = skinnedIL;

    // defaultWirePSO
    defaultWirePSO = defaultSolidPSO;
    defaultWirePSO.rasterizer_state = wireRS;

    // Skinned mesh wire
    skinnedWirePSO = skinnedSolidPSO;
    skinnedWirePSO.rasterizer_state = wireRS;

    // stencilMarkPSO;
    stencilMaskPSO = defaultSolidPSO;
    stencilMaskPSO.depth_stencil_state = maskDSS;
    stencilMaskPSO.stencil_ref = 1;
    stencilMaskPSO.vertex_shader = depthOnlyVS;
    stencilMaskPSO.pixel_shader = depthOnlyPS;

    // reflectSolidPSO: 반사되면 Winding 반대
    reflectSolidPSO = defaultSolidPSO;
    reflectSolidPSO.depth_stencil_state = drawMaskedDSS;
    reflectSolidPSO.rasterizer_state = solidCCWRS; // 반시계
    reflectSolidPSO.stencil_ref = 1;

    reflectSkinnedSolidPSO = reflectSolidPSO;
    reflectSkinnedSolidPSO.vertex_shader = skinnedVS;
    reflectSkinnedSolidPSO.input_layout = skinnedIL;

    // reflectWirePSO: 반사되면 Winding 반대
    reflectWirePSO = reflectSolidPSO;
    reflectWirePSO.rasterizer_state = wireCCWRS; // 반시계
    reflectWirePSO.stencil_ref = 1;

    reflectSkinnedWirePSO = reflectSkinnedSolidPSO;
    reflectSkinnedWirePSO.rasterizer_state = wireCCWRS; // 반시계
    reflectSkinnedWirePSO.stencil_ref = 1;

    // mirrorBlendSolidPSO;
    mirrorBlendSolidPSO = defaultSolidPSO;
    mirrorBlendSolidPSO.blend_state = mirrorBS;
    mirrorBlendSolidPSO.depth_stencil_state = drawMaskedDSS;
    mirrorBlendSolidPSO.stencil_ref = 1;

    // mirrorBlendWirePSO;
    mirrorBlendWirePSO = defaultWirePSO;
    mirrorBlendWirePSO.blend_state = mirrorBS;
    mirrorBlendWirePSO.depth_stencil_state = drawMaskedDSS;
    mirrorBlendWirePSO.stencil_ref = 1;

    // skyboxSolidPSO
    skyboxSolidPSO = defaultSolidPSO;
    skyboxSolidPSO.vertex_shader = skyboxVS;
    skyboxSolidPSO.pixel_shader = skyboxPS;
    skyboxSolidPSO.input_layout = skyboxIL;

    // skyboxWirePSO
    skyboxWirePSO = skyboxSolidPSO;
    skyboxWirePSO.rasterizer_state = wireRS;

    // reflectSkyboxSolidPSO
    reflectSkyboxSolidPSO = skyboxSolidPSO;
    reflectSkyboxSolidPSO.depth_stencil_state = drawMaskedDSS;
    reflectSkyboxSolidPSO.rasterizer_state = solidCCWRS; // 반시계
    reflectSkyboxSolidPSO.stencil_ref = 1;

    // reflectSkyboxWirePSO
    reflectSkyboxWirePSO = reflectSkyboxSolidPSO;
    reflectSkyboxWirePSO.rasterizer_state = wireCCWRS;
    reflectSkyboxWirePSO.stencil_ref = 1;

    // normalsPSO
    normalsPSO = defaultSolidPSO;
    normalsPSO.vertex_shader = normalVS;
    normalsPSO.geometry_shader = normalGS;
    normalsPSO.pixel_shader = normalPS;
    normalsPSO.primitive_topology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

    // depthOnlyPSO
    depthOnlyPSO = defaultSolidPSO;
    depthOnlyPSO.vertex_shader = depthOnlyVS;
    depthOnlyPSO.pixel_shader = depthOnlyPS;

    depthOnlySkinnedPSO = depthOnlyPSO;
    depthOnlySkinnedPSO.vertex_shader = depthOnlySkinnedVS;
    depthOnlySkinnedPSO.input_layout = skinnedIL;

    // postEffectsPSO
    postEffectsPSO.vertex_shader = samplingVS;
    postEffectsPSO.pixel_shader = postEffectsPS;
    postEffectsPSO.input_layout = samplingIL;
    postEffectsPSO.rasterizer_state = postProcessingRS;

    // postProcessingPSO
    postProcessingPSO.vertex_shader = samplingVS;
    postProcessingPSO.pixel_shader = depthOnlyPS; // dummy
    postProcessingPSO.input_layout = samplingIL;
    postProcessingPSO.rasterizer_state = postProcessingRS;
}

} // namespace core
