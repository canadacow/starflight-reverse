#include "BasicStructures.fxh"
#include "VertexProcessing.fxh"
#include "SF_PBR_Structures.fxh"
#include "SF_RenderPBR_Structures.fxh"

#include "VSInputStruct.generated"
//struct VSInput
//{
//    float3 Pos     : ATTRIB0;
//    float3 Normal  : ATTRIB1;
//    float2 UV0     : ATTRIB2;
//    float2 UV1     : ATTRIB3;
//    float4 Joint0  : ATTRIB4;
//    float4 Weight0 : ATTRIB5;
//    float4 Color   : ATTRIB6; // May be float3
//    float3 Tangent : ATTRIB7;
//    uint InstanceID : SV_InstanceID;
//};

#include "VSOutputStruct.generated"
// struct VSOutput
// {
//     float4 ClipPos     : SV_Position;
//     float3 WorldPos    : WORLD_POS;
//     float4 Color       : COLOR;
//     float3 Normal      : NORMAL;
//     float2 UV0         : UV0;
//     float2 UV1         : UV1;
//     
//     float3 Tangent     : TANGENT;
//     float4 PrevClipPos : PREV_CLIP_POS;
//     float Height : HEIGHT;
// };

#ifdef USE_HEIGHTMAP
cbuffer cbHeightmapAttribs
{
    PBRHeightmapAttribs g_HeightmapAttribs;
}
Texture2D g_Heightmap;
SamplerState g_Heightmap_sampler;

float4 sampleBicubic(float v) {
    float4 n = float4(1.0, 2.0, 3.0, 4.0) - v;
    float4 s = n * n * n;
    float4 o;
    o.x = s.x;
    o.y = s.y - 4.0 * s.x;
    o.z = s.z - 4.0 * s.y + 6.0 * s.x;
    o.w = 6.0 - o.x - o.y - o.z;
    return o;
}

float4 textureBicubic(Texture2D tex, SamplerState samplerState, float2 st)
{
    int2 texResolution;
    tex.GetDimensions(texResolution.x, texResolution.y);
    float2 pixel = 1.0 / float2(texResolution);

    st = st * texResolution - 0.5;

    float2 fxy = frac(st);
    st -= fxy;

    float4 xcubic = sampleBicubic(fxy.x);
    float4 ycubic = sampleBicubic(fxy.y);

    float4 c = st.xxyy + float2 (-0.5, 1.5).xyxy;

    float4 s = float4(xcubic.xz + xcubic.yw, ycubic.xz + ycubic.yw);
    float4 offset = c + float4(xcubic.yw, ycubic.yw) / s;

    offset *= pixel.xxyy;
    
    float4 sample0 = tex.Sample(samplerState, offset.xz);
    float4 sample1 = tex.Sample(samplerState, offset.yz);
    float4 sample2 = tex.Sample(samplerState, offset.xw);
    float4 sample3 = tex.Sample(samplerState, offset.yw);

    float sx = s.x / (s.x + s.y);
    float sy = s.z / (s.z + s.w);

    return lerp(    lerp(sample3, sample2, sx), 
                    lerp(sample1, sample0, sx), 
                    sy);
}

#endif

#if USE_INSTANCING
StructuredBuffer<PBRInstanceAttribs> instanceBuffer : register(t1);
#endif

#if USE_TERRAINING
cbuffer cbTerrainAttribs
{
    PBRTerrainAttribs g_Terrain;
}
#endif

#ifndef MAX_JOINT_COUNT
#   define MAX_JOINT_COUNT 64
#endif

cbuffer cbFrameAttribs 
{
    PBRFrameAttribs g_Frame;
}

cbuffer cbPrimitiveAttribs
{
#if PRIMITIVE_ARRAY_SIZE > 0
    PBRPrimitiveAttribs g_Primitive[PRIMITIVE_ARRAY_SIZE];
#else
    PBRPrimitiveAttribs g_Primitive;
#endif
}


#if PRIMITIVE_ARRAY_SIZE > 0
// PRIMITIVE_ID is defined by the host as gl_DrawID or gl_DrawIDARB
#   define PRIMITIVE g_Primitive[PRIMITIVE_ID]
#else
#   define PRIMITIVE g_Primitive
#endif


#if MAX_JOINT_COUNT > 0 && USE_JOINTS
cbuffer cbJointTransforms
{
    float4x4 g_Joints[MAX_JOINT_COUNT];
#if COMPUTE_MOTION_VECTORS
    float4x4 g_PrevJoints[MAX_JOINT_COUNT];
#endif
}
#endif

float4 GetVertexColor(float3 Color)
{
    return float4(Color, 1.0);
}

float4 GetVertexColor(float4 Color)
{
    return Color;
}

