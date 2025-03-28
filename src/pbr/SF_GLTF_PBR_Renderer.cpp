/*
 *  Copyright 2019-2024 Diligent Graphics LLC
 *  Copyright 2015-2019 Egor Yusov
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  In no event and under no legal theory, whether in tort (including negligence),
 *  contract, or otherwise, unless required by applicable law (such as deliberate
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental,
 *  or consequential damages of any character arising as a result of this License or
 *  out of the use or inability to use the software (including but not limited to damages
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and
 *  all other commercial damages or losses), even if such Contributor has been advised
 *  of the possibility of such damages.
 */

#include "SF_GLTF_PBR_Renderer.hpp"

#include <array>
#include <functional>
#include <algorithm>
#include <cmath>

#include "BasicMath.hpp"
#include "MapHelper.hpp"
#include "GraphicsAccessories.hpp"
#include "SF_GLTFLoader.hpp"

namespace Diligent
{

    SF_PBR_Renderer::ALPHA_MODE SF_GLTF_PBR_Renderer::GltfAlphaModeToAlphaMode(SF_GLTF::Material::ALPHA_MODE GltfAlphaMode)
{
    static_assert(static_cast<ALPHA_MODE>(SF_GLTF::Material::ALPHA_MODE_OPAQUE) == ALPHA_MODE_OPAQUE, "SF_GLTF::Material::ALPHA_MODE_OPAQUE != ALPHA_MODE_OPAQUE");
    static_assert(static_cast<ALPHA_MODE>(SF_GLTF::Material::ALPHA_MODE_MASK) == ALPHA_MODE_MASK, "SF_GLTF::Material::ALPHA_MODE_MASK != ALPHA_MODE_MASK");
    static_assert(static_cast<ALPHA_MODE>(SF_GLTF::Material::ALPHA_MODE_BLEND) == ALPHA_MODE_BLEND, "SF_GLTF::Material::ALPHA_MODE_BLEND != ALPHA_MODE_BLEND");
    static_assert(static_cast<ALPHA_MODE>(SF_GLTF::Material::ALPHA_MODE_NUM_MODES) == ALPHA_MODE_NUM_MODES, "SF_GLTF::Material::ALPHA_MODE_NUM_MODES != ALPHA_MODE_NUM_MODES");
    return static_cast<ALPHA_MODE>(GltfAlphaMode);
}

namespace HLSL
{

#include "shaders/SF_PBR_Structures.fxh"
#include "Shaders/Common/public/BasicStructures.fxh"
#include "shaders/SF_RenderPBR_Structures.fxh"

} // namespace HLSL

namespace
{

struct PBRRendererCreateInfoWrapper
{
    PBRRendererCreateInfoWrapper(const SF_PBR_Renderer::CreateInfo& _CI) :
        CI{_CI}
    {
        if (CI.InputLayout.NumElements == 0)
        {
            InputLayout    = SF_GLTF::VertexAttributesToInputLayout(SF_GLTF::DefaultVertexAttributes.data(), SF_GLTF::DefaultVertexAttributes.size());
            CI.InputLayout = InputLayout;
        }

        CI.TextureAttribIndices[SF_PBR_Renderer::TEXTURE_ATTRIB_ID_BASE_COLOR]            = SF_GLTF::DefaultBaseColorTextureAttribId;
        CI.TextureAttribIndices[SF_PBR_Renderer::TEXTURE_ATTRIB_ID_PHYS_DESC]             = SF_GLTF::DefaultMetallicRoughnessTextureAttribId;
        CI.TextureAttribIndices[SF_PBR_Renderer::TEXTURE_ATTRIB_ID_NORMAL]                = SF_GLTF::DefaultNormalTextureAttribId;
        CI.TextureAttribIndices[SF_PBR_Renderer::TEXTURE_ATTRIB_ID_OCCLUSION]             = SF_GLTF::DefaultOcclusionTextureAttribId;
        CI.TextureAttribIndices[SF_PBR_Renderer::TEXTURE_ATTRIB_ID_EMISSIVE]              = SF_GLTF::DefaultEmissiveTextureAttribId;
        CI.TextureAttribIndices[SF_PBR_Renderer::TEXTURE_ATTRIB_ID_CLEAR_COAT]            = SF_GLTF::DefaultClearcoatTextureAttribId;
        CI.TextureAttribIndices[SF_PBR_Renderer::TEXTURE_ATTRIB_ID_CLEAR_COAT_ROUGHNESS]  = SF_GLTF::DefaultClearcoatRoughnessTextureAttribId;
        CI.TextureAttribIndices[SF_PBR_Renderer::TEXTURE_ATTRIB_ID_CLEAR_COAT_NORMAL]     = SF_GLTF::DefaultClearcoatNormalTextureAttribId;
        CI.TextureAttribIndices[SF_PBR_Renderer::TEXTURE_ATTRIB_ID_SHEEN_COLOR]           = SF_GLTF::DefaultSheenColorTextureAttribId;
        CI.TextureAttribIndices[SF_PBR_Renderer::TEXTURE_ATTRIB_ID_SHEEN_ROUGHNESS]       = SF_GLTF::DefaultSheenRoughnessTextureAttribId;
        CI.TextureAttribIndices[SF_PBR_Renderer::TEXTURE_ATTRIB_ID_ANISOTROPY]            = SF_GLTF::DefaultAnisotropyTextureAttribId;
        CI.TextureAttribIndices[SF_PBR_Renderer::TEXTURE_ATTRIB_ID_IRIDESCENCE]           = SF_GLTF::DefaultIridescenceTextureAttribId;
        CI.TextureAttribIndices[SF_PBR_Renderer::TEXTURE_ATTRIB_ID_IRIDESCENCE_THICKNESS] = SF_GLTF::DefaultIridescenceThicknessTextureAttribId;
        CI.TextureAttribIndices[SF_PBR_Renderer::TEXTURE_ATTRIB_ID_TRANSMISSION]          = SF_GLTF::DefaultTransmissionTextureAttribId;
        CI.TextureAttribIndices[SF_PBR_Renderer::TEXTURE_ATTRIB_ID_THICKNESS]             = SF_GLTF::DefaultThicknessTextureAttribId;
        static_assert(SF_PBR_Renderer::TEXTURE_ATTRIB_ID_COUNT == 17, "Please update the initializer list above");

        if (_CI.ShaderTexturesArrayMode == SF_PBR_Renderer::SHADER_TEXTURE_ARRAY_MODE_DYNAMIC)
        {
            UNEXPECTED("Dynamic shader texture arrays are not supported in GLTF renderer");
            CI.ShaderTexturesArrayMode = SF_PBR_Renderer::SHADER_TEXTURE_ARRAY_MODE_NONE;
        }
    }

