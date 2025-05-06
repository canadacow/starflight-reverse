#include "CloudVolumeRenderer.hpp"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"
#include "Graphics/GraphicsTools/interface/GraphicsUtilities.h"
#include "Graphics/GraphicsEngine/interface/GraphicsTypesX.hpp"
#include "Graphics/GraphicsTools/interface/ShaderSourceFactoryUtils.hpp"
#include "Utilities/interface/DiligentFXShaderSourceStreamFactory.hpp"

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
    
    // New volumetric cloud parameters
    m_CloudParams.ScatteringParams = float4(0.04f, 0.02f, 0.2f, 1.0f);  // Scattering, absorption, phase g, density multiplier
    m_CloudParams.NoiseParams = float4(0.0003f, 0.001f, 0.3f, 0.2f);    // Base scale, detail scale, detail strength, curl strength
    m_CloudParams.ShapeParams = float4(0.65f, 0.15f, 0.1f, 0.8f);       // Coverage, base density, anvil bias, height gradient scale
    m_CloudParams.AnimationParams = float4(1.0f, 0.0f, 0.2f, 0.0f);     // Wind direction x,y,z and time
}

CloudVolumeRenderer::~CloudVolumeRenderer()
{
}

void CloudVolumeRenderer::Update(double elapsedTime)
{
    // Update time for cloud animation
    m_Time = elapsedTime;
    
    // Update animation parameters
    m_CloudParams.AnimationParams.w = m_Time * 0.05f; // Control animation speed
}

