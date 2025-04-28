#pragma once

#include <memory>
#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/PipelineState.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/ShaderResourceBinding.h"
#include "Common/interface/BasicMath.hpp"

namespace Diligent
{
    struct IRenderDevice;
    struct IDeviceContext;
    struct IBuffer;
    struct ITextureView;
    class SF_GLTF_PBR_Renderer;
}

using namespace Diligent;

class SunRenderer
{
public:
    SunRenderer(IRenderDevice* pDevice, IDeviceContext* pContext);
    
    void Render(IDeviceContext* pContext, 
                const float3& sunDirection, 
                const float3& sunColor,
                const float4x4& viewProj,
                const float3& cameraPos,
                float sunSize = 0.025f, 
                float sunIntensity = 1.0f,
                TEXTURE_FORMAT renderTargetFormat = TEX_FORMAT_RGBA8_UNORM,
                TEXTURE_FORMAT depthFormat = TEX_FORMAT_D32_FLOAT);

private:
    RefCntAutoPtr<IRenderDevice>          m_pDevice;
    RefCntAutoPtr<IPipelineState>         m_pPSO;
    RefCntAutoPtr<IShaderResourceBinding> m_pSRB;
    RefCntAutoPtr<IBuffer>                m_pSunAttribsCB;
    RefCntAutoPtr<IBuffer>                m_pCameraAttribsCB;
    TEXTURE_FORMAT                        m_LastRTFormat = TEX_FORMAT_UNKNOWN;
    TEXTURE_FORMAT                        m_LastDSFormat = TEX_FORMAT_UNKNOWN;

    struct SunAttribs
    {
        float4 Position;    // xyz = position in normalized screen space, w = size scale factor
        float4 Color;       // xyz = color, w = intensity
    };
    
    struct CameraAttribs
    {
        float4x4 ViewProj;
        float4   CameraPos;
    };
    
    void InitializePipeline(TEXTURE_FORMAT renderTargetFormat, TEXTURE_FORMAT depthFormat);
}; 