    operator const SF_PBR_Renderer::CreateInfo &() const
    {
        return CI;
    }

    SF_PBR_Renderer::CreateInfo CI;
    InputLayoutDescX         InputLayout;
};

} // namespace

void InitializeGLTFPBRCreateInfo(SF_PBR_Renderer::CreateInfo& CI)
{
    CI = PBRRendererCreateInfoWrapper{CI}.CI;
}

SF_GLTF_PBR_Renderer::SF_GLTF_PBR_Renderer(IRenderDevice*     pDevice,
                                     IRenderStateCache* pStateCache,
                                     IDeviceContext*    pCtx,
                                     const CreateInfo&  CI) :
    SF_PBR_Renderer{pDevice, pStateCache, pCtx, PBRRendererCreateInfoWrapper{CI}}
{
    {
        GraphicsPipelineDesc GraphicsDesc;
        GraphicsDesc.NumRenderTargets = CI.NumRenderTargets;
        for (Uint32 i = 0; i < CI.NumRenderTargets; ++i)
            GraphicsDesc.RTVFormats[i] = CI.RTVFormats[i];
        GraphicsDesc.DSVFormat = CI.DSVFormat;

        GraphicsDesc.PrimitiveTopology                    = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        GraphicsDesc.RasterizerDesc.FrontCounterClockwise = CI.FrontCounterClockwise;

        m_PbrPSOCache = GetPsoCacheAccessor(GraphicsDesc);

        GraphicsPipelineDesc TerrainGraphicsDesc = GraphicsDesc;
        TerrainGraphicsDesc.DepthStencilDesc.DepthFunc = COMPARISON_FUNC_LESS_EQUAL;
        for(int i = 0; i < CI.NumRenderTargets; i++)
        {
            TerrainGraphicsDesc.BlendDesc.RenderTargets[i].BlendEnable    = true;
            TerrainGraphicsDesc.BlendDesc.RenderTargets[i].SrcBlend       = BLEND_FACTOR_ONE;
            TerrainGraphicsDesc.BlendDesc.RenderTargets[i].DestBlend      = BLEND_FACTOR_INV_SRC_ALPHA;
            TerrainGraphicsDesc.BlendDesc.RenderTargets[i].BlendOp        = BLEND_OPERATION_ADD;
            TerrainGraphicsDesc.BlendDesc.RenderTargets[i].SrcBlendAlpha  = BLEND_FACTOR_ONE;
            TerrainGraphicsDesc.BlendDesc.RenderTargets[i].DestBlendAlpha = BLEND_FACTOR_INV_SRC_ALPHA;
            TerrainGraphicsDesc.BlendDesc.RenderTargets[i].BlendOpAlpha   = BLEND_OPERATION_ADD;
        }

        m_TerrainPSOCache = GetPsoCacheAccessor(TerrainGraphicsDesc);

        GraphicsDesc.RasterizerDesc.FillMode = FILL_MODE_WIREFRAME;

        m_WireframePSOCache = GetPsoCacheAccessor(GraphicsDesc);
    }
}

void SF_GLTF_PBR_Renderer::InitMaterialSRB(SF_GLTF::Model&            Model,
                                        SF_GLTF::Material&         Material,
                                        IBuffer*                pFrameAttribs,
                                        IShaderResourceBinding* pMaterialSRB,
                                        ITextureView*           pShadowMap,
                                        IBuffer* pHeightmapAttribs,
                                        ITextureView* pHeightmap)
{
    if (pMaterialSRB == nullptr)
    {
        LOG_ERROR_MESSAGE("Failed to create material SRB");
        return;
    }

    InitCommonSRBVars(pMaterialSRB, pFrameAttribs, true, pShadowMap, pHeightmapAttribs, pHeightmap);

    auto SetTexture = [&](TEXTURE_ATTRIB_ID ID, ITextureView* pDefaultTexSRV) //
    {
        const int TexAttribId = m_Settings.TextureAttribIndices[ID];
        if (TexAttribId < 0)
        {
            UNEXPECTED("Texture attribute is not initialized");
            return;
        }

        RefCntAutoPtr<ITextureView> pTexSRV;

        auto TexIdx = Material.GetTextureId(TexAttribId);
        if (TexIdx >= 0)
        {
            if (auto* pTexture = Model.GetTexture(TexIdx))
            {
                if (pTexture->GetDesc().Type == RESOURCE_DIM_TEX_2D_ARRAY)
                    pTexSRV = pTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
                else
                {
                    TextureViewDesc SRVDesc;
                    SRVDesc.ViewType   = TEXTURE_VIEW_SHADER_RESOURCE;
                    SRVDesc.TextureDim = RESOURCE_DIM_TEX_2D_ARRAY;
                    pTexture->CreateView(SRVDesc, &pTexSRV);
                }
            }
        }

        if (pTexSRV == nullptr)
            pTexSRV = pDefaultTexSRV;

        this->SetMaterialTexture(pMaterialSRB, pTexSRV, ID);
    };

    SetTexture(TEXTURE_ATTRIB_ID_BASE_COLOR, m_pWhiteTexSRV);
    SetTexture(TEXTURE_ATTRIB_ID_PHYS_DESC, m_pDefaultPhysDescSRV);
    SetTexture(TEXTURE_ATTRIB_ID_NORMAL, m_pDefaultNormalMapSRV);

    if (m_Settings.EnableAO)
    {
        SetTexture(TEXTURE_ATTRIB_ID_OCCLUSION, m_pWhiteTexSRV);
    }

    if (m_Settings.EnableEmissive)
    {
        SetTexture(TEXTURE_ATTRIB_ID_EMISSIVE, m_pWhiteTexSRV);
    }

    if (m_Settings.EnableClearCoat)
    {
        SetTexture(TEXTURE_ATTRIB_ID_CLEAR_COAT, m_pWhiteTexSRV);
        SetTexture(TEXTURE_ATTRIB_ID_CLEAR_COAT_ROUGHNESS, m_pWhiteTexSRV);
        SetTexture(TEXTURE_ATTRIB_ID_CLEAR_COAT_NORMAL, m_pWhiteTexSRV);
    }

    if (m_Settings.EnableSheen)
    {
        SetTexture(TEXTURE_ATTRIB_ID_SHEEN_COLOR, m_pWhiteTexSRV);
        SetTexture(TEXTURE_ATTRIB_ID_SHEEN_ROUGHNESS, m_pWhiteTexSRV);
    }

    if (m_Settings.EnableAnisotropy)
    {
        SetTexture(TEXTURE_ATTRIB_ID_ANISOTROPY, m_pWhiteTexSRV);
    }

    if (m_Settings.EnableIridescence)
    {
        SetTexture(TEXTURE_ATTRIB_ID_IRIDESCENCE, m_pWhiteTexSRV);
        SetTexture(TEXTURE_ATTRIB_ID_IRIDESCENCE_THICKNESS, m_pWhiteTexSRV);
    }

    if (m_Settings.EnableTransmission)
    {
        SetTexture(TEXTURE_ATTRIB_ID_TRANSMISSION, m_pWhiteTexSRV);
    }

    if (m_Settings.EnableVolume)
    {
        SetTexture(TEXTURE_ATTRIB_ID_THICKNESS, m_pWhiteTexSRV);
    }
}

void SF_GLTF_PBR_Renderer::CreateResourceCacheSRB(IRenderDevice*           pDevice,
                                               IDeviceContext*          pCtx,
                                               ResourceCacheUseInfo&    CacheUseInfo,
                                               IBuffer*                 pFrameAttribs,
                                               IShaderResourceBinding** ppCacheSRB)
{
    DEV_CHECK_ERR(CacheUseInfo.pResourceMgr != nullptr, "Resource manager must not be null");

    VERIFY_EXPR(m_ResourceSignatures.size() == 1);
    m_ResourceSignatures[0]->CreateShaderResourceBinding(ppCacheSRB, true);
    IShaderResourceBinding* const pSRB = *ppCacheSRB;
    if (pSRB == nullptr)
    {
        LOG_ERROR_MESSAGE("Failed to create an SRB");
        return;
    }

    InitCommonSRBVars(pSRB, pFrameAttribs);

    auto SetTexture = [&](TEXTURE_FORMAT Fmt, TEXTURE_ATTRIB_ID ID) //
    {
        if (auto* pTexture = CacheUseInfo.pResourceMgr->UpdateTexture(Fmt, pDevice, pCtx))
        {
            this->SetMaterialTexture(pSRB, pTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE), ID);
        }
    };