void CloudVolumeRenderer::Initialize(IRenderDevice* pDevice, IDeviceContext* pImmediateContext)
{
    // Store device reference
    m_pDevice = pDevice;

    // Create noise textures needed for cloud rendering
    CreateNoiseTextures(pDevice);

    RefCntAutoPtr<IShaderSourceInputStreamFactory> pMemorySourceFactory = CreateMemoryShaderSourceFactory({});      
    auto pCompoundSourceFactory = SFCreateCompoundShaderSourceFactory(pDevice, pMemorySourceFactory);

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
        .AddVariable(SHADER_TYPE_PIXEL, "LightAttribs", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
        .AddVariable(SHADER_TYPE_PIXEL, "CloudParams", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE)
        .AddVariable(SHADER_TYPE_PIXEL, "g_PerlinWorleyNoise", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
        .AddVariable(SHADER_TYPE_PIXEL, "g_WorleyNoise", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
        .AddVariable(SHADER_TYPE_PIXEL, "g_WeatherMap", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC);
        
    // Create sampler descriptions for immutable samplers
    SamplerDesc DepthSamplerDesc;
    DepthSamplerDesc.MinFilter = FILTER_TYPE_LINEAR;
    DepthSamplerDesc.MagFilter = FILTER_TYPE_LINEAR;
    DepthSamplerDesc.MipFilter = FILTER_TYPE_LINEAR;
    DepthSamplerDesc.AddressU = TEXTURE_ADDRESS_CLAMP;
    DepthSamplerDesc.AddressV = TEXTURE_ADDRESS_CLAMP;
    DepthSamplerDesc.AddressW = TEXTURE_ADDRESS_CLAMP;
    
    SamplerDesc NoiseTextureSamplerDesc;
    NoiseTextureSamplerDesc.MinFilter = FILTER_TYPE_LINEAR;
    NoiseTextureSamplerDesc.MagFilter = FILTER_TYPE_LINEAR;
    NoiseTextureSamplerDesc.MipFilter = FILTER_TYPE_LINEAR;
    NoiseTextureSamplerDesc.AddressU = TEXTURE_ADDRESS_MIRROR;
    NoiseTextureSamplerDesc.AddressV = TEXTURE_ADDRESS_MIRROR;
    NoiseTextureSamplerDesc.AddressW = TEXTURE_ADDRESS_MIRROR;
    
    ResourceLayout
        .AddImmutableSampler(SHADER_TYPE_PIXEL, "g_DepthSampler", DepthSamplerDesc)
        .AddImmutableSampler(SHADER_TYPE_PIXEL, "g_NoiseSampler", NoiseTextureSamplerDesc)
        .AddImmutableSampler(SHADER_TYPE_PIXEL, "g_WeatherSampler", NoiseTextureSamplerDesc);

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

    // Pixel shader with volumetric cloud rendering
    ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
    ShaderCI.CompileFlags = SHADER_COMPILE_FLAG_PACK_MATRIX_ROW_MAJOR;
    ShaderCI.EntryPoint = "main";
    ShaderCI.Desc.Name = "Volumetric Cloud PS";
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
        
        cbuffer LightAttribs
        {
            LightAttribs g_Light;
        };

        struct CloudParamsType
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
        
        cbuffer CloudParams
        {
            CloudParamsType g_CloudParams;
        };

        Texture2D g_DepthTexture;
        SamplerState g_DepthSampler;
        
        Texture3D g_PerlinWorleyNoise;
        Texture3D g_WorleyNoise;
        Texture2D g_WeatherMap;
        SamplerState g_NoiseSampler;
        SamplerState g_WeatherSampler;

        // Helper function to remap values from one range to another - using lerp
        float remap(float value, float oldMin, float oldMax, float newMin, float newMax)
        {
            float t = saturate((value - oldMin) / (oldMax - oldMin));
            return lerp(newMin, newMax, t);
        }

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
        
        // Henyey-Greenstein phase function
        float HenyeyGreenstein(float cosAngle, float g)
        {
            float g2 = g * g;
            return (1.0 - g2) / (4.0 * 3.14159265 * pow(1.0 + g2 - 2.0 * g * cosAngle, 1.5));
        }
        
        // Sample weather map for cloud coverage and type information
        float4 SampleWeatherMap(float3 worldPos)
        {
            // Project world position onto xz plane for weather map
            float2 weatherCoord = (worldPos.xz - g_CloudParams.CloudBoxMin.xz) / (g_CloudParams.CloudBoxMax.xz - g_CloudParams.CloudBoxMin.xz);
            
            // Ensure coordinates are properly clamped to avoid sampling outside the texture
            weatherCoord = saturate(weatherCoord);
            
            return g_WeatherMap.Sample(g_WeatherSampler, weatherCoord);
        }
        
        // Compute cloud shape from noise
        float SampleCloudDensity(float3 pos, float heightPercent)
        {
            // Apply animation
            float3 animatedPos = pos + g_CloudParams.AnimationParams.xyz * g_CloudParams.AnimationParams.w;
            
            // Compute base shape noise
            float3 baseNoiseUV = animatedPos * g_CloudParams.NoiseParams.x;
            float4 baseNoise = g_PerlinWorleyNoise.Sample(g_NoiseSampler, baseNoiseUV);
            
            // Compute detail noise
            float3 detailNoiseUV = animatedPos * g_CloudParams.NoiseParams.y;
            float4 detailNoise = g_WorleyNoise.Sample(g_NoiseSampler, detailNoiseUV);
            
            // Sample weather map
            float4 weatherData = SampleWeatherMap(pos);
            float coverage = weatherData.r * g_CloudParams.ShapeParams.x;
            float cloudType = weatherData.g;
            
            // Compute height gradient
            float heightGradient = saturate((heightPercent - 0.07) / 0.45);
            float densityHeightGradient = exp(-heightPercent * g_CloudParams.ShapeParams.w);
            
            // Compute anvil shape (more clouds at top)
            float anvilStrength = saturate(pow(heightPercent, 4.0) * g_CloudParams.ShapeParams.z * cloudType);
            
            // Combine base shape
            float baseShape = baseNoise.r;
            float baseShapeFBM = baseNoise.g * 0.625 + baseNoise.b * 0.25 + baseNoise.a * 0.125;
            
            // Compute density
            float baseCloudDensity = saturate(remap(baseShape, baseShapeFBM - 1.0, 1.0, 0.0, 1.0));
            baseCloudDensity = densityHeightGradient * saturate(remap(baseCloudDensity, 1.0 - coverage, 1.0, 0.0, 1.0));
            
            // Apply erosion from detail noise
            float detailFBM = detailNoise.r * 0.625 + detailNoise.g * 0.25 + detailNoise.b * 0.125;
            float detailErodeNoise = saturate(remap(detailFBM, 0.0, 1.0, 0.0, 1.0));
            
            // Erode the base density with detail noise
            float finalDensity = saturate(remap(baseCloudDensity, detailErodeNoise * g_CloudParams.NoiseParams.z, 1.0, 0.0, 1.0));
            
            // Apply anvil effect
            finalDensity = max(finalDensity, anvilStrength);
            
            // Apply global density multiplier
            return finalDensity * g_CloudParams.ScatteringParams.w * g_CloudParams.CloudOpacity;
        }
        
        // Sample along the ray with ray marching to compute cloud color and transparency
        float4 SampleClouds(float3 rayOrigin, float3 rayDir, float tNear, float tFar, float sceneDepthT)
        {
            // Limit ray length to scene depth or box distance
            tFar = min(tFar, sceneDepthT);
            if (tNear >= tFar) return float4(0.0, 0.0, 0.0, 0.0);
            
            // Ray marching parameters
            const int sampleCount = 64;
            float stepSize = (tFar - tNear) / float(sampleCount);
            
            // Initialize for ray marching
            float3 lightDir = normalize(g_Light.f4Direction.xyz);
            
            // Start ray marching
            float transmittance = 1.0;
            float3 totalLight = float3(0.0, 0.0, 0.0);
            float t = tNear;
            
            for (int i = 0; i < sampleCount; i++)
            {
                // Skip if ray has passed scene depth or we've accumulated enough opacity
                if (t > sceneDepthT || transmittance < 0.01)
                    break;
                
                // Position along ray
                float3 pos = rayOrigin + rayDir * t;
                
                // Compute height fraction within cloud layer
                float heightFraction = (pos.y - g_CloudParams.CloudBoxMin.y) / (g_CloudParams.CloudBoxMax.y - g_CloudParams.CloudBoxMin.y);
                
                // Sample cloud density at this position
                float density = SampleCloudDensity(pos, heightFraction);
                
                // Skip empty space
                if (density > 0.0)
                {
                    // Light attenuation (simplified)
                    float lightDist = 0.0;
                    const int lightSampleCount = 6;
                    float lightStepSize = 40.0 / float(lightSampleCount);
                    float3 lightSamplePos = pos;
                    float lightTransmittance = 1.0;
                    
                    // Sample light
                    for (int j = 0; j < lightSampleCount; j++)
                    {
                        lightSamplePos += lightDir * lightStepSize;
                        float heightFrac = (lightSamplePos.y - g_CloudParams.CloudBoxMin.y) / (g_CloudParams.CloudBoxMax.y - g_CloudParams.CloudBoxMin.y);
                        if (heightFrac < 0.0 || heightFrac > 1.0) break;
                        
                        float lightDensity = SampleCloudDensity(lightSamplePos, heightFrac) * 0.5;
                        float extinction = g_CloudParams.ScatteringParams.x + g_CloudParams.ScatteringParams.y;
                        lightTransmittance *= exp(-extinction * lightDensity * lightStepSize);
                        
                        if (lightTransmittance < 0.01) break;
                    }
                    
                    // Calculate in-scattering
                    float cosAngle = dot(rayDir, lightDir);
                    float phase = HenyeyGreenstein(cosAngle, g_CloudParams.ScatteringParams.z);
                    
                    // Ambient term for multi-scattering approximation
                    float ambient = 0.5 + 0.5 * heightFraction;
                    
                    // Calculate total light contribution
                    float scatteredLight = lightTransmittance * phase + ambient * 0.2;
                    float extinction = g_CloudParams.ScatteringParams.x + g_CloudParams.ScatteringParams.y;
                    float dTrans = exp(-extinction * density * stepSize);
                    
                    // Beer's law
                    float3 cloudColor = lerp(g_CloudParams.CloudColor.rgb * 0.15, g_CloudParams.CloudColor.rgb, heightFraction);
                    float3 S = cloudColor * scatteredLight * (1.0 - dTrans) * transmittance;
                    
                    // Accumulate light and update transmittance
                    totalLight += S;
                    transmittance *= dTrans;
                }
                
                // Step along ray
                t += stepSize;
            }
            
            // Final color with transmittance
            return float4(totalLight, 1.0 - transmittance);
        }

        PSOutput main(PSInput input)
        {
            PSOutput output;
            
            // Get world-space ray origin
            float3 rayOrigin = g_Camera.f4Position.xyz;
            float3 rayDir = normalize(input.rayDir);
            
            // Get the scene depth from depth buffer
            float depth = g_DepthTexture.Sample(g_DepthSampler, input.uv).r;
            float4 clipPos = float4(input.uv * 2.0 - 1.0, depth, 1.0);
            float4 worldPos = mul(g_Camera.mViewProjInv, clipPos);
            worldPos /= worldPos.w;
            
            // Calculate distance to scene geometry
            float sceneDepthT = length(worldPos.xyz - rayOrigin);
            
            // Find intersection with cloud box
            float tNear, tFar;
            bool intersect = IntersectBox(rayOrigin, rayDir, g_CloudParams.CloudBoxMin.xyz, g_CloudParams.CloudBoxMax.xyz, tNear, tFar);
            
            // Early exit if no intersection
            if (!intersect || tFar <= 0.0)
            {
                discard;
            }
            
            // Start ray just in front of camera if we're inside the volume
            tNear = max(0.0, tNear);
            
            // Sample clouds with ray marching
            float4 cloudColor = SampleClouds(rayOrigin, rayDir, tNear, tFar, sceneDepthT);
            
            // Early exit if totally transparent
            if (cloudColor.a < 0.001)
            {
                discard;
            }
            
            // Output to all PBR G-buffer targets
            output.Color        = float4(cloudColor.rgb, cloudColor.a);
            output.Normal       = float4(0.0, 1.0, 0.0, cloudColor.a); // Simplified normal
            output.BaseColor    = float4(cloudColor.rgb, cloudColor.a);
            output.MaterialData = float4(0.0, 0.0, 0.0, cloudColor.a); // No roughness/metallic
            output.MotionVec    = float4(0.0, 0.0, 0.0, cloudColor.a); // No motion
            output.SpecularIBL  = float4(0.0, 0.0, 0.0, cloudColor.a); // No IBL
            
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
    
    // Create and bind light attributes buffer
    RefCntAutoPtr<IBuffer> pLightAttribsCB;
    CBDesc.Name = "Light Attributes buffer";
    CBDesc.Size = sizeof(HLSL::LightAttribs);
    m_pDevice->CreateBuffer(CBDesc, nullptr, &pLightAttribsCB);
    
    // Fill the camera data from the HLSL::CameraAttribs structure
    {
        MapHelper<HLSL::CameraAttribs> MappedCamAttribs(pContext, pCameraAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD);
        *MappedCamAttribs = CamAttribs;
    }
    
    // Fill the light data from the HLSL::LightAttribs structure
    {
        MapHelper<HLSL::LightAttribs> MappedLightAttribs(pContext, pLightAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD);
        *MappedLightAttribs = LightAttrs;
    }
    
    // Bind the camera attributes and depth texture to the existing SRB
    m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_VERTEX, "CameraAttribs")->Set(pCameraAttribsCB);
    m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "CameraAttribs")->Set(pCameraAttribsCB);
    m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "LightAttribs")->Set(pLightAttribsCB);

    // Bind the depth texture
    auto depthSrv = m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_DepthTexture");
    if (depthSrv)
    {
        depthSrv->Set(pDepthBufferSRV);
    }
    
    // Bind the noise textures
    auto perlinWorleySrv = m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_PerlinWorleyNoise");
    if (perlinWorleySrv)
    {
        perlinWorleySrv->Set(m_pPerlinWorleyNoiseSRV);
    }
    
    auto worleySrv = m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_WorleyNoise");
    if (worleySrv)
    {
        worleySrv->Set(m_pWorleyNoiseSRV);
    }
    
    auto weatherSrv = m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_WeatherMap");
    if (weatherSrv)
    {
        weatherSrv->Set(m_pWeatherMapSRV);
    }
    
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
    // Set the cloud box boundaries - extend clouds beyond terrain with some buffer
    float cloudsMinHeight = 35.0f;  // Lower bound of clouds
    float cloudsMaxHeight = 75.0f;  // Upper bound of clouds
    float horizontalExtension = 50.0f; // Extend clouds beyond terrain boundaries
    
    m_CloudParams.CloudBoxMin = float4(
        terrainBounds.Min.x - horizontalExtension, 
        cloudsMinHeight, 
        terrainBounds.Min.z - horizontalExtension, 
        1.0f
    );
    
    m_CloudParams.CloudBoxMax = float4(
        terrainBounds.Max.x + horizontalExtension, 
        cloudsMaxHeight, 
        terrainBounds.Max.z + horizontalExtension, 
        1.0f
    );
    
    // Set default cloud parameters for a realistic look
    m_CloudParams.CloudColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    m_CloudParams.ScatteringParams = float4(0.04f, 0.02f, 0.2f, 1.0f);  // Scattering, absorption, phase g, density multiplier
    m_CloudParams.NoiseParams = float4(0.0003f, 0.001f, 0.3f, 0.2f);    // Base scale, detail scale, detail strength, curl strength
    m_CloudParams.ShapeParams = float4(0.65f, 0.15f, 0.1f, 0.8f);       // Coverage, base density, anvil bias, height gradient scale
    m_CloudParams.AnimationParams = float4(1.0f, 0.0f, 0.2f, m_Time);   // Wind direction and current time
    m_CloudParams.CloudOpacity = 0.7f;
}

void CloudVolumeRenderer::CreateNoiseTextures(IRenderDevice* pDevice)
{
    // For now, we'll create simple procedural noise textures
    // In a real implementation, you would likely load pre-baked noise textures
    
    // Create the 3D Perlin-Worley noise texture (128x128x128)
    {
        TextureDesc TexDesc;
        TexDesc.Name = "3D Perlin-Worley Noise";
        TexDesc.Type = RESOURCE_DIM_TEX_3D;
        TexDesc.Width = 128;
        TexDesc.Height = 128;
        TexDesc.Depth = 128;
        TexDesc.Format = TEX_FORMAT_RGBA8_UNORM;
        TexDesc.BindFlags = BIND_SHADER_RESOURCE;
        TexDesc.Usage = USAGE_IMMUTABLE;
        
        // Generate simple procedural 3D noise
        // In a real implementation, you would load a pre-computed noise texture
        std::vector<Uint8> NoiseData(TexDesc.Width * TexDesc.Height * TexDesc.Depth * 4);
        
        // Simple noise generation for demonstration
        srand(42); // Fixed seed for reproducibility
        for (Uint32 z = 0; z < TexDesc.Depth; ++z)
        {
            for (Uint32 y = 0; y < TexDesc.Height; ++y)
            {
                for (Uint32 x = 0; x < TexDesc.Width; ++x)
                {
                    Uint32 idx = (z * TexDesc.Height * TexDesc.Width + y * TexDesc.Width + x) * 4;
                    
                    // Pseudo-random noise values
                    NoiseData[idx + 0] = rand() % 256;     // Perlin noise (R)
                    NoiseData[idx + 1] = rand() % 256;     // Worley noise 1 (G)
                    NoiseData[idx + 2] = rand() % 256;     // Worley noise 2 (B)
                    NoiseData[idx + 3] = rand() % 256;     // Worley noise 3 (A)
                }
            }
        }
        
        TextureData InitData;
        InitData.pSubResources = new TextureSubResData[1];
        InitData.NumSubresources = 1;
        InitData.pSubResources[0].pData = NoiseData.data();
        InitData.pSubResources[0].Stride = TexDesc.Width * 4;
        InitData.pSubResources[0].DepthStride = TexDesc.Width * TexDesc.Height * 4;
        
        pDevice->CreateTexture(TexDesc, &InitData, &m_pPerlinWorleyNoise);
        m_pPerlinWorleyNoiseSRV = m_pPerlinWorleyNoise->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        
        delete[] InitData.pSubResources;
    }
    
    // Create the 3D Worley noise texture (32x32x32)
    {
        TextureDesc TexDesc;
        TexDesc.Name = "3D Worley Noise";
        TexDesc.Type = RESOURCE_DIM_TEX_3D;
        TexDesc.Width = 32;
        TexDesc.Height = 32;
        TexDesc.Depth = 32;
        TexDesc.Format = TEX_FORMAT_RGBA8_UNORM;
        TexDesc.BindFlags = BIND_SHADER_RESOURCE;
        TexDesc.Usage = USAGE_IMMUTABLE;
        
        // Generate simple procedural 3D noise
        std::vector<Uint8> NoiseData(TexDesc.Width * TexDesc.Height * TexDesc.Depth * 4);
        
        srand(24); // Different seed from the first texture
        for (Uint32 z = 0; z < TexDesc.Depth; ++z)
        {
            for (Uint32 y = 0; y < TexDesc.Height; ++y)
            {
                for (Uint32 x = 0; x < TexDesc.Width; ++x)
                {
                    Uint32 idx = (z * TexDesc.Height * TexDesc.Width + y * TexDesc.Width + x) * 4;
                    
                    // More detail noise
                    NoiseData[idx + 0] = rand() % 256;     // Worley noise 1 (R)
                    NoiseData[idx + 1] = rand() % 256;     // Worley noise 2 (G)
                    NoiseData[idx + 2] = rand() % 256;     // Worley noise 3 (B)
                    NoiseData[idx + 3] = rand() % 256;     // Curl noise (A)
                }
            }
        }
        
        TextureData InitData;
        InitData.pSubResources = new TextureSubResData[1];
        InitData.NumSubresources = 1;
        InitData.pSubResources[0].pData = NoiseData.data();
        InitData.pSubResources[0].Stride = TexDesc.Width * 4;
        InitData.pSubResources[0].DepthStride = TexDesc.Width * TexDesc.Height * 4;
        
        pDevice->CreateTexture(TexDesc, &InitData, &m_pWorleyNoise);
        m_pWorleyNoiseSRV = m_pWorleyNoise->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        
        delete[] InitData.pSubResources;
    }
    
    // Create the 2D weather map (512x512)
    {
        TextureDesc TexDesc;
        TexDesc.Name = "2D Weather Map";
        TexDesc.Type = RESOURCE_DIM_TEX_2D;
        TexDesc.Width = 512;
        TexDesc.Height = 512;
        TexDesc.Format = TEX_FORMAT_RGBA8_UNORM;
        TexDesc.BindFlags = BIND_SHADER_RESOURCE;
        TexDesc.Usage = USAGE_IMMUTABLE;
        
        // Generate simple 2D weather map
        std::vector<Uint8> MapData(TexDesc.Width * TexDesc.Height * 4);
        
        srand(36); // Different seed
        for (Uint32 y = 0; y < TexDesc.Height; ++y)
        {
            for (Uint32 x = 0; x < TexDesc.Width; ++x)
            {
                Uint32 idx = (y * TexDesc.Width + x) * 4;
                
                // Create a simple pattern for the weather map
                float fx = static_cast<float>(x) / TexDesc.Width;
                float fy = static_cast<float>(y) / TexDesc.Height;
                
                // Create some cloud formations with perlin-like patterns
                float coverage = (sin(fx * 5.0f) * 0.5f + 0.5f) * (cos(fy * 6.0f) * 0.5f + 0.5f);
                coverage = coverage * 0.5f + 0.3f; // Range 0.3 to 0.8
                
                float type = (cos(fx * 3.0f + fy * 2.0f) * 0.5f + 0.5f);
                float height = (sin(fx * 4.0f + fy * 3.0f) * 0.3f + 0.7f);
                float density = (cos(fx * 7.0f + fy * 8.0f) * 0.5f + 0.5f);
                
                MapData[idx + 0] = static_cast<Uint8>(coverage * 255);  // Cloud coverage (R)
                MapData[idx + 1] = static_cast<Uint8>(type * 255);      // Cloud type (G)
                MapData[idx + 2] = static_cast<Uint8>(height * 255);    // Cloud height (B)
                MapData[idx + 3] = static_cast<Uint8>(density * 255);   // Cloud density (A)
            }
        }
        
        TextureData InitData;
        InitData.pSubResources = new TextureSubResData[1];
        InitData.NumSubresources = 1;
        InitData.pSubResources[0].pData = MapData.data();
        InitData.pSubResources[0].Stride = TexDesc.Width * 4;
        
        pDevice->CreateTexture(TexDesc, &InitData, &m_pWeatherMap);
        m_pWeatherMapSRV = m_pWeatherMap->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        
        delete[] InitData.pSubResources;
    }
}

} // namespace Diligent