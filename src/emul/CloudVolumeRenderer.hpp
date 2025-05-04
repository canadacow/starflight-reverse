#pragma once

#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/TextureView.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/InputLayout.h"
#include "Common/interface/AdvancedMath.hpp"
#include "Common/interface/RefCntAutoPtr.hpp"
#include "Common/interface/BasicMath.hpp"

namespace Diligent
{

namespace HLSL
{
    struct CameraAttribs;
    struct LightAttribs;
}

class CloudVolumeRenderer
{
public:
    struct CloudParams
    {
        float4 CloudBoxMin;       // Bottom of cloud box
        float4 CloudBoxMax;       // Top of cloud box
        float4 CloudColor;        // Color of the cloud box
        float  CloudOpacity;      // Opacity of the cloud box
    };

    CloudVolumeRenderer();
    ~CloudVolumeRenderer();

    void Initialize(IRenderDevice* pDevice, IDeviceContext* pImmediateContext);

    void Render(IDeviceContext* pContext,
            const HLSL::CameraAttribs& CamAttribs,
            const HLSL::LightAttribs& LightAttrs,
            const float4& LightColor,
            ITextureView* pDepthBufferSRV,
            TEXTURE_FORMAT RTVFormat,
            TEXTURE_FORMAT DSVFormat);

    CloudParams& GetCloudParams() { return m_CloudParams; }
    const CloudParams& GetCloudParams() const { return m_CloudParams; }

    // Setup cloud parameters using terrain data
    void SetupTerrainParameters(const BoundBox& terrainBounds, float waterHeight);

private:
    // Full-screen quad resources
    RefCntAutoPtr<IBuffer>                m_pVertexBuffer;
    RefCntAutoPtr<IBuffer>                m_pIndexBuffer;
    RefCntAutoPtr<IPipelineState>         m_pRenderCloudsPSO;
    RefCntAutoPtr<IShaderResourceBinding> m_pRenderCloudsSRB;
    RefCntAutoPtr<IBuffer>                m_pCloudParamsCB;
    
    CloudParams m_CloudParams;
    RefCntAutoPtr<IRenderDevice>          m_pDevice;
};

} // namespace Diligent