void main(in  VSInput  VSIn,
          out VSOutput VSOut)
{
    // Warning: moving this block into GLTF_TransformVertex() function causes huge
    // performance degradation on Vulkan because glslang/SPIRV-Tools are apparently not able
    // to eliminate the copy of g_Transforms structure.
    float4x4 Transform = PRIMITIVE.Transforms.NodeMatrix;

#if COMPUTE_MOTION_VECTORS
    float4x4 PrevTransform = PRIMITIVE.PrevNodeMatrix;
#endif
    
#if MAX_JOINT_COUNT > 0 && USE_JOINTS
    if (PRIMITIVE.Transforms.JointCount > 0)
    {
        // Mesh is skinned
        float4x4 SkinMat = 
            VSIn.Weight0.x * g_Joints[int(VSIn.Joint0.x)] +
            VSIn.Weight0.y * g_Joints[int(VSIn.Joint0.y)] +
            VSIn.Weight0.z * g_Joints[int(VSIn.Joint0.z)] +
            VSIn.Weight0.w * g_Joints[int(VSIn.Joint0.w)];
        Transform = mul(SkinMat, Transform);

#       if COMPUTE_MOTION_VECTORS
        {
            float4x4 PrevSkinMat = 
                VSIn.Weight0.y * g_PrevJoints[int(VSIn.Joint0.y)] +
                VSIn.Weight0.x * g_PrevJoints[int(VSIn.Joint0.x)] +
                VSIn.Weight0.z * g_PrevJoints[int(VSIn.Joint0.z)] +
                VSIn.Weight0.w * g_PrevJoints[int(VSIn.Joint0.w)];
            PrevTransform = mul(PrevSkinMat, PrevTransform);
        }
#       endif
    }
#endif

#if USE_VERTEX_NORMALS
    float3 Normal = VSIn.Normal;
#else
    float3 Normal = float3(0.0, 0.0, 1.0);
#endif

#if USE_HEIGHTMAP

    #if USE_INSTANCING
        PBRInstanceAttribs instance = instanceBuffer[VSIn.InstanceID];
        Transform = mul(instance.NodeMatrix, Transform);
        float2 adjustedUV = VSIn.UV0 * float2(instance.HeightmapAttribs.ScaleX, instance.HeightmapAttribs.ScaleY) + float2(instance.HeightmapAttribs.OffsetX, instance.HeightmapAttribs.OffsetY);
    #else // USE_INSTANCING
        float2 adjustedUV = VSIn.UV0 * float2(g_HeightmapAttribs.ScaleX, g_HeightmapAttribs.ScaleY) + float2(g_HeightmapAttribs.OffsetX, g_HeightmapAttribs.OffsetY);
    #endif // USE_INSTANCING

    #if USE_TERRAINING
        adjustedUV += float2(g_Terrain.textureOffsetX, g_Terrain.textureOffsetY);
    #endif

    float height = textureBicubic(g_Heightmap, g_Heightmap_sampler, adjustedUV).r;
    float3 adjustedPos = VSIn.Pos + float3(0.0, height, 0.0);
    VSOut.Height = height;
#else
    float3 adjustedPos = VSIn.Pos;
#endif // USE_HEIGHTMAP

    GLTF_TransformedVertex TransformedVert = GLTF_TransformVertex(adjustedPos, Normal, Transform);    
    VSOut.ClipPos = mul(float4(TransformedVert.WorldPos, 1.0), g_Frame.Camera.mViewProj);

#if COMPUTE_MOTION_VECTORS
    GLTF_TransformedVertex PrevTransformedVert = GLTF_TransformVertex(VSIn.Pos, Normal, PrevTransform);
    VSOut.PrevClipPos  = mul(float4(PrevTransformedVert.WorldPos, 1.0), g_Frame.PrevCamera.mViewProj);
#endif  
    
    VSOut.WorldPos = TransformedVert.WorldPos;

#if USE_VERTEX_COLORS
    VSOut.Color    = GetVertexColor(VSIn.Color);
#endif

#if USE_VERTEX_NORMALS
    VSOut.Normal   = TransformedVert.Normal;
#endif

#if USE_TEXCOORD0
    VSOut.UV0      = VSIn.UV0;
#if USE_TERRAINING
#if !defined(USE_HEIGHTMAP) || !defined(USE_INSTANCING) && !defined(USE_TEXCOORD1)
    #error "Height map, instancing and texcoord1 must be enabled"
#endif
    int instanceX = VSIn.InstanceID % 61;
    int instanceY = VSIn.InstanceID / 61;
    float2 megaUV = VSIn.UV0 * float2(instance.HeightmapAttribs.ScaleX, instance.HeightmapAttribs.ScaleY) + float2(instance.HeightmapAttribs.OffsetX, instance.HeightmapAttribs.OffsetY);

    #if USE_TERRAINING
        megaUV += float2(g_Terrain.textureOffsetX, g_Terrain.textureOffsetY);
    #endif

    VSOut.UV2 = VSIn.UV0;
    VSOut.UV0 = float2(frac(megaUV.x * 9.0 * 38.0), frac(megaUV.y * 9.0 * 14.9));
    VSOut.UV1 = megaUV;

#endif // USE_TERRAINING
#endif // USE_TEXCOORD0

#if USE_TEXCOORD1 && !USE_TERRAINING
    VSOut.UV1      = VSIn.UV1;
#endif
    
#if USE_VERTEX_TANGENTS
    VSOut.Tangent  = normalize(mul(VSIn.Tangent, float3x3(Transform[0].xyz, Transform[1].xyz, Transform[2].xyz)));
#endif

#ifdef USE_GL_POINT_SIZE
#   if defined(GLSL) || defined(GL_ES)
        // If gl_PointSize is not defined, points are not rendered in GLES
        gl_PointSize = g_Frame.Renderer.PointSize;
#   else
        VSOut.PointSize = g_Frame.Renderer.PointSize;
#   endif
#endif
    
#if PRIMITIVE_ARRAY_SIZE > 0
    VSOut.PrimitiveID = PRIMITIVE_ID;
#endif
}
