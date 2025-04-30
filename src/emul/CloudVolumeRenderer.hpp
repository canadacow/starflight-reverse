#pragma once

#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsEngine/interface/TextureView.h"
#include "Graphics/GraphicsEngine/interface/Buffer.h"
#include "Graphics/GraphicsEngine/interface/InputLayout.h"
#include "Common/interface/RefCntAutoPtr.hpp"
#include "Common/interface/BasicMath.hpp"

// Forward declarations
namespace HLSL {
    struct CameraAttribs;
    struct LightAttribs;
}

namespace Diligent
{
    class CloudVolumeRenderer
    {
    public:
        struct CloudParams
        {
            float4 CloudBoxMin;       // Bottom of cloud box
            float4 CloudBoxMax;       // Top of cloud box
            float  CloudDensity;      // Overall density
            float  CloudCoverage;     // How much of the sky is covered
            float  CloudSpeed;        // Movement speed
            float  CloudShadowIntensity;
            float  CloudLightAbsorption;
            float3 WindDirection;     // Direction clouds move
            float  Time;              // For cloud animation
            uint   NoiseOctaves;      // Number of noise octaves
            float  DetailStrength;    // Strength of detail noise
            float  DetailScale;       // Scale of detail noise
            float  Padding[1];        // Padding to 16-byte alignment
        };

        CloudVolumeRenderer();
        ~CloudVolumeRenderer();

        void Initialize(IRenderDevice* pDevice, IDeviceContext* pImmediateContext);

        void Render(IDeviceContext* pContext,
                   const float4x4&  ViewProj,
                   const float3&    CameraPos,
                   const float3&    LightDir,
                   const float3&    LightColor,
                   ITextureView*    pDepthBufferSRV,
                   TEXTURE_FORMAT   RTVFormat,
                   TEXTURE_FORMAT   DSVFormat);

        // Simplified render function that works with the existing framework
        void Render(IDeviceContext* pContext,
                  const HLSL::CameraAttribs& CamAttribs,
                  const HLSL::LightAttribs& LightAttrs,
                  ITextureView* pDepthBufferSRV,
                  TEXTURE_FORMAT RTVFormat,
                  TEXTURE_FORMAT DSVFormat);

        CloudParams& GetCloudParams() { return m_CloudParams; }
        const CloudParams& GetCloudParams() const { return m_CloudParams; }

        // Setup cloud parameters using terrain data
        void SetupTerrainParameters(float terrainMaxX, float terrainMaxZ, float waterHeight);

        // Update time parameter for cloud animation
        void UpdateTime(float time);

    private:
        void CreateVolumeNoiseTexture(IRenderDevice* pDevice, IDeviceContext* pContext);
        void CreateDetailNoiseTexture(IRenderDevice* pDevice, IDeviceContext* pContext);
        void CreateWeatherMap(IRenderDevice* pDevice, IDeviceContext* pContext);

    private:
        RefCntAutoPtr<IPipelineState>         m_pRenderCloudsPSO;
        RefCntAutoPtr<IShaderResourceBinding> m_pRenderCloudsSRB;
        RefCntAutoPtr<IBuffer>                m_pCloudParamsCB;
        
        RefCntAutoPtr<ITexture>               m_pVolumeNoiseTexture;
        RefCntAutoPtr<ITextureView>           m_pVolumeNoiseTextureSRV;
        
        RefCntAutoPtr<ITexture>               m_pDetailNoiseTexture;
        RefCntAutoPtr<ITextureView>           m_pDetailNoiseTextureSRV;
        
        RefCntAutoPtr<ITexture>               m_pWeatherMapTexture;
        RefCntAutoPtr<ITextureView>           m_pWeatherMapTextureSRV;
        
        // Full-screen quad resources
        RefCntAutoPtr<IBuffer>                m_pVertexBuffer;
        RefCntAutoPtr<IBuffer>                m_pIndexBuffer;
        
        CloudParams m_CloudParams;
        RefCntAutoPtr<IRenderDevice>          m_pDevice;
    };
} 