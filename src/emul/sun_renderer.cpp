#include "sun_renderer.h"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsTools/interface/ShaderMacroHelper.hpp"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"
#include "Graphics/GraphicsTools/interface/ScopedDebugGroup.hpp"
#include "Common/interface/RefCntAutoPtr.hpp"
#include "Common/interface/BasicMath.hpp"
#include "../pbr/SF_GLTF_PBR_Renderer.hpp"

using namespace Diligent;

SunRenderer::SunRenderer(IRenderDevice* pDevice, IDeviceContext* pContext)
    : m_pDevice(pDevice)
{
    // Create constant buffer for sun attributes
    BufferDesc CBDesc;
    CBDesc.Name = "Sun Attributes CB";
    CBDesc.Size = sizeof(SunAttribs);
    CBDesc.Usage = USAGE_DYNAMIC;
    CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
    CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
    pDevice->CreateBuffer(CBDesc, nullptr, &m_pSunAttribsCB);
    
    // Create constant buffer for camera attributes
    CBDesc.Name = "Camera Attributes CB";
    CBDesc.Size = sizeof(CameraAttribs);
    pDevice->CreateBuffer(CBDesc, nullptr, &m_pCameraAttribsCB);
}

void SunRenderer::InitializePipeline(TEXTURE_FORMAT renderTargetFormat, TEXTURE_FORMAT depthFormat)
{
    // Create pipeline state for sun rendering
    GraphicsPipelineStateCreateInfo PSOCreateInfo;
    PipelineStateDesc&              PSODesc = PSOCreateInfo.PSODesc;

    PSODesc.Name = "Sun PSO";
    PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;
    
    // Define shader resource layout
    PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

    // Shader creation
    ShaderCreateInfo ShaderCI;
    ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
    ShaderCI.Desc.UseCombinedTextureSamplers = true;
    ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
    ShaderCI.EntryPoint = "main";
    ShaderCI.Desc.Name = "Sun VS";
    ShaderCI.Source = R"(
        struct VSInput
        {
            uint VertexID : SV_VertexID;
        };

        struct PSInput
        {
            float4 Position : SV_POSITION;
            float2 TexCoord : TEXCOORD0;
        };

        cbuffer cbCameraAttribs
        {
            float4x4 g_ViewProj;
            float4   g_CameraPos;
        };

        cbuffer cbSunAttribs
        {
            float4 g_SunPosition;  // xyz = position in normalized screen space, w = size scale
            float4 g_SunColor;     // xyz = color, w = intensity
        };

        PSInput main(VSInput vsIn)
        {
            PSInput psIn;
            
            // Create a quad from vertex ID (using triangle strip with 4 vertices)
            // Vertices will form a quad in this order: (0,1,2,3) = (TL,TR,BL,BR)
            float2 pos;
            
            // First vertex: Top-left (-1,1)
            if (vsIn.VertexID == 0) pos = float2(-1.0, 1.0);
            // Second vertex: Top-right (1,1)
            else if (vsIn.VertexID == 1) pos = float2(1.0, 1.0);
            // Third vertex: Bottom-left (-1,-1)
            else if (vsIn.VertexID == 2) pos = float2(-1.0, -1.0);
            // Fourth vertex: Bottom-right (1,-1)
            else pos = float2(1.0, -1.0);
            
            // Half size of the quad in screen space
            float sunSize = g_SunPosition.w;
            
            // Position in normalized device coordinates
            float2 sunPos = g_SunPosition.xy;
            
            // Calculate vertex position and texture coordinates
            psIn.Position = float4(sunPos + pos * float2(sunSize, sunSize * g_CameraPos.w), 1.0, 1.0);
            psIn.TexCoord = pos * 0.5 + 0.5; // Map from [-1,1] to [0,1]
            
            return psIn;
        }
    )";

    RefCntAutoPtr<IShader> pVS;
    m_pDevice->CreateShader(ShaderCI, &pVS);

    // Pixel shader with multiple render targets
    ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
    ShaderCI.EntryPoint = "main";
    ShaderCI.Desc.Name = "Sun PS";
    ShaderCI.Source = R"(
        struct PSInput
        {
            float4 Position : SV_POSITION;
            float2 TexCoord : TEXCOORD0;
        };
        
        // Match the PBR renderer's output structure
        struct PSOutput
        {
            float4 Color        : SV_Target0;
            float4 Normal       : SV_Target1;
            float4 BaseColor    : SV_Target2;
            float4 MaterialData : SV_Target3;
            float4 MotionVec    : SV_Target4;
            float4 SpecularIBL  : SV_Target5;
        };

        cbuffer cbSunAttribs
        {
            float4 g_SunPosition;  // xyz = position in normalized screen space, w = size scale
            float4 g_SunColor;     // xyz = color, w = intensity
        };

        PSOutput main(PSInput psIn)
        {
            PSOutput output;
            
            // Calculate vector from center (0.5, 0.5) to current pixel
            float2 dir = psIn.TexCoord - float2(0.5, 0.5);
            
            // Distance from center (squared)
            float dist2 = dot(dir, dir);
            
            // Smoothly fade out towards the edges of the circle
            float alpha = saturate(1.0 - dist2 * 4.0);
            
            // Apply sun color and intensity
            float3 color = g_SunColor.rgb * g_SunColor.w;
            
            // Create a soft glow
            float glow = smoothstep(0.0, 0.5, alpha);
            color *= glow * 2.0;
            
            // Output to main color target
            output.Color = float4(color, alpha);
            
            // Fill other render targets with appropriate values
            output.Normal = float4(0.0, 0.0, 1.0, 1.0);           // Default normal pointing up
            output.BaseColor = float4(g_SunColor.rgb, alpha);      // Use sun color as base color
            output.MaterialData = float4(0.0, 0.0, 0.0, alpha);    // No roughness or metallic
            output.MotionVec = float4(0.0, 0.0, 0.0, 1.0);         // No motion
            output.SpecularIBL = float4(0.0, 0.0, 0.0, alpha);     // No IBL contribution
            
            return output;
        }
    )";

    RefCntAutoPtr<IShader> pPS;
    m_pDevice->CreateShader(ShaderCI, &pPS);

    // Pipeline state description
    PSOCreateInfo.pVS = pVS;
    PSOCreateInfo.pPS = pPS;

    // Set up multi-render target configuration to match PBR renderer
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 6;
    
    // Based on the error message, configure the render target formats to match PBR renderer:
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = TEX_FORMAT_RGBA16_FLOAT;   // Color
    PSOCreateInfo.GraphicsPipeline.RTVFormats[1] = TEX_FORMAT_RGBA16_FLOAT;   // Normal
    PSOCreateInfo.GraphicsPipeline.RTVFormats[2] = TEX_FORMAT_RGBA8_UNORM;    // BaseColor
    PSOCreateInfo.GraphicsPipeline.RTVFormats[3] = TEX_FORMAT_RG8_UNORM;      // MaterialData
    PSOCreateInfo.GraphicsPipeline.RTVFormats[4] = TEX_FORMAT_RG16_FLOAT;     // MotionVec
    PSOCreateInfo.GraphicsPipeline.RTVFormats[5] = TEX_FORMAT_RGBA16_FLOAT;   // SpecularIBL
    
    PSOCreateInfo.GraphicsPipeline.DSVFormat = depthFormat;
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = TRUE;
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = FALSE;
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthFunc = COMPARISON_FUNC_EQUAL;
    // Set reference value to 1.0 (maximum depth)
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.StencilEnable = FALSE;

    // Enable alpha blending for all render targets
    for (Uint32 i = 0; i < PSOCreateInfo.GraphicsPipeline.NumRenderTargets; ++i)
    {
        PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[i].BlendEnable = true;
        PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[i].SrcBlend = BLEND_FACTOR_SRC_ALPHA;
        PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[i].DestBlend = BLEND_FACTOR_ONE;
        PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[i].BlendOp = BLEND_OPERATION_ADD;
        PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[i].SrcBlendAlpha = BLEND_FACTOR_ONE;
        PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[i].DestBlendAlpha = BLEND_FACTOR_ZERO;
        PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[i].BlendOpAlpha = BLEND_OPERATION_ADD;
    }

    // Create the pipeline state
    m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSO);

    // Create shader resource binding
    m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);

    // Now bind the resources to the SRB directly
    auto* pSunAttribsVSVar = m_pSRB->GetVariableByName(SHADER_TYPE_VERTEX, "cbSunAttribs");
    if (pSunAttribsVSVar)
        pSunAttribsVSVar->Set(m_pSunAttribsCB);

    auto* pSunAttribsPSVar = m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "cbSunAttribs");
    if (pSunAttribsPSVar)
        pSunAttribsPSVar->Set(m_pSunAttribsCB);
        
    auto* pCameraAttribsVar = m_pSRB->GetVariableByName(SHADER_TYPE_VERTEX, "cbCameraAttribs");
    if (pCameraAttribsVar)
        pCameraAttribsVar->Set(m_pCameraAttribsCB);

    // Store current formats
    m_LastRTFormat = renderTargetFormat;
    m_LastDSFormat = depthFormat;
}

