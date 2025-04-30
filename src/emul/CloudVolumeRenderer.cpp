#include "CloudVolumeRenderer.hpp"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"
#include "Graphics/GraphicsTools/interface/GraphicsUtilities.h"
#include <random>
#include <cmath>

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
    m_CloudParams.CloudDensity = 0.5f;
    m_CloudParams.CloudCoverage = 0.6f;
    m_CloudParams.CloudSpeed = 0.1f;
    m_CloudParams.CloudShadowIntensity = 0.5f;
    m_CloudParams.CloudLightAbsorption = 0.1f;
    m_CloudParams.WindDirection = float3(1.0f, 0.0f, 0.0f);
    m_CloudParams.Time = 0.0f;
    m_CloudParams.NoiseOctaves = 4;
    m_CloudParams.DetailStrength = 0.5f;
    m_CloudParams.DetailScale = 3.0f;
}

CloudVolumeRenderer::~CloudVolumeRenderer()
{
}

void CloudVolumeRenderer::Initialize(IRenderDevice* pDevice, IDeviceContext* pImmediateContext)
{
    // Store device reference
    m_pDevice = pDevice;

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

    // Create volume noise texture
    CreateVolumeNoiseTexture(pDevice, pImmediateContext);
    
    // Create detail noise texture
    CreateDetailNoiseTexture(pDevice, pImmediateContext);
    
    // Create weather map
    CreateWeatherMap(pDevice, pImmediateContext);

    // Create PSO and shaders
    GraphicsPipelineStateCreateInfo PSOCreateInfo;
    PipelineStateDesc&              PSODesc = PSOCreateInfo.PSODesc;

    PSODesc.Name = "Cloud volume renderer PSO";
    PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

    // Shader creation
    ShaderCreateInfo ShaderCI;
    ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

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

        cbuffer CameraAttribs
        {
            float4x4 g_ViewProj;
            float4x4 g_InvViewProj;
            float4   g_CameraPosition;
            float4   g_LightDirection;
            float4   g_LightColor;
        };

        PSInput main(VSInput input)
        {
            PSInput output;
            output.pos = float4(input.pos, 0.0, 1.0);
            output.uv = input.uv;
            
            // Calculate ray direction in world space
            float4 rayStart = float4(input.pos, -1.0, 1.0);
            float4 rayEnd = float4(input.pos, 1.0, 1.0);
            
            float4 rayStartWorld = mul(g_InvViewProj, rayStart);
            float4 rayEndWorld = mul(g_InvViewProj, rayEnd);
            
            rayStartWorld /= rayStartWorld.w;
            rayEndWorld /= rayEndWorld.w;
            
            output.rayDir = normalize(rayEndWorld.xyz - rayStartWorld.xyz);
            
            return output;
        }
    )";

    RefCntAutoPtr<IShader> pVS;
    pDevice->CreateShader(ShaderCI, &pVS);

    // Pixel shader with raymarching
    ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
    ShaderCI.EntryPoint = "main";
    ShaderCI.Desc.Name = "Cloud Volume PS";
    ShaderCI.Source = R"(
        struct PSInput
        {
            float4 pos : SV_POSITION;
            float2 uv  : TEX_COORD;
            float3 rayDir : RAY_DIR;
        };

        cbuffer CameraAttribs
        {
            float4x4 g_ViewProj;
            float4x4 g_InvViewProj;
            float4   g_CameraPosition;
            float4   g_LightDirection;
            float4   g_LightColor;
        };

        cbuffer CloudParams
        {
            float4 g_CloudBoxMin;       // Bottom of cloud box
            float4 g_CloudBoxMax;       // Top of cloud box
            float  g_CloudDensity;      // Overall density
            float  g_CloudCoverage;     // How much of the sky is covered
            float  g_CloudSpeed;        // Movement speed
            float  g_CloudShadowIntensity;
            float  g_CloudLightAbsorption;
            float3 g_WindDirection;     // Direction clouds move
            float  g_Time;              // For cloud animation
            uint   g_NoiseOctaves;      // Number of noise octaves
            float  g_DetailStrength;    // Strength of detail noise
            float  g_DetailScale;       // Scale of detail noise
            float  g_Padding[1];        // Padding to 16-byte alignment
        };

        Texture3D g_VolumeNoiseTexture;
        Texture3D g_DetailNoiseTexture;
        Texture2D g_WeatherMapTexture;
        Texture2D g_DepthTexture;
        SamplerState g_VolumeNoiseSampler;
        SamplerState g_DetailNoiseSampler;
        SamplerState g_WeatherMapSampler;
        SamplerState g_DepthSampler;

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
        
        // Sample volume noise with multiple octaves
        float SampleCloudDensity(float3 pos)
        {
            // Animate clouds position over time
            float3 animatedPos = pos + g_WindDirection * g_Time * g_CloudSpeed;
            
            // Normalize position within the cloud box
            float3 normalizedPos = (animatedPos - g_CloudBoxMin.xyz) / (g_CloudBoxMax.xyz - g_CloudBoxMin.xyz);
            
            // Sample weather map (2D coverage texture)
            float2 weatherMapUV = float2(normalizedPos.x, normalizedPos.z);
            float coverage = g_WeatherMapTexture.SampleLevel(g_WeatherMapSampler, weatherMapUV, 0).r;
            
            // Apply coverage to cloud height
            float heightPercent = (pos.y - g_CloudBoxMin.y) / (g_CloudBoxMax.y - g_CloudBoxMin.y);
            float heightGradient = saturate(heightPercent * 4.0) * saturate(1.0 - heightPercent * 2.0);
            
            // Apply coverage threshold
            coverage = saturate((coverage - (1.0 - g_CloudCoverage)) * 4.0);
            
            // Early exit if no cloud potential
            if (coverage * heightGradient <= 0.01)
                return 0.0;
            
            // Base shape from volume noise
            float density = 0.0;
            float amplitude = 1.0;
            float frequency = 1.0;
            float totalAmplitude = 0.0;
            
            // Sample main noise with multiple octaves
            for (uint i = 0; i < g_NoiseOctaves; i++)
            {
                float3 noisePos = animatedPos * frequency * 0.01;
                float noiseSample = g_VolumeNoiseTexture.SampleLevel(g_VolumeNoiseSampler, frac(noisePos), 0).r;
                
                density += noiseSample * amplitude;
                totalAmplitude += amplitude;
                
                amplitude *= 0.5;
                frequency *= 2.0;
            }
            
            // Normalize result
            density /= totalAmplitude;
            
            // Apply detail noise for smaller features
            float3 detailPos = animatedPos * g_DetailScale * 0.01;
            float detailNoise = g_DetailNoiseTexture.SampleLevel(g_DetailNoiseSampler, frac(detailPos), 0).r;
            detailNoise = (detailNoise - 0.5) * 2.0;
            
            // Apply detail noise to erode main shape
            density = saturate(density - detailNoise * g_DetailStrength);
            
            // Combine with coverage and height
            density = density * coverage * heightGradient;
            
            // Apply overall density adjustment
            return density * g_CloudDensity;
        }
        
        // Calculate light energy through the cloud volume
        float SampleCloudLight(float3 pos, float3 lightDir)
        {
            // March along light direction a few steps to calculate approximate light attenuation
            float3 samplePos = pos;
            float totalDensity = 0.0;
            
            const int LIGHT_SAMPLES = 6;
            float stepSize = (g_CloudBoxMax.y - g_CloudBoxMin.y) / float(LIGHT_SAMPLES);
            
            for (int i = 0; i < LIGHT_SAMPLES; i++)
            {
                samplePos += lightDir * stepSize;
                
                // Exit if outside cloud box
                if (any(samplePos < g_CloudBoxMin.xyz) || any(samplePos > g_CloudBoxMax.xyz))
                    break;
                
                totalDensity += SampleCloudDensity(samplePos) * stepSize;
            }
            
            // Beer's law for light attenuation
            return exp(-totalDensity * g_CloudLightAbsorption);
        }

        float4 main(PSInput input) : SV_TARGET
        {
            // Get world-space position of the current pixel from depth buffer
            float depth = g_DepthTexture.Sample(g_DepthSampler, input.uv).r;
            float4 clipPos = float4(input.uv * 2.0 - 1.0, depth, 1.0);
            clipPos.y = -clipPos.y; // Adjust for DirectX coordinate system
            
            float4 worldPos = mul(g_InvViewProj, clipPos);
            worldPos /= worldPos.w;
            
            float3 rayOrigin = g_CameraPosition.xyz;
            float3 rayDir = normalize(input.rayDir);
            
            // Find intersection with cloud box
            float tNear, tFar;
            bool intersect = IntersectBox(rayOrigin, rayDir, g_CloudBoxMin.xyz, g_CloudBoxMax.xyz, tNear, tFar);
            
            // Early exit if no intersection
            if (!intersect)
                return float4(0, 0, 0, 0);
            
            // Calculate distance to scene geometry
            float sceneDistance = length(worldPos.xyz - rayOrigin);
            
            // Adjust tFar if scene geometry is closer than the cloud box exit point
            tFar = min(tFar, sceneDistance);
            
            // Skip if intersection is behind camera or completely behind geometry
            if (tFar <= 0 || tNear >= sceneDistance)
                return float4(0, 0, 0, 0);
            
            // Adjust tNear to start from camera if we're inside the box
            tNear = max(0, tNear);
            
            // Ray marching parameters
            const int MAX_STEPS = 64;
            float stepSize = (tFar - tNear) / float(MAX_STEPS);
            
            // Ray marching variables
            float3 pos = rayOrigin + rayDir * tNear;
            float3 lightDir = normalize(-g_LightDirection.xyz);
            float transmittance = 1.0;
            float3 totalLight = float3(0, 0, 0);
            
            // Do ray marching through the cloud volume
            for (int i = 0; i < MAX_STEPS; i++)
            {
                // Get current sample position
                pos = rayOrigin + rayDir * (tNear + i * stepSize);
                
                // Sample cloud density at this position
                float density = SampleCloudDensity(pos);
                
                // Skip empty samples
                if (density > 0.001)
                {
                    // Calculate light contribution at this point
                    float lightEnergy = SampleCloudLight(pos, lightDir);
                    
                    // Apply Beer's law for extinction
                    float extinction = exp(-density * stepSize);
                    
                    // Apply Henyey-Greenstein phase function approximation
                    float phase = 0.5 + 0.5 * dot(rayDir, lightDir); // Simplified approximation
                    
                    // Standard cloud color (white with slight blue tint)
                    float3 baseCloudColor = float3(1.0, 1.0, 1.02);
                    
                    // Calculate light contribution
                    float3 luminance = g_LightColor.xyz * lightEnergy * density * stepSize * phase;
                    
                    // Add light contribution, attenuated by current transmittance
                    totalLight += baseCloudColor * luminance * transmittance;
                    
                    // Update transmittance
                    transmittance *= extinction;
                    
                    // Early exit if nearly opaque
                    if (transmittance < 0.01)
                        break;
                }
            }
            
            // Return final color with alpha based on transmittance
            return float4(totalLight, 1.0 - transmittance);
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
    PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
    PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = TEX_FORMAT_RGBA8_UNORM; // Will be overridden in Render()
    PSOCreateInfo.GraphicsPipeline.DSVFormat = TEX_FORMAT_D32_FLOAT;       // Will be overridden in Render()

    // Set blend state
    PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendEnable = True;
    PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlend = BLEND_FACTOR_SRC_ALPHA;
    PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlend = BLEND_FACTOR_INV_SRC_ALPHA;
    PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOp = BLEND_OPERATION_ADD;
    PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].SrcBlendAlpha = BLEND_FACTOR_ONE;
    PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].DestBlendAlpha = BLEND_FACTOR_INV_SRC_ALPHA;
    PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].BlendOpAlpha = BLEND_OPERATION_ADD;
    PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[0].RenderTargetWriteMask = COLOR_MASK_ALL;

    // Set depth-stencil state (no depth testing for clouds)
    PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;
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

    // Bind resources
    m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_VolumeNoiseTexture")->Set(m_pVolumeNoiseTextureSRV);
    m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_DetailNoiseTexture")->Set(m_pDetailNoiseTextureSRV);
    m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_WeatherMapTexture")->Set(m_pWeatherMapTextureSRV);
    m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_CloudParams")->Set(m_pCloudParamsCB);
}

