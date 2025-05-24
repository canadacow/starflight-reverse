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
    m_CloudParams.CloudDensity = 0.5f;
    m_CloudParams.NoiseScale = 0.025f;
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

    // Create camera attributes buffer
    {
        BufferDesc CBDesc;
        CBDesc.Name = "Camera Attributes buffer";
        CBDesc.Size = sizeof(HLSL::CameraAttribs);
        CBDesc.Usage = USAGE_DYNAMIC;
        CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
        CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        pDevice->CreateBuffer(CBDesc, nullptr, &m_pCameraAttribsCB);
    }

    // Create light attributes buffer
    {
        BufferDesc CBDesc;
        CBDesc.Name = "Light Attributes buffer";
        CBDesc.Size = sizeof(HLSL::LightAttribs);
        CBDesc.Usage = USAGE_DYNAMIC;
        CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
        CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        pDevice->CreateBuffer(CBDesc, nullptr, &m_pLightAttribsCB);
    }

    // Create PSO and shaders
    GraphicsPipelineStateCreateInfo PSOCreateInfo;
    PipelineStateDesc&              PSODesc = PSOCreateInfo.PSODesc;

    PSODesc.Name = "Cloud volume renderer PSO";
    PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

    // Define shader resource variable types
    PipelineResourceLayoutDescX ResourceLayout;
    ResourceLayout
        .SetDefaultVariableType(SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
        .AddVariable(SHADER_TYPE_VERTEX | SHADER_TYPE_PIXEL, "CameraAttribs", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
        .AddVariable(SHADER_TYPE_VERTEX | SHADER_TYPE_PIXEL, "LightAttribs", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
        .AddVariable(SHADER_TYPE_PIXEL, "CloudParams", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
        .AddVariable(SHADER_TYPE_PIXEL, "g_DepthTexture", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
        .AddVariable(SHADER_TYPE_PIXEL, "g_HighFreqNoiseTexture", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
        .AddVariable(SHADER_TYPE_PIXEL, "g_LowFreqNoiseTexture", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
        .AddVariable(SHADER_TYPE_PIXEL, "g_WeatherMapTexture", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC);
        
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
        .AddImmutableSampler(SHADER_TYPE_PIXEL, "g_NoiseSampler", NoiseSamplerDesc)
        .AddImmutableSampler(SHADER_TYPE_PIXEL, "g_WeatherMapSampler", NoiseSamplerDesc);

    PSOCreateInfo.PSODesc.ResourceLayout = ResourceLayout;

    // Shader creation
    ShaderCreateInfo ShaderCI;
    ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
    #ifdef _DEBUG
        ShaderCI.CompileFlags = SHADER_COMPILE_FLAG_PACK_MATRIX_ROW_MAJOR | SHADER_COMPILE_FULL_DEBUG;
    #else
        ShaderCI.CompileFlags = SHADER_COMPILE_FLAG_PACK_MATRIX_ROW_MAJOR;
    #endif
    ShaderCI.pShaderSourceStreamFactory = pCompoundSourceFactory;
    // Vertex shader
    ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
    ShaderCI.ShaderCompiler = SHADER_COMPILER_DXC;
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

        cbuffer CameraAttribs : register(b0)
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
    #ifdef _DEBUG
        ShaderCI.CompileFlags = SHADER_COMPILE_FLAG_PACK_MATRIX_ROW_MAJOR | SHADER_COMPILE_FULL_DEBUG;
    #else
        ShaderCI.CompileFlags = SHADER_COMPILE_FLAG_PACK_MATRIX_ROW_MAJOR;
    #endif
    ShaderCI.ShaderCompiler = SHADER_COMPILER_DXC;
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

        cbuffer CameraAttribs : register(b1)
        {
            CameraAttribs g_Camera;
        };

        cbuffer LightAttribs : register(b2)
        {
            LightAttribs g_Light;
        };

        cbuffer CloudParams : register(b3)
        {
            float4 g_CloudBoxMin;       // Bottom of cloud box
            float4 g_CloudBoxMax;       // Top of cloud box
            float4 g_CloudColor;        // Color of the cloud box
            float  g_CloudOpacity;      // Opacity of the cloud box
            float  g_CloudDensity;      // Density of the cloud volume
            float  g_NoiseScale;        // Scale of the noise texture
        };

        SamplerState g_DepthSampler : register(s4);
        Texture2D g_DepthTexture : register(t5);
        
        // 3D noise textures for cloud detail
        Texture3D g_HighFreqNoiseTexture : register(t6);
        Texture3D g_LowFreqNoiseTexture : register(t7);
        SamplerState g_NoiseSampler : register(s8);    
        
        // Weather map texture for density modulation
        Texture2D g_WeatherMapTexture : register(t9);
        SamplerState g_WeatherMapSampler : register(s10);

        // hash function              
        float hash(float n)
        {
            return frac(cos(n) * 114514.1919);
        }

        // 3d noise function
        float noise(in float3 x)
        {
            float3 p = floor(x);
            float3 f = smoothstep(0.0, 1.0, frac(x));
                
            float n = p.x + p.y * 10.0 + p.z * 100.0;
            
            return lerp(
                lerp(lerp(hash(n + 0.0), hash(n + 1.0), f.x),
                     lerp(hash(n + 10.0), hash(n + 11.0), f.x), f.y),
                lerp(lerp(hash(n + 100.0), hash(n + 101.0), f.x),
                     lerp(hash(n + 110.0), hash(n + 111.0), f.x), f.y), f.z);
        }

        // Define rotation matrix for fbm
        static const float3x3 m = float3x3(
            0.00, 1.60, 1.20,
            -1.60, 0.72, -0.96,
            -1.20, -0.96, 1.28
        );

        // Fractional Brownian motion
        float fbm(float3 p)
        {
            float f = 0.5000 * noise(p);
            p = mul(m, p);
            f += 0.2500 * noise(p);
            p = mul(m, p);
            f += 0.1666 * noise(p);
            p = mul(m, p);
            f += 0.0834 * noise(p);
            return f;
        }       

        // Cloud animation parameters
        struct CloudAnimation
        {
            float3 baseWind;              // Base wind at low altitude
            float3 highWind;              // Base wind at high altitude
            float largeScaleMovement;     // Large-scale oscillation
            float time;                   // Current time
        };

        // Get base wind vector based on height and time
        float3 getBaseWindVector(float height, float time)
        {
            // Create a height-dependent wind vector
            float3 windVec = float3(
                sin(time * 0.1 + height * 0.01) * 5.0,
                cos(time * 0.05) * 0.5,
                sin(time * 0.07 + height * 0.02) * 3.0
            );
            return windVec;
        }

        // Pre-compute cloud animation parameters
        CloudAnimation computeCloudAnimation(float time)
        {
            CloudAnimation anim;
            
            anim.time = time;
            anim.baseWind = getBaseWindVector(g_CloudBoxMin.y, time);
            anim.highWind = getBaseWindVector(g_CloudBoxMax.y, time);
            anim.largeScaleMovement = sin(time * 0.05) * 2000.0;
            
            return anim;
        }

        // Get animated cloud position
        float3 getAnimatedCloudPos(float3 pos, CloudAnimation anim)
        {
            // Interpolate between base and high altitude wind based on height
            float heightRatio = saturate((pos.y - g_CloudBoxMin.y) / (g_CloudBoxMax.y - g_CloudBoxMin.y));
            float3 wind = lerp(anim.baseWind, anim.highWind, heightRatio);
            
            // Apply local turbulence based on position
            float3 turbulencePos = pos * 0.0001 + anim.time * 0.1;
            float3 turbulence = float3(
                noise(turbulencePos + float3(0.0, 13.7, 31.1)) - 0.5,
                (noise(turbulencePos + float3(43.5, 0.0, 7.2)) - 0.5) * 0.2, // Less vertical turbulence
                noise(turbulencePos + float3(8.3, 53.1, 0.0)) - 0.5
            );
            
            // Wind turbulence and speed constants
            const float windTurbulence = 1.0;
            const float windSpeed = 10.0;
            
            // Combine all displacement effects
            float3 animatedPos = pos;
            
            // Wind displacement
            animatedPos -= (wind + turbulence * windTurbulence * windSpeed) * anim.time;
            
            // Large scale movement
            animatedPos.x += anim.largeScaleMovement;
            
            // Height-based differential movement (lower clouds move differently)
            animatedPos.x += sin(heightRatio * 3.14159 + anim.time * 0.2) * 1000.0;
            
            return animatedPos;
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

            float time = 0.0;
            CloudAnimation cloudAnim = computeCloudAnimation(time * 10.0);
            //float densityFactor = lerp(0.66, 1.8, mo.x);
            
            float3 cloudrange = float3(g_CloudBoxMin.y, g_CloudBoxMax.y, 0.0);
            float3 light = normalize(g_Light.f4Direction.xyz);

            float4 sum = float4(0.0, 0.0, 0.0, 0.0);
            float t_min = tNear;
            float t_max = tFar;
            float3 campos = g_Camera.f4Position.xyz;
            
            int stepCount = 64;
            float stepSize = (t_max - t_min) / float(stepCount);

            for (float t = t_min; t < t_max; t += stepSize) {
                float3 pos = campos + rayDir * t;
                float3 adjPos = pos * float3(g_NoiseScale, g_NoiseScale, g_NoiseScale);    

                // Get animated cloud position using precomputed animation parameters
                float3 animatedPos = getAnimatedCloudPos(adjPos, cloudAnim);
                
                // Sample weather map for density modification
                // Map world position to UV coordinates (assuming cloud box maps to 0-1 UV space)
                float2 weatherUV = (pos.xz - g_CloudBoxMin.xz) / (g_CloudBoxMax.xz - g_CloudBoxMin.xz);
                float weatherDensity = g_WeatherMapTexture.Sample(g_WeatherMapSampler, weatherUV).g; // Sample green channel

                // Remap weather density from [0,1] to [-1,1] range
                weatherDensity = weatherDensity * 2.0 - 1.0;
                float adjustedDensity = g_CloudDensity + weatherDensity;
                float densityFactor = lerp(0.66, 1.8, adjustedDensity);
                
                float noiseTimesDensity = fbm(animatedPos) * densityFactor;
                float density = smoothstep(0.5, 1.0, noiseTimesDensity);
                //float density = noiseTimesDensity;

                float3 cloudColor = lerp(float3(1.1, 1.05, 1.0), float3(0.3, 0.3, 0.2), density);
                
                // Lighting - brighten top of clouds, darken bottom
                float heightGradient = smoothstep(cloudrange.x, cloudrange.y, pos.y);
                cloudColor = lerp(cloudColor * 0.7, cloudColor * 1.3, heightGradient);
                
                // Sun effects
                float sundot = saturate(dot(rayDir, light));
                cloudColor += 0.4 * float3(1.0, 0.7, 0.4) * pow(sundot, 10.0) * density;
                
                // Accumulate color with transmission
                float alpha = density * 0.1; // Lower alpha for better translucency
                alpha *= (1.0 - sum.a); // Multiply by remaining transparency
                sum += float4(cloudColor * alpha, alpha);
                
                // Early exit if nearly opaque
                if (sum.a > 0.99) {
                    break;
                }
            }
            float3 cloudColor = sum.rgb * g_CloudColor.rgb;
            float cloudAlpha = sum.a * g_CloudOpacity;

            // Apply premultiplied alpha
            cloudColor *= cloudAlpha;
            
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
        PSOCreateInfo.GraphicsPipeline.BlendDesc.RenderTargets[i].SrcBlend = BLEND_FACTOR_ONE;
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

#ifdef _DEBUG
    PSOCreateInfo.Flags = PSO_CREATE_FLAG_DONT_REMAP_SHADER_RESOURCES | PSO_CREATE_FLAG_DONT_VERIFY_SHADER_RESOURCES;
#endif

    // Create the pipeline state
    pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pRenderCloudsPSO);

    // Create shader resource binding
    m_pRenderCloudsPSO->CreateShaderResourceBinding(&m_pRenderCloudsSRB, true);

    LoadNoiseTextures();
    LoadWeatherMap();
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
    
    // Update camera attributes buffer
    {
        MapHelper<HLSL::CameraAttribs> MappedCamAttribs(pContext, m_pCameraAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD);
        *MappedCamAttribs = CamAttribs;
    }

    // Update light attributes buffer
    {
        MapHelper<HLSL::LightAttribs> MappedLightAttribs(pContext, m_pLightAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD);
        *MappedLightAttribs = LightAttrs;
    }
    
#ifdef _DEBUG
    // Bind the camera attributes and depth texture to the existing SRB
    m_pRenderCloudsSRB->GetVariableByIndex(SHADER_TYPE_VERTEX, 0)->Set(m_pCameraAttribsCB);
    m_pRenderCloudsSRB->GetVariableByIndex(SHADER_TYPE_PIXEL, 0)->Set(m_pCameraAttribsCB);
    m_pRenderCloudsSRB->GetVariableByIndex(SHADER_TYPE_PIXEL, 1)->Set(m_pCloudParamsCB);
    m_pRenderCloudsSRB->GetVariableByIndex(SHADER_TYPE_PIXEL, 2)->Set(m_pLightAttribsCB);

    auto srv = m_pRenderCloudsSRB->GetVariableByIndex(SHADER_TYPE_PIXEL, 3);
    if (srv)
        srv->Set(pDepthBufferSRV);

    auto highFreqVar = m_pRenderCloudsSRB->GetVariableByIndex(SHADER_TYPE_PIXEL, 4);
    if (highFreqVar)
        highFreqVar->Set(m_pHighFreqNoiseSRV);
        
    auto lowFreqVar = m_pRenderCloudsSRB->GetVariableByIndex(SHADER_TYPE_PIXEL, 5);
    if (lowFreqVar)
        lowFreqVar->Set(m_pLowFreqNoiseSRV);
        
    auto weatherMapVar = m_pRenderCloudsSRB->GetVariableByIndex(SHADER_TYPE_PIXEL, 6);
    if (weatherMapVar)
        weatherMapVar->Set(m_pWeatherMapSRV);
#else
    // Bind the camera attributes and depth texture to the existing SRB
    m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_VERTEX, "CameraAttribs")->Set(m_pCameraAttribsCB);
    m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "CameraAttribs")->Set(m_pCameraAttribsCB);
    m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "LightAttribs")->Set(m_pLightAttribsCB);
    m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "CloudParams")->Set(m_pCloudParamsCB);

    auto srv = m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_DepthTexture");
    if (srv)
        srv->Set(pDepthBufferSRV);

    auto highFreqVar = m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_HighFreqNoiseTexture");
    if (highFreqVar)
        highFreqVar->Set(m_pHighFreqNoiseSRV);

    auto lowFreqVar = m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_LowFreqNoiseTexture");
    if (lowFreqVar)
        lowFreqVar->Set(m_pLowFreqNoiseSRV);
        
    auto weatherMapVar = m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_WeatherMapTexture");
    if (weatherMapVar)
        weatherMapVar->Set(m_pWeatherMapSRV);
#endif

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
#ifdef _DEBUG
        auto highFreqVar = m_pRenderCloudsSRB->GetVariableByIndex(SHADER_TYPE_PIXEL, 4);
        if (highFreqVar)
            highFreqVar->Set(m_pHighFreqNoiseSRV);  
            
        auto lowFreqVar = m_pRenderCloudsSRB->GetVariableByIndex(SHADER_TYPE_PIXEL, 5);
        if (lowFreqVar)
            lowFreqVar->Set(m_pLowFreqNoiseSRV);
#else
        auto highFreqVar = m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_HighFreqNoiseTexture");
        if (highFreqVar)
            highFreqVar->Set(m_pHighFreqNoiseSRV);  
            
        auto lowFreqVar = m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL,"g_LowFreqNoiseTexture");
        if (lowFreqVar)
            lowFreqVar->Set(m_pLowFreqNoiseSRV);
#endif
    }
}

