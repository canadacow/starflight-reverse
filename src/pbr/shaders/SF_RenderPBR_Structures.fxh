#ifndef _RENDER_PBR_STRUCTURES_FXH_
#define _RENDER_PBR_STRUCTURES_FXH_

// #include "BasicStructures.fxh"
// #include "PBR_Structures.fxh"

#ifndef COMPUTE_MOTION_VECTORS
#   define COMPUTE_MOTION_VECTORS 0
#endif

struct PBRFrameAttribs
{
    CameraAttribs               Camera;
    CameraAttribs               PrevCamera; // Previous frame camera used to compute motion vectors
    PBRRendererShaderParameters Renderer;
       
#if defined(PBR_MAX_LIGHTS) && PBR_MAX_LIGHTS > 0    
    PBRLightAttribs Lights[PBR_MAX_LIGHTS];
#endif
    
#if defined(ENABLE_SHADOWS) && ENABLE_SHADOWS && defined(PBR_MAX_SHADOW_MAPS) && PBR_MAX_SHADOW_MAPS > 0
    PBRShadowMapInfo ShadowMaps[PBR_MAX_SHADOW_MAPS];
#endif
};
#ifdef CHECK_STRUCT_ALIGNMENT
	CHECK_STRUCT_ALIGNMENT(PBRFrameAttribs);
#endif


struct PBRPrimitiveAttribs
{
    GLTFNodeShaderTransforms Transforms;
#if COMPUTE_MOTION_VECTORS
    float4x4                 PrevNodeMatrix;
#endif
    PBRMaterialShaderInfo    Material;

    float4 CustomData;
};
#ifdef CHECK_STRUCT_ALIGNMENT
	CHECK_STRUCT_ALIGNMENT(PBRPrimitiveAttribs);
#endif

struct PBRHeightmapAttribs
{
    float ScaleX;
    float ScaleY;
    float OffsetX;
    float OffsetY;
    float HeightFactor;
    float padding0;
    float padding1;
    float padding2;
};
#ifdef CHECK_STRUCT_ALIGNMENT
	CHECK_STRUCT_ALIGNMENT(PBRHeightmapAttribs);
#endif

struct PBRInstanceAttribs
{
    float4x4            NodeMatrix;
    PBRHeightmapAttribs HeightmapAttribs;
    int2 PlanetLocation;
    int2 IntPadding;
    float4 Padding[2]; // 64 bytes padding
};
#ifdef CHECK_STRUCT_ALIGNMENT
	CHECK_STRUCT_ALIGNMENT(PBRInstanceAttribs);
#endif

struct PBRTessellationParams
{
    float MaxTessellationFactor;  // Maximum tessellation factor
    float MinDistance;            // Distance at which maximum tessellation is applied
    float MaxDistance;            // Distance at which minimum tessellation is applied
    float FalloffExponent;        // Controls the rate of tessellation falloff with distance
};
#ifdef CHECK_STRUCT_ALIGNMENT
    CHECK_STRUCT_ALIGNMENT(PBRTessellationParams);
#endif

struct PBRTerrainAttribs
{
    float startBiomHeight;
    float endBiomHeight;
    float textureOffsetX;
    float textureOffsetY;

    float3 egaColor;
    float waterHeight;

    float4 convertEgaColors[8];
};
#ifdef CHECK_STRUCT_ALIGNMENT
	CHECK_STRUCT_ALIGNMENT(PBRTerrainAttribs);
#endif

#endif // _RENDER_PBR_STRUCTURES_FXH_