void CloudVolumeRenderer::CreateVolumeNoiseTexture(IRenderDevice* pDevice, IDeviceContext* pContext)
{
    const Uint32 NoiseTexSize = 64;
    
    // Create 3D noise texture
    TextureDesc TexDesc;
    TexDesc.Type = RESOURCE_DIM_TEX_3D;
    TexDesc.Width = NoiseTexSize;
    TexDesc.Height = NoiseTexSize;
    TexDesc.Depth = NoiseTexSize;
    TexDesc.MipLevels = 1;
    TexDesc.Format = TEX_FORMAT_R8_UNORM;
    TexDesc.BindFlags = BIND_SHADER_RESOURCE;
    TexDesc.Usage = USAGE_IMMUTABLE;
    TexDesc.Name = "Cloud volume noise texture";
    
    // Generate Perlin-like noise data
    std::vector<Uint8> NoiseData(NoiseTexSize * NoiseTexSize * NoiseTexSize);
    
    // Using standard C++ random number generators instead of custom RNG
    std::mt19937 gen(42); // Fixed seed of 42 for reproducibility
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    for (Uint32 z = 0; z < NoiseTexSize; ++z)
    {
        for (Uint32 y = 0; y < NoiseTexSize; ++y)
        {
            for (Uint32 x = 0; x < NoiseTexSize; ++x)
            {
                // Generate a value between 0 and 255
                float value = (dist(gen) * 0.5f + 0.5f) * 255.0f;
                
                // Bias towards lower values for more wispy clouds
                value = std::pow(value / 255.0f, 2.2f) * 255.0f;
                
                // Write to the texture data
                NoiseData[z * NoiseTexSize * NoiseTexSize + y * NoiseTexSize + x] = static_cast<Uint8>(value);
            }
        }
    }
    
    // Create the texture with the noise data
    TextureSubResData SubResData;
    SubResData.pData = NoiseData.data();
    SubResData.Stride = NoiseTexSize;
    SubResData.DepthStride = NoiseTexSize * NoiseTexSize;
    
    TextureData InitData;
    InitData.pSubResources = &SubResData;
    InitData.NumSubresources = 1;
    
    pDevice->CreateTexture(TexDesc, &InitData, &m_pVolumeNoiseTexture);
    m_pVolumeNoiseTextureSRV = m_pVolumeNoiseTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
}