void CloudVolumeRenderer::LoadWeatherMap()
{
    std::string weatherMapPath = "weathermap.png";

    if (!m_pDevice)
        return;
    
    // Load weather map texture
    {
        // Load PNG using lodepng
        std::vector<unsigned char> imageData;
        unsigned width, height;
        unsigned error = lodepng::decode(imageData, width, height, weatherMapPath.c_str());
        
        if (error)
        {
            // Handle loading error - could log or use a default texture
            return;
        }
        
        // Create 2D texture
        TextureDesc TexDesc;
        TexDesc.Name = "Weather map texture";
        TexDesc.Type = RESOURCE_DIM_TEX_2D;
        TexDesc.Width = width;
        TexDesc.Height = height;
        TexDesc.MipLevels = 1;
        TexDesc.Format = TEX_FORMAT_RGBA8_UNORM;
        TexDesc.Usage = USAGE_IMMUTABLE;
        TexDesc.BindFlags = BIND_SHADER_RESOURCE;
        
        // Prepare texture data
        TextureSubResData Level0Data;
        Level0Data.pData = imageData.data();
        Level0Data.Stride = width * 4; // 4 bytes per pixel (RGBA)
        
        TextureData InitData;
        InitData.NumSubresources = 1;
        InitData.pSubResources = &Level0Data;
        
        m_pDevice->CreateTexture(TexDesc, &InitData, &m_pWeatherMapTexture);
        m_pWeatherMapSRV = m_pWeatherMapTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    }
    
    // If we already have a shader resource binding, update it with the new texture
    if (m_pRenderCloudsSRB)
    {
#ifdef _DEBUG
        auto weatherMapVar = m_pRenderCloudsSRB->GetVariableByIndex(SHADER_TYPE_PIXEL, 6);
        if (weatherMapVar)
            weatherMapVar->Set(m_pWeatherMapSRV);
#else
        auto weatherMapVar = m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_WeatherMapTexture");
        if (weatherMapVar)
            weatherMapVar->Set(m_pWeatherMapSRV);
#endif
    }
}

} // namespace Diligent