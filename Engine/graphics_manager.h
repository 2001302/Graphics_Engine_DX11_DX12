#ifndef _GRAPHICSMANAGER
#define _GRAPHICSMANAGER

#define MAX_LIGHTS 3
#define LIGHT_OFF 0x00
#define LIGHT_DIRECTIONAL 0x01
#define LIGHT_POINT 0x02
#define LIGHT_SPOT 0x04
#define LIGHT_SHADOW 0x10

#include "graphics_common.h"
#include "graphics_util.h"

namespace engine {
class GraphicsManager {
  public:
    static GraphicsManager &Instance() {
        static GraphicsManager instance;
        return instance;
    }
    bool Initialize();
    void SetMainViewport();
    void SetShadowViewport();
    bool CreateBuffer();
    void SetPipelineState(const GraphicsPSO &pso);
    void SetGlobalConsts(ComPtr<ID3D11Buffer> &globalConstsGPU);

    bool useMSAA = true;
    UINT num_quality_levels = 0;

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> device_context;

    ComPtr<IDXGISwapChain> swap_chain;
    ComPtr<ID3D11RenderTargetView> back_buffer_RTV;

    ComPtr<ID3D11Texture2D> float_buffer;
    ComPtr<ID3D11RenderTargetView> float_RTV;

    ComPtr<ID3D11Texture2D> resolved_buffer;
    ComPtr<ID3D11RenderTargetView> resolved_RTV;
    ComPtr<ID3D11ShaderResourceView> resolved_SRV;

    ComPtr<ID3D11Texture2D> postEffectsBuffer;
    ComPtr<ID3D11RenderTargetView> postEffectsRTV;
    ComPtr<ID3D11ShaderResourceView> postEffectsSRV;

    ComPtr<ID3D11Texture2D> m_depthOnlyBuffer; // No MSAA
    ComPtr<ID3D11DepthStencilView> m_depthOnlyDSV;
    ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    ComPtr<ID3D11ShaderResourceView> m_depthOnlySRV;

    // Shadow maps
    int m_shadowWidth = 1280;
    int m_shadowHeight = 1280;
    ComPtr<ID3D11Texture2D> m_shadowBuffers[MAX_LIGHTS]; // No MSAA
    ComPtr<ID3D11DepthStencilView> m_shadowDSVs[MAX_LIGHTS];
    ComPtr<ID3D11ShaderResourceView> m_shadowSRVs[MAX_LIGHTS];

    D3D11_VIEWPORT viewport;

  private:
    GraphicsManager()
        : swap_chain(0), device(0), device_context(0),
          viewport(D3D11_VIEWPORT()) {}
    void CreateDepthBuffer();
};

} // namespace engine
#endif