void CloudVolumeRenderer::CreateDetailNoiseTexture(IRenderDevice* pDevice, IDeviceContext* pContext)
{
    const Uint32 NoiseTexSize = 32;
    
    // Create 3D detail noise texture (smaller, higher frequency)
    TextureDesc TexDesc;
    TexDesc.Type = RESOURCE_DIM_TEX_3D;
    TexDesc.Width = NoiseTexSize;
    TexDesc.Height = NoiseTexSize;
    TexDesc.Depth = NoiseTexSize;
    TexDesc.MipLevels = 1;
    TexDesc.Format = TEX_FORMAT_R8_UNORM;
    TexDesc.BindFlags = BIND_SHADER_RESOURCE;
    TexDesc.Usage = USAGE_IMMUTABLE;
    TexDesc.Name = "Cloud detail noise texture";
    
    // Generate worley-like noise for detail
    std::vector<Uint8> NoiseData(NoiseTexSize * NoiseTexSize * NoiseTexSize);
    
    // Using standard C++ random number generators instead of custom RNG
    std::mt19937 gen(123); // Different seed for detail texture
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    for (Uint32 z = 0; z < NoiseTexSize; ++z)
    {
        for (Uint32 y = 0; y < NoiseTexSize; ++y)
        {
            for (Uint32 x = 0; x < NoiseTexSize; ++x)
            {
                // Generate a sharper noise for detail
                float value = (dist(gen) * 0.8f + 0.2f) * 255.0f;
                
                // Write to the texture data
                NoiseData[z * NoiseTexSize * NoiseTexSize + y * NoiseTexSize + x] = static_cast<Uint8>(value);
            }
        }
    }
    
    // Create the texture with the noise data
    TextureSubResData SubResData;
    SubResData.pData = NoiseData.data();
    SubResData.Stride = NoiseTexSize;
    SubResData.DepthStride = NoiseTexSize * NoiseTexSize;
    
    TextureData InitData;
    InitData.pSubResources = &SubResData;
    InitData.NumSubresources = 1;
    
    pDevice->CreateTexture(TexDesc, &InitData, &m_pDetailNoiseTexture);
    m_pDetailNoiseTextureSRV = m_pDetailNoiseTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
}

