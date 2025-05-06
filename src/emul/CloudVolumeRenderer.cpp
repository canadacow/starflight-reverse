#include <vector>

#include "CloudVolumeRenderer.hpp"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"
#include "Graphics/GraphicsTools/interface/GraphicsUtilities.h"
#include "Graphics/GraphicsEngine/interface/GraphicsTypesX.hpp"
#include "Graphics/GraphicsTools/interface/ShaderSourceFactoryUtils.hpp"
#include "Utilities/interface/DiligentFXShaderSourceStreamFactory.hpp"

#include "../util/lodepng.h"

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

        // Sample the 3D Perlin-Worley noise texture
        float4 SamplePerlinWorleyNoise(float3 position)
        {
            return g_PerlinWorleyNoise.Sample(g_NoiseSampler, position);
        }

        // Sample the 3D detail Worley noise texture
        float3 SampleWorleyNoise(float3 position)
        {
            return g_WorleyNoise.Sample(g_NoiseSampler, position).xyz;
        }

        // Sample the 2D weather texture for cloud coverage and type
        float3 SampleWeatherTexture(float2 position)
        {
            return g_WeatherMap.Sample(g_WeatherSampler, position).xyz;
        }

        // Sample the curl noise texture for cloud detail
        float3 SampleCurlNoise(float2 position)
        {
            return g_WeatherMap.Sample(g_WeatherSampler, position).xyz;
        }

        // Helper function to remap values from one range to another
        float Remap(float value, float oldMin, float oldMax, float newMin, float newMax)
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
        
        // Henyey-Greenstein phase function for light scattering
        float HenyeyGreenstein(float cosAngle, float g)
        {
            float g2 = g * g;
            return (1.0 - g2) / (4.0 * 3.14159265 * pow(1.0 + g2 - 2.0 * g * cosAngle, 1.5));
        }
        
        // Compute the cloud density at a given sample point
        float SampleCloudDensity(float3 samplePoint, float heightPercent, bool isCheap)
        {
            // Apply wind animation
            float3 animatedPos = samplePoint + g_CloudParams.AnimationParams.xyz * g_CloudParams.AnimationParams.w;
            
            // Sample base shape noise (low frequency Perlin-Worley)
            float3 baseNoiseUV = animatedPos * g_CloudParams.NoiseParams.x;
            float4 lowFreqNoise = SamplePerlinWorleyNoise(baseNoiseUV);
            
            // Calculate FBM (Fractal Brownian Motion) from the noise channels
            // Using weights similar to the GLSL shader (G: 0.625, B: 0.25, A: 0.125)
            float lowFreqFBM = lowFreqNoise.g * 0.625 + lowFreqNoise.b * 0.25 + lowFreqNoise.a * 0.125;
            
            // Remap base cloud density with Worley noise (similar to the provided GLSL)
            float baseCloud = Remap(lowFreqNoise.r, -(1.0 - lowFreqFBM), 1.0, 0.0, 1.0);
            baseCloud = saturate(baseCloud);
            
            // Sample weather data to control cloud coverage
            float2 weatherMapCoord = (samplePoint.xz - g_CloudParams.CloudBoxMin.xz) / (g_CloudParams.CloudBoxMax.xz - g_CloudParams.CloudBoxMin.xz);
            weatherMapCoord = saturate(weatherMapCoord);
            float3 weatherData = SampleWeatherTexture(weatherMapCoord);
            
            // Apply height gradient - denser at bottom, less dense at top
            float densityHeightGradient = exp(-heightPercent * g_CloudParams.ShapeParams.w);
            
            // Combine base shape with height gradient
            baseCloud = baseCloud * densityHeightGradient;
            
            // Apply coverage from weather map (controlls where clouds appear)
            float cloudCoverage = weatherData.r * g_CloudParams.ShapeParams.x;
            baseCloud = saturate(Remap(baseCloud, 1.0 - cloudCoverage, 1.0, 0.0, 1.0));
            
            float finalCloud = baseCloud;
            
            // Add detail erosion with high-frequency noise if not using cheap sampling
            if (!isCheap)
            {
                // Add curl noise to the sample position for more natural variation
                float3 curlNoise = SampleCurlNoise(samplePoint.xy);
                float3 detailSamplePos = animatedPos;
                detailSamplePos.xy += curlNoise.xy * g_CloudParams.NoiseParams.w;
                
                // Sample high frequency detail noise
                float3 highFreqNoise = SampleWorleyNoise(detailSamplePos * g_CloudParams.NoiseParams.y);
                
                // Compute detail FBM
                float highFreqFBM = highFreqNoise.r * 0.625 + highFreqNoise.g * 0.25 + highFreqNoise.b * 0.125;
                highFreqFBM = saturate(highFreqFBM);
                
                // Adjust based on height (less detail erosion at tops of clouds)
                float detailStrength = g_CloudParams.NoiseParams.z;
                float detailModifier = lerp(detailStrength, detailStrength * 0.2, saturate(heightPercent * 2.0));
                
                // Erode base cloud with detail noise
                finalCloud = Remap(finalCloud, highFreqFBM * detailModifier, 1.0, 0.0, 1.0);
                finalCloud = saturate(finalCloud);
            }
            
            // Apply anvil bias for cumulonimbus-like shapes (more volume at the top)
            float anvilBias = g_CloudParams.ShapeParams.z;
            float anvilStrength = saturate(pow(heightPercent, 4.0) * anvilBias * weatherData.g);
            finalCloud = max(finalCloud, anvilStrength);
            
            // Apply global density multiplier and opacity
            return finalCloud * g_CloudParams.ScatteringParams.w * g_CloudParams.CloudOpacity;
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
            
            // Earth shadow approximation (optional - for atmospheric effects)
            float earthShadow = 1.0;
            
            for (int i = 0; i < sampleCount; i++)
            {
                // Skip if ray has passed scene depth or we've accumulated enough opacity
                if (t > sceneDepthT || transmittance < 0.01)
                    break;
                
                // Position along ray
                float3 pos = rayOrigin + rayDir * t;
                
                // Compute height fraction within cloud layer
                float heightFraction = (pos.y - g_CloudParams.CloudBoxMin.y) / (g_CloudParams.CloudBoxMax.y - g_CloudParams.CloudBoxMin.y);
                
                // Skip if outside the valid height range
                if (heightFraction < 0.0 || heightFraction > 1.0)
                {
                    t += stepSize;
                    continue;
                }
                
                // First do a cheap density sample to check if there are clouds
                float cheapDensity = SampleCloudDensity(pos, heightFraction, true);
                
                // If there's some density, do a more detailed sample
                if (cheapDensity > 0.0)
                {
                    // Sample cloud density at this position
                    float density = SampleCloudDensity(pos, heightFraction, false);
                    
                    if (density > 0.0)
                    {
                        // Light attenuation through clouds
                        float lightDist = 0.0;
                        const int lightSampleCount = 6;
                        float lightStepSize = 40.0 / float(lightSampleCount);
                        float3 lightSamplePos = pos;
                        float lightTransmittance = 1.0;
                        
                        // Sample along light direction
                        for (int j = 0; j < lightSampleCount; j++)
                        {
                            lightSamplePos += lightDir * lightStepSize;
                            float lightHeightFrac = (lightSamplePos.y - g_CloudParams.CloudBoxMin.y) / (g_CloudParams.CloudBoxMax.y - g_CloudParams.CloudBoxMin.y);
                            if (lightHeightFrac < 0.0 || lightHeightFrac > 1.0) break;
                            
                            float lightDensity = SampleCloudDensity(lightSamplePos, lightHeightFrac, true) * 0.5;
                            float extinction = g_CloudParams.ScatteringParams.x + g_CloudParams.ScatteringParams.y;
                            lightTransmittance *= exp(-extinction * lightDensity * lightStepSize);
                            
                            if (lightTransmittance < 0.01) break;
                        }
                        
                        // Combine with earth shadow
                        lightTransmittance *= earthShadow;
                        
                        // Phase function for directional scattering
                        float cosAngle = dot(rayDir, lightDir);
                        float phase = HenyeyGreenstein(cosAngle, g_CloudParams.ScatteringParams.z);
                        
                        // Ambient term (multi-scattering approximation)
                        float ambient = 0.5 + 0.5 * heightFraction;
                        
                        // Calculate total light contribution
                        float scatteredLight = lightTransmittance * phase + ambient * 0.2;
                        float extinction = g_CloudParams.ScatteringParams.x + g_CloudParams.ScatteringParams.y;
                        float dTrans = exp(-extinction * density * stepSize);
                        
                        // Beer's law for light absorption
                        float3 cloudColor = lerp(g_CloudParams.CloudColor.rgb * 0.15, g_CloudParams.CloudColor.rgb, heightFraction);
                        float3 S = cloudColor * scatteredLight * (1.0 - dTrans) * transmittance;
                        
                        // Accumulate light and update transmittance
                        totalLight += S;
                        transmittance *= dTrans;
                    }
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
    
    m_CloudParams.CloudBoxMin = float4(
        terrainBounds.Min.x,
        cloudsMinHeight, 
        terrainBounds.Min.z, 
        1.0f
    );
    
    m_CloudParams.CloudBoxMax = float4(
        terrainBounds.Max.x, 
        cloudsMaxHeight, 
        terrainBounds.Max.z, 
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
        
        // Load the 3D noise texture from a 2D image file
        // The image is 128x16384, which represents 128x128x128 3D texture (128 slices of 128x128)
        std::vector<unsigned char> image;
        unsigned width, height;
        
        // Decode the PNG
        unsigned error = lodepng::decode(image, width, height, "LowFrequency3DTexture.png");
        if (error || width != 128 || height != 16384) {
            LOG_ERROR_MESSAGE("Failed to decode LowFrequency3DTexture.png or incorrect dimensions: ", 
                              lodepng_error_text(error), " (", width, "x", height, ")");
            // Fallback to procedural noise if decoding fails
            return;
        }
        
        // Prepare texture data
        TextureData InitData;
        std::vector<TextureSubResData> SubResData(1);
        InitData.pSubResources = SubResData.data();
        InitData.NumSubresources = 1;
        InitData.pSubResources[0].pData = image.data();
        InitData.pSubResources[0].Stride = width * 4; // 4 bytes per pixel (RGBA)
        InitData.pSubResources[0].DepthStride = width * 128 * 4; // Each depth slice is 128 rows
        
        pDevice->CreateTexture(TexDesc, &InitData, &m_pPerlinWorleyNoise);
        m_pPerlinWorleyNoiseSRV = m_pPerlinWorleyNoise->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
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
        
        // Load the 3D noise texture from a 2D image file
        // The image is 32x1024, which represents 32x32x32 3D texture (32 slices of 32x32)
        std::vector<unsigned char> image;
        unsigned width, height;
        
        // Decode the PNG
        unsigned error = lodepng::decode(image, width, height, "HighFrequency3DTexture.png");
        if (error || width != 32 || height != 1024) {
            LOG_ERROR_MESSAGE("Failed to decode HighFrequency3DTexture.png or incorrect dimensions: ", 
                              lodepng_error_text(error), " (", width, "x", height, ")");
            // Fallback to procedural noise if decoding fails
            return;
        }
        
        // Prepare texture data
        TextureData InitData;
        std::vector<TextureSubResData> SubResData(1);
        InitData.pSubResources = SubResData.data();
        InitData.NumSubresources = 1;
        InitData.pSubResources[0].pData = image.data();
        InitData.pSubResources[0].Stride = width * 4; // 4 bytes per pixel (RGBA)
        InitData.pSubResources[0].DepthStride = width * 32 * 4; // Each depth slice is 32 rows
        
        pDevice->CreateTexture(TexDesc, &InitData, &m_pWorleyNoise);
        m_pWorleyNoiseSRV = m_pWorleyNoise->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    }
    
    // Create the 2D weather map (512x512)
    {
        TextureDesc TexDesc;
        TexDesc.Name = "2D Weather Map";
        TexDesc.Type = RESOURCE_DIM_TEX_2D;
        TexDesc.BindFlags = BIND_SHADER_RESOURCE;
        TexDesc.Usage = USAGE_IMMUTABLE;
        
        // Load the weather map from a PNG file
        std::vector<unsigned char> pngData;
        unsigned int width, height;
        
        // Use lodepng to load the weather map texture
        unsigned error = lodepng::decode(pngData, width, height, "weathermap.png");
        if (error) {
            // If loading fails, log error and continue with empty texture
            LOG_ERROR("Failed to load weathermap.png: ", lodepng_error_text(error));
        }
        
        // Set texture dimensions based on the loaded data
        TexDesc.Width = width;
        TexDesc.Height = height;
        TexDesc.Format = TEX_FORMAT_RGBA8_UNORM;
        
        TextureData InitData;
        std::vector<TextureSubResData> SubResData(1);
        InitData.pSubResources = SubResData.data();
        InitData.NumSubresources = 1;
        InitData.pSubResources[0].pData = pngData.data();
        InitData.pSubResources[0].Stride = TexDesc.Width * 4;
        
        pDevice->CreateTexture(TexDesc, &InitData, &m_pWeatherMap);
        m_pWeatherMapSRV = m_pWeatherMap->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    }
}

} // namespace Diligent