    SetTexture(CacheUseInfo.BaseColorFormat, TEXTURE_ATTRIB_ID_BASE_COLOR);
    SetTexture(CacheUseInfo.PhysicalDescFormat, TEXTURE_ATTRIB_ID_PHYS_DESC);
    SetTexture(CacheUseInfo.NormalFormat, TEXTURE_ATTRIB_ID_NORMAL);
    if (m_Settings.EnableAO)
    {
        SetTexture(CacheUseInfo.OcclusionFormat, TEXTURE_ATTRIB_ID_OCCLUSION);
    }
    if (m_Settings.EnableEmissive)
    {
        SetTexture(CacheUseInfo.EmissiveFormat, TEXTURE_ATTRIB_ID_EMISSIVE);
    }

    if (m_Settings.EnableClearCoat)
    {
        SetTexture(CacheUseInfo.ClearCoatFormat, TEXTURE_ATTRIB_ID_CLEAR_COAT);
        SetTexture(CacheUseInfo.ClearCoatRoughnessFormat, TEXTURE_ATTRIB_ID_CLEAR_COAT_ROUGHNESS);
        SetTexture(CacheUseInfo.ClearCoatNormalFormat, TEXTURE_ATTRIB_ID_CLEAR_COAT_NORMAL);
    }

    if (m_Settings.EnableSheen)
    {
        SetTexture(CacheUseInfo.SheenColorFormat, TEXTURE_ATTRIB_ID_SHEEN_COLOR);
        SetTexture(CacheUseInfo.SheenRoughnessFormat, TEXTURE_ATTRIB_ID_SHEEN_ROUGHNESS);
    }

    if (m_Settings.EnableAnisotropy)
    {
        SetTexture(CacheUseInfo.AnisotropyFormat, TEXTURE_ATTRIB_ID_ANISOTROPY);
    }

    if (m_Settings.EnableIridescence)
    {
        SetTexture(CacheUseInfo.IridescenceFormat, TEXTURE_ATTRIB_ID_IRIDESCENCE);
        SetTexture(CacheUseInfo.IridescenceThicknessFormat, TEXTURE_ATTRIB_ID_IRIDESCENCE_THICKNESS);
    }

    if (m_Settings.EnableTransmission)
    {
        SetTexture(CacheUseInfo.TransmissionFormat, TEXTURE_ATTRIB_ID_TRANSMISSION);
    }

    if (m_Settings.EnableVolume)
    {
        SetTexture(CacheUseInfo.ThicknessFormat, TEXTURE_ATTRIB_ID_THICKNESS);
    }
}

SF_GLTF_PBR_Renderer::ModelResourceBindings SF_GLTF_PBR_Renderer::CreateResourceBindings(
    SF_GLTF::Model& GLTFModel,
    IBuffer*     pFrameAttribs,
    ITextureView* pShadowMap,
    IBuffer* pHeightmapAttribs,
    ITextureView* pHeightmap)
{
    ModelResourceBindings ResourceBindings;
    ResourceBindings.MaterialSRB.resize(GLTFModel.GetMaterials().size());
    for (size_t mat = 0; mat < GLTFModel.GetMaterials().size(); ++mat)
    {
        auto& pMatSRB = ResourceBindings.MaterialSRB[mat];
        CreateResourceBinding(&pMatSRB);
        InitMaterialSRB(GLTFModel, GLTFModel.GetMaterials()[mat], pFrameAttribs, pMatSRB, pShadowMap, pHeightmapAttribs, pHeightmap);
    }
    return ResourceBindings;
}

void SF_GLTF_PBR_Renderer::Begin(IDeviceContext* pCtx)
{
    if (m_JointsBuffer)
    {
        // In next-gen backends, dynamic buffers must be mapped before the first use in every frame
        MapHelper<float4x4> pJoints{pCtx, m_JointsBuffer, MAP_WRITE, MAP_FLAG_DISCARD};
    }
}

