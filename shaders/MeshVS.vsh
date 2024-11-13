#include "BasicStructures.fxh"
#include "SF_PBR_Structures.fxh"
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

#if USE_HEIGHTMAP
cbuffer cbHeightmapAttribs
{
    PBRHeightmapAttribs g_HeightmapAttribs;
};
Texture2D g_Heightmap;
SamplerState g_Heightmap_sampler : register(s0);

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

#if USE_HEIGHTMAP
    #if USE_INSTANCING
        PBRInstanceAttribs instance = instanceBuffer[VSIn.InstanceID];
        Transform = mul(instance.NodeMatrix, Transform);
        float2 adjustedUV = VSIn.UV0 * float2(instance.HeightmapAttribs.ScaleX, instance.HeightmapAttribs.ScaleY) + float2(instance.HeightmapAttribs.OffsetX, instance.HeightmapAttribs.OffsetY);
    #else // USE_INSTANCING
        float2 adjustedUV = VSIn.UV0 * float2(g_HeightmapAttribs.ScaleX, g_HeightmapAttribs.ScaleY) + float2(g_HeightmapAttribs.OffsetX, g_HeightmapAttribs.OffsetY);
    #endif // USE_INSTANCING

    float height = textureBicubic(g_Heightmap, g_Heightmap_sampler, adjustedUV).r;
    float3 adjustedPos = VSIn.Position + float3(0.0, height, 0.0);
#else
    float3 adjustedPos = VSIn.Position;
#endif // USE_HEIGHTMAP

    GLTF_TransformedVertex TransformedVert = GLTF_TransformVertex(adjustedPos, Normal, Transform);    
    VSOut.PositionPS = mul(float4(TransformedVert.WorldPos, 1.0), g_CameraAttribs.mViewProj);
};
