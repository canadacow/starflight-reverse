#include "CommsSystem.h"
#include <algorithm>

using namespace Diligent;

// Returns a literal string containing the JSON animation keyframes for the comms system head movement
const char* GetCommsHeadAnimationJSON()
{
    return R"json(
{
  "duration": 8.3,
  "keyframes": [
    {
      "time": 0.0,
      "pitch": 0,
      "yaw": 0,
      "description": "neutral start"
    },
    {
      "time": 1.5,
      "pitch": 5,
      "yaw": -3,
      "description": "slight up/left - 'establish'"
    },
    {
      "time": 3.2,
      "pitch": 8,
      "yaw": 0,
      "description": "upward - 'heaven'"
    },
    {
      "time": 4.0,
      "pitch": 2,
      "yaw": 12,
      "description": "turn toward listener - 'you'"
    },
    {
      "time": 5.5,
      "pitch": 0,
      "yaw": 8,
      "description": "slight right - 'Group 9'"
    },
    {
      "time": 6.5,
      "pitch": -2,
      "yaw": 0,
      "description": "slight down - 'but'"
    },
    {
      "time": 8.0,
      "pitch": -8,
      "yaw": -5,
      "description": "down/away - disappointment"
    },
    {
      "time": 8.3,
      "pitch": -8,
      "yaw": -5,
      "description": "hold final position"
    }
  ]
}
)json";
}

// Forward declarations of graphics utility functions
float4x4 GetAdjustedProjectionMatrix(float FOV, float NearPlane, float FarPlane);
float4x4 GetSurfacePretransformMatrix(const float3& f3CameraViewAxis);
void InitModel(std::string modelPath, SFModel& model, int defaultCameraIndex);

void Diligent::InitComms(SFModel& comms)
{
    InitModel("mechan9.glb", comms, 0);
}

void Diligent::UpdateComms(VulkanContext::frame_id_t inFlightIndex, 
    VulkanContext::frame_id_t frameCount, 
    double currentTimeInSeconds,
    SF_GLTF_PBR_Renderer::RenderInfo& renderParams,
    std::unique_ptr<HLSL::CameraAttribs[]>& cameraAttribs,
    SFModel& comms,
    int2 screenSize)
{
    float4x4 RotationMatrixCam = float4x4::Identity();
    float4x4 RotationMatrixModel = float4x4::Identity();

    // Find and rotate the Head node BEFORE computing transforms
    const auto& scene = comms.model->GetScenes()[renderParams.SceneIndex];
    for (const auto* pNode : scene.LinearNodes)
    {
        if (pNode->Name == "Head")
        {
            // Create rotation around Y-axis (you can change this to any axis)
            float rotationAngle = static_cast<float>(currentTimeInSeconds) * 1.0f; // 1 radian per second
            
            // Modify the node's rotation (this is non-const, so we need to cast)
            SF_GLTF::Node* headNode = const_cast<SF_GLTF::Node*>(pNode);
            headNode->Rotation = QuaternionF::RotationFromAxisAngle(float3{0, 1, 0}, rotationAngle);
            
            break; // Found the head, no need to continue
        }
    }

    // Now compute transforms with the modified rotation
    comms.model->ComputeTransforms(renderParams.SceneIndex, comms.transforms[0]);
    comms.aabb = comms.model->ComputeBoundingBox(renderParams.SceneIndex, comms.transforms[0], nullptr);
    comms.worldspaceAABB = comms.aabb;

    // Center and scale model
    float  MaxDim = 0;
    float3 ModelDim{ comms.aabb.Max - comms.aabb.Min };
    MaxDim = std::max(MaxDim, ModelDim.x);
    MaxDim = std::max(MaxDim, ModelDim.y);
    MaxDim = std::max(MaxDim, ModelDim.z);

    float4x4 InvYAxis = float4x4::Identity();
    InvYAxis._22 = -1;

    comms.scale = (1.0f / std::max(MaxDim, 0.01f)) * 0.5f;
    auto     Translate = -comms.aabb.Min - 0.5f * ModelDim;
    InvYAxis._22 = -1;

    comms.modelTransform = float4x4::Translation(Translate) * float4x4::Scale(comms.scale) * InvYAxis;
    comms.scaleAndTransform = float4x4::Translation(Translate) * float4x4::Scale(comms.scale);

    comms.model->ComputeTransforms(renderParams.SceneIndex, comms.transforms[0], comms.modelTransform);

    comms.aabb = comms.model->ComputeBoundingBox(renderParams.SceneIndex, comms.transforms[0], nullptr);
    comms.transforms[1] = comms.transforms[0];

    float YFov = PI_F / 4.0f;
    float ZNear = 0.1f;
    float ZFar = 100.f;

    float4x4 CameraView;

    const auto* pCameraNode = comms.camera;
    const auto* pCamera = pCameraNode->pCamera;
    const auto& CameraGlobalTransform = comms.transforms[inFlightIndex & 0x01].NodeGlobalMatrices[pCameraNode->Index];

    // GLTF camera is defined such that the local +X axis is to the right,
    // the lens looks towards the local -Z axis, and the top of the camera
    // is aligned with the local +Y axis.
    // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#cameras
    // We need to inverse the Z axis as our camera looks towards +Z.
    float4x4 InvZAxis = float4x4::Identity();
    InvZAxis._33 = -1;

    CameraView = CameraGlobalTransform.Inverse() * InvZAxis;
    renderParams.ModelTransform = RotationMatrixModel;

    YFov = pCamera->Perspective.YFov;
    ZNear = pCamera->Perspective.ZNear;
    ZFar = pCamera->Perspective.ZFar;

    // Apply pretransform matrix that rotates the scene according the surface orientation
    CameraView *= GetSurfacePretransformMatrix(float3{ 0, 0, 1 });

    float4x4 CameraWorld = CameraView.Inverse();

    // Get projection matrix adjusted to the current screen orientation
    const auto CameraProj = GetAdjustedProjectionMatrix(YFov, ZNear, ZFar);
    const auto CameraViewProj = CameraView * CameraProj;

    float3 CameraWorldPos = float3::MakeVector(CameraWorld[3]);

    auto& CurrCamAttribs = cameraAttribs[inFlightIndex & 0x01];

    CurrCamAttribs.f4ViewportSize = float4{ static_cast<float>(screenSize.x), static_cast<float>(screenSize.y), 1.f / (float)screenSize.x, 1.f / (float)screenSize.y };
    CurrCamAttribs.fHandness = CameraView.Determinant() > 0 ? 1.f : -1.f;
    CurrCamAttribs.mView = CameraView.Transpose();
    CurrCamAttribs.mProj = CameraProj.Transpose();
    CurrCamAttribs.mViewProj = CameraViewProj.Transpose();
    CurrCamAttribs.mViewInv = CameraView.Inverse().Transpose();
    CurrCamAttribs.mProjInv = CameraProj.Inverse().Transpose();
    CurrCamAttribs.mViewProjInv = CameraViewProj.Inverse().Transpose();
    CurrCamAttribs.f4Position = float4(CameraWorldPos, 1);
    CurrCamAttribs.fNearPlaneZ = ZNear;
    CurrCamAttribs.fFarPlaneZ = ZFar;

    cameraAttribs[(inFlightIndex + 1) & 0x01] = CurrCamAttribs;
}