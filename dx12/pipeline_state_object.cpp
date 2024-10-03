#include "pipeline_state_object.h"

namespace graphics {
namespace sampler {
D3D12_SAMPLER_DESC linearWrapSS = {D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                                   D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                   D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                   D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                   0.0f,
                                   1,
                                   D3D12_COMPARISON_FUNC_NEVER,
                                   {0.0f, 0.0f, 0.0f, 0.0f},
                                   0.0f,
                                   D3D12_FLOAT32_MAX};

D3D12_SAMPLER_DESC linearClampSS = {D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                                    D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                    D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                    D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                    0.0f,
                                    1,
                                    D3D12_COMPARISON_FUNC_NEVER,
                                    {0.0f, 0.0f, 0.0f, 0.0f},
                                    0.0f,
                                    D3D12_FLOAT32_MAX};

D3D12_SAMPLER_DESC shadowPointSS = {D3D12_FILTER_MIN_MAG_MIP_POINT,
                                    D3D12_TEXTURE_ADDRESS_MODE_BORDER,
                                    D3D12_TEXTURE_ADDRESS_MODE_BORDER,
                                    D3D12_TEXTURE_ADDRESS_MODE_BORDER,
                                    0.0f,
                                    1,
                                    D3D12_COMPARISON_FUNC_NEVER,
                                    {1.0f, 1.0f, 1.0f, 1.0f},
                                    0.0f,
                                    D3D12_FLOAT32_MAX};

D3D12_SAMPLER_DESC shadowCompareSS = {
    D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
    D3D12_TEXTURE_ADDRESS_MODE_BORDER,
    D3D12_TEXTURE_ADDRESS_MODE_BORDER,
    D3D12_TEXTURE_ADDRESS_MODE_BORDER,
    0.0f,
    1,
    D3D12_COMPARISON_FUNC_LESS_EQUAL,
    {100.0f, 100.0f, 100.0f, 100.0f},
    0.0f,
    D3D12_FLOAT32_MAX};

D3D12_SAMPLER_DESC pointWrapSS = {D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                                  D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                  D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                  D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                  0.0f,
                                  1,
                                  D3D12_COMPARISON_FUNC_NEVER,
                                  {0.0f, 0.0f, 0.0f, 0.0f},
                                  0.0f,
                                  D3D12_FLOAT32_MAX};

D3D12_SAMPLER_DESC linearMirrorSS = {D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                                     D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                     D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                     D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                     0.0f,
                                     1,
                                     D3D12_COMPARISON_FUNC_NEVER,
                                     {0.0f, 0.0f, 0.0f, 0.0f},
                                     0.0f,
                                     D3D12_FLOAT32_MAX};

D3D12_SAMPLER_DESC pointClampSS = {D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                                   D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                   D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                   D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                   0.0f,
                                   1,
                                   D3D12_COMPARISON_FUNC_NEVER,
                                   {0.0f, 0.0f, 0.0f, 0.0f},
                                   0.0f,
                                   D3D12_FLOAT32_MAX};
} // namespace sampler

namespace layout {
D3D12_INPUT_ELEMENT_DESC combineIEs[2] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
};

D3D12_INPUT_ELEMENT_DESC basicIEs[4] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,
     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
};
} // namespace layout

namespace rasterizer {
D3D12_RASTERIZER_DESC solidRS = {D3D12_FILL_MODE_SOLID,
                                 D3D12_CULL_MODE_BACK,
                                 false,
                                 0,
                                 0.0f,
                                 0.0f,
                                 true,
                                 true,
                                 0,
                                 D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF};

D3D12_RASTERIZER_DESC solidCCWRS = {D3D12_FILL_MODE_SOLID,
                                    D3D12_CULL_MODE_BACK,
                                    true,
                                    0,
                                    0.0f,
                                    0.0f,
                                    true,
                                    true,
                                    0,
                                    D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF};

D3D12_RASTERIZER_DESC wireRS = {D3D12_FILL_MODE_WIREFRAME,
                                D3D12_CULL_MODE_BACK,
                                false,
                                0,
                                0.0f,
                                0.0f,
                                true,
                                true,
                                0,
                                D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF};

D3D12_RASTERIZER_DESC wireCCWRS = {D3D12_FILL_MODE_WIREFRAME,
                                   D3D12_CULL_MODE_BACK,
                                   true,
                                   0,
                                   0.0f,
                                   0.0f,
                                   true,
                                   true,
                                   0,
                                   D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF};

D3D12_RASTERIZER_DESC postRS = {D3D12_FILL_MODE_SOLID,
                                D3D12_CULL_MODE_NONE,
                                false,
                                0,
                                0.0f,
                                0.0f,
                                false,
                                false,
                                0,
                                D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF};
} // namespace rasterizer

namespace depth {
D3D12_DEPTH_STENCIL_DESC drawDSS = {
    true,
    D3D12_DEPTH_WRITE_MASK_ALL,
    D3D12_COMPARISON_FUNC_LESS,
    false,
    D3D12_DEFAULT_STENCIL_READ_MASK,
    D3D12_DEFAULT_STENCIL_WRITE_MASK,
    {D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP,
     D3D12_COMPARISON_FUNC_ALWAYS},
    {D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_REPLACE,
     D3D12_COMPARISON_FUNC_ALWAYS}};

// it's a DSS that marks 1 on Stencil
D3D12_DEPTH_STENCIL_DESC maskDSS = {
    true,
    D3D12_DEPTH_WRITE_MASK_ZERO,
    D3D12_COMPARISON_FUNC_LESS,
    true,
    D3D12_DEFAULT_STENCIL_READ_MASK,
    D3D12_DEFAULT_STENCIL_WRITE_MASK,
    {D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_REPLACE,
     D3D12_COMPARISON_FUNC_ALWAYS},
    {D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_REPLACE,
     D3D12_COMPARISON_FUNC_ALWAYS}};

// it's a DSS that draws "only" when marked as 1 in Stencil.
// D3D12_COMPARISON_FUNC_EQUAL: draw only when already marked as 1
// OMSetDepthStencilState(..., 1); <- 1
D3D12_DEPTH_STENCIL_DESC drawMaskedDSS = {
    true,
    D3D12_DEPTH_WRITE_MASK_ALL,
    D3D12_COMPARISON_FUNC_LESS_EQUAL,
    true,
    D3D12_DEFAULT_STENCIL_READ_MASK,
    D3D12_DEFAULT_STENCIL_WRITE_MASK,
    {D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP,
     D3D12_COMPARISON_FUNC_EQUAL},
    {D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_REPLACE,
     D3D12_COMPARISON_FUNC_ALWAYS}};
} // namespace depth

namespace blend {
D3D12_BLEND_DESC mirrorBS = {
    false /*MSAA*/,
    false,
    {true, false, D3D12_BLEND_BLEND_FACTOR, D3D12_BLEND_INV_BLEND_FACTOR,
     D3D12_BLEND_OP_ADD, D3D12_BLEND_ONE, D3D12_BLEND_ONE, D3D12_BLEND_OP_ADD}};
} // namespace blend

} // namespace graphics
