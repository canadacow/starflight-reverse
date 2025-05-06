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
        float4 CloudBoxMin;            // Bottom of cloud box
        float4 CloudBoxMax;            // Top of cloud box
        float4 CloudColor;             // Base color of clouds
        float4 ScatteringParams;       // x: scattering, y: absorption, z: phase parameter g, w: density multiplier
        float4 NoiseParams;            // x: base scale, y: detail scale, z: detail strength, w: curl strength
        float4 ShapeParams;            // x: coverage, y: base density, z: anvil bias, w: height gradient scale
        float4 AnimationParams;        // x,y,z: wind direction and speed, w: time
        float  CloudOpacity;           // Master opacity control
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
    void SetupTerrainParameters(const BoundBox& terrainBounds);

    // Update cloud animation based on time
    void Update(double elapsedTime);

private:
    // Full-screen quad resources
    RefCntAutoPtr<IBuffer>                m_pVertexBuffer;
    RefCntAutoPtr<IBuffer>                m_pIndexBuffer;
    RefCntAutoPtr<IPipelineState>         m_pRenderCloudsPSO;
    RefCntAutoPtr<IShaderResourceBinding> m_pRenderCloudsSRB;
    RefCntAutoPtr<IBuffer>                m_pCloudParamsCB;
    
    // Noise textures for cloud generation
    RefCntAutoPtr<ITexture>               m_pPerlinWorleyNoise;
    RefCntAutoPtr<ITextureView>           m_pPerlinWorleyNoiseSRV;
    RefCntAutoPtr<ITexture>               m_pWorleyNoise;
    RefCntAutoPtr<ITextureView>           m_pWorleyNoiseSRV;
    RefCntAutoPtr<ITexture>               m_pWeatherMap;
    RefCntAutoPtr<ITextureView>           m_pWeatherMapSRV;
    
    // Time elapsed for animation
    double m_Time = 0.0;
    
    CloudParams m_CloudParams;
    RefCntAutoPtr<IRenderDevice>          m_pDevice;
    
    // Creates noise textures needed for cloud rendering
    void CreateNoiseTextures(IRenderDevice* pDevice);
};

} // namespace Diligent