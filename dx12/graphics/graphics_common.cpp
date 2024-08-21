//#include "graphics_common.h"
//
//namespace dx12 {
//
//namespace pso {
//
//// Sampler States
//D3D12_SAMPLER_DESC linearWrapSS;
//D3D12_SAMPLER_DESC linearClampSS;
//D3D12_SAMPLER_DESC shadowPointSS;
//D3D12_SAMPLER_DESC shadowCompareSS;
//std::vector<D3D12_SAMPLER_DESC> sampleStates;
//
//// Rasterizer States
//D3D12_RASTERIZER_DESC solidRS;
//D3D12_RASTERIZER_DESC solidCCWRS;
//D3D12_RASTERIZER_DESC wireRS;
//D3D12_RASTERIZER_DESC wireCCWRS;
//D3D12_RASTERIZER_DESC postProcessingRS;
//
//// Depth Stencil States
//D3D12_DEPTH_STENCIL_DESC drawDSS;       // ?쇰컲?곸쑝濡?洹몃━湲?
//D3D12_DEPTH_STENCIL_DESC maskDSS;       // ?ㅽ뀗?ㅻ쾭?쇱뿉 ?쒖떆
//D3D12_DEPTH_STENCIL_DESC drawMaskedDSS; // ?ㅽ뀗???쒖떆??怨노쭔
//
//// Blend States
//D3D12_BLEND_DESC mirrorBS;
//
//// Shaders
//ComPtr<ID3DBlob> basicVS;
//ComPtr<ID3DBlob> skinnedVS;
//ComPtr<ID3DBlob> skyboxVS;
//ComPtr<ID3DBlob> samplingVS;
//ComPtr<ID3DBlob> normalVS;
//ComPtr<ID3DBlob> depthOnlyVS;
//ComPtr<ID3DBlob> depthOnlySkinnedVS;
//
//ComPtr<ID3DBlob> basicPS;
//ComPtr<ID3DBlob> skyboxPS;
//ComPtr<ID3DBlob> combinePS;
//ComPtr<ID3DBlob> bloomDownPS;
//ComPtr<ID3DBlob> bloomUpPS;
//ComPtr<ID3DBlob> normalPS;
//ComPtr<ID3DBlob> depthOnlyPS;
//ComPtr<ID3DBlob> postEffectsPS;
//ComPtr<ID3DBlob> normalGS;
//ComPtr<ID3DBlob> brightPassCS;
//ComPtr<ID3DBlob> blurVerticalCS;
//ComPtr<ID3DBlob> blurHorizontalCS;
//ComPtr<ID3DBlob> bloomComposite;
//
//// Graphics Pipeline States
//ComPtr<ID3D12PipelineState> defaultSolidPSO;
//ComPtr<ID3D12PipelineState> skinnedSolidPSO;
//ComPtr<ID3D12PipelineState> defaultWirePSO;
//ComPtr<ID3D12PipelineState> skinnedWirePSO;
//ComPtr<ID3D12PipelineState> stencilMaskPSO;
//ComPtr<ID3D12PipelineState> reflectSolidPSO;
//ComPtr<ID3D12PipelineState> reflectSkinnedSolidPSO;
//ComPtr<ID3D12PipelineState> reflectWirePSO;
//ComPtr<ID3D12PipelineState> reflectSkinnedWirePSO;
//ComPtr<ID3D12PipelineState> mirrorBlendSolidPSO;
//ComPtr<ID3D12PipelineState> mirrorBlendWirePSO;
//ComPtr<ID3D12PipelineState> skyboxSolidPSO;
//ComPtr<ID3D12PipelineState> skyboxWirePSO;
//ComPtr<ID3D12PipelineState> reflectSkyboxSolidPSO;
//ComPtr<ID3D12PipelineState> reflectSkyboxWirePSO;
//ComPtr<ID3D12PipelineState> normalsPSO;
//ComPtr<ID3D12PipelineState> depthOnlyPSO;
//ComPtr<ID3D12PipelineState> depthOnlySkinnedPSO;
//ComPtr<ID3D12PipelineState> postEffectsPSO;
//ComPtr<ID3D12PipelineState> postProcessingPSO;
//
//} // namespace pso
//
//void CheckResult(HRESULT hr, ID3DBlob *errorBlob) {
//    if (FAILED(hr)) {
//        if ((hr & D3D11_ERROR_FILE_NOT_FOUND) != 0) {
//            std::cout << "File not found." << std::endl;
//        }
//        if (errorBlob) {
//            std::cout << "Shader compile error\n"
//                      << (char *)errorBlob->GetBufferPointer() << std::endl;
//        }
//    }
//}
//
//void pso::CreateVertexShader(ComPtr<ID3D12Device> &device, std::wstring filename,
//                       ComPtr<ID3DBlob> &m_vertexShader,
//                       const std::vector<D3D_SHADER_MACRO> shaderMacros) {
//
//    UINT compileFlags = 0;
//#if defined(DEBUG) || defined(_DEBUG)
//    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
//#endif
//
//    ComPtr<ID3DBlob> errorBlob;
//    HRESULT hr = D3DCompileFromFile(
//        filename.c_str(), shaderMacros.empty() ? NULL : shaderMacros.data(),
//        D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", compileFlags, 0,
//        &m_vertexShader, &errorBlob);
//    CheckResult(hr, errorBlob.Get());
//}
//
//void pso::CreatePixelShader(ComPtr<ID3D12Device> &device, std::wstring filename,
//                       ComPtr<ID3DBlob> &shader) 
//{
//    UINT compileFlags = 0;
//#if defined(DEBUG) || defined(_DEBUG)
//    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
//#endif
//
//    ComPtr<ID3DBlob> errorBlob;
//    HRESULT hr = D3DCompileFromFile(
//        filename.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main",
//        "ps_5_0", compileFlags, 0, &shader, &errorBlob);
//    CheckResult(hr, errorBlob.Get());
//}
//
//void pso::InitCommonStates(ComPtr<ID3D12Device> &device,
//                           ComPtr<ID3D12RootSignature> &rootSignature) {
//
//    InitSamplers(device);
//    InitRasterizerStates(device);
//    InitBlendStates(device);
//    InitDepthStencilStates(device);
//    InitPipelineStates(device, rootSignature);
//}
//
//void pso::InitSamplers(ComPtr<ID3D12Device> &device) {
//
//    D3D12_SAMPLER_DESC sampDesc;
//    ZeroMemory(&sampDesc, sizeof(sampDesc));
//    sampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
//    sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
//    sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
//    sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
//    sampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
//    sampDesc.MinLOD = 0;
//    sampDesc.MaxLOD = D3D12_FLOAT32_MAX;
//    linearWrapSS = sampDesc;
//
//    sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
//    sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
//    sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
//    linearClampSS = sampDesc;
//
//    // shadowPointSS
//    sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
//    sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
//    sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
//    sampDesc.BorderColor[0] = 1.0f; // ??Z媛?
//    sampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
//    shadowPointSS = sampDesc;
//
//    // shadowCompareSS, ?먯씠???덉뿉?쒕뒗 SamplerComparisonState
//    // Filter = "_COMPARISON_" 二쇱쓽
//    // https://www.gamedev.net/forums/topic/670575-uploading-samplercomparisonstate-in-hlsl/
//    sampDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
//    sampDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
//    sampDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
//    sampDesc.BorderColor[0] = 100.0f; // ??Z媛?
//    sampDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
//    sampDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
//    shadowCompareSS = sampDesc;
//
//    // ?섑뵆???쒖꽌媛 "Common.hlsli"?먯꽌? ?쇨????덉뼱????
//    sampleStates.push_back(linearWrapSS);
//    sampleStates.push_back(linearClampSS);
//    sampleStates.push_back(shadowPointSS);
//    sampleStates.push_back(shadowCompareSS);
//}
//
//void pso::InitRasterizerStates(ComPtr<ID3D12Device> &device) {
//
//    // Rasterizer States
//    D3D12_RASTERIZER_DESC rastDesc;
//    ZeroMemory(&rastDesc, sizeof(D3D12_RASTERIZER_DESC));
//    rastDesc.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
//    rastDesc.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_BACK;
//    rastDesc.FrontCounterClockwise = false;
//    rastDesc.DepthClipEnable = true;
//    rastDesc.MultisampleEnable = true;
//    solidRS = rastDesc;
//
//    // 嫄곗슱??諛섏궗?섎㈃ ?쇨컖?뺤쓽 Winding??諛붾뚭린 ?뚮Ц??CCW濡?洹몃젮?쇳븿
//    rastDesc.FrontCounterClockwise = true;
//    solidCCWRS = rastDesc;
//
//    rastDesc.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_WIREFRAME;
//    wireCCWRS = rastDesc;
//
//    rastDesc.FrontCounterClockwise = false;
//    wireRS = rastDesc;
//
//    ZeroMemory(&rastDesc, sizeof(D3D12_RASTERIZER_DESC));
//    rastDesc.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
//    rastDesc.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
//    rastDesc.FrontCounterClockwise = false;
//    rastDesc.DepthClipEnable = false;
//    postProcessingRS = rastDesc;
//}
//
//void pso::InitBlendStates(ComPtr<ID3D12Device> &device) {
//
//    // "?대? 洹몃젮?몄엳???붾㈃"怨??대뼸寃??욎쓣吏瑜?寃곗젙
//    // Dest: ?대? 洹몃젮???덈뒗 媛믩뱾???섎?
//    // Src: ?쎌? ?먯씠?붽? 怨꾩궛??媛믩뱾???섎? (?ш린?쒕뒗 留덉?留?嫄곗슱)
//
//    D3D12_BLEND_DESC mirrorBlendDesc;
//    ZeroMemory(&mirrorBlendDesc, sizeof(mirrorBlendDesc));
//    mirrorBlendDesc.AlphaToCoverageEnable = true; // MSAA
//    mirrorBlendDesc.IndependentBlendEnable = false;
//    // 媛쒕퀎 RenderTarget????댁꽌 ?ㅼ젙 (理쒕? 8媛?
//    mirrorBlendDesc.RenderTarget[0].BlendEnable = true;
//    mirrorBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_BLEND_FACTOR;
//    mirrorBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_BLEND_FACTOR;
//    mirrorBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
//
//    mirrorBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
//    mirrorBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
//    mirrorBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
//
//    // ?꾩슂?섎㈃ RGBA 媛곴컖????댁꽌??議곗젅 媛??
//    mirrorBlendDesc.RenderTarget[0].RenderTargetWriteMask =
//        D3D12_COLOR_WRITE_ENABLE_ALL;
//
//    mirrorBS = mirrorBlendDesc;
//}
//
//void pso::InitDepthStencilStates(ComPtr<ID3D12Device> &device) {
//
//    // D3D11_DEPTH_STENCIL_DESC ?듭뀡 ?뺣━
//    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_depth_stencil_desc
//    // StencilRead/WriteMask: ?? uint8 以??대뼡 鍮꾪듃瑜??ъ슜?좎?
//
//    // D3D11_DEPTH_STENCILOP_DESC ?듭뀡 ?뺣━
//    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_depth_stencilop_desc
//    // StencilPassOp : ????pass????????
//    // StencilDepthFailOp : Stencil pass, Depth fail ????????
//    // StencilFailOp : ????fail ????????
//
//    // m_drawDSS: 湲곕낯 DSS
//    D3D12_DEPTH_STENCIL_DESC dsDesc;
//    ZeroMemory(&dsDesc, sizeof(dsDesc));
//    dsDesc.DepthEnable = true;
//    dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
//    dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
//    dsDesc.StencilEnable = false; // Stencil 遺덊븘??
//    dsDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
//    dsDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
//    // ?욌㈃????댁꽌 ?대뼸寃??묐룞?좎? ?ㅼ젙
//    dsDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
//    dsDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
//    dsDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
//    dsDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
//    // ?룸㈃??????대뼸寃??묐룞?좎? ?ㅼ젙 (?룸㈃??洹몃┫ 寃쎌슦)
//    dsDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
//    dsDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
//    dsDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
//    dsDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
//    drawDSS = dsDesc;
//
//    // Stencil??1濡??쒓린?댁＜??DSS
//    dsDesc.DepthEnable = true; // ?대? 洹몃젮吏?臾쇱껜 ?좎?
//    dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
//    dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
//    dsDesc.StencilEnable = true;    // Stencil ?꾩닔
//    dsDesc.StencilReadMask = 0xFF;  // 紐⑤뱺 鍮꾪듃 ???ъ슜
//    dsDesc.StencilWriteMask = 0xFF; // 紐⑤뱺 鍮꾪듃 ???ъ슜
//    // ?욌㈃????댁꽌 ?대뼸寃??묐룞?좎? ?ㅼ젙
//    dsDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
//    dsDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
//    dsDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
//    dsDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
//    maskDSS = dsDesc;
//
//    // Stencil??1濡??쒓린??寃쎌슦??留? 洹몃━??DSS
//    // DepthBuffer??珥덇린?붾맂 ?곹깭濡?媛??
//    // D3D11_COMPARISON_EQUAL ?대? 1濡??쒓린??寃쎌슦?먮쭔 洹몃━湲?
//    // OMSetDepthStencilState(..., 1); <- ?ш린??1
//    dsDesc.DepthEnable = true;   // 嫄곗슱 ?띿쓣 ?ㅼ떆 洹몃┫???꾩슂
//    dsDesc.StencilEnable = true; // Stencil ?ъ슜
//    dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
//    dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // <- 二쇱쓽
//    dsDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
//    dsDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
//    dsDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
//    dsDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
//    drawMaskedDSS = dsDesc;
//}
//
//void pso::InitPipelineStates(ComPtr<ID3D12Device> &device,
//                             ComPtr<ID3D12RootSignature> &rootSignature) {
//    D3D12_INPUT_ELEMENT_DESC basicIEs[] = {
//        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
//         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
//         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
//         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
//         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//    };
//
//    D3D12_INPUT_ELEMENT_DESC skinnedIEs[] = {
//        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
//         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
//         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
//         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
//         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//        {"BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44,
//         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//        {"BLENDWEIGHT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 60,
//         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//        {"BLENDINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 76,
//         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//        {"BLENDINDICES", 1, DXGI_FORMAT_R8G8B8A8_UINT, 0, 80,
//         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//    };
//
//    D3D12_INPUT_ELEMENT_DESC samplingIED[] = {
//        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
//         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3,
//         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 3 + 4 * 3,
//         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//    };
//
//    D3D12_INPUT_ELEMENT_DESC skyboxIE[] = {
//        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
//         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
//         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
//         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
//         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//    };
//
//    CreateVertexShader(device, L"graphics/BasicVS.hlsl", basicVS);
//    CreateVertexShader(device, L"graphics/BasicVS.hlsl", skinnedVS,
//                 std::vector<D3D_SHADER_MACRO>{{"SKINNED", "1"}, {NULL, NULL}});
//    CreateVertexShader(device, L"graphics/NormalVS.hlsl", normalVS);
//    CreateVertexShader(device, L"graphics/SamplingVS.hlsl", samplingVS);
//    CreateVertexShader(device, L"graphics/SkyboxVS.hlsl", skyboxVS);
//    CreateVertexShader(device, L"graphics/DepthOnlyVS.hlsl", depthOnlyVS);
//    CreateVertexShader(device, L"graphics/DepthOnlyVS.hlsl", depthOnlySkinnedVS,
//                 std::vector<D3D_SHADER_MACRO>{{"SKINNED", "1"}, {NULL, NULL}});
//
//    CreatePixelShader(device, L"graphics/BasicPS.hlsl", basicPS);
//    CreatePixelShader(device, L"graphics/NormalPS.hlsl", normalPS);
//    CreatePixelShader(device, L"graphics/SkyboxPS.hlsl", skyboxPS);
//    CreatePixelShader(device, L"graphics/CombinePS.hlsl", combinePS);
//    CreatePixelShader(device, L"graphics/BloomDownPS.hlsl", bloomDownPS);
//    CreatePixelShader(device, L"graphics/BloomUpPS.hlsl", bloomUpPS);
//    CreatePixelShader(device, L"graphics/DepthOnlyPS.hlsl", depthOnlyPS);
//    CreatePixelShader(device, L"graphics/PostEffectsPS.hlsl", postEffectsPS);
//
//    //CreateVertexShader(device, L"graphics/NormalGS.hlsl", normalGS);
//    //CreateVertexShader(device, L"graphics/BrightPassCS.hlsl", brightPassCS);
//    //CreateVertexShader(device, L"graphics/BlurVertical.hlsl", blurVerticalCS);
//    //CreateVertexShader(device, L"graphics/BlurHorizontal.hlsl", blurHorizontalCS);
//    //CreateVertexShader(device, L"graphics/BloomCompositeCS.hlsl", bloomComposite);
//
//    {
//        // defaultSolidPSO;
//        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//        psoDesc.InputLayout = {basicIEs, _countof(basicIEs)};
//        psoDesc.pRootSignature = rootSignature.Get();
//        psoDesc.VS = CD3DX12_SHADER_BYTECODE(basicVS.Get());
//        psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
//        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidRS);
//        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//        psoDesc.DepthStencilState.DepthEnable = FALSE;
//        psoDesc.DepthStencilState.StencilEnable = FALSE;
//        psoDesc.SampleMask = UINT_MAX;
//        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//        psoDesc.NumRenderTargets = 1;
//        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//        psoDesc.SampleDesc.Count = 1;
//        ThrowIfFailed(device->CreateGraphicsPipelineState(
//            &psoDesc, IID_PPV_ARGS(&defaultSolidPSO)));
//    }
//    {
//        // Skinned mesh solid
//        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//        psoDesc.InputLayout = {skinnedIEs, _countof(skinnedIEs)};
//        psoDesc.pRootSignature = rootSignature.Get();
//        psoDesc.VS = CD3DX12_SHADER_BYTECODE(skinnedVS.Get());
//        psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
//        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidRS);
//        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//        psoDesc.DepthStencilState.DepthEnable = FALSE;
//        psoDesc.DepthStencilState.StencilEnable = FALSE;
//        psoDesc.SampleMask = UINT_MAX;
//        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//        psoDesc.NumRenderTargets = 1;
//        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//        psoDesc.SampleDesc.Count = 1;
//        ThrowIfFailed(device->CreateGraphicsPipelineState(
//            &psoDesc, IID_PPV_ARGS(&skinnedSolidPSO)));
//    }
//    {
//        // defaultWirePSO;
//        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//        psoDesc.InputLayout = {basicIEs, _countof(basicIEs)};
//        psoDesc.pRootSignature = rootSignature.Get();
//        psoDesc.VS = CD3DX12_SHADER_BYTECODE(basicVS.Get());
//        psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
//        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(wireRS);
//        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//        psoDesc.DepthStencilState.DepthEnable = FALSE;
//        psoDesc.DepthStencilState.StencilEnable = FALSE;
//        psoDesc.SampleMask = UINT_MAX;
//        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//        psoDesc.NumRenderTargets = 1;
//        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//        psoDesc.SampleDesc.Count = 1;
//        ThrowIfFailed(device->CreateGraphicsPipelineState(
//            &psoDesc, IID_PPV_ARGS(&defaultWirePSO)));
//    }
//    //{
//    //    // Skinned mesh wire;
//    //    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//    //    psoDesc.InputLayout = {skinnedIEs, _countof(skinnedIEs)};
//    //    psoDesc.pRootSignature = rootSignature.Get();
//    //    psoDesc.VS = CD3DX12_SHADER_BYTECODE(skinnedVS.Get());
//    //    psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
//    //    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(wireRS);
//    //    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//    //    psoDesc.DepthStencilState.DepthEnable = FALSE;
//    //    psoDesc.DepthStencilState.StencilEnable = FALSE;
//    //    psoDesc.SampleMask = UINT_MAX;
//    //    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//    //    psoDesc.NumRenderTargets = 1;
//    //    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//    //    psoDesc.SampleDesc.Count = 1;
//    //    ThrowIfFailed(device->CreateGraphicsPipelineState(
//    //        &psoDesc, IID_PPV_ARGS(&skinnedWirePSO)));
//    //}
//    //{
//    //    // stencilMarkPSO;
//    //    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//    //    psoDesc.InputLayout = {basicIEs, _countof(basicIEs)};
//    //    psoDesc.pRootSignature = rootSignature.Get();
//    //    psoDesc.VS = CD3DX12_SHADER_BYTECODE(depthOnlyVS.Get());
//    //    psoDesc.PS = CD3DX12_SHADER_BYTECODE(depthOnlyPS.Get());
//    //    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidRS);
//    //    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//    //    psoDesc.DepthStencilState = maskDSS;
//    //    psoDesc.SampleMask = UINT_MAX;
//    //    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//    //    psoDesc.NumRenderTargets = 1;
//    //    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//    //    psoDesc.SampleDesc.Count = 1;
//    //    ThrowIfFailed(device->CreateGraphicsPipelineState(
//    //        &psoDesc, IID_PPV_ARGS(&depthOnlyPSO)));
//    //}
//    //{
//    //    // reflectSolidPSO: 諛섏궗?섎㈃ Winding 諛섎?
//    //    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//    //    psoDesc.InputLayout = {basicIEs, _countof(basicIEs)};
//    //    psoDesc.pRootSignature = rootSignature.Get();
//    //    psoDesc.VS = CD3DX12_SHADER_BYTECODE(basicVS.Get());
//    //    psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
//    //    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidCCWRS);
//    //    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//    //    psoDesc.DepthStencilState = drawMaskedDSS;
//    //    psoDesc.SampleMask = UINT_MAX;
//    //    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//    //    psoDesc.NumRenderTargets = 1;
//    //    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//    //    psoDesc.SampleDesc.Count = 1;
//    //    ThrowIfFailed(device->CreateGraphicsPipelineState(
//    //        &psoDesc, IID_PPV_ARGS(&reflectSolidPSO)));
//    //}
//    //{
//    //    // Skinned mesh solid
//    //    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//    //    psoDesc.InputLayout = {skinnedIEs, _countof(skinnedIEs)};
//    //    psoDesc.pRootSignature = rootSignature.Get();
//    //    psoDesc.VS = CD3DX12_SHADER_BYTECODE(skinnedVS.Get());
//    //    psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
//    //    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidCCWRS);
//    //    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//    //    psoDesc.DepthStencilState = drawMaskedDSS;
//    //    psoDesc.SampleMask = UINT_MAX;
//    //    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//    //    psoDesc.NumRenderTargets = 1;
//    //    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//    //    psoDesc.SampleDesc.Count = 1;
//    //    ThrowIfFailed(device->CreateGraphicsPipelineState(
//    //        &psoDesc, IID_PPV_ARGS(&reflectSkinnedSolidPSO)));
//    //}
//    //{
//    //    // reflectWirePSO: 諛섏궗?섎㈃ Winding 諛섎?
//    //    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//    //    psoDesc.InputLayout = {skinnedIEs, _countof(skinnedIEs)};
//    //    psoDesc.pRootSignature = rootSignature.Get();
//    //    psoDesc.VS = CD3DX12_SHADER_BYTECODE(skinnedVS.Get());
//    //    psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
//    //    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(wireCCWRS);
//    //    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//    //    psoDesc.DepthStencilState = drawMaskedDSS;
//    //    psoDesc.SampleMask = UINT_MAX;
//    //    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//    //    psoDesc.NumRenderTargets = 1;
//    //    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//    //    psoDesc.SampleDesc.Count = 1;
//    //    ThrowIfFailed(device->CreateGraphicsPipelineState(
//    //        &psoDesc, IID_PPV_ARGS(&reflectWirePSO)));
//    //}
//    //{
//    //    // reflectSkinnedSolidPSO
//    //    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//    //    psoDesc.InputLayout = {skinnedIEs, _countof(skinnedIEs)};
//    //    psoDesc.pRootSignature = rootSignature.Get();
//    //    psoDesc.VS = CD3DX12_SHADER_BYTECODE(skinnedVS.Get());
//    //    psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
//    //    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(wireCCWRS);
//    //    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//    //    psoDesc.DepthStencilState = drawMaskedDSS;
//    //    psoDesc.SampleMask = UINT_MAX;
//    //    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//    //    psoDesc.NumRenderTargets = 1;
//    //    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//    //    psoDesc.SampleDesc.Count = 1;
//    //    ThrowIfFailed(device->CreateGraphicsPipelineState(
//    //        &psoDesc, IID_PPV_ARGS(&reflectSkinnedSolidPSO)));
//    //}
//    //{
//    //    // mirrorBlendSolidPSO
//    //    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//    //    psoDesc.InputLayout = {basicIEs, _countof(basicIEs)};
//    //    psoDesc.pRootSignature = rootSignature.Get();
//    //    psoDesc.VS = CD3DX12_SHADER_BYTECODE(basicVS.Get());
//    //    psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
//    //    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidRS);
//    //    psoDesc.BlendState = CD3DX12_BLEND_DESC(mirrorBS);
//    //    psoDesc.DepthStencilState = drawMaskedDSS;
//    //    psoDesc.SampleMask = UINT_MAX;
//    //    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//    //    psoDesc.NumRenderTargets = 1;
//    //    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//    //    psoDesc.SampleDesc.Count = 1;
//    //    ThrowIfFailed(device->CreateGraphicsPipelineState(
//    //        &psoDesc, IID_PPV_ARGS(&mirrorBlendSolidPSO)));
//    //}
//    //{
//    //    // mirrorBlendWirePSO;
//    //    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//    //    psoDesc.InputLayout = {basicIEs, _countof(basicIEs)};
//    //    psoDesc.pRootSignature = rootSignature.Get();
//    //    psoDesc.VS = CD3DX12_SHADER_BYTECODE(basicVS.Get());
//    //    psoDesc.PS = CD3DX12_SHADER_BYTECODE(basicPS.Get());
//    //    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(wireRS);
//    //    psoDesc.BlendState = CD3DX12_BLEND_DESC(mirrorBS);
//    //    psoDesc.DepthStencilState = drawMaskedDSS;
//    //    psoDesc.SampleMask = UINT_MAX;
//    //    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//    //    psoDesc.NumRenderTargets = 1;
//    //    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//    //    psoDesc.SampleDesc.Count = 1;
//    //    ThrowIfFailed(device->CreateGraphicsPipelineState(
//    //        &psoDesc, IID_PPV_ARGS(&mirrorBlendWirePSO)));
//    //}
//    //{
//    //    // skyboxSolidPSO;
//    //    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//    //    psoDesc.InputLayout = {skyboxIE, _countof(skyboxIE)};
//    //    psoDesc.pRootSignature = rootSignature.Get();
//    //    psoDesc.VS = CD3DX12_SHADER_BYTECODE(skyboxVS.Get());
//    //    psoDesc.PS = CD3DX12_SHADER_BYTECODE(skyboxPS.Get());
//    //    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidRS);
//    //    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//    //    psoDesc.DepthStencilState.DepthEnable = FALSE;
//    //    psoDesc.DepthStencilState.StencilEnable = FALSE;
//    //    psoDesc.SampleMask = UINT_MAX;
//    //    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//    //    psoDesc.NumRenderTargets = 1;
//    //    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//    //    psoDesc.SampleDesc.Count = 1;
//    //    ThrowIfFailed(device->CreateGraphicsPipelineState(
//    //        &psoDesc, IID_PPV_ARGS(&skyboxSolidPSO)));
//    //}
//    //{
//    //    // skyboxWirePSO;
//    //    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//    //    psoDesc.InputLayout = {skyboxIE, _countof(skyboxIE)};
//    //    psoDesc.pRootSignature = rootSignature.Get();
//    //    psoDesc.VS = CD3DX12_SHADER_BYTECODE(skyboxVS.Get());
//    //    psoDesc.PS = CD3DX12_SHADER_BYTECODE(skyboxPS.Get());
//    //    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(wireRS);
//    //    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//    //    psoDesc.DepthStencilState.DepthEnable = FALSE;
//    //    psoDesc.DepthStencilState.StencilEnable = FALSE;
//    //    psoDesc.SampleMask = UINT_MAX;
//    //    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//    //    psoDesc.NumRenderTargets = 1;
//    //    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//    //    psoDesc.SampleDesc.Count = 1;
//    //    ThrowIfFailed(device->CreateGraphicsPipelineState(
//    //        &psoDesc, IID_PPV_ARGS(&skyboxWirePSO)));
//    //}
//    //{
//    //    // reflectSkyboxSolidPSO;
//    //    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//    //    psoDesc.InputLayout = {skyboxIE, _countof(skyboxIE)};
//    //    psoDesc.pRootSignature = rootSignature.Get();
//    //    psoDesc.VS = CD3DX12_SHADER_BYTECODE(skyboxVS.Get());
//    //    psoDesc.PS = CD3DX12_SHADER_BYTECODE(skyboxPS.Get());
//    //    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidCCWRS);
//    //    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//    //    psoDesc.DepthStencilState = drawMaskedDSS;
//    //    psoDesc.SampleMask = UINT_MAX;
//    //    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//    //    psoDesc.NumRenderTargets = 1;
//    //    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//    //    psoDesc.SampleDesc.Count = 1;
//    //    ThrowIfFailed(device->CreateGraphicsPipelineState(
//    //        &psoDesc, IID_PPV_ARGS(&skyboxSolidPSO)));
//    //}
//    //{
//    //    // reflectSkyboxWirePSO;
//    //    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//    //    psoDesc.InputLayout = {skyboxIE, _countof(skyboxIE)};
//    //    psoDesc.pRootSignature = rootSignature.Get();
//    //    psoDesc.VS = CD3DX12_SHADER_BYTECODE(skyboxVS.Get());
//    //    psoDesc.PS = CD3DX12_SHADER_BYTECODE(skyboxPS.Get());
//    //    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(wireCCWRS);
//    //    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//    //    psoDesc.DepthStencilState.DepthEnable = FALSE;
//    //    psoDesc.DepthStencilState.StencilEnable = FALSE;
//    //    psoDesc.SampleMask = UINT_MAX;
//    //    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//    //    psoDesc.NumRenderTargets = 1;
//    //    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//    //    psoDesc.SampleDesc.Count = 1;
//    //    ThrowIfFailed(device->CreateGraphicsPipelineState(
//    //        &psoDesc, IID_PPV_ARGS(&skyboxWirePSO)));
//    //}
//    //{
//    //    // depthOnlyPSO;
//    //    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//    //    psoDesc.InputLayout = {basicIEs, _countof(basicIEs)};
//    //    psoDesc.pRootSignature = rootSignature.Get();
//    //    psoDesc.VS = CD3DX12_SHADER_BYTECODE(depthOnlyVS.Get());
//    //    psoDesc.PS = CD3DX12_SHADER_BYTECODE(depthOnlyPS.Get());
//    //    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidRS);
//    //    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//    //    psoDesc.DepthStencilState.DepthEnable = FALSE;
//    //    psoDesc.DepthStencilState.StencilEnable = FALSE;
//    //    psoDesc.SampleMask = UINT_MAX;
//    //    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//    //    psoDesc.NumRenderTargets = 1;
//    //    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//    //    psoDesc.SampleDesc.Count = 1;
//    //    ThrowIfFailed(device->CreateGraphicsPipelineState(
//    //        &psoDesc, IID_PPV_ARGS(&defaultSolidPSO)));
//    //}
//    //{
//    //    // depthOnlyPSO;
//    //    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//    //    psoDesc.InputLayout = {skinnedIEs, _countof(skinnedIEs)};
//    //    psoDesc.pRootSignature = rootSignature.Get();
//    //    psoDesc.VS = CD3DX12_SHADER_BYTECODE(depthOnlySkinnedVS.Get());
//    //    psoDesc.PS = CD3DX12_SHADER_BYTECODE(depthOnlyPS.Get());
//    //    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(solidRS);
//    //    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//    //    psoDesc.DepthStencilState.DepthEnable = FALSE;
//    //    psoDesc.DepthStencilState.StencilEnable = FALSE;
//    //    psoDesc.SampleMask = UINT_MAX;
//    //    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//    //    psoDesc.NumRenderTargets = 1;
//    //    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//    //    psoDesc.SampleDesc.Count = 1;
//    //    ThrowIfFailed(device->CreateGraphicsPipelineState(
//    //        &psoDesc, IID_PPV_ARGS(&defaultSolidPSO)));
//    //}
//    //{
//    //    // postEffectsPSO;
//    //    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//    //    psoDesc.InputLayout = {samplingIED, _countof(samplingIED)};
//    //    psoDesc.pRootSignature = rootSignature.Get();
//    //    psoDesc.VS = CD3DX12_SHADER_BYTECODE(samplingVS.Get());
//    //    psoDesc.PS = CD3DX12_SHADER_BYTECODE(postEffectsPS.Get());
//    //    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(postProcessingRS);
//    //    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//    //    psoDesc.DepthStencilState.DepthEnable = FALSE;
//    //    psoDesc.DepthStencilState.StencilEnable = FALSE;
//    //    psoDesc.SampleMask = UINT_MAX;
//    //    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//    //    psoDesc.NumRenderTargets = 1;
//    //    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//    //    psoDesc.SampleDesc.Count = 1;
//    //    ThrowIfFailed(device->CreateGraphicsPipelineState(
//    //        &psoDesc, IID_PPV_ARGS(&defaultSolidPSO)));
//    //}
//    //{
//    //    // postProcessingPSO;
//    //    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
//    //    psoDesc.InputLayout = {samplingIED, _countof(samplingIED)};
//    //    psoDesc.pRootSignature = rootSignature.Get();
//    //    psoDesc.VS = CD3DX12_SHADER_BYTECODE(samplingVS.Get());
//    //    psoDesc.PS = CD3DX12_SHADER_BYTECODE(depthOnlyPS.Get()); // dummy
//    //    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(postProcessingRS);
//    //    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
//    //    psoDesc.DepthStencilState.DepthEnable = FALSE;
//    //    psoDesc.DepthStencilState.StencilEnable = FALSE;
//    //    psoDesc.SampleMask = UINT_MAX;
//    //    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//    //    psoDesc.NumRenderTargets = 1;
//    //    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//    //    psoDesc.SampleDesc.Count = 1;
//    //    ThrowIfFailed(device->CreateGraphicsPipelineState(
//    //        &psoDesc, IID_PPV_ARGS(&defaultSolidPSO)));
//    //}
//}
//
//} // namespace dx12
