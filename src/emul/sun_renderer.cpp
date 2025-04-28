#include "sun_renderer.h"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/Texture.h"
#include "Graphics/GraphicsTools/interface/ShaderMacroHelper.hpp"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"
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
            
            // Create a quad from vertex ID
            float2 pos = float2(
                (vsIn.VertexID == 1) ? 1.0 : -1.0, 
                (vsIn.VertexID == 2) ? -1.0 : 1.0
            );
            
            // Half size of the quad in screen space
            float sunSize = g_SunPosition.w;
            
            // Position in normalized device coordinates
            float2 sunPos = g_SunPosition.xy;
            
            // Calculate vertex position and texture coordinates
            psIn.Position = float4(sunPos + pos * sunSize, 0.0, 1.0);
            psIn.TexCoord = pos * 0.5 + 0.5;
            
            return psIn;
        }
    )";

    RefCntAutoPtr<IShader> pVS;
    m_pDevice->CreateShader(ShaderCI, &pVS);

    // Pixel shader
    ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
    ShaderCI.EntryPoint = "main";
    ShaderCI.Desc.Name = "Sun PS";
    ShaderCI.Source = R"(
        struct PSInput
        {
            float4 Position : SV_POSITION;
            float2 TexCoord : TEXCOORD0;
        };

        cbuffer cbSunAttribs
        {
            float4 g_SunPosition;  // xyz = position in normalized screen space, w = size scale
            float4 g_SunColor;     // xyz = color, w = intensity
        };

        float4 main(PSInput psIn) : SV_TARGET
        {
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
            
            return float4(color, alpha);
        }
    )";

    RefCntAutoPtr<IShader> pPS;
    m_pDevice->CreateShader(ShaderCI, &pPS);

    // Pipeline state description
    PSOCreateInfo.pVS = pVS;
    PSOCreateInfo.pPS = pPS;

    // Use provided render target and depth formats
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = renderTargetFormat;
    PSOCreateInfo.GraphicsPipeline.DSVFormat = depthFormat;
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = TRUE;
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = FALSE;
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthFunc = COMPARISON_FUNC_LESS_EQUAL;

    // Enable alpha blending
    PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = true;
    PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend = BLEND_FACTOR_SRC_ALPHA;
    PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend = BLEND_FACTOR_ONE;
    PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOp = BLEND_OPERATION_ADD;
    PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlendAlpha = BLEND_FACTOR_ONE;
    PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlendAlpha = BLEND_FACTOR_ZERO;
    PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOpAlpha = BLEND_OPERATION_ADD;

    // Create the pipeline state
    m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSO);

    // Create shader resource binding
    m_pPSO->CreateShaderResourceBinding(&m_pSRB, true);

    // Set the sun attributes constant buffer
    auto* pSunAttribsVar = m_pSRB->GetVariableByName(SHADER_TYPE_VERTEX, "cbSunAttribs");
    if (pSunAttribsVar)
        pSunAttribsVar->Set(m_pSunAttribsCB);

    pSunAttribsVar = m_pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "cbSunAttribs");
    if (pSunAttribsVar)
        pSunAttribsVar->Set(m_pSunAttribsCB);
        
    // Set the camera attributes constant buffer
    auto* pCameraAttribsVar = m_pSRB->GetVariableByName(SHADER_TYPE_VERTEX, "cbCameraAttribs");
    if (pCameraAttribsVar)
        pCameraAttribsVar->Set(m_pCameraAttribsCB);
        
    // Store current formats
    m_LastRTFormat = renderTargetFormat;
    m_LastDSFormat = depthFormat;
}

void SunRenderer::Render(IDeviceContext* pContext, 
                         const float3& sunDirection, 
                         const float3& sunColor,
                         const float4x4& viewProj,
                         const float3& cameraPos,
                         float sunSize, 
                         float sunIntensity,
                         TEXTURE_FORMAT renderTargetFormat,
                         TEXTURE_FORMAT depthFormat)
{
    // Skip rendering if behind camera
    if (sunDirection.z <= 0)
        return;
        
    // Initialize or recreate pipeline if needed
    if (m_pPSO == nullptr || 
        renderTargetFormat != m_LastRTFormat || 
        depthFormat != m_LastDSFormat)
    {
        InitializePipeline(renderTargetFormat, depthFormat);
    }

    // Project sun position to NDC space
    // The sun is at "infinity", so we use just the direction vector to compute its position on screen
    float2 sunScreenPos = float2(sunDirection.x / sunDirection.z, sunDirection.y / sunDirection.z);
    
    // Update the sun attributes
    {
        MapHelper<SunAttribs> SunAttribs(pContext, m_pSunAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD);
        SunAttribs->Position = float4(sunScreenPos.x, sunScreenPos.y, 0.0f, sunSize);
        SunAttribs->Color = float4(sunColor.x, sunColor.y, sunColor.z, sunIntensity);
    }
    
    // Update the camera attributes
    {
        MapHelper<CameraAttribs> CamAttribs(pContext, m_pCameraAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD);
        CamAttribs->ViewProj = viewProj;
        CamAttribs->CameraPos = float4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f);
    }

    // Set pipeline state and shader resource binding
    pContext->SetPipelineState(m_pPSO);
    pContext->CommitShaderResources(m_pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // Draw a quad (4 vertices)
    DrawAttribs drawAttribs;
    drawAttribs.NumVertices = 4;
    pContext->Draw(drawAttribs);
} 