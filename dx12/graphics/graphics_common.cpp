#include "graphics_common.h"

namespace dx12 {

namespace pso {

// Sampler States
D3D12_SAMPLER_DESC linearWrapSS;
D3D12_SAMPLER_DESC linearClampSS;
D3D12_SAMPLER_DESC shadowPointSS;
D3D12_SAMPLER_DESC shadowCompareSS;
std::vector<D3D12_SAMPLER_DESC> sampleStates;

// Rasterizer States
D3D12_RASTERIZER_DESC solidRS;
D3D12_RASTERIZER_DESC solidCCWRS;
D3D12_RASTERIZER_DESC wireRS;
D3D12_RASTERIZER_DESC wireCCWRS;
D3D12_RASTERIZER_DESC postProcessingRS;

// Depth Stencil States
D3D12_DEPTH_STENCIL_DESC drawDSS;       // 일반적으로 그리기
D3D12_DEPTH_STENCIL_DESC maskDSS;       // 스텐실버퍼에 표시
D3D12_DEPTH_STENCIL_DESC drawMaskedDSS; // 스텐실 표시된 곳만

// Blend States
D3D12_BLEND_DESC mirrorBS;

// Shaders
ComPtr<ID3DBlob> basicVS;
ComPtr<ID3DBlob> skinnedVS;
ComPtr<ID3DBlob> skyboxVS;
ComPtr<ID3DBlob> samplingVS;
ComPtr<ID3DBlob> normalVS;
ComPtr<ID3DBlob> depthOnlyVS;
ComPtr<ID3DBlob> depthOnlySkinnedVS;

ComPtr<ID3DBlob> basicPS;
ComPtr<ID3DBlob> skyboxPS;
ComPtr<ID3DBlob> combinePS;
ComPtr<ID3DBlob> bloomDownPS;
ComPtr<ID3DBlob> bloomUpPS;
ComPtr<ID3DBlob> normalPS;
ComPtr<ID3DBlob> depthOnlyPS;
ComPtr<ID3DBlob> postEffectsPS;
ComPtr<ID3DBlob> normalGS;
ComPtr<ID3DBlob> brightPassCS;
ComPtr<ID3DBlob> blurVerticalCS;
ComPtr<ID3DBlob> blurHorizontalCS;
ComPtr<ID3DBlob> bloomComposite;

// Graphics Pipeline States
ID3D12PipelineState *defaultSolidPSO;
ID3D12PipelineState *skinnedSolidPSO;
ID3D12PipelineState *defaultWirePSO;
ID3D12PipelineState *skinnedWirePSO;
ID3D12PipelineState *stencilMaskPSO;
ID3D12PipelineState *reflectSolidPSO;
ID3D12PipelineState *reflectSkinnedSolidPSO;
ID3D12PipelineState *reflectWirePSO;
ID3D12PipelineState *reflectSkinnedWirePSO;
ID3D12PipelineState *mirrorBlendSolidPSO;
ID3D12PipelineState *mirrorBlendWirePSO;
ID3D12PipelineState *skyboxSolidPSO;
ID3D12PipelineState *skyboxWirePSO;
ID3D12PipelineState *reflectSkyboxSolidPSO;
ID3D12PipelineState *reflectSkyboxWirePSO;
ID3D12PipelineState *normalsPSO;
ID3D12PipelineState *depthOnlyPSO;
ID3D12PipelineState *depthOnlySkinnedPSO;
ID3D12PipelineState *postEffectsPSO;
ID3D12PipelineState *postProcessingPSO;

} // namespace pso

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

void pso::CreateShader(ComPtr<ID3D12Device> &device, std::wstring filename,
                       ComPtr<ID3DBlob> &m_vertexShader,
                       const std::vector<D3D_SHADER_MACRO> shaderMacros) {

    UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DCompileFromFile(
        filename.c_str(), shaderMacros.empty() ? NULL : shaderMacros.data(),
        D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", compileFlags, 0,
        &m_vertexShader, &errorBlob);
    CheckResult(hr, errorBlob.Get());
}

void pso::InitCommonStates(ComPtr<ID3D12Device> &device,
                           ComPtr<ID3D12RootSignature> &rootSignature) {

    InitSamplers(device);
    InitRasterizerStates(device);
    InitBlendStates(device);
    InitDepthStencilStates(device);
    InitPipelineStates(device, rootSignature);
}

void pso::InitSamplers(ComPtr<ID3D12Device> &device) {

    D3D12_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D12_FLOAT32_MAX;
    linearWrapSS = sampDesc;

    sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    linearClampSS = sampDesc;

    // shadowPointSS
    sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampDesc.BorderColor[0] = 1.0f; // 큰 Z값
    sampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    shadowPointSS = sampDesc;

    // shadowCompareSS, 쉐이더 안에서는 SamplerComparisonState
    // Filter = "_COMPARISON_" 주의
    // https://www.gamedev.net/forums/topic/670575-uploading-samplercomparisonstate-in-hlsl/
    sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampDesc.BorderColor[0] = 100.0f; // 큰 Z값
    sampDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    sampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    shadowCompareSS = sampDesc;

    // 샘플러 순서가 "Common.hlsli"에서와 일관성 있어야 함
    sampleStates.push_back(linearWrapSS);
    sampleStates.push_back(linearClampSS);
    sampleStates.push_back(shadowPointSS);
    sampleStates.push_back(shadowCompareSS);
}

void pso::InitRasterizerStates(ComPtr<ID3D12Device> &device) {

    // Rasterizer States
    D3D12_RASTERIZER_DESC rastDesc;
    ZeroMemory(&rastDesc, sizeof(D3D12_RASTERIZER_DESC));
    rastDesc.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
    rastDesc.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.DepthClipEnable = true;
    rastDesc.MultisampleEnable = true;
    solidRS = rastDesc;

    // 거울에 반사되면 삼각형의 Winding이 바뀌기 때문에 CCW로 그려야함
    rastDesc.FrontCounterClockwise = true;
    solidCCWRS = rastDesc;

    rastDesc.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_WIREFRAME;
    wireCCWRS = rastDesc;

    rastDesc.FrontCounterClockwise = false;
    wireRS = rastDesc;

    ZeroMemory(&rastDesc, sizeof(D3D12_RASTERIZER_DESC));
    rastDesc.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
    rastDesc.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
    rastDesc.FrontCounterClockwise = false;
    rastDesc.DepthClipEnable = false;
    postProcessingRS = rastDesc;
}

void pso::InitBlendStates(ComPtr<ID3D12Device> &device) {

    // "이미 그려져있는 화면"과 어떻게 섞을지를 결정
    // Dest: 이미 그려져 있는 값들을 의미
    // Src: 픽셀 쉐이더가 계산한 값들을 의미 (여기서는 마지막 거울)

    D3D12_BLEND_DESC mirrorBlendDesc;
    ZeroMemory(&mirrorBlendDesc, sizeof(mirrorBlendDesc));
    mirrorBlendDesc.AlphaToCoverageEnable = true; // MSAA
    mirrorBlendDesc.IndependentBlendEnable = false;
    // 개별 RenderTarget에 대해서 설정 (최대 8개)
    mirrorBlendDesc.RenderTarget[0].BlendEnable = true;
    mirrorBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_BLEND_FACTOR;
    mirrorBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_BLEND_FACTOR;
    mirrorBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

    mirrorBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    mirrorBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
    mirrorBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

    // 필요하면 RGBA 각각에 대해서도 조절 가능
    mirrorBlendDesc.RenderTarget[0].RenderTargetWriteMask =
        D3D12_COLOR_WRITE_ENABLE_ALL;

    mirrorBS = mirrorBlendDesc;
}

void pso::InitDepthStencilStates(ComPtr<ID3D12Device> &device) {

    // D3D11_DEPTH_STENCIL_DESC 옵션 정리
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_depth_stencil_desc
    // StencilRead/WriteMask: 예) uint8 중 어떤 비트를 사용할지

    // D3D11_DEPTH_STENCILOP_DESC 옵션 정리
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_depth_stencilop_desc
    // StencilPassOp : 둘 다 pass일 때 할 일
    // StencilDepthFailOp : Stencil pass, Depth fail 일 때 할 일
    // StencilFailOp : 둘 다 fail 일 때 할 일

    // m_drawDSS: 기본 DSS
    D3D12_DEPTH_STENCIL_DESC dsDesc;
    ZeroMemory(&dsDesc, sizeof(dsDesc));
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    dsDesc.StencilEnable = false; // Stencil 불필요
    dsDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    dsDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    // 앞면에 대해서 어떻게 작동할지 설정
    dsDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    // 뒷면에 대해 어떻게 작동할지 설정 (뒷면도 그릴 경우)
    dsDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
    dsDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    drawDSS = dsDesc;

    // Stencil에 1로 표기해주는 DSS
    dsDesc.DepthEnable = true; // 이미 그려진 물체 유지
    dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    dsDesc.StencilEnable = true;    // Stencil 필수
    dsDesc.StencilReadMask = 0xFF;  // 모든 비트 다 사용
    dsDesc.StencilWriteMask = 0xFF; // 모든 비트 다 사용
    // 앞면에 대해서 어떻게 작동할지 설정
    dsDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
    dsDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    maskDSS = dsDesc;

    // Stencil에 1로 표기된 경우에"만" 그리는 DSS
    // DepthBuffer는 초기화된 상태로 가정
    // D3D11_COMPARISON_EQUAL 이미 1로 표기된 경우에만 그리기
    // OMSetDepthStencilState(..., 1); <- 여기의 1
    dsDesc.DepthEnable = true;   // 거울 속을 다시 그릴때 필요
    dsDesc.StencilEnable = true; // Stencil 사용
    dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // <- 주의
    dsDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
    drawMaskedDSS = dsDesc;
}

void pso::InitPipelineStates(ComPtr<ID3D12Device> &device,
                             ComPtr<ID3D12RootSignature> &rootSignature) {
    D3D12_INPUT_ELEMENT_DESC basicIEs[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    D3D12_INPUT_ELEMENT_DESC skinnedIEs[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"BLENDWEIGHT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 60,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 76,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"BLENDINDICES", 1, DXGI_FORMAT_R8G8B8A8_UINT, 0, 80,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    D3D12_INPUT_ELEMENT_DESC samplingIED[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    D3D12_INPUT_ELEMENT_DESC skyboxIE[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    CreateShader(device, L"graphics/BasicVS.hlsl", basicVS);
    CreateShader(device, L"graphics/BasicVS.hlsl", skinnedVS,
                 std::vector<D3D_SHADER_MACRO>{{"SKINNED", "1"}, {NULL, NULL}});
    CreateShader(device, L"graphics/NormalVS.hlsl", normalVS);
    CreateShader(device, L"graphics/SamplingVS.hlsl", samplingVS);
    CreateShader(device, L"graphics/SkyboxVS.hlsl", skyboxVS);
    CreateShader(device, L"graphics/DepthOnlyVS.hlsl", depthOnlyVS);
    CreateShader(device, L"graphics/DepthOnlyVS.hlsl", depthOnlySkinnedVS,
                 std::vector<D3D_SHADER_MACRO>{{"SKINNED", "1"}, {NULL, NULL}});

    CreateShader(device, L"graphics/BasicPS.hlsl", basicPS);
    CreateShader(device, L"graphics/NormalPS.hlsl", normalPS);
    CreateShader(device, L"graphics/SkyboxPS.hlsl", skyboxPS);
    CreateShader(device, L"graphics/CombinePS.hlsl", combinePS);
    CreateShader(device, L"graphics/BloomDownPS.hlsl", bloomDownPS);
    CreateShader(device, L"graphics/BloomUpPS.hlsl", bloomUpPS);
    CreateShader(device, L"graphics/DepthOnlyPS.hlsl", depthOnlyPS);
    CreateShader(device, L"graphics/PostEffectsPS.hlsl", postEffectsPS);

    CreateShader(device, L"graphics/NormalGS.hlsl", normalGS);
    CreateShader(device, L"graphics/BrightPassCS.hlsl", brightPassCS);
    CreateShader(device, L"graphics/BlurVertical.hlsl", blurVerticalCS);
    CreateShader(device, L"graphics/BlurHorizontal.hlsl", blurHorizontalCS);
    CreateShader(device, L"graphics/BloomCompositeCS.hlsl", bloomComposite);

    {
        // defaultSolidPSO;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {basicIEs, _countof(basicIEs)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(basicVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&defaultSolidPSO)));
    }
    {
        // Skinned mesh solid
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {skinnedIEs, _countof(skinnedIEs)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(skinnedVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&skinnedSolidPSO)));
    }
    {
        // defaultWirePSO;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {basicIEs, _countof(basicIEs)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(basicVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(wireRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&defaultWirePSO)));
    }
    {
        // Skinned mesh wire;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {skinnedIEs, _countof(skinnedIEs)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(skinnedVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(wireRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&skinnedWirePSO)));
    }
    {
        // stencilMarkPSO;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {basicIEs, _countof(basicIEs)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(depthOnlyVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(depthOnlyPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = maskDSS;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&depthOnlyPSO)));
    }
    {
        // reflectSolidPSO: 반사되면 Winding 반대
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {basicIEs, _countof(basicIEs)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(basicVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidCCWRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = drawMaskedDSS;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&reflectSolidPSO)));
    }
    {
        // Skinned mesh solid
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {skinnedIEs, _countof(skinnedIEs)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(skinnedVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidCCWRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = drawMaskedDSS;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&reflectSkinnedSolidPSO)));
    }
    {
        // reflectWirePSO: 반사되면 Winding 반대
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {skinnedIEs, _countof(skinnedIEs)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(skinnedVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(wireCCWRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = drawMaskedDSS;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&reflectWirePSO)));
    }
    {
        // reflectSkinnedSolidPSO
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {skinnedIEs, _countof(skinnedIEs)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(skinnedVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(wireCCWRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = drawMaskedDSS;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&reflectSkinnedSolidPSO)));
    }
    {
        // mirrorBlendSolidPSO
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {basicIEs, _countof(basicIEs)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(basicVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(mirrorBS);
        psoDesc.DepthStencilState = drawMaskedDSS;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&mirrorBlendSolidPSO)));
    }
    {
        // mirrorBlendWirePSO;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {basicIEs, _countof(basicIEs)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(basicVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(wireRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(mirrorBS);
        psoDesc.DepthStencilState = drawMaskedDSS;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&mirrorBlendWirePSO)));
    }
    {
        // skyboxSolidPSO;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {skyboxIE, _countof(skyboxIE)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(skyboxVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(skyboxPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&skyboxSolidPSO)));
    }
    {
        // skyboxWirePSO;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {skyboxIE, _countof(skyboxIE)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(skyboxVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(skyboxPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(wireRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&skyboxWirePSO)));
    }
    {
        // reflectSkyboxSolidPSO;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {skyboxIE, _countof(skyboxIE)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(skyboxVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(skyboxPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidCCWRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState = drawMaskedDSS;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&skyboxSolidPSO)));
    }
    {
        // reflectSkyboxWirePSO;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {skyboxIE, _countof(skyboxIE)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(skyboxVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(skyboxPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(wireCCWRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&skyboxWirePSO)));
    }
    {
        // depthOnlyPSO;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {basicIEs, _countof(basicIEs)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(depthOnlyVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(depthOnlyPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&defaultSolidPSO)));
    }
    {
        // depthOnlyPSO;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {skinnedIEs, _countof(skinnedIEs)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(depthOnlySkinnedVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(depthOnlyPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&defaultSolidPSO)));
    }
    {
        // postEffectsPSO;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {samplingIED, _countof(samplingIED)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(samplingVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(postEffectsPS.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(postProcessingRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&defaultSolidPSO)));
    }
    {
        // postProcessingPSO;
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = {samplingIED, _countof(samplingIED)};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(samplingVS.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(depthOnlyPS.Get()); // dummy
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(postProcessingRS);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(device->CreateGraphicsPipelineState(
            &psoDesc, IID_PPV_ARGS(&defaultSolidPSO)));
    }
}

} // namespace dx12