void SF_GLTF_PBR_Renderer::Begin(IRenderDevice*         pDevice,
                              IDeviceContext*        pCtx,
                              ResourceCacheUseInfo&  CacheUseInfo,
                              ResourceCacheBindings& Bindings,
                              IBuffer*               pFrameAttribs)
{
    VERIFY(CacheUseInfo.pResourceMgr != nullptr, "Resource manager must not be null.");
    VERIFY(CacheUseInfo.VtxLayoutKey != SF_GLTF::ResourceManager::VertexLayoutKey{}, "Vertex layout key must not be null.");

    Begin(pCtx);

    auto TextureVersion = CacheUseInfo.pResourceMgr->GetTextureVersion();
    if (!Bindings.pSRB || Bindings.Version != TextureVersion)
    {
        Bindings.pSRB.Release();
        CreateResourceCacheSRB(pDevice, pCtx, CacheUseInfo, pFrameAttribs, &Bindings.pSRB);
        if (!Bindings.pSRB)
        {
            LOG_ERROR_MESSAGE("Failed to create an SRB for GLTF resource cache");
            return;
        }
        Bindings.Version = TextureVersion;
    }

    pCtx->TransitionShaderResources(Bindings.pSRB);

    if (auto* pVertexPool = CacheUseInfo.pResourceMgr->GetVertexPool(CacheUseInfo.VtxLayoutKey))
    {
        const auto& PoolDesc = pVertexPool->GetDesc();

        std::array<IBuffer*, 8> pVBs; // Do not zero-initialize
        for (Uint32 i = 0; i < PoolDesc.NumElements; ++i)
        {
            pVBs[i] = pVertexPool->Update(i, pDevice, pCtx);
            if ((pVBs[i]->GetDesc().BindFlags & BIND_VERTEX_BUFFER) == 0)
                pVBs[i] = nullptr;
        }

        pCtx->SetVertexBuffers(0, PoolDesc.NumElements, pVBs.data(), nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
    }

    auto* pIndexBuffer = CacheUseInfo.pResourceMgr->UpdateIndexBuffer(pDevice, pCtx);
    pCtx->SetIndexBuffer(pIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

SF_GLTF_PBR_Renderer::PSO_FLAGS SF_GLTF_PBR_Renderer::GetMaterialPSOFlags(const SF_GLTF::Material& Mat) const
{
    // Color, normal and physical descriptor maps are always enabled
    PSO_FLAGS PSOFlags =
        PSO_FLAG_USE_COLOR_MAP |
        PSO_FLAG_USE_NORMAL_MAP |
        PSO_FLAG_USE_PHYS_DESC_MAP;

    if (m_Settings.EnableAO)
    {
        PSOFlags |= PSO_FLAG_USE_AO_MAP;
    }

    if (m_Settings.EnableEmissive)
    {
        PSOFlags |= PSO_FLAG_USE_EMISSIVE_MAP;
    }

    if (m_Settings.EnableClearCoat && Mat.HasClearcoat)
    {
        PSOFlags |=
            PSO_FLAG_ENABLE_CLEAR_COAT |
            PSO_FLAG_USE_CLEAR_COAT_MAP |
            PSO_FLAG_USE_CLEAR_COAT_ROUGHNESS_MAP |
            PSO_FLAG_USE_CLEAR_COAT_NORMAL_MAP;
    }

    if (m_Settings.EnableSheen && Mat.Sheen)
    {
        PSOFlags |=
            PSO_FLAG_ENABLE_SHEEN |
            PSO_FLAG_USE_SHEEN_COLOR_MAP |
            PSO_FLAG_USE_SHEEN_ROUGHNESS_MAP;
    }

    if (m_Settings.EnableAnisotropy && Mat.Anisotropy)
    {
        PSOFlags |=
            PSO_FLAG_ENABLE_ANISOTROPY |
            PSO_FLAG_USE_ANISOTROPY_MAP;
    }

    if (m_Settings.EnableIridescence && Mat.Iridescence)
    {
        PSOFlags |=
            PSO_FLAG_ENABLE_IRIDESCENCE |
            PSO_FLAG_USE_IRIDESCENCE_MAP |
            PSO_FLAG_USE_IRIDESCENCE_THICKNESS_MAP;
    }

    if (m_Settings.EnableTransmission && Mat.Transmission)
    {
        PSOFlags |=
            PSO_FLAG_ENABLE_TRANSMISSION |
            PSO_FLAG_USE_TRANSMISSION_MAP;
    }

    if (m_Settings.EnableVolume && Mat.Volume)
    {
        PSOFlags |=
            PSO_FLAG_ENABLE_VOLUME |
            PSO_FLAG_USE_THICKNESS_MAP;
    }

    return PSOFlags;
}


void SF_GLTF_PBR_Renderer::RenderTerrain(IDeviceContext*              pCtx,
                                        const SF_GLTF::Model&           GLTFModel,
                                        const SF_GLTF::ModelTransforms& Transforms,
                                        const RenderInfo&            RenderParams,
                                        ModelResourceBindings*       pModelBindings)
{

}

void SF_GLTF_PBR_Renderer::Render(IDeviceContext*              pCtx,
                               const SF_GLTF::Model&           GLTFModel,
                               const SF_GLTF::ModelTransforms& Transforms,
                               const SF_GLTF::ModelTransforms* PrevTransforms,
                               const RenderInfo&            RenderParams,
                               ModelResourceBindings*       pModelBindings,
                               ResourceCacheBindings*       pCacheBindings)
{
    static_assert(static_cast<LIGHT_TYPE>(SF_GLTF::Light::TYPE::DIRECTIONAL) == LIGHT_TYPE_DIRECTIONAL, "SF_GLTF::Light::TYPE::DIRECTIONAL != LIGHT_TYPE_DIRECTIONAL");
    static_assert(static_cast<LIGHT_TYPE>(SF_GLTF::Light::TYPE::POINT) == LIGHT_TYPE_POINT, "SF_GLTF::Light::TYPE::POINT != LIGHT_TYPE_POINT");
    static_assert(static_cast<LIGHT_TYPE>(SF_GLTF::Light::TYPE::SPOT) == LIGHT_TYPE_SPOT, "SF_GLTF::Light::TYPE::SPOT != LIGHT_TYPE_SPOT");

    DEV_CHECK_ERR((pModelBindings != nullptr) ^ (pCacheBindings != nullptr), "Either model bindings or cache bindings must not be null");
    DEV_CHECK_ERR(pModelBindings == nullptr || pModelBindings->MaterialSRB.size() == GLTFModel.GetMaterials().size(),
                  "The number of material shader resource bindings is not consistent with the number of materials");

    if (!GLTFModel.CompatibleWithTransforms(Transforms))
    {
        DEV_ERROR("Model transforms are incompatible with the model");
        return;
    }
    if (RenderParams.SceneIndex >= GLTFModel.GetScenes().size())
    {
        DEV_ERROR("Invalid scene index ", RenderParams.SceneIndex);
        return;
    }
    const auto& Scene = GLTFModel.GetScenes()[RenderParams.SceneIndex];

    m_RenderParams = RenderParams;

    GLTFModel.ClearSetVertexBuffers();

    for (auto& List : m_RenderLists)
        List.clear();

    for (const auto* pNode : Scene.LinearNodes)
    {
        VERIFY_EXPR(pNode != nullptr);
        if (pNode->pMesh == nullptr)
            continue;

        bool isTerrain = RenderParams.TerrainInfos.size() > 0 && pNode->isTerrain;

        for (const auto& primitive : pNode->pMesh->Primitives)
        {
            if (primitive.VertexCount == 0 && primitive.IndexCount == 0)
                continue;

            if(isTerrain)
            {
                Uint32 TerrainInfoIndex = 0;
                for(const auto& terrainInfo : RenderParams.TerrainInfos)
                {
                    const auto& Material  = GLTFModel.GetMaterials()[terrainInfo.MaterialIndex];
                    const auto  AlphaMode = Material.Attribs.AlphaMode;
                    if ((RenderParams.AlphaModes & (1u << AlphaMode)) == 0)
                        continue;

                    m_RenderLists[AlphaMode].emplace_back(primitive, *pNode, terrainInfo.MaterialIndex, TerrainInfoIndex);
                    TerrainInfoIndex++;
                }
            }
            else
            {
                const auto& Material  = GLTFModel.GetMaterials()[primitive.MaterialId];
                const auto  AlphaMode = Material.Attribs.AlphaMode;
                if ((RenderParams.AlphaModes & (1u << AlphaMode)) == 0)
                    continue;

                m_RenderLists[AlphaMode].emplace_back(primitive, *pNode, primitive.MaterialId, 0);
            }
        }
    }

    const std::array<SF_GLTF::Material::ALPHA_MODE, 3> AlphaModes //
        {
            SF_GLTF::Material::ALPHA_MODE_OPAQUE, // Opaque primitives - first
            SF_GLTF::Material::ALPHA_MODE_MASK,   // Alpha-masked primitives - second
            SF_GLTF::Material::ALPHA_MODE_BLEND,  // Transparent primitives - last (TODO: depth sorting)
        };

    IPipelineState*         pCurrPSO = nullptr;
    IShaderResourceBinding* pCurrSRB = nullptr;
    PSOKey                  CurrPsoKey;

    if (PrevTransforms == nullptr)
        PrevTransforms = &Transforms;

    for (auto AlphaMode : AlphaModes)
    {
        const auto& RenderList = m_RenderLists[AlphaMode];
        for (const auto& PrimRI : RenderList)
        {
            const auto& Node                 = PrimRI.Node;
            const auto& primitive            = PrimRI.Primitive;
            const auto& material             = GLTFModel.GetMaterials()[PrimRI.MaterialIndex];
            const auto& NodeGlobalMatrix     = Transforms.NodeGlobalMatrices[Node.Index];
            const auto& PrevNodeGlobalMatrix = PrevTransforms->NodeGlobalMatrices[Node.Index];

            if (pModelBindings != nullptr)
            {
                GLTFModel.SetVertexBuffersForNode(pCtx, &Node);
            }

            auto VertexAttribFlags = PSO_FLAG_NONE;
            for (Uint32 i = 0; i < GLTFModel.GetNumVertexAttributesForNode(&Node); ++i)
            {
                if (!GLTFModel.IsVertexAttributeEnabledForNode(&Node, i))
                    continue;
                const auto& Attrib = GLTFModel.GetVertexAttributeForNode(&Node, i);
                if (strcmp(Attrib.Name, SF_GLTF::PositionAttributeName) == 0)
                    VertexAttribFlags |= PSO_FLAG_NONE; // Position is always enabled
                else if (strcmp(Attrib.Name, SF_GLTF::NormalAttributeName) == 0)
                    VertexAttribFlags |= PSO_FLAG_USE_VERTEX_NORMALS;
                else if (strcmp(Attrib.Name, SF_GLTF::Texcoord0AttributeName) == 0)
                    VertexAttribFlags |= PSO_FLAG_USE_TEXCOORD0;
                else if (strcmp(Attrib.Name, SF_GLTF::Texcoord1AttributeName) == 0)
                    VertexAttribFlags |= PSO_FLAG_USE_TEXCOORD1;
                else if (strcmp(Attrib.Name, SF_GLTF::JointsAttributeName) == 0)
                    VertexAttribFlags |= PSO_FLAG_USE_JOINTS;
                else if (strcmp(Attrib.Name, SF_GLTF::VertexColorAttributeName) == 0)
                    VertexAttribFlags |= PSO_FLAG_USE_VERTEX_COLORS;
                else if (strcmp(Attrib.Name, SF_GLTF::TangentAttributeName) == 0)
                    VertexAttribFlags |= PSO_FLAG_USE_VERTEX_TANGENTS;
            }

            const auto FirstIndexLocation = GLTFModel.GetFirstIndexLocationForNode(&Node);
            const auto BaseVertex = GLTFModel.GetBaseVertexForNode(&Node);

            auto PSOFlags = VertexAttribFlags | GetMaterialPSOFlags(material);

            bool isTerrain = RenderParams.TerrainInfos.size() > 0 && Node.isTerrain;

            // These flags will be filtered out by RenderParams.Flags
            PSOFlags |= PSO_FLAG_USE_TEXTURE_ATLAS |
                PSO_FLAG_ENABLE_TEXCOORD_TRANSFORM |
                PSO_FLAG_CONVERT_OUTPUT_TO_SRGB |
                PSO_FLAG_ENABLE_TONE_MAPPING |
                PSO_FLAG_COMPUTE_MOTION_VECTORS |
                PSO_FLAG_USE_LIGHTS;
            if (m_Settings.EnableIBL)
            {
                PSOFlags |= PSO_FLAG_USE_IBL;
            }

            PSOFlags &= RenderParams.Flags;

            PSOFlags |= PSO_FLAG_ENABLE_SHADOWS;

            if(Node.isHeightmap)
            {
                PSOFlags |= PSO_FLAG_USE_HEIGHTMAP;
            }
            if (Node.Instances.size() > 0)
            {
                PSOFlags |= PSO_FLAG_USE_INSTANCING;
            }

            if (isTerrain)
            {
                PSOFlags |= PSO_FLAG_USE_TERRAINING;
                PSOFlags |= PSO_FLAG_USE_HEIGHTMAP;

                if(RenderParams.Flags & PSO_FLAG_USE_EGA_COLOR)
                {
                    PSOFlags |= PSO_FLAG_USE_EGA_COLOR;
                }
            }

            if (RenderParams.Wireframe)
                PSOFlags |= PSO_FLAG_UNSHADED;

            const PSOKey NewKey{PSOFlags, GltfAlphaModeToAlphaMode(AlphaMode), material.DoubleSided ? CULL_MODE_NONE : CULL_MODE_BACK, RenderParams.DebugView};
            if (NewKey != CurrPsoKey)
            {
                CurrPsoKey = NewKey;
                pCurrPSO   = nullptr;
            }

            auto psoCache = (isTerrain) ? m_TerrainPSOCache : (RenderParams.Wireframe ? m_WireframePSOCache : m_PbrPSOCache);

            if (pCurrPSO == nullptr)
            {
                pCurrPSO = psoCache.Get(NewKey, PsoCacheAccessor::GET_FLAG_CREATE_IF_NULL);
                VERIFY_EXPR(pCurrPSO != nullptr);
                pCtx->SetPipelineState(pCurrPSO);
            }
            else
            {
                VERIFY_EXPR(pCurrPSO == psoCache.Get(NewKey, PsoCacheAccessor::GET_FLAG_CREATE_IF_NULL));
            }

            if (pModelBindings != nullptr)
            {
                VERIFY(PrimRI.MaterialIndex < pModelBindings->MaterialSRB.size(),
                       "Material index is out of bounds. This most likely indicates that shader resources were initialized for a different model.");

                IShaderResourceBinding* const pSRB = pModelBindings->MaterialSRB[PrimRI.MaterialIndex];
                DEV_CHECK_ERR(pSRB != nullptr, "Unable to find SRB for GLTF material.");

                if(isTerrain)
                {
                    pSRB->GetVariableByName(SHADER_TYPE_VERTEX, "g_WaterHeightMap")->Set(RenderParams.pWaterHeightMap);
                }

                //if (pCurrSRB != pSRB)
                {
                    pCurrSRB = pSRB;
                    pCtx->CommitShaderResources(pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                }
            }
            else
            {
                VERIFY_EXPR(pCacheBindings != nullptr);
                if (pCurrSRB != pCacheBindings->pSRB)
                {
                    pCurrSRB = pCacheBindings->pSRB;
                    pCtx->CommitShaderResources(pCurrSRB, RESOURCE_STATE_TRANSITION_MODE_VERIFY);
                }
            }

            size_t JointCount = 0;
            if (Node.SkinTransformsIndex >= 0 && Node.SkinTransformsIndex < static_cast<int>(Transforms.Skins.size()))
            {
                const auto& JointMatrices = Transforms.Skins[Node.SkinTransformsIndex].JointMatrices;

                JointCount = JointMatrices.size();
                if (JointCount > m_Settings.MaxJointCount)
                {
                    LOG_WARNING_MESSAGE("The number of joints in the mesh (", JointCount, ") exceeds the maximum number (", m_Settings.MaxJointCount,
                                        ") reserved in the buffer. Increase MaxJointCount when initializing the renderer.");
                    JointCount = m_Settings.MaxJointCount;
                }

                if (JointCount != 0)
                {
                    MapHelper<float4x4> pJoints{pCtx, m_JointsBuffer, MAP_WRITE, MAP_FLAG_DISCARD};
                    WriteShaderMatrices(pJoints, JointMatrices.data(), JointCount, !m_Settings.PackMatrixRowMajor);
                    if ((CurrPsoKey.GetFlags() & PSO_FLAG_COMPUTE_MOTION_VECTORS) != 0)
                    {
                        const auto& PrevJointMatrices = PrevTransforms->Skins[Node.SkinTransformsIndex].JointMatrices;
                        WriteShaderMatrices(pJoints + m_Settings.MaxJointCount, PrevJointMatrices.data(), JointCount, !m_Settings.PackMatrixRowMajor);
                    }
                }
            }

            {
                void* pAttribsData = nullptr;
                pCtx->MapBuffer(m_PBRPrimitiveAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD, pAttribsData);
                if (pAttribsData != nullptr)
                {
                    static_assert(static_cast<PBR_WORKFLOW>(SF_GLTF::Material::PBR_WORKFLOW_METALL_ROUGH) == PBR_WORKFLOW_METALL_ROUGH, "SF_GLTF::Material::PBR_WORKFLOW_METALL_ROUGH != PBR_WORKFLOW_METALL_ROUGH");
                    static_assert(static_cast<PBR_WORKFLOW>(SF_GLTF::Material::PBR_WORKFLOW_SPEC_GLOSS) == PBR_WORKFLOW_SPEC_GLOSS, "SF_GLTF::Material::PBR_WORKFLOW_SPEC_GLOSS != PBR_WORKFLOW_SPEC_GLOSS");
                    static_assert(static_cast<PBR_WORKFLOW>(SF_GLTF::Material::PBR_WORKFLOW_UNLIT) == PBR_WORKFLOW_UNLIT, "SF_GLTF::Material::PBR_WORKFLOW_UNLIT != PBR_WORKFLOW_UNLIT");

                    const float4x4  NodeTransform     = NodeGlobalMatrix * RenderParams.ModelTransform;
                    const float4x4& PrevNodeTransform = (CurrPsoKey.GetFlags() & PSO_FLAG_COMPUTE_MOTION_VECTORS) != 0 ?
                        PrevNodeGlobalMatrix * RenderParams.ModelTransform :
                        NodeTransform;

                    PBRPrimitiveShaderAttribsData AttribsData{
                        CurrPsoKey.GetFlags(),
                        &NodeTransform,
                        &PrevNodeTransform,
                        static_cast<Uint32>(JointCount),
                    };
                    auto* pEndPtr = WritePBRPrimitiveShaderAttribs(pAttribsData, AttribsData, m_Settings.TextureAttribIndices, material, !m_Settings.PackMatrixRowMajor);

                    VERIFY(reinterpret_cast<uint8_t*>(pEndPtr) <= static_cast<uint8_t*>(pAttribsData) + m_PBRPrimitiveAttribsCB->GetDesc().Size,
                           "Not enough space in the buffer to store primitive attributes");

                    pCtx->UnmapBuffer(m_PBRPrimitiveAttribsCB, MAP_WRITE);
                }
                else
                {
                    UNEXPECTED("Unable to map the buffer");
                }

                if(isTerrain)
                {
                    MapHelper<HLSL::PBRTerrainAttribs> TerrainAttribs{ pCtx, m_TerrainAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD };

                    auto it = RenderParams.TerrainInfos.begin() + PrimRI.TerrainInfoIndex;
                    if(it != RenderParams.TerrainInfos.end())
                    {
                        TerrainAttribs->startBiomHeight = it->StartBiomHeight;
                        TerrainAttribs->endBiomHeight = it->EndBiomHeight; 
                        TerrainAttribs->textureOffsetX = RenderParams.TerrainTextureOffset.x;
                        TerrainAttribs->textureOffsetY = RenderParams.TerrainTextureOffset.y;
                        TerrainAttribs->waterHeight = RenderParams.WaterHeight;
                    }

                    for(int i = 0; i < 8; ++i)
                    {
                        TerrainAttribs->convertEgaColors[i] = float4(RenderParams.EgaColors[i], 1.0);
                    }
                }

                if(Node.Instances.size() > 0)
                {
                    MapHelper<HLSL::PBRInstanceAttribs> InstanceAttribs{ pCtx, m_InstanceAttribsSB, MAP_WRITE, MAP_FLAG_DISCARD };
                    for(int i = 0; i < Node.Instances.size(); ++i)
                    {
                        InstanceAttribs[i].NodeMatrix = Node.Instances[i].NodeMatrix.Transpose();
                        InstanceAttribs[i].HeightmapAttribs.ScaleX = Node.Instances[i].ScaleX;
                        InstanceAttribs[i].HeightmapAttribs.ScaleY = Node.Instances[i].ScaleY;
                        InstanceAttribs[i].HeightmapAttribs.OffsetX = Node.Instances[i].OffsetX;
                        InstanceAttribs[i].HeightmapAttribs.OffsetY = Node.Instances[i].OffsetY;
                        InstanceAttribs[i].PlanetLocation = Node.Instances[i].PlanetLocation;
                    }

                    StateTransitionDesc Barriers[] = {
                        {m_InstanceAttribsSB, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_SHADER_RESOURCE, STATE_TRANSITION_FLAG_UPDATE_STATE},
                    };
                    pCtx->TransitionResourceStates(_countof(Barriers), Barriers);
                }
                else if(Node.isHeightmap)
                {
                    MapHelper<HLSL::PBRHeightmapAttribs> HeightmapAttribs{ pCtx, m_HeightmapAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD };
                    HeightmapAttribs->ScaleX = Node.HeightmapScaleX.x;
                    HeightmapAttribs->ScaleY = Node.HeightmapScaleY.y;
                    HeightmapAttribs->OffsetX = Node.HeightmapOffsetX.x;
                    HeightmapAttribs->OffsetY = Node.HeightmapOffsetY.y;
                }
            }

            if (primitive.HasIndices())
            {
                DrawIndexedAttribs drawAttrs{primitive.IndexCount, VT_UINT32, DRAW_FLAG_VERIFY_ALL};
                drawAttrs.FirstIndexLocation = FirstIndexLocation + primitive.FirstIndex;
                drawAttrs.BaseVertex         = BaseVertex;
                if(Node.Instances.size() > 0)
                {
                    drawAttrs.FirstInstanceLocation = 0;
                    drawAttrs.NumInstances = static_cast<Uint32>(Node.Instances.size());
                }
                pCtx->DrawIndexed(drawAttrs);
            }
            else
            {
                DrawAttribs drawAttrs{primitive.VertexCount, DRAW_FLAG_VERIFY_ALL};
                drawAttrs.StartVertexLocation = BaseVertex;
                pCtx->Draw(drawAttrs);
            }
        }
    }
}

template <typename ShaderStructType, typename HostStructType>
Uint8* WriteShaderAttribs(Uint8* pDstPtr, HostStructType* pSrc, const char* DebugName)
{
    static_assert(sizeof(ShaderStructType) == sizeof(HostStructType), "Size of HLSL and C++ structures must be the same");
    if (pSrc != nullptr)
    {
        memcpy(pDstPtr, pSrc, sizeof(ShaderStructType));
    }
    else
    {
        UNEXPECTED("Shader attribute ", DebugName, " is not initialized in the material");
        memset(pDstPtr, 0, sizeof(ShaderStructType));
    }
    static_assert(sizeof(ShaderStructType) % 16 == 0, "Size structure must be a multiple of 16");
    return pDstPtr + sizeof(ShaderStructType);
}


void* SF_GLTF_PBR_Renderer::WritePBRPrimitiveShaderAttribs(void*                                           pDstShaderAttribs,
                                                        const PBRPrimitiveShaderAttribsData&            AttribsData,
                                                        const std::array<int, TEXTURE_ATTRIB_ID_COUNT>& TextureAttribIndices,
                                                        const SF_GLTF::Material&                           Material,
                                                        bool                                            TransposeMatrices)
{
    // When adding new members, don't forget to update PBR_Renderer::GetPBRPrimitiveAttribsSize!

    //struct PBRPrimitiveAttribs
    //{
    //    GLTFNodeShaderTransforms Transforms;
    //    float4x4                 PrevNodeMatrix; // #if ENABLE_MOTION_VECTORS
    //    struct PBRMaterialShaderInfo
    //    {
    //        PBRMaterialBasicAttribs        Basic;
    //        PBRMaterialSheenAttribs        Sheen;        // #if ENABLE_SHEEN
    //        PBRMaterialAnisotropyAttribs   Anisotropy;   // #if ENABLE_ANISOTROPY
    //        PBRMaterialIridescenceAttribs  Iridescence;  // #if ENABLE_IRIDESCENCE
    //        PBRMaterialTransmissionAttribs Transmission; // #if ENABLE_TRANSMISSION
    //        PBRMaterialVolumeAttribs       Volume;       // #if ENABLE_VOLUME
    //        PBRMaterialTextureAttribs Textures[PBR_NUM_TEXTURE_ATTRIBUTES];
    //    } Material;
    //    UserDefined CustomData;
    //};

    Uint8* pDstPtr = reinterpret_cast<Uint8*>(pDstShaderAttribs);

    {
        HLSL::GLTFNodeShaderTransforms* pDstTransforms = reinterpret_cast<HLSL::GLTFNodeShaderTransforms*>(pDstPtr);
        if (AttribsData.NodeMatrix != nullptr)
        {
            WriteShaderMatrix(&pDstTransforms->NodeMatrix, *AttribsData.NodeMatrix, TransposeMatrices);
        }
        else
        {
            UNEXPECTED("Node matrix must not be null");
        }
        pDstTransforms->JointCount = static_cast<int>(AttribsData.JointCount);

        static_assert(sizeof(HLSL::GLTFNodeShaderTransforms) % 16 == 0, "Size of HLSL::GLTFNodeShaderTransforms must be a multiple of 16");
        pDstPtr += sizeof(HLSL::GLTFNodeShaderTransforms);
    }

    if (AttribsData.PSOFlags & PSO_FLAG_COMPUTE_MOTION_VECTORS)
    {
        if (AttribsData.PrevNodeMatrix != nullptr)
        {
            WriteShaderMatrix(pDstPtr, *AttribsData.PrevNodeMatrix, TransposeMatrices);
        }
        else
        {
            UNEXPECTED("Prev node matrix must not be null when motion vectors are enabled");
        }
        pDstPtr += sizeof(float4x4);
    }

    if (AttribsData.pMaterialBasicAttribsDstPtr != nullptr)
        *AttribsData.pMaterialBasicAttribsDstPtr = reinterpret_cast<HLSL::PBRMaterialBasicAttribs*>(pDstPtr);
    pDstPtr = WriteShaderAttribs<HLSL::PBRMaterialBasicAttribs>(pDstPtr, &Material.Attribs, "Basic Attribs");

    if (AttribsData.PSOFlags & PSO_FLAG_ENABLE_SHEEN)
    {
        pDstPtr = WriteShaderAttribs<HLSL::PBRMaterialSheenAttribs>(pDstPtr, Material.Sheen.get(), "Sheen Attribs");
    }

    if (AttribsData.PSOFlags & PSO_FLAG_ENABLE_ANISOTROPY)
    {
        pDstPtr = WriteShaderAttribs<HLSL::PBRMaterialAnisotropyAttribs>(pDstPtr, Material.Anisotropy.get(), "Anisotropy Attribs");
    }

    if (AttribsData.PSOFlags & PSO_FLAG_ENABLE_IRIDESCENCE)
    {
        pDstPtr = WriteShaderAttribs<HLSL::PBRMaterialIridescenceAttribs>(pDstPtr, Material.Iridescence.get(), "Iridescence Attribs");
    }

    if (AttribsData.PSOFlags & PSO_FLAG_ENABLE_TRANSMISSION)
    {
        pDstPtr = WriteShaderAttribs<HLSL::PBRMaterialTransmissionAttribs>(pDstPtr, Material.Transmission.get(), "Transmission Attribs");
    }

    if (AttribsData.PSOFlags & PSO_FLAG_ENABLE_VOLUME)
    {
        pDstPtr = WriteShaderAttribs<HLSL::PBRMaterialVolumeAttribs>(pDstPtr, Material.Volume.get(), "Volume Attribs");
    }

    {
        HLSL::PBRMaterialTextureAttribs* pDstTextures = reinterpret_cast<HLSL::PBRMaterialTextureAttribs*>(pDstPtr);
        static_assert(sizeof(HLSL::PBRMaterialTextureAttribs) % 16 == 0, "Size of HLSL::PBRMaterialTextureAttribs must be a multiple of 16");

        Uint32 NumTextureAttribs = 0;
        ProcessTexturAttribs(AttribsData.PSOFlags, [&](int CurrIndex, SF_PBR_Renderer::TEXTURE_ATTRIB_ID AttribId) //
                             {
                                 const int SrcAttribIndex = TextureAttribIndices[AttribId];
                                 if (SrcAttribIndex < 0)
                                 {
                                     UNEXPECTED("Shader attribute ", Uint32{AttribId}, " is not initialized");
                                     return;
                                 }

                                 static_assert(sizeof(HLSL::PBRMaterialTextureAttribs) == sizeof(SF_GLTF::Material::TextureShaderAttribs),
                                               "The sizeof(HLSL::PBRMaterialTextureAttribs) is inconsistent with sizeof(SF_GLTF::Material::TextureShaderAttribs)");
                                 memcpy(pDstTextures + CurrIndex, &Material.GetTextureAttrib(SrcAttribIndex), sizeof(HLSL::PBRMaterialTextureAttribs));
                                 ++NumTextureAttribs;
                             });

        pDstPtr = reinterpret_cast<Uint8*>(pDstTextures + NumTextureAttribs);
    }

    {
        if (AttribsData.CustomData != nullptr)
        {
            VERIFY_EXPR(AttribsData.CustomDataSize > 0);
            memcpy(pDstPtr, AttribsData.CustomData, AttribsData.CustomDataSize);
        }
        pDstPtr += AttribsData.CustomDataSize;
    }

    return pDstPtr;
}

void SF_GLTF_PBR_Renderer::WritePBRLightShaderAttribs(const PBRLightShaderAttribsData& AttribsData,
                                                   HLSL::PBRLightAttribs*           pShaderAttribs)
{
    VERIFY_EXPR(pShaderAttribs != nullptr);
    VERIFY_EXPR(AttribsData.Light != nullptr);
    const auto& Light = *AttribsData.Light;

    pShaderAttribs->Type = static_cast<int>(Light.Type);

    if (AttribsData.Position != nullptr)
    {
        pShaderAttribs->PosX = AttribsData.Position->x;
        pShaderAttribs->PosY = AttribsData.Position->y;
        pShaderAttribs->PosZ = AttribsData.Position->z;
    }

    if (AttribsData.Direction != nullptr)
    {
        pShaderAttribs->DirectionX = AttribsData.Direction->x;
        pShaderAttribs->DirectionY = AttribsData.Direction->y;
        pShaderAttribs->DirectionZ = AttribsData.Direction->z;
    }

    pShaderAttribs->ShadowMapIndex = AttribsData.ShadowMapIndex;
    pShaderAttribs->NumCascades = AttribsData.NumCascades;

    auto Intensity = Light.Intensity;
    if (pShaderAttribs->Type != LIGHT_TYPE_DIRECTIONAL)
    {
        Intensity *= AttribsData.DistanceScale * AttribsData.DistanceScale;
    }
    pShaderAttribs->IntensityR = Light.Color.r * Intensity;
    pShaderAttribs->IntensityG = Light.Color.g * Intensity;
    pShaderAttribs->IntensityB = Light.Color.b * Intensity;

    float Range            = Light.Range * AttribsData.DistanceScale;
    float Range2           = Range * Range;
    pShaderAttribs->Range4 = Range2 * Range2;

    float SpotAngleScale  = 1.f / std::max(0.001f, std::cos(Light.InnerConeAngle) - std::cos(Light.OuterConeAngle));
    float SpotAngleOffset = -std::cos(Light.OuterConeAngle) * SpotAngleScale;

    pShaderAttribs->SpotAngleOffset = SpotAngleOffset;
    pShaderAttribs->SpotAngleScale  = SpotAngleScale;
}

} // namespace Diligent