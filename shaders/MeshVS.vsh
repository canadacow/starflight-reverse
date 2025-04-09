#include "BasicStructures.fxh"
#include "SF_PBR_Structures.fxh"
#include "SF_PBR_Shading.fxh"
#include "VertexProcessing.fxh"
#include "SF_RenderPBR_Structures.fxh"

cbuffer cbCameraAttribs
{
    CameraAttribs g_CameraAttribs;
};

cbuffer cbPrimitiveAttribs
{
    GLTFNodeShaderTransforms g_Primitive;
};

#define PRIMITIVE g_Primitive

#define MAX_JOINT_COUNT 64

cbuffer cbJointTransforms
{
    float4x4 g_Joints[MAX_JOINT_COUNT];
};

#if USE_TERRAINING
cbuffer cbTerrainAttribs
{
    PBRTerrainAttribs g_Terrain;
}
#endif

#if USE_HEIGHTMAP
cbuffer cbHeightmapAttribs
{
    PBRHeightmapAttribs g_HeightmapAttribs;
};
Texture2D g_Heightmap;
SamplerState g_Heightmap_sampler : register(s0);

#endif

#if USE_INSTANCING
StructuredBuffer<PBRInstanceAttribs> instanceBuffer : register(t1);
#endif

struct VSInput
{
    float3 Position  : ATTRIB0;
    float3 Normal    : ATTRIB1;
    float2 UV0       : ATTRIB2;
    float4 Joint0    : ATTRIB3;
    float4 Weight0   : ATTRIB4;
    uint InstanceID  : SV_InstanceID;
};

struct VSOutput
{
    float4 PositionPS 	        : SV_Position;
    float3 PosInLightViewSpace 	: LIGHT_SPACE_POS;
    float3 NormalWS 	        : NORMALWS;
    float2 TexCoord 	        : TEXCOORD;
};

void MeshVS(in  VSInput  VSIn,
            out VSOutput VSOut)
{
    VSOut.PosInLightViewSpace = float3(0.0, 0.0, 0.0);
    VSOut.NormalWS     = VSIn.Normal;
    VSOut.TexCoord     = VSIn.UV0;

    float3 Normal = float3(0.0, 0.0, 1.0);

    float4x4 Transform = PRIMITIVE.NodeMatrix;
    
    if (PRIMITIVE.JointCount > 0)
    {
        // Mesh is skinned
        float4x4 SkinMat = 
            VSIn.Weight0.x * g_Joints[int(VSIn.Joint0.x)] +
            VSIn.Weight0.y * g_Joints[int(VSIn.Joint0.y)] +
            VSIn.Weight0.z * g_Joints[int(VSIn.Joint0.z)] +
            VSIn.Weight0.w * g_Joints[int(VSIn.Joint0.w)];
        Transform = mul(SkinMat, Transform);
    }

    float3 adjustedPos = VSIn.Position;

#if USE_INSTANCING
    PBRInstanceAttribs instance = instanceBuffer[VSIn.InstanceID];
    // Apply the node matrix transformation to the position
    // This ensures the vertex position is correctly transformed by the instance's matrix
    adjustedPos = mul(float4(adjustedPos, 1.0), instance.NodeMatrix).xyz;
#endif

#if USE_HEIGHTMAP
    #if USE_INSTANCING
        float2 adjustedUV = VSIn.UV0 * float2(instance.HeightmapAttribs.ScaleX, instance.HeightmapAttribs.ScaleY) + float2(instance.HeightmapAttribs.OffsetX, instance.HeightmapAttribs.OffsetY);
    #else // USE_INSTANCING
        float2 adjustedUV = VSIn.UV0 * float2(g_HeightmapAttribs.ScaleX, g_HeightmapAttribs.ScaleY) + float2(g_HeightmapAttribs.OffsetX, g_HeightmapAttribs.OffsetY);
    #endif // USE_INSTANCING

    #if USE_TERRAINING
        adjustedUV += float2(g_Terrain.textureOffsetX, g_Terrain.textureOffsetY);
    #endif

    float height = textureBicubic(g_Heightmap, g_Heightmap_sampler, adjustedUV).r;

    
    if(height < g_Terrain.waterHeight)
    {
        // Water is always flush with land
        adjustedPos += float3(0.0, g_Terrain.waterHeight, 0.0);
    }
    else
    {
        adjustedPos += float3(0.0, height, 0.0);
    }

    #if USE_TERRAINING
        // Apply global terrain curvature based on distance from camera
        float3 cameraPos = g_CameraAttribs.f4Position.xyz;
        float2 horizontalDelta = float2(adjustedPos.x - cameraPos.x, adjustedPos.z - cameraPos.z);
        float distanceSquared = dot(horizontalDelta, horizontalDelta);
        
        // Calculate curvature factor - reduces height as distance increases
        // The 0.00001 factor controls how strong the curvature effect is
        float curvatureFactor = distanceSquared * 0.000005;
        
        // Apply the curvature by reducing the y-coordinate based on distance
        adjustedPos.y -= curvatureFactor;
    #endif
#endif // USE_HEIGHTMAP

    GLTF_TransformedVertex TransformedVert = GLTF_TransformVertex(adjustedPos, Normal, Transform);    
    VSOut.PositionPS = mul(float4(TransformedVert.WorldPos, 1.0), g_CameraAttribs.mViewProj);
};