void CloudVolumeRenderer::CreateWeatherMap(IRenderDevice* pDevice, IDeviceContext* pContext)
{
    const Uint32 MapSize = 256;
    
    // Create 2D texture for cloud coverage
    TextureDesc TexDesc;
    TexDesc.Type = RESOURCE_DIM_TEX_2D;
    TexDesc.Width = MapSize;
    TexDesc.Height = MapSize;
    TexDesc.MipLevels = 1;
    TexDesc.Format = TEX_FORMAT_R8_UNORM;
    TexDesc.BindFlags = BIND_SHADER_RESOURCE;
    TexDesc.Usage = USAGE_IMMUTABLE;
    TexDesc.Name = "Cloud weather map texture";
    
    // Generate cloud coverage pattern
    std::vector<Uint8> MapData(MapSize * MapSize);
    
    // Using standard C++ random number generators instead of custom RNG
    std::mt19937 gen(789); // Different seed for weather map
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    // Create a more structured pattern for cloud distribution
    for (Uint32 y = 0; y < MapSize; ++y)
    {
        for (Uint32 x = 0; x < MapSize; ++x)
        {
            // Normalized coordinates (0-1)
            float nx = static_cast<float>(x) / MapSize;
            float ny = static_cast<float>(y) / MapSize;
            
            // Simple Perlin-like noise approximation
            float noise = 0.0f;
            float amplitude = 1.0f;
            float frequency = 1.0f;
            float totalAmplitude = 0.0f;
            
            // Add multiple frequencies of noise
            for (int i = 0; i < 4; ++i)
            {
                float fx = nx * frequency;
                float fy = ny * frequency;
                
                // Generate a simple noise value
                int ix = static_cast<int>(fx) & 255;
                int iy = static_cast<int>(fy) & 255;
                
                // Use a deterministic seed based on position
                std::mt19937 localGen(ix * 171 + iy * 57);
                float value = dist(localGen);
                
                noise += value * amplitude;
                totalAmplitude += amplitude;
                
                amplitude *= 0.5f;
                frequency *= 2.0f;
            }
            
            // Normalize and adjust contrast
            noise /= totalAmplitude;
            noise = std::pow(noise, 1.5f); // Increase contrast
            
            // Add some large-scale variation
            float largeScalePattern = 0.65f + 0.35f * std::sin(nx * 3.14159f * 2.0f) * std::sin(ny * 3.14159f * 3.0f);
            
            // Combine and scale to byte range
            float finalValue = (noise * 0.7f + largeScalePattern * 0.3f) * 255.0f;
            
            // Write to the texture data
            MapData[y * MapSize + x] = static_cast<Uint8>(std::min(255.0f, std::max(0.0f, finalValue)));
        }
    }
    
    // Create the texture with the weather map data
    TextureSubResData SubResData;
    SubResData.pData = MapData.data();
    SubResData.Stride = MapSize;
    
    TextureData InitData;
    InitData.pSubResources = &SubResData;
    InitData.NumSubresources = 1;
    
    pDevice->CreateTexture(TexDesc, &InitData, &m_pWeatherMapTexture);
    m_pWeatherMapTextureSRV = m_pWeatherMapTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
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

    // Create a temporary SRB to bind camera and depth texture
    RefCntAutoPtr<IShaderResourceBinding> pSRB;
    m_pRenderCloudsPSO->CreateShaderResourceBinding(&pSRB, false);
    
    // Copy static resources manually
    pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_VolumeNoiseTexture")->Set(m_pVolumeNoiseTextureSRV);
    pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_DetailNoiseTexture")->Set(m_pDetailNoiseTextureSRV);
    pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_WeatherMapTexture")->Set(m_pWeatherMapTextureSRV);
    pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_CloudParams")->Set(m_pCloudParamsCB);

    // Create and bind camera attributes buffer
    struct CameraAttribsData
    {
        float4x4 ViewProj;
        float4x4 InvViewProj;
        float4   CameraPosition;
        float4   LightDirection;
        float4   LightColor;
    };
    
    RefCntAutoPtr<IBuffer> pCameraAttribsCB;
    BufferDesc CBDesc;
    CBDesc.Name = "Camera Attributes buffer";
    CBDesc.Size = sizeof(CameraAttribsData);
    CBDesc.Usage = USAGE_DYNAMIC;
    CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
    CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
    m_pDevice->CreateBuffer(CBDesc, nullptr, &pCameraAttribsCB);
    
    // Fill the camera data from the HLSL structures
    {
        MapHelper<CameraAttribsData> CamAttribsData(pContext, pCameraAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD);
        CamAttribsData->ViewProj = CamAttribs.mViewProj;
        
        // Calculate inverse view-projection matrix
        CamAttribsData->InvViewProj = CamAttribs.mViewProjInv;
        
        CamAttribsData->CameraPosition = float4(CamAttribs.f4Position.x, CamAttribs.f4Position.y, CamAttribs.f4Position.z, 1.0f);
        CamAttribsData->LightDirection = float4(LightAttrs.f4Direction.x, LightAttrs.f4Direction.y, LightAttrs.f4Direction.z, 0.0f);
        CamAttribsData->LightColor = LightColor;
    }
    
    // Create samplers
    RefCntAutoPtr<ISampler> pSampler;
    Diligent::SamplerDesc SamplerDesc;
    SamplerDesc.MinFilter = FILTER_TYPE_LINEAR;
    SamplerDesc.MagFilter = FILTER_TYPE_LINEAR;
    SamplerDesc.MipFilter = FILTER_TYPE_LINEAR;
    SamplerDesc.AddressU = TEXTURE_ADDRESS_WRAP;
    SamplerDesc.AddressV = TEXTURE_ADDRESS_WRAP;
    SamplerDesc.AddressW = TEXTURE_ADDRESS_WRAP;
    m_pDevice->CreateSampler(SamplerDesc, &pSampler);
    
    // Create depth sampler
    RefCntAutoPtr<ISampler> pDepthSampler;
    Diligent::SamplerDesc DepthSamplerDesc;
    DepthSamplerDesc.MinFilter = FILTER_TYPE_LINEAR;
    DepthSamplerDesc.MagFilter = FILTER_TYPE_LINEAR;
    DepthSamplerDesc.MipFilter = FILTER_TYPE_LINEAR;
    DepthSamplerDesc.AddressU = TEXTURE_ADDRESS_CLAMP;
    DepthSamplerDesc.AddressV = TEXTURE_ADDRESS_CLAMP;
    DepthSamplerDesc.AddressW = TEXTURE_ADDRESS_CLAMP;
    m_pDevice->CreateSampler(DepthSamplerDesc, &pDepthSampler);
    
    // Bind the camera attributes and depth texture
    IShaderResourceVariable* pCameraCBVar = pSRB->GetVariableByName(SHADER_TYPE_VERTEX, "CameraAttribs");
    if (pCameraCBVar)
        pCameraCBVar->Set(pCameraAttribsCB);
    
    pCameraCBVar = pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "CameraAttribs");
    if (pCameraCBVar)
        pCameraCBVar->Set(pCameraAttribsCB);
    
    IShaderResourceVariable* pDepthTexVar = pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_DepthTexture");
    if (pDepthTexVar)
        pDepthTexVar->Set(pDepthBufferSRV);
    
    // Bind samplers
    IShaderResourceVariable* pSamplerVar = pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_VolumeNoiseSampler");
    if (pSamplerVar)
        pSamplerVar->Set(pSampler);
    
    pSamplerVar = pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_DetailNoiseSampler");
    if (pSamplerVar)
        pSamplerVar->Set(pSampler);
    
    pSamplerVar = pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_WeatherMapSampler");
    if (pSamplerVar)
        pSamplerVar->Set(pSampler);
    
    pSamplerVar = pSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_DepthSampler");
    if (pSamplerVar)
        pSamplerVar->Set(pDepthSampler);
    
    // Commit shader resources
    pContext->CommitShaderResources(pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    
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

void CloudVolumeRenderer::SetupTerrainParameters(float terrainMaxX, float terrainMaxZ, float waterHeight)
{
    // Set the cloud box boundaries
    m_CloudParams.CloudBoxMin = float4(0.0f, waterHeight + 25.0f, 0.0f, 1.0f);
    m_CloudParams.CloudBoxMax = float4(terrainMaxX, waterHeight + 60.0f, terrainMaxZ, 1.0f);
    
    // Set default cloud parameters
    m_CloudParams.CloudDensity = 0.3f;
    m_CloudParams.CloudCoverage = 0.6f;
    m_CloudParams.CloudSpeed = 0.05f;
    m_CloudParams.CloudShadowIntensity = 0.7f;
    m_CloudParams.CloudLightAbsorption = 0.1f;
    m_CloudParams.WindDirection = float3(1.0f, 0.0f, 0.5f);
    m_CloudParams.NoiseOctaves = 4;
    m_CloudParams.DetailStrength = 0.7f;
    m_CloudParams.DetailScale = 5.0f;
}

void CloudVolumeRenderer::UpdateTime(float time)
{
    m_CloudParams.Time = time;
}

} // namespace Diligent