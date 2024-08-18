//#ifndef _GRAPHICSCOMMON
//#define _GRAPHICSCOMMON
//
//#include "graphics_pso.h"
//
//namespace dx12 {
//namespace pso {
//
//// Samplers
//extern D3D12_SAMPLER_DESC linearWrapSS;
//extern D3D12_SAMPLER_DESC linearClampSS;
//extern D3D12_SAMPLER_DESC shadowPointSS;
//extern D3D12_SAMPLER_DESC shadowCompareSS;
//extern std::vector<D3D12_SAMPLER_DESC> sampleStates;
//
//// Rasterizer States
//extern D3D12_RASTERIZER_DESC solidRS;
//extern D3D12_RASTERIZER_DESC solidCCWRS;
//extern D3D12_RASTERIZER_DESC wireRS;
//extern D3D12_RASTERIZER_DESC wireCCWRS;
//extern D3D12_RASTERIZER_DESC postProcessingRS;
//
//// Depth Stencil States
//extern D3D12_DEPTH_STENCIL_DESC drawDSS;
//extern D3D12_DEPTH_STENCIL_DESC maskDSS;
//extern D3D12_DEPTH_STENCIL_DESC drawMaskedDSS;
//
//// Blend States
//extern D3D12_BLEND_DESC mirrorBS;
//
//// Shaders
//extern ComPtr<ID3DBlob> basicVS;
//extern ComPtr<ID3DBlob> skinnedVS;
//extern ComPtr<ID3DBlob> skyboxVS;
//extern ComPtr<ID3DBlob> samplingVS;
//extern ComPtr<ID3DBlob> normalVS;
//extern ComPtr<ID3DBlob> depthOnlyVS;
//extern ComPtr<ID3DBlob> depthOnlySkinnedVS;
//
//extern ComPtr<ID3DBlob> basicPS;
//extern ComPtr<ID3DBlob> skyboxPS;
//extern ComPtr<ID3DBlob> combinePS;
//extern ComPtr<ID3DBlob> bloomDownPS;
//extern ComPtr<ID3DBlob> bloomUpPS;
//extern ComPtr<ID3DBlob> normalPS;
//extern ComPtr<ID3DBlob> depthOnlyPS;
//extern ComPtr<ID3DBlob> postEffectsPS;
//extern ComPtr<ID3DBlob> normalGS;
//extern ComPtr<ID3DBlob> brightPassCS;
//extern ComPtr<ID3DBlob> blurVerticalCS;
//extern ComPtr<ID3DBlob> blurHorizontalCS;
//extern ComPtr<ID3DBlob> bloomComposite;
//
//// Graphics Pipeline States
//extern ComPtr<ID3D12PipelineState> defaultSolidPSO;
//extern ComPtr<ID3D12PipelineState> skinnedSolidPSO;
//extern ComPtr<ID3D12PipelineState> defaultWirePSO;
//extern ComPtr<ID3D12PipelineState> skinnedWirePSO;
//extern ComPtr<ID3D12PipelineState> stencilMaskPSO;
//extern ComPtr<ID3D12PipelineState> reflectSolidPSO;
//extern ComPtr<ID3D12PipelineState> reflectSkinnedSolidPSO;
//extern ComPtr<ID3D12PipelineState> reflectWirePSO;
//extern ComPtr<ID3D12PipelineState> reflectSkinnedWirePSO;
//extern ComPtr<ID3D12PipelineState> mirrorBlendSolidPSO;
//extern ComPtr<ID3D12PipelineState> mirrorBlendWirePSO;
//extern ComPtr<ID3D12PipelineState> skyboxSolidPSO;
//extern ComPtr<ID3D12PipelineState> skyboxWirePSO;
//extern ComPtr<ID3D12PipelineState> reflectSkyboxSolidPSO;
//extern ComPtr<ID3D12PipelineState> reflectSkyboxWirePSO;
//extern ComPtr<ID3D12PipelineState> normalsPSO;
//extern ComPtr<ID3D12PipelineState> depthOnlyPSO;
//extern ComPtr<ID3D12PipelineState> depthOnlySkinnedPSO;
//extern ComPtr<ID3D12PipelineState> postEffectsPSO;
//extern ComPtr<ID3D12PipelineState> postProcessingPSO;
//
//void InitCommonStates(ComPtr<ID3D12Device> &device,
//                      ComPtr<ID3D12RootSignature> &rootSignature);
//
//void InitSamplers(ComPtr<ID3D12Device> &device);
//void InitRasterizerStates(ComPtr<ID3D12Device> &device);
//void InitBlendStates(ComPtr<ID3D12Device> &device);
//void InitDepthStencilStates(ComPtr<ID3D12Device> &device);
//void InitPipelineStates(ComPtr<ID3D12Device> &device,
//                        ComPtr<ID3D12RootSignature> &rootSignature);
//
//void CreateVertexShader(ComPtr<ID3D12Device> &device, std::wstring filename,
//                  ComPtr<ID3DBlob> &m_vertexShader,
//                  const std::vector<D3D_SHADER_MACRO> shaderMacros = {
//                      /* Empty default */});
//void CreatePixelShader(ComPtr<ID3D12Device> &device, std::wstring filename,
//                        ComPtr<ID3DBlob> &shader);
//} // namespace pso
//
//} // namespace graphics
//
//#endif