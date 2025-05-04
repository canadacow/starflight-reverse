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
        .AddVariable(SHADER_TYPE_PIXEL, "CloudParams", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE);
        
    // Create sampler descriptions for immutable samplers
    SamplerDesc DepthSamplerDesc;
    DepthSamplerDesc.MinFilter = FILTER_TYPE_LINEAR;
    DepthSamplerDesc.MagFilter = FILTER_TYPE_LINEAR;
    DepthSamplerDesc.MipFilter = FILTER_TYPE_LINEAR;
    DepthSamplerDesc.AddressU = TEXTURE_ADDRESS_CLAMP;
    DepthSamplerDesc.AddressV = TEXTURE_ADDRESS_CLAMP;
    DepthSamplerDesc.AddressW = TEXTURE_ADDRESS_CLAMP;
    
    ResourceLayout
        .AddImmutableSampler(SHADER_TYPE_PIXEL, "g_DepthSampler", DepthSamplerDesc);

    PSOCreateInfo.PSODesc.ResourceLayout = ResourceLayout;

    // Shader creation
    ShaderCreateInfo ShaderCI;
    ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
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
            clipPos.y = -clipPos.y; // Adjust for DirectX coordinate system
            
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
            tNear = max(0, tNear);
            // Simple semi-transparent box
            float3 color = g_CloudColor.rgb;
            float opacity = g_CloudOpacity;
            // Output to all PBR G-buffer targets
            output.Color        = float4(color, opacity);
            output.Normal       = float4(0.0, 1.0, 0.0, 1.0); // Upward normal
            output.BaseColor    = float4(color, opacity);
            output.MaterialData = float4(0.0, 0.0, 0.0, opacity); // No roughness/metallic
            output.MotionVec    = float4(0.0, 0.0, 0.0, 1.0); // No motion
            output.SpecularIBL  = float4(0.0, 0.0, 0.0, opacity); // No IBL
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
    m_pRenderCloudsSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_DepthTexture")->Set(pDepthBufferSRV);
    
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

void CloudVolumeRenderer::SetupTerrainParameters(const BoundBox& terrainBounds, float waterHeight)
{
    // Set the cloud box boundaries
    m_CloudParams.CloudBoxMin = float4(terrainBounds.Min.x, waterHeight + 25.0f, terrainBounds.Min.z, 1.0f);
    m_CloudParams.CloudBoxMax = float4(terrainBounds.Max.x, waterHeight + 60.0f, terrainBounds.Max.z, 1.0f);
    
    // Set default cloud parameters
    m_CloudParams.CloudColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    m_CloudParams.CloudOpacity = 0.3f;
}

} // namespace Diligent