void SunRenderer::Render(IDeviceContext* pContext, 
                         const float4& sunDirection, 
                         const float3& sunColor,
                         const float4x4& viewProj,
                         const float3& cameraPos,
                         float sunSize, 
                         float sunIntensity,
                         int renderTargetWidth,
                         int renderTargetHeight,
                         TEXTURE_FORMAT renderTargetFormat,
                         TEXTURE_FORMAT depthFormat)
{
    ScopedDebugGroup DebugGroupGlobal{pContext, "SunRenderer::Render"};

   float4 f4LightPosPS = float4(-sunDirection.x, -sunDirection.y, -sunDirection.z, 0.0f) * viewProj.Transpose();

   if (f4LightPosPS.w <= 0.0f)
   {
       // Sun is behind the camera, skip rendering
       return;
   }

   f4LightPosPS.x /= f4LightPosPS.w;
   f4LightPosPS.y /= f4LightPosPS.w;
   f4LightPosPS.z /= f4LightPosPS.w;
   float fDistToLightOnScreen = length((float2&)f4LightPosPS);
   float fMaxDist             = 100;
   if (fDistToLightOnScreen > fMaxDist)
   {
        f4LightPosPS.x *= fMaxDist / fDistToLightOnScreen;
        f4LightPosPS.y *= fMaxDist / fDistToLightOnScreen;
   }
        
    // Initialize or recreate pipeline if needed
    if (m_pPSO == nullptr || 
        renderTargetFormat != m_LastRTFormat || 
        depthFormat != m_LastDSFormat)
    {
        InitializePipeline(renderTargetFormat, depthFormat);
    }
    // Update the sun attributes
    {
        MapHelper<SunAttribs> SunAttribs(pContext, m_pSunAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD);
        SunAttribs->Position = float4(f4LightPosPS.x, f4LightPosPS.y, 0.0f, sunSize);
        SunAttribs->Color = float4(sunColor.x, sunColor.y, sunColor.z, sunIntensity);
    }
    
    // Update the camera attributes
    {
        MapHelper<CameraAttribs> CamAttribs(pContext, m_pCameraAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD);
        CamAttribs->ViewProj = viewProj;
        // Store aspect ratio in the w component (height/width for proper circle scaling)
        float aspectRatio = static_cast<float>(renderTargetWidth) / 
                           static_cast<float>(renderTargetHeight);
        CamAttribs->CameraPos = float4(cameraPos.x, cameraPos.y, cameraPos.z, aspectRatio);
    }

    // Set pipeline state and shader resource binding
    pContext->SetPipelineState(m_pPSO);
    pContext->CommitShaderResources(m_pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // Draw a quad (4 vertices)
    DrawAttribs drawAttribs;
    drawAttribs.NumVertices = 4;
    pContext->Draw(drawAttribs);
} 