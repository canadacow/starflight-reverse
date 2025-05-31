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
        float  CloudDensity;      // Density of the cloud volume
        float  NoiseScale;        // Scale of the noise texture
    };

    CloudVolumeRenderer();
    ~CloudVolumeRenderer();

    void Initialize(IRenderDevice* pDevice, IDeviceContext* pImmediateContext);

    void Render(IDeviceContext* pContext,
            const HLSL::CameraAttribs& CamAttribs,
            const HLSL::LightAttribs& LightAttrs,
            const float4& LightColor,
            ITexture* pDepthTexture,
            TEXTURE_FORMAT RTVFormat,
            TEXTURE_FORMAT DSVFormat);

    CloudParams& GetCloudParams() { return m_CloudParams; }
    const CloudParams& GetCloudParams() const { return m_CloudParams; }

private:

    void LoadNoiseTextures();
    void LoadWeatherMap();
    void CreateDepthConversionShaders(IRenderDevice* pDevice, IShaderSourceInputStreamFactory* pShaderSourceFactory);

    // Full-screen quad resources
    RefCntAutoPtr<IBuffer>                m_pVertexBuffer;
    RefCntAutoPtr<IBuffer>                m_pIndexBuffer;
    RefCntAutoPtr<IPipelineState>         m_pRenderCloudsPSO;
    RefCntAutoPtr<IShaderResourceBinding> m_pRenderCloudsSRB;
    RefCntAutoPtr<IBuffer>                m_pCloudParamsCB;
    RefCntAutoPtr<IBuffer>                m_pCameraAttribsCB; 
    RefCntAutoPtr<IBuffer>                m_pLightAttribsCB;

    RefCntAutoPtr<ITexture>               m_pHighFreqNoiseTexture;
    RefCntAutoPtr<ITexture>               m_pLowFreqNoiseTexture;
    RefCntAutoPtr<ITexture>               m_pWeatherMapTexture;
    RefCntAutoPtr<ISampler>               m_pNoiseSampler;
    RefCntAutoPtr<ITextureView>           m_pHighFreqNoiseSRV;
    RefCntAutoPtr<ITextureView>           m_pLowFreqNoiseSRV;
    RefCntAutoPtr<ITextureView>           m_pWeatherMapSRV;
    
    // Depth texture copy for UAV access
    RefCntAutoPtr<ITexture>               m_pDepthCopyTexture;
    RefCntAutoPtr<ITextureView>           m_pDepthBufferUAV;
    Uint32                                m_DepthCopyWidth = 0;
    Uint32                                m_DepthCopyHeight = 0;
    
    // Depth conversion shaders
    RefCntAutoPtr<IPipelineState>         m_pDepthCopyComputePSO;
    RefCntAutoPtr<IShaderResourceBinding> m_pDepthCopyComputeSRB;
    RefCntAutoPtr<IPipelineState>         m_pDepthWriteBackPSO;
    RefCntAutoPtr<IShaderResourceBinding> m_pDepthWriteBackSRB;

    CloudParams m_CloudParams;
    RefCntAutoPtr<IRenderDevice>          m_pDevice;
};

} // namespace Diligent