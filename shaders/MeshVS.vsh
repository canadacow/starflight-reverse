#include "BasicStructures.fxh"
#include "PBR_Structures.fxh"
#include "RenderPBR_Structures.fxh"

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

struct VSInput
{
    float3 Position  : ATTRIB0;
    float3 Normal    : ATTRIB1;
    float2 TexCoord  : ATTRIB2;
    float4 Joint0    : ATTRIB3;
    float4 Weight0   : ATTRIB4;
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
    VSOut.TexCoord     = VSIn.TexCoord;

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

    float4 locPos = mul(float4(VSIn.Position, 1.0), Transform);
    locPos = locPos.xyzw / locPos.w;
    VSOut.PositionPS = mul(locPos, g_CameraAttribs.mViewProj);
};
