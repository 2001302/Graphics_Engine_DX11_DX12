#ifndef _GRAPHICSCOMMON
#define _GRAPHICSCOMMON

#include "graphics_pso.h"

namespace core {
inline void ThrowIfFailed(HRESULT hr) {
    if (FAILED(hr)) {
        throw std::exception();
    }
}

namespace graphics {

// Samplers
extern ComPtr<ID3D11SamplerState> linearWrapSS;
extern ComPtr<ID3D11SamplerState> linearClampSS;
extern ComPtr<ID3D11SamplerState> shadowPointSS;
extern ComPtr<ID3D11SamplerState> shadowCompareSS;
extern std::vector<ID3D11SamplerState *> sampleStates;

// Rasterizer States
extern ComPtr<ID3D11RasterizerState> solidRS;
extern ComPtr<ID3D11RasterizerState> solidCCWRS;
extern ComPtr<ID3D11RasterizerState> wireRS;
extern ComPtr<ID3D11RasterizerState> wireCCWRS;
extern ComPtr<ID3D11RasterizerState> postProcessingRS;

// Depth Stencil States
extern ComPtr<ID3D11DepthStencilState> drawDSS;
extern ComPtr<ID3D11DepthStencilState> maskDSS;
extern ComPtr<ID3D11DepthStencilState> drawMaskedDSS;

// Shaders
extern ComPtr<ID3D11VertexShader> basicVS;
extern ComPtr<ID3D11VertexShader> skinnedVS;
extern ComPtr<ID3D11VertexShader> skyboxVS;
extern ComPtr<ID3D11VertexShader> samplingVS;
extern ComPtr<ID3D11VertexShader> normalVS;
extern ComPtr<ID3D11VertexShader> depthOnlyVS;
extern ComPtr<ID3D11VertexShader> depthOnlySkinnedVS;

extern ComPtr<ID3D11PixelShader> basicPS;
extern ComPtr<ID3D11PixelShader> skyboxPS;
extern ComPtr<ID3D11PixelShader> combinePS;
extern ComPtr<ID3D11PixelShader> bloomDownPS;
extern ComPtr<ID3D11PixelShader> bloomUpPS;
extern ComPtr<ID3D11PixelShader> normalPS;
extern ComPtr<ID3D11PixelShader> depthOnlyPS;
extern ComPtr<ID3D11PixelShader> postEffectsPS;
extern ComPtr<ID3D11GeometryShader> normalGS;
extern ComPtr<ID3D11ComputeShader> brightPassCS;
extern ComPtr<ID3D11ComputeShader> blurVerticalCS;
extern ComPtr<ID3D11ComputeShader> blurHorizontalCS;
extern ComPtr<ID3D11ComputeShader> bloomComposite;

// Input Layouts
extern ComPtr<ID3D11InputLayout> basicIL;
extern ComPtr<ID3D11InputLayout> samplingIL;
extern ComPtr<ID3D11InputLayout> skyboxIL;
extern ComPtr<ID3D11InputLayout> postProcessingIL;

// Blend States
extern ComPtr<ID3D11BlendState> mirrorBS;

// Graphics Pipeline States
extern PipelineState defaultSolidPSO;
extern PipelineState skinnedSolidPSO;
extern PipelineState defaultWirePSO;
extern PipelineState skinnedWirePSO;
extern PipelineState stencilMaskPSO;
extern PipelineState reflectSolidPSO;
extern PipelineState reflectSkinnedSolidPSO;
extern PipelineState reflectWirePSO;
extern PipelineState reflectSkinnedWirePSO;
extern PipelineState mirrorBlendSolidPSO;
extern PipelineState mirrorBlendWirePSO;
extern PipelineState skyboxSolidPSO;
extern PipelineState skyboxWirePSO;
extern PipelineState reflectSkyboxSolidPSO;
extern PipelineState reflectSkyboxWirePSO;
extern PipelineState normalsPSO;
extern PipelineState depthOnlyPSO;
extern PipelineState depthOnlySkinnedPSO;
extern PipelineState postEffectsPSO;
extern PipelineState postProcessingPSO;

void InitCommonStates(ComPtr<ID3D11Device> &device);

void InitSamplers(ComPtr<ID3D11Device> &device);
void InitRasterizerStates(ComPtr<ID3D11Device> &device);
void InitBlendStates(ComPtr<ID3D11Device> &device);
void InitDepthStencilStates(ComPtr<ID3D11Device> &device);
void InitPipelineStates(ComPtr<ID3D11Device> &device);
void InitShaders(ComPtr<ID3D11Device> &device);

void CreateVertexShaderAndInputLayout(
    ComPtr<ID3D11Device> &device, std::wstring filename,
    const std::vector<D3D11_INPUT_ELEMENT_DESC> &inputElements,
    ComPtr<ID3D11VertexShader> &m_vertexShader,
    ComPtr<ID3D11InputLayout> &m_inputLayout,
    const std::vector<D3D_SHADER_MACRO> shaderMacros = {/* Empty default */});

void CreateHullShader(ComPtr<ID3D11Device> &device,
                      const std::wstring &filename,
                      ComPtr<ID3D11HullShader> &m_hullShader);

void CreateDomainShader(ComPtr<ID3D11Device> &device,
                        const std::wstring &filename,
                        ComPtr<ID3D11DomainShader> &m_domainShader);

void CreateGeometryShader(ComPtr<ID3D11Device> &device,
                          const std::wstring &filename,
                          ComPtr<ID3D11GeometryShader> &m_geometryShader);

void CreatePixelShader(ComPtr<ID3D11Device> &device,
                       const std::wstring &filename,
                       ComPtr<ID3D11PixelShader> &m_pixelShader);

void CreateComputeShader(ComPtr<ID3D11Device> &device,
                         const std::wstring &filename,
                         ComPtr<ID3D11ComputeShader> &computeShader);

} // namespace graphics

} // namespace core

#endif