#include "CloudVolumeRenderer.hpp"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"
#include "Graphics/GraphicsTools/interface/GraphicsUtilities.h"
#include "Graphics/GraphicsEngine/interface/GraphicsTypesX.hpp"
#include "Graphics/GraphicsTools/interface/ShaderSourceFactoryUtils.hpp"
#include "Utilities/interface/DiligentFXShaderSourceStreamFactory.hpp"
#include "../util/lodepng.h"
#include <vector>

// No need to re-declare this function - it's already in graphics.cpp
// Just declare it as extern so we can use it
extern Diligent::RefCntAutoPtr<Diligent::IShaderSourceInputStreamFactory> SFCreateCompoundShaderSourceFactory(Diligent::IRenderDevice* pDevice, Diligent::IShaderSourceInputStreamFactory* pMemorySourceFactory);

namespace Diligent
{

namespace HLSL
{
#include "Shaders/Common/public/BasicStructures.fxh"
}

// Vertex structure used for the full-screen quad
struct FullScreenQuadVertex
{
    float2 pos;
    float2 uv;
};

CloudVolumeRenderer::CloudVolumeRenderer()
{
    // Default cloud parameters
    m_CloudParams.CloudBoxMin = float4(0.0f, 0.0f, 0.0f, 1.0f);
    m_CloudParams.CloudBoxMax = float4(100.0f, 50.0f, 100.0f, 1.0f);
    m_CloudParams.CloudColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    m_CloudParams.CloudOpacity = 0.3f;
}

CloudVolumeRenderer::~CloudVolumeRenderer()
{
}

void CloudVolumeRenderer::Initialize(IRenderDevice* pDevice, IDeviceContext* pImmediateContext)
{
    // Store device reference
    m_pDevice = pDevice;

    RefCntAutoPtr<IShaderSourceInputStreamFactory> pMemorySourceFactory = CreateMemoryShaderSourceFactory({});      
    auto pCompoundSourceFactory         = SFCreateCompoundShaderSourceFactory(pDevice, pMemorySourceFactory);

    // Create a full-screen quad for rendering
    {
        // Define vertices for a quad
        FullScreenQuadVertex QuadVerts[] =
        {
            {{-1.0f,  1.0f}, {0.0f, 0.0f}}, // Top-left
            {{ 1.0f,  1.0f}, {1.0f, 0.0f}}, // Top-right
            {{ 1.0f, -1.0f}, {1.0f, 1.0f}}, // Bottom-right
            {{-1.0f, -1.0f}, {0.0f, 1.0f}}  // Bottom-left
        };

        BufferDesc VertBuffDesc;
        VertBuffDesc.Name = "Cloud volume renderer quad vertices";
        VertBuffDesc.Usage = USAGE_IMMUTABLE;
        VertBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
        VertBuffDesc.Size = sizeof(QuadVerts);

        BufferData VBData;
        VBData.pData = QuadVerts;
        VBData.DataSize = sizeof(QuadVerts);
        pDevice->CreateBuffer(VertBuffDesc, &VBData, &m_pVertexBuffer);
    }

    // Create index buffer
    {
        Uint32 Indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        BufferDesc IndBuffDesc;
        IndBuffDesc.Name = "Cloud volume renderer quad indices";
        IndBuffDesc.Usage = USAGE_IMMUTABLE;
        IndBuffDesc.BindFlags = BIND_INDEX_BUFFER;
        IndBuffDesc.Size = sizeof(Indices);

        BufferData IBData;
        IBData.pData = Indices;
        IBData.DataSize = sizeof(Indices);
        pDevice->CreateBuffer(IndBuffDesc, &IBData, &m_pIndexBuffer);
    }

    // Create constant buffer
    {
        BufferDesc CBDesc;
        CBDesc.Name = "Cloud parameters buffer";
        CBDesc.Size = sizeof(CloudParams);
        CBDesc.Usage = USAGE_DYNAMIC;
        CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
        CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        pDevice->CreateBuffer(CBDesc, nullptr, &m_pCloudParamsCB);
    }

    // Create PSO and shaders
    GraphicsPipelineStateCreateInfo PSOCreateInfo;
    PipelineStateDesc&              PSODesc = PSOCreateInfo.PSODesc;

    PSODesc.Name = "Cloud volume renderer PSO";
    PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

    // Define shader resource variable types
    PipelineResourceLayoutDescX ResourceLayout;
    ResourceLayout
        .SetDefaultVariableType(SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE)
        .AddVariable(SHADER_TYPE_PIXEL, "g_DepthTexture", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
        .AddVariable(SHADER_TYPE_VERTEX, "CameraAttribs", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
        .AddVariable(SHADER_TYPE_PIXEL, "CameraAttribs", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
        .AddVariable(SHADER_TYPE_PIXEL, "CloudParams", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE)
        .AddVariable(SHADER_TYPE_PIXEL, "g_HighFreqNoiseTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE)
        .AddVariable(SHADER_TYPE_PIXEL, "g_LowFreqNoiseTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE);
        
    // Create sampler descriptions for immutable samplers
    SamplerDesc DepthSamplerDesc;
    DepthSamplerDesc.MinFilter = FILTER_TYPE_LINEAR;
    DepthSamplerDesc.MagFilter = FILTER_TYPE_LINEAR;
    DepthSamplerDesc.MipFilter = FILTER_TYPE_LINEAR;
    DepthSamplerDesc.AddressU = TEXTURE_ADDRESS_CLAMP;
    DepthSamplerDesc.AddressV = TEXTURE_ADDRESS_CLAMP;
    DepthSamplerDesc.AddressW = TEXTURE_ADDRESS_CLAMP;
    
    // Add sampler descriptions for the noise textures
    SamplerDesc NoiseSamplerDesc;
    NoiseSamplerDesc.MinFilter = FILTER_TYPE_LINEAR;
    NoiseSamplerDesc.MagFilter = FILTER_TYPE_LINEAR;
    NoiseSamplerDesc.MipFilter = FILTER_TYPE_LINEAR;
    NoiseSamplerDesc.AddressU = TEXTURE_ADDRESS_WRAP;
    NoiseSamplerDesc.AddressV = TEXTURE_ADDRESS_WRAP;
    NoiseSamplerDesc.AddressW = TEXTURE_ADDRESS_WRAP;
    
    ResourceLayout
        .AddImmutableSampler(SHADER_TYPE_PIXEL, "g_DepthSampler", DepthSamplerDesc)
        .AddImmutableSampler(SHADER_TYPE_PIXEL, "g_NoiseSampler", NoiseSamplerDesc);

    PSOCreateInfo.PSODesc.ResourceLayout = ResourceLayout;

    // Shader creation
    ShaderCreateInfo ShaderCI;
    ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
    ShaderCI.CompileFlags = SHADER_COMPILE_FLAG_PACK_MATRIX_ROW_MAJOR;
    ShaderCI.pShaderSourceStreamFactory = pCompoundSourceFactory;
    // Vertex shader
    ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
    ShaderCI.EntryPoint = "main";
    ShaderCI.Desc.Name = "Cloud Volume VS";
    ShaderCI.Source = R"(
        struct VSInput
        {
            float2 pos : ATTRIB0;
            float2 uv  : ATTRIB1;
        };

        struct PSInput
        {
            float4 pos : SV_POSITION;
            float2 uv  : TEX_COORD;
            float3 rayDir : RAY_DIR;
        };

        #include "BasicStructures.fxh"

        cbuffer CameraAttribs
        {
            CameraAttribs g_Camera;
        };

        PSInput main(VSInput input)
        {
            PSInput output;
            output.pos = float4(input.pos, 0.0, 1.0);
            output.uv = input.uv;
            
            // Calculate ray direction in world space
            float4 rayStart = float4(input.pos, -1.0, 1.0);
            float4 rayEnd = float4(input.pos, 1.0, 1.0);
            
            float4 rayStartWorld = mul(g_Camera.mViewProjInv, rayStart);
            float4 rayEndWorld = mul(g_Camera.mViewProjInv, rayEnd);
            
            rayStartWorld /= rayStartWorld.w;
            rayEndWorld /= rayEndWorld.w;
            
            output.rayDir = normalize(rayEndWorld.xyz - rayStartWorld.xyz);
            
            return output;
        }
    )";

    RefCntAutoPtr<IShader> pVS;
    pDevice->CreateShader(ShaderCI, &pVS);

    // Pixel shader with simple box rendering
    ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
    ShaderCI.CompileFlags = SHADER_COMPILE_FLAG_PACK_MATRIX_ROW_MAJOR;
    ShaderCI.EntryPoint = "main";
    ShaderCI.Desc.Name = "Cloud Volume PS";
    ShaderCI.Source = R"(
        struct PSInput
        {
            float4 pos : SV_POSITION;
            float2 uv  : TEX_COORD;
            float3 rayDir : RAY_DIR;
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

        #include "BasicStructures.fxh"

        cbuffer CameraAttribs
        {
            CameraAttribs g_Camera;
        };

        cbuffer CloudParams
        {
            float4 g_CloudBoxMin;       // Bottom of cloud box
            float4 g_CloudBoxMax;       // Top of cloud box
            float4 g_CloudColor;        // Color of the cloud box
            float  g_CloudOpacity;      // Opacity of the cloud box
        };

        Texture2D g_DepthTexture;
        SamplerState g_DepthSampler;
        
        // 3D noise textures for cloud detail
        Texture3D g_HighFreqNoiseTexture;
        Texture3D g_LowFreqNoiseTexture;
        SamplerState g_NoiseSampler;

        // Ray-box intersection function
        bool IntersectBox(float3 rayOrigin, float3 rayDir, float3 boxMin, float3 boxMax, out float tNear, out float tFar)
        {
            float3 invDir = 1.0 / rayDir;
            float3 tMin = (boxMin - rayOrigin) * invDir;
            float3 tMax = (boxMax - rayOrigin) * invDir;
            
            float3 t1 = min(tMin, tMax);
            float3 t2 = max(tMin, tMax);
            
            tNear = max(max(t1.x, t1.y), t1.z);
            tFar = min(min(t2.x, t2.y), t2.z);
            
            return tFar > tNear && tFar > 0.0;
        }
        
        // Sample cloud density using the 3D noise textures
        float SampleCloudDensity(float3 pos)
        {
            // Normalize position within cloud box
            float3 normalizedPos = (pos - g_CloudBoxMin.xyz) / (g_CloudBoxMax.xyz - g_CloudBoxMin.xyz);
            
            // Sample low-frequency base shape
            float baseDensity = g_LowFreqNoiseTexture.Sample(g_NoiseSampler, normalizedPos).r;
            
            // Apply height falloff (more density in the middle, less at top/bottom)
            float heightGradient = 1.0 - 2.0 * abs(normalizedPos.y - 0.5);
            heightGradient = saturate(heightGradient * 3.0); // Sharpen the gradient
            
            // Early exit if no base density
            if (baseDensity < 0.05)
                return 0;
                
            // Sample high-frequency detail
            float3 highFreqUV = normalizedPos * 4.0; // Scale for more variation
            float detailNoise = g_HighFreqNoiseTexture.Sample(g_NoiseSampler, highFreqUV).r;
            
            // Combine for final density
            float density = saturate(baseDensity * heightGradient);
            density = saturate(density - (1.0 - detailNoise) * 0.3); // Apply detail noise as erosion
            
            return density * g_CloudOpacity * 2.0; // Scale by opacity param
        }

        PSOutput main(PSInput input)
        {
            PSOutput output;
            // Get world-space position of the current pixel from depth buffer
            float depth = g_DepthTexture.Sample(g_DepthSampler, input.uv).r;
            float4 clipPos = float4(input.uv * 2.0 - 1.0, depth, 1.0);
            
            float4 worldPos = mul(g_Camera.mViewProjInv, clipPos);
            worldPos /= worldPos.w;
            
            float3 rayOrigin = g_Camera.f4Position.xyz;
            float3 rayDir = normalize(input.rayDir);
            
            // Find intersection with cloud box
            float tNear, tFar;
            bool intersect = IntersectBox(rayOrigin, rayDir, g_CloudBoxMin.xyz, g_CloudBoxMax.xyz, tNear, tFar);
            
            // Early exit if no intersection
            if (!intersect || tFar <= 0 || tNear >= length(worldPos.xyz - rayOrigin))
            {
                discard; // Early exit with discard instead of returning empty output
            }
            
            // Ensure tNear is at least 0 (camera inside cloud)
            tNear = max(0, tNear);
            
            // Cloud raymarching parameters
            const int numSteps = 16;
            float stepSize = (tFar - tNear) / float(numSteps);
            float totalDensity = 0.0;
            float transmittance = 1.0;
            float3 lightAccumulation = float3(0, 0, 0);
            
            // Simple directional light from above
            float3 lightDir = normalize(float3(0.5, 1.0, 0.3));
            
            // Raymarch through cloud volume
            for (int i = 0; i < numSteps; i++)
            {
                // Current sample position
                float t = tNear + stepSize * (float(i) + 0.5); // Sample at center of step
                float3 samplePos = rayOrigin + rayDir * t;
                
                // Sample cloud density at this position
                float density = SampleCloudDensity(samplePos);
                
                if (density > 0.0)
                {
                    // Apply Beer's law for light absorption
                    float absorption = exp(-density * stepSize);
                    transmittance *= absorption;
                    
                    // Add light contribution (simple ambient + directional)
                    float3 ambientLight = g_CloudColor.rgb * 0.2;
                    
                    // Simple light scattering approximation
                    float scattering = pow(max(0.0, dot(rayDir, lightDir) * 0.5 + 0.5), 8.0) * 0.5 + 0.5;
                    float3 directLight = g_CloudColor.rgb * scattering;
                    
                    float3 lightContrib = (ambientLight + directLight) * density * stepSize;
                    lightAccumulation += lightContrib * transmittance;
                    
                    // Early exit if cloud becomes opaque
                    if (transmittance < 0.01)
                        break;
                }
            }
            
            // Final cloud color and opacity
            float3 cloudColor = lightAccumulation;
            float cloudAlpha = 1.0 - transmittance;
            
            // Output to all PBR G-buffer targets
            output.Color        = float4(cloudColor, cloudAlpha);
            output.Normal       = float4(0.0, 1.0, 0.0, 1.0); // Upward normal
            output.BaseColor    = float4(g_CloudColor.rgb, cloudAlpha);
            output.MaterialData = float4(0.0, 0.0, 0.0, cloudAlpha); // No roughness/metallic
            output.MotionVec    = float4(0.0, 0.0, 0.0, 1.0); // No motion
            output.SpecularIBL  = float4(0.0, 0.0, 0.0, cloudAlpha); // No IBL
            return output;
        }
    )";

    RefCntAutoPtr<IShader> pPS;
    pDevice->CreateShader(ShaderCI, &pPS);

    // Define vertex layout
    LayoutElement LayoutElems[] =
    {
        LayoutElement{0, 0, 2, VT_FLOAT32, False},  // pos
        LayoutElement{1, 0, 2, VT_FLOAT32, False}   // uv
    };
    
    PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
    PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

    // Set primitive topology
    PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Set render target and depth formats
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 6;
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = TEX_FORMAT_RGBA16_FLOAT;   // Color
    PSOCreateInfo.GraphicsPipeline.RTVFormats[1] = TEX_FORMAT_RGBA16_FLOAT;   // Normal
    PSOCreateInfo.GraphicsPipeline.RTVFormats[2] = TEX_FORMAT_RGBA8_UNORM;    // BaseColor
    PSOCreateInfo.GraphicsPipeline.RTVFormats[3] = TEX_FORMAT_RG8_UNORM;      // MaterialData
    PSOCreateInfo.GraphicsPipeline.RTVFormats[4] = TEX_FORMAT_RG16_FLOAT;     // MotionVec
    PSOCreateInfo.GraphicsPipeline.RTVFormats[5] = TEX_FORMAT_RGBA16_FLOAT;   // SpecularIBL;
    PSOCreateInfo.GraphicsPipeline.DSVFormat = TEX_FORMAT_UNKNOWN;       // Will be overridden in Render()

    // Set blend state
    for (Uint32 i = 0; i < PSOCreateInfo.GraphicsPipeline.NumRenderTargets; ++i)
    {
        PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[i].BlendEnable = True;
        PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[i].SrcBlend = BLEND_FACTOR_SRC_ALPHA;
        PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[i].DestBlend = BLEND_FACTOR_INV_SRC_ALPHA;
        PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[i].BlendOp = BLEND_OPERATION_ADD;
        PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[i].SrcBlendAlpha = BLEND_FACTOR_ONE;
        PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[i].DestBlendAlpha = BLEND_FACTOR_INV_SRC_ALPHA;
        PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[i].BlendOpAlpha = BLEND_OPERATION_ADD;
        PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[i].RenderTargetWriteMask = COLOR_MASK_ALL;
    }

    // Set depth-stencil state (no depth testing for clouds)
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = False;
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthFunc = COMPARISON_FUNC_LESS_EQUAL;

    // Set rasterizer state
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
    PSOCreateInfo.GraphicsPipeline.RasterizerDesc.FillMode = FILL_MODE_SOLID;

    // Assign shaders
    PSOCreateInfo.pVS = pVS;
    PSOCreateInfo.pPS = pPS;

    // Create the pipeline state
    pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pRenderCloudsPSO);

    // Create shader resource binding
    m_pRenderCloudsPSO->CreateShaderResourceBinding(&m_pRenderCloudsSRB, true);

    // Bind static resources
    m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "CloudParams")->Set(m_pCloudParamsCB);
}

void CloudVolumeRenderer::Render(IDeviceContext* pContext,
                                const HLSL::CameraAttribs& CamAttribs,
                                const HLSL::LightAttribs& LightAttrs,
                                const float4& LightColor,
                                ITextureView* pDepthBufferSRV,
                                TEXTURE_FORMAT RTVFormat,
                                TEXTURE_FORMAT DSVFormat)
{
    // Skip rendering if not initialized
    if (!m_pRenderCloudsPSO)
        return;

    // Update cloud parameters constant buffer
    {
        MapHelper<CloudParams> CloudParams(pContext, m_pCloudParamsCB, MAP_WRITE, MAP_FLAG_DISCARD);
        *CloudParams = m_CloudParams;
    }

    // Set pipeline state
    pContext->SetPipelineState(m_pRenderCloudsPSO);
    
    // Create and bind camera attributes buffer
    RefCntAutoPtr<IBuffer> pCameraAttribsCB;
    BufferDesc CBDesc;
    CBDesc.Name = "Camera Attributes buffer";
    CBDesc.Size = sizeof(HLSL::CameraAttribs);
    CBDesc.Usage = USAGE_DYNAMIC;
    CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
    CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
    m_pDevice->CreateBuffer(CBDesc, nullptr, &pCameraAttribsCB);
    
    // Fill the camera data from the HLSL::CameraAttribs structure
    {
        MapHelper<HLSL::CameraAttribs> MappedCamAttribs(pContext, pCameraAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD);
        *MappedCamAttribs = CamAttribs;
    }
    
    // Bind the camera attributes and depth texture to the existing SRB
    m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_VERTEX, "CameraAttribs")->Set(pCameraAttribsCB);
    m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "CameraAttribs")->Set(pCameraAttribsCB);

    auto srv = m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_DepthTexture");
    if (srv)
        srv->Set(pDepthBufferSRV);

    auto highFreqVar = m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_HighFreqNoiseTexture");
    if (highFreqVar)
        highFreqVar->Set(m_pHighFreqNoiseSRV);
        
    auto lowFreqVar = m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_LowFreqNoiseTexture");
    if (lowFreqVar)
        lowFreqVar->Set(m_pLowFreqNoiseSRV);

    // Commit shader resources
    pContext->CommitShaderResources(m_pRenderCloudsSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    
    // Set vertex and index buffers
    const Uint64 Offsets = 0;
    IBuffer* pBuffs[] = {m_pVertexBuffer};
    pContext->SetVertexBuffers(0, 1, pBuffs, &Offsets, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
    pContext->SetIndexBuffer(m_pIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    
    // Draw the full-screen quad
    DrawIndexedAttribs DrawAttrs;
    DrawAttrs.IndexType  = VT_UINT32;
    DrawAttrs.NumIndices = 6;
    DrawAttrs.Flags      = DRAW_FLAG_VERIFY_ALL;
    pContext->DrawIndexed(DrawAttrs);
}

void CloudVolumeRenderer::SetupTerrainParameters(const BoundBox& terrainBounds)
{
    // Set the cloud box boundaries
    m_CloudParams.CloudBoxMin = float4(terrainBounds.Min.x, 35.0f, terrainBounds.Min.z, 1.0f);
    m_CloudParams.CloudBoxMax = float4(terrainBounds.Max.x, 100.0f, terrainBounds.Max.z, 1.0f);
    
    // Set default cloud parameters
    m_CloudParams.CloudColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    m_CloudParams.CloudOpacity = 0.3f;
}

void CloudVolumeRenderer::LoadNoiseTextures()
{
    std::string highFreqTexturePath = "HighFrequency3DTexture.png";
    std::string lowFreqTexturePath = "LowFrequency3DTexture.png";

    if (!m_pDevice)
        return;
    
    // Load high frequency noise texture (32x1024 = 32x32x32)
    {
        // Load PNG using lodepng
        std::vector<unsigned char> imageData;
        unsigned width, height;
        unsigned error = lodepng::decode(imageData, width, height, highFreqTexturePath.c_str());
        
        if (error)
        {
            // Handle loading error
            return;
        }
        
        if (width != 32 || height != 1024) 
        {
            // Wrong dimensions
            return;
        }
        
        // Create 3D texture
        TextureDesc TexDesc;
        TexDesc.Name = "High frequency 3D noise texture";
        TexDesc.Type = RESOURCE_DIM_TEX_3D;
        TexDesc.Width = 32;
        TexDesc.Height = 32;
        TexDesc.Depth = 32;
        TexDesc.MipLevels = 1;
        TexDesc.Format = TEX_FORMAT_RGBA8_UNORM;
        TexDesc.Usage = USAGE_IMMUTABLE;
        TexDesc.BindFlags = BIND_SHADER_RESOURCE;
        
        // Prepare texture data
        TextureSubResData Level0Data;
        Level0Data.pData = imageData.data();
        Level0Data.Stride = 32 * 4; // 4 bytes per pixel (RGBA)
        Level0Data.DepthStride = 32 * 32 * 4; // Stride for each depth slice
        
        TextureData InitData;
        InitData.NumSubresources = 1;
        InitData.pSubResources = &Level0Data;
        
        m_pDevice->CreateTexture(TexDesc, &InitData, &m_pHighFreqNoiseTexture);
        m_pHighFreqNoiseSRV = m_pHighFreqNoiseTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    }
    
    // Load low frequency noise texture (128x15384 = 128x128x128)
    {
        // Load PNG using lodepng
        std::vector<unsigned char> imageData;
        unsigned width, height;
        unsigned error = lodepng::decode(imageData, width, height, lowFreqTexturePath.c_str());
        
        if (error)
        {
            // Handle loading error
            return;
        }
        
        if (width != 128 || height != 16384) 
        {
            // Wrong dimensions
            return;
        }
        
        // Create 3D texture
        TextureDesc TexDesc;
        TexDesc.Name = "Low frequency 3D noise texture";
        TexDesc.Type = RESOURCE_DIM_TEX_3D;
        TexDesc.Width = 128;
        TexDesc.Height = 128;
        TexDesc.Depth = 128;
        TexDesc.MipLevels = 1;
        TexDesc.Format = TEX_FORMAT_RGBA8_UNORM;
        TexDesc.Usage = USAGE_IMMUTABLE;
        TexDesc.BindFlags = BIND_SHADER_RESOURCE;
        
        // Prepare texture data
        TextureSubResData Level0Data;
        Level0Data.pData = imageData.data();
        Level0Data.Stride = 128 * 4; // 4 bytes per pixel (RGBA)
        Level0Data.DepthStride = 128 * 128 * 4; // Stride for each depth slice
        
        TextureData InitData;
        InitData.NumSubresources = 1;
        InitData.pSubResources = &Level0Data;
        
        m_pDevice->CreateTexture(TexDesc, &InitData, &m_pLowFreqNoiseTexture);
        m_pLowFreqNoiseSRV = m_pLowFreqNoiseTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    }
    
    // If we already have a shader resource binding, update it with the new textures
    if (m_pRenderCloudsSRB)
    {
        auto highFreqVar = m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_HighFreqNoiseTexture");
        if (highFreqVar)
            highFreqVar->Set(m_pHighFreqNoiseSRV);
            
        auto lowFreqVar = m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_LowFreqNoiseTexture");
        if (lowFreqVar)
            lowFreqVar->Set(m_pLowFreqNoiseSRV);
    }
}

} // namespace Diligent