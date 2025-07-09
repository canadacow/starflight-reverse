#include "CommsSystem.h"
#include <algorithm>
#include <Magnum/Animation/Track.h>
#include <Magnum/Animation/Interpolation.h>
#include <Magnum/Math/Quaternion.h>
#include <Magnum/Math/Vector3.h>
#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/ArrayView.h>
#include <Corrade/Containers/Tags.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>

#define MINIMP3_IMPLEMENTATION
#include <minimp3/minimp3.h>
#include <minimp3/minimp3_ex.h>

using namespace Diligent;
using namespace Magnum::Math::Literals;

// Forward declarations for audio functions from graphics.cpp
extern void queue_speech(int16_t* voiceAudio, uint64_t length);
extern void StartAudioPlayback();

// MP3 playback globals
static std::vector<int16_t> s_mp3AudioData;
static std::mutex s_mp3Mutex;
static double s_mp3StartTime = -1.0;
static bool s_mp3Loaded = false;
static double s_mp3Duration = 8.3; // Will be calculated from actual MP3 file

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

// Creates a Magnum Animation Track for head rotation based on the JSON keyframe data
Magnum::Animation::Track<Magnum::Float, Magnum::Quaternion> CreateHeadRotationTrack()
{
    // Parse the JSON animation data
    const char* jsonStr = GetCommsHeadAnimationJSON();
    nlohmann::json animData = nlohmann::json::parse(jsonStr);
    
    // Extract keyframes from JSON and build them into the format expected by Track constructor
    auto keyframes = animData["keyframes"];
    auto numKeyframes = keyframes.size();
    
    // Create a Corrade::Containers::Array for the keyframes
    Corrade::Containers::Array<std::pair<Magnum::Float, Magnum::Quaternion>> trackData{Corrade::Containers::NoInit, numKeyframes};
    
    for (size_t i = 0; i < numKeyframes; ++i) {
        const auto& keyframe = keyframes[i];
        float time = keyframe["time"];
        float pitch = keyframe["pitch"];
        float yaw = keyframe["yaw"];
        
        // Convert degrees to radians and create quaternion
        // Note: Negating pitch for proper rotation direction
        Magnum::Quaternion rotation = 
            Magnum::Quaternion::rotation(Magnum::Deg(-pitch), Magnum::Vector3::xAxis()) * 
            Magnum::Quaternion::rotation(Magnum::Deg(yaw), Magnum::Vector3::yAxis());
        
        trackData[i] = {time, rotation};
    }
    
    // Create the animation track with the parsed keyframes
    return Magnum::Animation::Track<Magnum::Float, Magnum::Quaternion>{
        std::move(trackData),
        Magnum::Math::slerp,
        Magnum::Animation::Extrapolation::Constant,
        Magnum::Animation::Extrapolation::Constant
    };
}

// Forward declarations of graphics utility functions
float4x4 GetAdjustedProjectionMatrix(float FOV, float NearPlane, float FarPlane);
float4x4 GetSurfacePretransformMatrix(const float3& f3CameraViewAxis);
void InitModel(std::string modelPath, SFModel& model, int defaultCameraIndex);

void Diligent::InitComms(SFModel& comms)
{
    InitModel("mechan9.glb", comms, 0);
    InitCommsAudio();
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
            // Create static instance of the head rotation track
            static auto headRotationTrack = CreateHeadRotationTrack();
            
            // Get the animation time (loop based on MP3 duration)
            float animationTime = std::fmod(static_cast<float>(currentTimeInSeconds), static_cast<float>(s_mp3Duration));
            
            // Get the interpolated quaternion from the track
            auto headRotation = headRotationTrack.at(animationTime);
            
            // Convert Magnum quaternion to the format expected by the model
            // Modify the node's rotation (this is non-const, so we need to cast)
            SF_GLTF::Node* headNode = const_cast<SF_GLTF::Node*>(pNode);
            headNode->Rotation = QuaternionF(headRotation.vector().x(), 
                                           headRotation.vector().y(), 
                                           headRotation.vector().z(), 
                                           headRotation.scalar());
            
            break; // Found the head, no need to continue
        }
    }

    // Play MP3 audio synchronized with the animation
    PlayCommsAudio("mechan9.mp3", currentTimeInSeconds);

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

void Diligent::InitCommsAudio()
{
    // Initialize MP3 audio system - called once at startup
    std::lock_guard<std::mutex> lock(s_mp3Mutex);
    s_mp3AudioData.clear();
    s_mp3Loaded = false;
    s_mp3StartTime = -1.0;
    s_mp3Duration = 8.3; // Default fallback duration
}

void Diligent::PlayCommsAudio(const char* mp3FilePath, double currentTimeInSeconds)
{
    std::lock_guard<std::mutex> lock(s_mp3Mutex);
    
    // Load MP3 file if not already loaded
    if (!s_mp3Loaded) {
        std::ifstream file(mp3FilePath, std::ios::binary);
        if (!file.is_open()) {
            printf("Error: Could not open MP3 file: %s\n", mp3FilePath);
            return;
        }
        
        // Read entire file into memory
        std::vector<uint8_t> fileData((std::istreambuf_iterator<char>(file)),
                                      std::istreambuf_iterator<char>());
        file.close();
        
        // Decode MP3 data using minimp3
        mp3dec_t mp3d;
        mp3dec_ex_t dec;
        
        if (mp3dec_ex_open_buf(&dec, fileData.data(), fileData.size(), MP3D_SEEK_TO_SAMPLE)) {
            printf("Error: Failed to decode MP3 file: %s\n", mp3FilePath);
            return;
        }
        
        // Allocate buffer for decoded audio
        s_mp3AudioData.resize(dec.samples);
        
        // Decode all samples
        size_t samples = mp3dec_ex_read(&dec, s_mp3AudioData.data(), dec.samples);
        
        if (samples != dec.samples) {
            printf("Warning: Expected %zu samples, got %zu\n", dec.samples, samples);
        }
        
        // Convert to mono if stereo (average channels)
        if (dec.info.channels == 2) {
            for (size_t i = 0; i < samples; i += 2) {
                s_mp3AudioData[i / 2] = (s_mp3AudioData[i] + s_mp3AudioData[i + 1]) / 2;
            }
            s_mp3AudioData.resize(samples / 2);
        }
        
        // Calculate MP3 duration from sample count and sample rate
        s_mp3Duration = static_cast<double>(s_mp3AudioData.size()) / static_cast<double>(dec.info.hz);
        
        mp3dec_ex_close(&dec);
        
        printf("MP3 loaded: %zu samples, %d Hz, %d channels, duration: %.3f seconds\n", 
               s_mp3AudioData.size(), dec.info.hz, dec.info.channels, s_mp3Duration);
        
        s_mp3Loaded = true;
    }
    
    // Start playback synchronized with animation
    if (s_mp3Loaded) {
        // Calculate animation cycle position
        double animationTime = std::fmod(currentTimeInSeconds, s_mp3Duration);
        
        // Start MP3 playback if we're at the beginning of the animation cycle
        if (animationTime < 0.1 && (s_mp3StartTime < 0 || currentTimeInSeconds - s_mp3StartTime > s_mp3Duration)) {
            s_mp3StartTime = currentTimeInSeconds;
            
            // Ensure audio device is unpaused and ready for playback
            StartAudioPlayback();
            
            // Queue the entire MP3 audio data for playback
            queue_speech(s_mp3AudioData.data(), s_mp3AudioData.size());
            
            printf("Started MP3 playback at time %.3f (animation time %.3f, duration %.3f)\n", 
                   currentTimeInSeconds, animationTime, s_mp3Duration);
        }
    }
}