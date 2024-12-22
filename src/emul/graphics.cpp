#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_image.h>
#include <atomic>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "graphics.h"
#include "../util/lodepng.h"
#include <direct.h> 

#include <vector>
#include <assert.h>
#include <algorithm>
#include <deque>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <semaphore>
#include <unordered_map>
#include <array>
#include <utility>
#include <deque>

#include <zstd.h>
#include <xxhash.h>

#include "../cpu/cpu.h"
#include "../tts/speech.h"

#include "instance.h"

#include "vulkan_helper.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_SDLSURFACE_IMPLEMENTATION

#include "nuklear.h"

#define DE_SSR 1
//#define FX_SSR 1

#include "sdl2surface_rawfb.h"

#if defined(PLANES)
    #undef PLANES
#endif

#include "Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"
#include "Graphics/GraphicsEngineVulkan/interface/RenderDeviceVk.h"
#include "Graphics/GraphicsEngineVulkan/interface/DeviceContextVk.h"
#include "Graphics/GraphicsEngineVulkan/include/VulkanTypeConversions.hpp"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"
#include "Graphics/GraphicsEngine/interface/Shader.h"
#include "Graphics/GraphicsTools/interface/ShaderSourceFactoryUtils.hpp"
#include "Utilities/interface/DiligentFXShaderSourceStreamFactory.hpp"
#include "RenderStateNotation/interface/RenderStateNotationLoader.h"
#include "AssetLoader/interface/DXSDKMeshLoader.hpp"
#include "BasicMath.hpp"
#include "CommonlyUsedStates.h"
#include "PostFXContext.hpp"
#include "Bloom.hpp"
#include "ScreenSpaceReflection.hpp"
#include "ScreenSpaceAmbientOcclusion.hpp"
#include "MapHelper.hpp"
#include "TextureUtilities.h"
#include "TextureLoader.h"
#include "EnvMapRenderer.hpp"
#include "DiligentShadowMapManager.hpp"
#include "DiligentEpipolarLightScattering.hpp"

namespace Diligent
{

namespace HLSL
{

#include "Shaders/Common/public/BasicStructures.fxh"
#include "../pbr/shaders/SF_PBR_Structures.fxh"
#include "../pbr/shaders/SF_RenderPBR_Structures.fxh"
#include "Shaders/PostProcess/Bloom/public/BloomStructures.fxh"
#include "Shaders/PostProcess/ToneMapping/public/ToneMappingStructures.fxh"
#include "Shaders/PostProcess/ScreenSpaceReflection/public/ScreenSpaceReflectionStructures.fxh"
#include "Shaders/PostProcess/ScreenSpaceAmbientOcclusion/public/ScreenSpaceAmbientOcclusionStructures.fxh"
#include "Shaders/PostProcess/EpipolarLightScattering/public/EpipolarLightScatteringStructures.fxh"

} // namespace HLSL

void PrepareForNewFrame(void* pEpipolarLightScattering, EpipolarLightScattering::FrameAttribs& FrameAttribs, void* m_PPAttribs);

} // namespace Diligent

#include "Common/interface/RefCntAutoPtr.hpp"

#include "../pbr/SF_GLTFLoader.hpp"
#include "DynamicMesh.hpp"

//#define SF_PBR_Renderer GLTF_PBR_Renderer
//#include "GLTF_PBR_Renderer.hpp"
#include "../pbr/SF_GLTF_PBR_Renderer.hpp"

#include "BasicMath.hpp"
#include "GBuffer.hpp"

#include "FidelityFX/host/backends/vk/ffx_vk.h"
#include "FidelityFX/host/ffx_sssr.h"

using namespace Diligent;

enum GBUFFER_RT : Uint32
{
    GBUFFER_RT_RADIANCE,
    GBUFFER_RT_NORMAL,
    GBUFFER_RT_BASE_COLOR,
    GBUFFER_RT_MATERIAL_DATA,
    GBUFFER_RT_MOTION_VECTORS,
    GBUFFER_RT_SPECULAR_IBL,
    GBUFFER_RT_DEPTH0,
    GBUFFER_RT_DEPTH1,
    GBUFFER_RT_COUNT,
    GBUFFER_RT_NUM_COLOR_TARGETS = GBUFFER_RT_SPECULAR_IBL + 1,
};

enum GBUFFER_RT_FLAG : Uint32
{
    GBUFFER_RT_FLAG_COLOR = 1u << GBUFFER_RT_RADIANCE,
    GBUFFER_RT_FLAG_NORMAL = 1u << GBUFFER_RT_NORMAL,
    GBUFFER_RT_FLAG_BASE_COLOR = 1u << GBUFFER_RT_BASE_COLOR,
    GBUFFER_RT_FLAG_MATERIAL_DATA = 1u << GBUFFER_RT_MATERIAL_DATA,
    GBUFFER_RT_FLAG_MOTION_VECTORS = 1u << GBUFFER_RT_MOTION_VECTORS,
    GBUFFER_RT_FLAG_SPECULAR_IBL = 1u << GBUFFER_RT_SPECULAR_IBL,
    GBUFFER_RT_FLAG_LAST_COLOR_TARGET = GBUFFER_RT_FLAG_SPECULAR_IBL,
    GBUFFER_RT_FLAG_ALL_COLOR_TARGETS = (GBUFFER_RT_FLAG_LAST_COLOR_TARGET << 1u) - 1u,
    GBUFFER_RT_FLAG_DEPTH0 = 1u << GBUFFER_RT_DEPTH0,
    GBUFFER_RT_FLAG_DEPTH1 = 1u << GBUFFER_RT_DEPTH1
};
DEFINE_FLAG_ENUM_OPERATORS(GBUFFER_RT_FLAG);

std::promise<void> initPromise;
std::future<void> initFuture = initPromise.get_future();

//#define USE_CPU_RASTERIZATION 1

#define TEXT_MODE_WIDTH 640
#define TEXT_MODE_HEIGHT 200

#define GRAPHICS_MODE_WIDTH 160
#define GRAPHICS_MODE_HEIGHT 200
#define GRAPHICS_PAGE_COUNT 2
#define GRAPHICS_MEMORY_ALLOC 65536
static_assert(GRAPHICS_MODE_WIDTH * GRAPHICS_MODE_HEIGHT * GRAPHICS_PAGE_COUNT < GRAPHICS_MEMORY_ALLOC);

static uint32_t WINDOW_WIDTH = 1920;
static uint32_t WINDOW_HEIGHT = 1440;

static uint32_t OFFSCREEN_WINDOW_WIDTH = 960;
static uint32_t OFFSCREEN_WINDOW_HEIGHT = 720;

#define TARGET_RESOLUTION_WIDTH 3840
#define TARGET_RESOLUTION_HEIGHT 2160

#define ROTOSCOPE_MODE_WIDTH  160
#define ROTOSCOPE_MODE_HEIGHT 200

static SDL_Window *window = NULL;
static SDL_Renderer *renderer  = NULL;
static SDL_Texture* graphicsTexture = NULL;
static SDL_Texture* windowTexture = NULL;
static SDL_Texture* textTexture = NULL;

static SDL_Surface* nk_surface = NULL;
static sdlsurface_context* nk_context = NULL;

//#define ENABLE_OFFSCREEN_VIDEO_RENDERER 1

#if defined(ENABLE_OFFSCREEN_VIDEO_RENDERER)
static SDL_Renderer *offscreenRenderer  = NULL;
static SDL_Texture* offscreenTexture = NULL;
static SDL_Window *offscreenWindow = NULL;
#endif

static SDL_AudioDeviceID audioDevice = 0;

#define FREQUENCY 44100 // Samples per second

static double toneInHz = 440.0;

static bool s_useRotoscope = true;
static bool s_shouldToggleMenu = true;
static bool s_useEGA = true;
static bool s_showHelp = false;
static bool s_helpShown = false;
static std::atomic<uint32_t> s_alienVar1 = 0;
static float s_adjust = 0.0f;

static std::mutex s_deadReckoningMutex;

static vec2<int16_t> s_pastWorld = {};
static std::chrono::time_point<std::chrono::steady_clock> s_deadReckoningSet;

static uint64_t s_frameAccelCount;

struct ApplyPosteffects
{
    RefCntAutoPtr<IPipelineState>         pPSO;
    RefCntAutoPtr<IShaderResourceBinding> pSRB;

    IShaderResourceVariable* ptex2DRadianceVar         = nullptr;
    IShaderResourceVariable* ptex2DNormalVar           = nullptr;
    IShaderResourceVariable* ptex2DSSR                 = nullptr;
    IShaderResourceVariable* ptex2DPecularIBL          = nullptr;
    IShaderResourceVariable* ptex2DBaseColorVar        = nullptr;
    IShaderResourceVariable* ptex2DMaterialDataVar     = nullptr;
    IShaderResourceVariable* ptex2DPreintegratedGGXVar = nullptr;
    IShaderResourceVariable* ptex2DSSAOVar             = nullptr;

    void Initialize(IRenderDevice* pDevice, TEXTURE_FORMAT RTVFormat, IBuffer* pFrameAttribsCB);
    operator bool() const { return pPSO != nullptr; }
};

RefCntAutoPtr<IShaderSourceInputStreamFactory> SFCreateCompoundShaderSourceFactory(IRenderDevice* pDevice)
{
    RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
    pDevice->GetEngineFactory()->CreateDefaultShaderSourceStreamFactory("shaders", &pShaderSourceFactory);
    return CreateCompoundShaderSourceFactory({ &DiligentFXShaderSourceStreamFactory::GetInstance(), pShaderSourceFactory });
}

void ApplyPosteffects::Initialize(IRenderDevice* pDevice, TEXTURE_FORMAT RTVFormat, IBuffer* pFrameAttribsCB)
{
    ShaderCreateInfo ShaderCI;
    ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

    auto pCompoundSourceFactory         = SFCreateCompoundShaderSourceFactory(pDevice);
    ShaderCI.pShaderSourceStreamFactory = pCompoundSourceFactory;

    ShaderMacroHelper Macros;
    Macros.Add("CONVERT_OUTPUT_TO_SRGB", RTVFormat == TEX_FORMAT_RGBA8_UNORM || RTVFormat == TEX_FORMAT_BGRA8_UNORM);
    Macros.Add("TONE_MAPPING_MODE", TONE_MAPPING_MODE_UNCHARTED2);
    ShaderCI.Macros = Macros;

    RefCntAutoPtr<IShader> pVS;
    {
        ShaderCI.Desc       = {"Full Screen Triangle VS", SHADER_TYPE_VERTEX, true};
        ShaderCI.EntryPoint = "FullScreenTriangleVS";
        ShaderCI.FilePath   = "FullScreenTriangleVS.fx";

        pDevice->CreateShader(ShaderCI, &pVS);
        VERIFY_EXPR(pVS);
    }

    RefCntAutoPtr<IShader> pPS;
    {
        ShaderCI.Desc       = {"Apply Post Effects PS", SHADER_TYPE_PIXEL, true};
        ShaderCI.EntryPoint = "main";
        ShaderCI.FilePath   = "ApplyPostEffects.psh";

        pDevice->CreateShader(ShaderCI, &pPS);
        VERIFY_EXPR(pPS);
    }

    PipelineResourceLayoutDescX ResourceLauout;
    ResourceLauout
        .SetDefaultVariableType(SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
        .AddVariable(SHADER_TYPE_PIXEL, "cbFrameAttribs", SHADER_RESOURCE_VARIABLE_TYPE_STATIC)
        .AddImmutableSampler(SHADER_TYPE_PIXEL, "g_tex2DPreintegratedGGX", Sam_LinearClamp);

    GraphicsPipelineStateCreateInfoX PsoCI{"Apply Post Effects"};
    PsoCI
        .AddRenderTarget(RTVFormat)
        .AddShader(pVS)
        .AddShader(pPS)
        .SetDepthStencilDesc(DSS_DisableDepth)
        .SetRasterizerDesc(RS_SolidFillNoCull)
        .SetResourceLayout(ResourceLauout)
        .SetPrimitiveTopology(PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    pDevice->CreatePipelineState(PsoCI, &pPSO);
    VERIFY_EXPR(pPSO);
    pPSO->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbFrameAttribs")->Set(pFrameAttribsCB);

    pPSO->CreateShaderResourceBinding(&pSRB, true);

    auto GetVariable = [&](const char* Name) {
        IShaderResourceVariable* pVar = pSRB->GetVariableByName(SHADER_TYPE_PIXEL, Name);
        VERIFY_EXPR(pVar != nullptr);
        return pVar;
    };
    ptex2DRadianceVar         = GetVariable("g_tex2DRadiance");
    ptex2DNormalVar           = GetVariable("g_tex2DNormal");
    ptex2DSSR                 = GetVariable("g_tex2DSSR");
    ptex2DPecularIBL          = GetVariable("g_tex2DSpecularIBL");
    ptex2DBaseColorVar        = GetVariable("g_tex2DBaseColor");
    ptex2DMaterialDataVar     = GetVariable("g_tex2DMaterialData");
    ptex2DPreintegratedGGXVar = GetVariable("g_tex2DPreintegratedGGX");
    ptex2DSSAOVar             = GetVariable("g_tex2DSSAO");
}

struct ShadowMap;

enum Scene {
    SCENE_NONE,
    SCENE_STATION,
    SCENE_PLANET,
    SCENE_TERRAIN
};

struct GraphicsContext
{
    RefCntAutoPtr<IRenderDevice>    m_pDevice;
    RefCntAutoPtr<IRenderDeviceVk>  m_pDeviceVk;
    RefCntAutoPtr<IDeviceContext>   m_pImmediateContext;
    RefCntAutoPtr<IDeviceContextVk> m_pImmediateContextVk;

    VulkanContext vc;
    avk::queue* mQueue;

    struct BufferData {
        avk::buffer frameStaging;
        avk::buffer rotoscope;
        avk::buffer uniform;
        avk::buffer iconUniform;
        avk::buffer orreryUniform;
        avk::buffer starmapUniform;
        avk::command_buffer command;
        avk::image_sampler navigation;
        avk::image_sampler orrery;
        avk::image_sampler starmap;
        avk::image_sampler ui;
        avk::image_sampler gameOutput;

        avk::image_sampler avkdColorBuffer;
        avk::image         avkdDepthBuffer;

        avk::image_sampler avkdSsrBuffer;

        ITextureView* diligentColorBuffer;
        ITextureView* diligentDepthBuffer;

        ITextureView* diligentSsrBuffer;
        ITextureView* diligentSsrBufferSrv;

        avk::image    avkdShadowDepthBuffer;
        ITextureView* diligentShadowDepthBuffer;

        ITexture* offscreenColorBuffer;
        ITexture* offscreenDepthBuffer;

        constexpr static uint32_t waterHeightMapSize = 2048;

        ITexture* waterHeightMap;
    };
    
    std::vector<BufferData> buffers;

    struct WaterComputeShader
    {
        RefCntAutoPtr<IPipelineState> pWaterComputePSO;
        RefCntAutoPtr<IShaderResourceBinding> pWaterComputeSRB;
        RefCntAutoPtr<IBuffer> pTimeConstantBuffer;
    };

    WaterComputeShader waterComputeShader;

    avk::image_sampler LOGO1;
    avk::image_sampler LOGO2;
    avk::image_sampler FONT1;
    avk::image_sampler FONT2;
    avk::image_sampler FONT3;

    avk::image_sampler PORTPIC;

    avk::image_sampler RACEDOSATLAS;

    avk::image_sampler textImage;

    avk::image_sampler shipImage;
    avk::image_sampler planetAlbedoImages;

    avk::image_sampler boxArtImage;
    avk::image_sampler fourDeeNoise;

    ITextureView* diligentFourDeeNoise;

    avk::image_sampler alienColorImage;
    avk::image_sampler alienDepthImage;
    avk::image_sampler alienBackgroundImage;

    avk::compute_pipeline rotoscopePipeline;
    avk::compute_pipeline navigationPipeline;
    avk::compute_pipeline orbitPipeline;
    avk::compute_pipeline orreryPipeline;
    avk::compute_pipeline textPipeline;
    avk::compute_pipeline titlePipeline;
    avk::compute_pipeline starmapPipeline;
    avk::compute_pipeline encounterPipeline;
    avk::compute_pipeline compositorPipeline;

    avk::descriptor_cache descriptorCache;

    std::chrono::time_point<std::chrono::steady_clock> epoch;

    std::unordered_map<uint32_t, PlanetSurface> surfaceData{};

    std::unordered_map<uint32_t, uint32_t> seedToIndex;

    bool shouldInitPlanets = false;
    std::binary_semaphore planetsDone{0};

    struct SFModel
    {
        std::shared_ptr<SF_GLTF::Model> model;
        SF_GLTF_PBR_Renderer::ModelResourceBindings bindings;
        BoundBox aabb;
        BoundBox worldspaceAABB;
        std::array<SF_GLTF::ModelTransforms, 2> transforms; // [0] - current frame, [1] - previous frame
        float4x4                             modelTransform;
        float                                scale = 1.f;
        float4x4                             scaleAndTransform;
        RefCntAutoPtr<ITextureView> env;
        const SF_GLTF::Node* camera;
        std::vector<const SF_GLTF::Node*> lights;

        std::unique_ptr<SF_GLTF::DynamicMesh> dynamicMesh;
        std::array<SF_GLTF::ModelTransforms, 2> dynamicMeshTransforms; // [0] - current frame, [1] - previous frame

        std::unordered_map<std::string, Uint32> biomMaterialIndex;

        struct BiomBoundary
        {
            std::string name;
            float startHeight;
        };

        struct PlanetType
        {
            std::string name;
            std::vector<BiomBoundary> boundaries;
        };

        std::vector<PlanetType> planetTypes;
    };

    SFModel station{};
    SFModel planet{};
    SFModel terrain{};

    Scene currentScene = SCENE_NONE;

    Uint32 terrainMaterialIndex = 0;

    struct MouseState
    {
        float2 pos = float2(-1.0f, -1.0f);
        bool leftDown;
        bool rightDown;
    };

    static inline const float TileSize = 4.0f;    

    float3 terrainDelta{};
    //float3 terrainMovement = { 0.0f, -15.0f, 0.0 };
    //#define TV_LOCATION_START_X 389.0f
    //#define TV_LOCATION_START_Y 245.0f
    #define TV_LOCATION_START_X (1608.0f + 30.f)
    #define TV_LOCATION_START_Y (230.0f + 30.f)
    float2 tvLocation = {TV_LOCATION_START_X, TV_LOCATION_START_Y};
    float2 tvDelta{};
    Quaternion<float> tvNudge = {};
    Quaternion<float> tvRotation = {};

    //float3 terrainMovement = { TV_LOCATION_START_X * TileSize, -40.0f, TV_LOCATION_START_Y * TileSize };
    float3 terrainMovement = { TV_LOCATION_START_X * TileSize, -200.0f, TV_LOCATION_START_Y * TileSize };
    float2 terrainTextureOffset = { 0.0f, 0.0f };
    float2 terrainSize = {};

    

    MouseState mouseState;
    float FPVpitchAngle = 0.18f;
    float FPVyawAngle = 0.23f;
    float NormalizedXCoordForSunRotation = 0.0f;
    float4x4 terrainFPVRotation = float4x4::Identity();

    std::unique_ptr<HLSL::CameraAttribs[]> cameraAttribs;

    std::unique_ptr<SF_GLTF_PBR_Renderer> pbrRenderer;
    std::unique_ptr<GBuffer> gBuffer;
    SF_GLTF_PBR_Renderer::RenderInfo renderParams;
    ApplyPosteffects applyPostFX;
    RefCntAutoPtr<IBuffer> frameAttribsCB;
    RefCntAutoPtr<IBuffer> cameraAttribsCB;
    RefCntAutoPtr<IBuffer> PBRPrimitiveAttribsCB;
    RefCntAutoPtr<IBuffer> terrainAttribsCB;
    RefCntAutoPtr<IBuffer> jointsBuffer;
    RefCntAutoPtr<IBuffer> instanceAttribsSB;
    RefCntAutoPtr<IBufferView> instanceAttribsSBView;    
    RefCntAutoPtr<IBuffer> dummyVertexBuffer;
    RefCntAutoPtr<IBuffer> heightmapAttribsCB;
    RefCntAutoPtr<ITexture> heightmap;
    RefCntAutoPtr<ITextureView> heightmapView;

    int2 heightmapSize;
    std::vector<float> heightmapData;

    std::unique_ptr<PostFXContext> postFXContext;
    std::unique_ptr<Bloom> bloom;
    std::unique_ptr<ScreenSpaceAmbientOcclusion> ssao;

    HLSL::BloomAttribs bloomSettings{};

    RefCntAutoPtr<IBuffer> pcbCameraAttribs;
    RefCntAutoPtr<IBuffer> pcbLightAttribs;

#if defined(DE_SSR)
    std::unique_ptr<ScreenSpaceReflection> ssr;
#endif

#if defined(FX_SSR)
    FfxInterface ffxInterface;
    FfxSssrContext sssrContext;
#endif
    std::unique_ptr<EnvMapRenderer> envMapRenderer;

    std::unique_ptr<EpipolarLightScattering> epipolarLightScattering;

    std::mutex spaceManMutex;
    Interpolator spaceMan;

    struct SpaceManState {
        enum class AnimationState {
            Standing,
            StartingWalking,
            Walking,
            StoppingWalking
        };

        enum class AnimationDirection {
            Forward,
            Reverse
        };

        AnimationState currentState = AnimationState::Standing;
        std::chrono::steady_clock::time_point stateChangeTimestamp = std::chrono::steady_clock::now();
        double restingPoseLength = 0.0;
        double restingPoseStart = 0.0;
        double walkingAnimationLength = 0.0;
        double walkingAnimationStart = 0.0;
        AnimationDirection currentDirection = AnimationDirection::Forward;
        std::optional<std::chrono::steady_clock::time_point> walkingEndTimestamp;

        double getTimeSinceStateChange() const {
            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(now - stateChangeTimestamp);
            return duration.count();
        }
        
        void changeState(AnimationState newState) {
            if (currentState != newState) {
                currentState = newState;
                stateChangeTimestamp = std::chrono::steady_clock::now();
                currentDirection = (newState == AnimationState::Walking || newState == AnimationState::StartingWalking) 
                                ? AnimationDirection::Forward : AnimationDirection::Reverse;
                if (newState != AnimationState::Walking) {
                    walkingEndTimestamp.reset();
                }
            }
        }

        std::pair<AnimationState, double> getAnimationState(bool continueWalking) {
            double timeSinceChange = getTimeSinceStateChange();
            switch (currentState) {
                case AnimationState::Standing:
                    if (continueWalking) {
                        changeState(AnimationState::StartingWalking);
                        return {AnimationState::StartingWalking, 0.0};
                    }
                    return {AnimationState::Standing, 0.0};

                case AnimationState::StartingWalking:
                    if (timeSinceChange >= restingPoseLength) {
                        changeState(AnimationState::Walking);
                        return {AnimationState::Walking, 0.0};
                    }
                    return {AnimationState::StartingWalking, timeSinceChange};

                case AnimationState::Walking:
                    if (!continueWalking && !walkingEndTimestamp) {
                        double timeInCurrentCycle = fmod(timeSinceChange, walkingAnimationLength);
                        double remainingTime = walkingAnimationLength - timeInCurrentCycle;
                        walkingEndTimestamp = std::chrono::steady_clock::now() + std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>(remainingTime));
                    }
                    if (continueWalking) {
                        walkingEndTimestamp.reset();
                    }
                    if (walkingEndTimestamp && std::chrono::steady_clock::now() >= *walkingEndTimestamp) {
                        changeState(AnimationState::Standing);
                        return {AnimationState::Standing, 0.0};
                    }
                    return {AnimationState::Walking, fmod(timeSinceChange, walkingAnimationLength)};

                case AnimationState::StoppingWalking:
                    if (timeSinceChange >= restingPoseLength) {
                        changeState(AnimationState::Standing);
                        return {AnimationState::Standing, 0.0};
                    }
                    return {AnimationState::StoppingWalking, restingPoseLength - timeSinceChange};
            }
            return {currentState, 0.0}; // Default return, should not be reached
        }
    };
    
    SpaceManState spaceManState;

    std::unique_ptr<ShadowMap> shadowMap;
};

static GraphicsContext s_gc{};

struct ShadowMap
{
    struct ShadowSettings
    {
        bool           SnapCascades = true;
        bool           StabilizeExtents = true;
        bool           EqualizeExtents = true;
        bool           SearchBestCascade = true;
        bool           FilterAcrossCascades = true;
        int            Resolution = 2048;
        float          PartitioningFactor = 0.95f;
        TEXTURE_FORMAT Format = TEX_FORMAT_D32_FLOAT;
        int            iShadowMode = SHADOW_MODE_PCF;

        bool Is32BitFilterableFmt = true;
    } m_ShadowSettings;

    void Initialize();

    void DrawMesh(IDeviceContext* pCtx,                                 
                const SF_GLTF::Model& GLTFModel,
                const SF_GLTF::ModelTransforms& Transforms,
                const HLSL::CameraAttribs& cameraAttribs,
                const SF_GLTF_PBR_Renderer::RenderInfo & RenderParams);

    void RenderShadowMap(const HLSL::CameraAttribs& CurrCamAttribs, float3 Direction, VulkanContext::frame_id_t inFlightIndex, const SF_GLTF_PBR_Renderer::RenderInfo& RenderParams, HLSL::PBRShadowMapInfo* shadowInfo, GraphicsContext::SFModel& model);

    ITextureView* GetShadowMap()
    {
        return m_ShadowMapMgr.GetSRV();
    }

    void InitializeResourceBindings(const std::shared_ptr<SF_GLTF::Model>& mesh);

    HLSL::LightAttribs GetLightAttribs()
    {
        return m_LightAttribs;
    }

private:

    HLSL::LightAttribs m_LightAttribs;
    DiligentShadowMapManager m_ShadowMapMgr;

    RefCntAutoPtr<IBuffer>                                          m_LightAttribsCB;
    std::vector<Uint32>                                             m_PSOIndex;
    std::array<RefCntAutoPtr<IPipelineState>, 3>                    m_RenderMeshShadowPSO;
    std::vector<std::vector<RefCntAutoPtr<IShaderResourceBinding>>> m_ShadowSRBs;

    RefCntAutoPtr<IRenderStateNotationLoader> m_pRSNLoader;

    RefCntAutoPtr<ISampler> m_pComparisonSampler;
    RefCntAutoPtr<ISampler> m_pFilterableShadowMapSampler;

    //DXSDKMesh m_Mesh;
};

void InitModel(std::string modelPath, GraphicsContext::SFModel& model, int defaultCameraIndex = 0);
void InitStation();
void InitPlanet();
void InitTerrain();
void InitializeCommonResources();
void DoDemoKeys(SDL_Event event, VulkanContext::frame_id_t inFlightIndex);

using SunBehaviorFn = std::function<float3(const float4x4& lightGlobalTransform, double currentTimeInSeconds)>;

static SunBehaviorFn DefaultSunBehavior = [](const float4x4& lightGlobalTransform, double currentTimeInSeconds) {
    float3 lightDir = float3{ lightGlobalTransform._31, lightGlobalTransform._32, lightGlobalTransform._33 };
    float3 Direction = normalize(lightDir);
    return Direction;
};

void RenderSFModel(VulkanContext::frame_id_t inFlightIndex, GraphicsContext::SFModel& model, const SunBehaviorFn& sunBehavior = DefaultSunBehavior);

/*

void GLTF_PBR_Renderer::InitMaterialSRB(GLTF::Model&            Model,
                                        GLTF::Material&         Material,
                                        IBuffer*                pFrameAttribs,
                                        IShaderResourceBinding* pMaterialSRB)
{
    if (pMaterialSRB == nullptr)
    {
        LOG_ERROR_MESSAGE("Failed to create material SRB");
        return;
    }

    InitCommonSRBVars(pMaterialSRB, pFrameAttribs);

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

*/

void ShadowMap::InitializeResourceBindings(const std::shared_ptr<SF_GLTF::Model>& mesh)
{
    m_ShadowSRBs.clear();
    m_ShadowSRBs.resize(3);

    for (size_t shaderIdx = 0; shaderIdx < 3; ++shaderIdx)
    {
        m_ShadowSRBs[shaderIdx].resize(mesh->GetMaterials().size());
        for (Uint32 mat = 0; mat < mesh->GetMaterials().size(); ++mat)
        {
            RefCntAutoPtr<IShaderResourceBinding> pShadowSRB;
            m_RenderMeshShadowPSO[shaderIdx]->CreateShaderResourceBinding(&pShadowSRB, true);
            m_ShadowSRBs[shaderIdx][mat] = std::move(pShadowSRB);
        }
    }
}

void ShadowMap::DrawMesh(IDeviceContext* pCtx,
                         const SF_GLTF::Model& GLTFModel,
                         const SF_GLTF::ModelTransforms& Transforms,
                         const HLSL::CameraAttribs& cameraAttribs,
                         const SF_GLTF_PBR_Renderer::RenderInfo& RenderParams)
{
    // Iterate through each scene node
    for (const auto& Scene : GLTFModel.GetScenes())
    {
        for (const auto* pNode : Scene.LinearNodes)
        {
            if (pNode->pMesh == nullptr)
                continue;

            RefCntAutoPtr<IPipelineState> pPSO;

            int shaderIdx = 0;
            if(pNode->Instances.size() > 0)
            {
                if(pNode->isTerrain)
                {
                    shaderIdx = 1;
                }
                else
                {
                    shaderIdx = 2;
                }
            }

            pPSO = m_RenderMeshShadowPSO[shaderIdx];

            GLTFModel.SetVertexBuffersForNode(pCtx, pNode);

            pCtx->SetPipelineState(pPSO);

            // Need to make sure we're updating the CameraAttribs properly
            MapHelper<HLSL::CameraAttribs> CameraAttribs{ pCtx, s_gc.cameraAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD };
            *CameraAttribs = cameraAttribs;

            MapHelper<float4x4> pJoints{ pCtx, s_gc.jointsBuffer, MAP_WRITE, MAP_FLAG_DISCARD };

            size_t JointCount = 0;
            if (pNode->SkinTransformsIndex >= 0 && pNode->SkinTransformsIndex < static_cast<int>(Transforms.Skins.size()))
            {
                const auto& JointMatrices = Transforms.Skins[pNode->SkinTransformsIndex].JointMatrices;

                JointCount = JointMatrices.size();

                if (JointCount != 0)
                {
                    WriteShaderMatrices(pJoints, JointMatrices.data(), JointCount, true);
                }
            }            

            MapHelper<HLSL::GLTFNodeShaderTransforms> GLTFNodeShaderTransforms{ pCtx, s_gc.PBRPrimitiveAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD };
            GLTFNodeShaderTransforms->JointCount = JointCount; 

            const auto& NodeGlobalMatrix      = Transforms.NodeGlobalMatrices[pNode->Index];
            const float4x4  NodeTransform     = NodeGlobalMatrix * RenderParams.ModelTransform;

            WriteShaderMatrix(&GLTFNodeShaderTransforms->NodeMatrix, NodeTransform, true);            

            if(pNode->isTerrain )
            {
                MapHelper<HLSL::PBRTerrainAttribs> TerrainAttribs{ pCtx, s_gc.terrainAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD };
                TerrainAttribs->startBiomHeight = 0.0f;
                TerrainAttribs->endBiomHeight = 0.0f;
                TerrainAttribs->textureOffsetX = s_gc.terrainTextureOffset.x;
                TerrainAttribs->textureOffsetY = s_gc.terrainTextureOffset.y;
                TerrainAttribs->waterHeight = 2.0f;
            }
            
            // Iterate through each primitive in the mesh
            for (const auto& primitive : pNode->pMesh->Primitives)
            {
                if (primitive.VertexCount == 0 && primitive.IndexCount == 0)
                    continue;
                
                IShaderResourceBinding* pSRB = m_ShadowSRBs[shaderIdx][primitive.MaterialId];
                pCtx->CommitShaderResources(pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

                if(pNode->Instances.size() > 0)
                {
                    MapHelper<HLSL::PBRInstanceAttribs> InstanceAttribs{ pCtx, s_gc.instanceAttribsSB, MAP_WRITE, MAP_FLAG_DISCARD };
                    for(int i = 0; i < pNode->Instances.size(); ++i)
                    {
                        InstanceAttribs[i].NodeMatrix = pNode->Instances[i].NodeMatrix.Transpose();
                        InstanceAttribs[i].HeightmapAttribs.ScaleX = pNode->Instances[i].ScaleX;
                        InstanceAttribs[i].HeightmapAttribs.ScaleY = pNode->Instances[i].ScaleY;
                        InstanceAttribs[i].HeightmapAttribs.OffsetX = pNode->Instances[i].OffsetX;
                        InstanceAttribs[i].HeightmapAttribs.OffsetY = pNode->Instances[i].OffsetY;
                    }

                    StateTransitionDesc Barriers[] = {
                        {s_gc.instanceAttribsSB, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_SHADER_RESOURCE, STATE_TRANSITION_FLAG_UPDATE_STATE},
                    };
                    pCtx->TransitionResourceStates(_countof(Barriers), Barriers);
                }

                // Draw the primitive
                if (primitive.HasIndices())
                {
                    DrawIndexedAttribs drawAttrs{primitive.IndexCount, VT_UINT32, DRAW_FLAG_VERIFY_ALL};
                    drawAttrs.FirstIndexLocation = GLTFModel.GetFirstIndexLocationForNode(pNode) + primitive.FirstIndex;
                    drawAttrs.BaseVertex = GLTFModel.GetBaseVertexForNode(pNode);

                    if(pNode->Instances.size() > 0)
                    {
                        drawAttrs.FirstInstanceLocation = 0;
                        drawAttrs.NumInstances = static_cast<Uint32>(pNode->Instances.size());
                    }

                    pCtx->DrawIndexed(drawAttrs);
                }
                else
                {
                    DrawAttribs drawAttrs{primitive.VertexCount, DRAW_FLAG_VERIFY_ALL};
                    drawAttrs.StartVertexLocation = GLTFModel.GetBaseVertexForNode(pNode);
                    pCtx->Draw(drawAttrs);
                }
            }
        }
    }

    s_gc.m_pImmediateContext->Flush();
}

void ShadowMap::Initialize()
{
    for (int shaderInit = 0; shaderInit < 3; ++shaderInit)
    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo{};

        PipelineResourceLayoutDescX ResourceLayout;
        ResourceLayout
            .SetDefaultVariableType(SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
            .AddVariable(SHADER_TYPE_VERTEX, "cbCameraAttribs", SHADER_RESOURCE_VARIABLE_TYPE_STATIC)
            .AddVariable(SHADER_TYPE_VERTEX, "cbPrimitiveAttribs", SHADER_RESOURCE_VARIABLE_TYPE_STATIC)
            .AddVariable(SHADER_TYPE_VERTEX, "cbJointTransforms", SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
            
        if(shaderInit == 1)
        {
            ResourceLayout.AddVariable(SHADER_TYPE_VERTEX, "instanceBuffer", SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
            ResourceLayout.AddVariable(SHADER_TYPE_VERTEX, "cbHeightmapAttribs", SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
            ResourceLayout.AddVariable(SHADER_TYPE_VERTEX, "g_Heightmap", SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
            ResourceLayout.AddImmutableSampler(SHADER_TYPE_VERTEX, "g_Heightmap", Sam_LinearMirror);
            //ResourceLayout.AddImmutableSampler(SHADER_TYPE_VERTEX, "g_Heightmap", Sam_PointWrap);
            ResourceLayout.AddVariable(SHADER_TYPE_VERTEX, "cbTerrainAttribs", SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
        }
        else if (shaderInit == 2)
        {
            ResourceLayout.AddVariable(SHADER_TYPE_VERTEX, "instanceBuffer", SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
        }

        PSOCreateInfo.PSODesc.Name = "Mesh Shadow PSO";

        // This is a graphics pipeline
        PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

        // clang-format off
        // Shadow pass doesn't use any render target outputs
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 0;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0] = TEX_FORMAT_UNKNOWN;
        // The DSV format is the shadow map format
        PSOCreateInfo.GraphicsPipeline.DSVFormat = TEX_FORMAT_D32_FLOAT; // m_ShadowMapFormat;
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        // Cull back faces
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.SlopeScaledDepthBias = 2.0f;
        // Enable depth testing
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthFunc = COMPARISON_FUNC_LESS_EQUAL;
        // clang-format on

        auto pCompoundSourceFactory = SFCreateCompoundShaderSourceFactory(s_gc.m_pDevice);

        ShaderCreateInfo ShaderCI;
        ShaderCI.pShaderSourceStreamFactory = pCompoundSourceFactory;
        // Tell the system that the shader source code is in HLSL.
        // For OpenGL, the engine will convert this into GLSL under the hood.
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
        ShaderCI.Desc.UseCombinedTextureSamplers = true;

        ShaderMacroHelper Macros;
        Macros.AddShaderMacro("SHADOW_MODE", SHADOW_MODE_PCF);
        Macros.AddShaderMacro("PCF_FILTER_SIZE", 5);
        Macros.AddShaderMacro("FILTER_ACROSS_CASCADES", true);
        Macros.AddShaderMacro("BEST_CASCADE_SEARCH", true);
        Macros.AddShaderMacro("SHADOW_PASS", true);
        Macros.AddShaderMacro("USE_INSTANCING", shaderInit >= 1);
        Macros.AddShaderMacro("USE_HEIGHTMAP", shaderInit == 1);
        Macros.AddShaderMacro("USE_TERRAINING", shaderInit == 1);

        // Create shadow vertex shader
        RefCntAutoPtr<IShader> pShadowVS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
            ShaderCI.EntryPoint = "MeshVS";
            ShaderCI.Desc.Name = "Mesh VS";
            ShaderCI.FilePath = "MeshVS.vsh";
            ShaderCI.Macros = Macros;
            s_gc.m_pDevice->CreateShader(ShaderCI, &pShadowVS);
        }
        PSOCreateInfo.pVS = pShadowVS;

        // clang-format off
        // Define vertex shader input layout
        LayoutElement LayoutElems[] =
        {
            // Attribute 0 - vertex position
            LayoutElement{0, 0, 3, VT_FLOAT32, False},
            // Attribute 1 - normal
            LayoutElement{1, 0, 3, VT_FLOAT32, False},
            // Attribute 2 - texture coordinates
            LayoutElement{2, 0, 2, VT_FLOAT32, False},
            // Attribute 3 - joint indices
            LayoutElement{3, 1, 4, VT_FLOAT32, False},
            // Attribute 4 - joint weights
            LayoutElement{4, 1, 4, VT_FLOAT32, False}
        };
        // clang-format on

        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements = _countof(LayoutElems);

        PSOCreateInfo.PSODesc.ResourceLayout = ResourceLayout;

        if (s_gc.m_pDevice->GetDeviceInfo().Features.DepthClamp)
        {
            // Disable depth clipping to render objects that are closer than near
            // clipping plane. This is not required for this tutorial, but real applications
            // will most likely want to do this.
            PSOCreateInfo.GraphicsPipeline.RasterizerDesc.DepthClipEnable = False;
        }

        RefCntAutoPtr<IPipelineState> pRenderMeshShadowPSO;

        s_gc.m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &pRenderMeshShadowPSO);

        auto count = pRenderMeshShadowPSO->GetStaticVariableCount(SHADER_TYPE_VERTEX);
        std::vector<ShaderResourceDesc> debugDescs;

        for (Uint32 i = 0; i < count; ++i)
        {
            auto pVar = pRenderMeshShadowPSO->GetStaticVariableByIndex(SHADER_TYPE_VERTEX, i);
            ShaderResourceDesc varDesc;
            pVar->GetResourceDesc(varDesc);
            debugDescs.push_back(varDesc);
        }

        pRenderMeshShadowPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbCameraAttribs")->Set(s_gc.cameraAttribsCB);
        pRenderMeshShadowPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbPrimitiveAttribs")->Set(s_gc.PBRPrimitiveAttribsCB);
        pRenderMeshShadowPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbJointTransforms")->Set(s_gc.jointsBuffer);
        if(shaderInit == 1)     
        {
            pRenderMeshShadowPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "instanceBuffer")->Set(s_gc.instanceAttribsSBView);
            pRenderMeshShadowPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "g_Heightmap")->Set(s_gc.heightmapView);
            pRenderMeshShadowPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbTerrainAttribs")->Set(s_gc.terrainAttribsCB);
        }
        else if (shaderInit == 2)
        {
            pRenderMeshShadowPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "instanceBuffer")->Set(s_gc.instanceAttribsSBView);
        }

        //pRenderMeshShadowPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbHeightmapAttribs")->Set(s_gc.heightmapAttribsCB);

        m_RenderMeshShadowPSO[shaderInit] = std::move(pRenderMeshShadowPSO);
    }

    StateTransitionDesc Barriers[] = {
            {s_gc.cameraAttribsCB, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, STATE_TRANSITION_FLAG_UPDATE_STATE},
            {s_gc.PBRPrimitiveAttribsCB, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, STATE_TRANSITION_FLAG_UPDATE_STATE },
            {s_gc.jointsBuffer, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, STATE_TRANSITION_FLAG_UPDATE_STATE },
            {s_gc.heightmapAttribsCB, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, STATE_TRANSITION_FLAG_UPDATE_STATE },
            {s_gc.instanceAttribsSB, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_SHADER_RESOURCE, STATE_TRANSITION_FLAG_UPDATE_STATE },
            {s_gc.terrainAttribsCB, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, STATE_TRANSITION_FLAG_UPDATE_STATE },
    };
    s_gc.m_pImmediateContext->TransitionResourceStates(_countof(Barriers), Barriers);

    m_LightAttribs.ShadowAttribs.iNumCascades = 2;
    m_LightAttribs.ShadowAttribs.iFixedFilterSize = 5;
    m_LightAttribs.ShadowAttribs.fFilterWorldSize = 0.1f;

    /*
    if (m_ShadowSettings.Resolution >= 2048)
        m_LightAttribs.ShadowAttribs.fFixedDepthBias = 0.0025f;
    else if (m_ShadowSettings.Resolution >= 1024)
        m_LightAttribs.ShadowAttribs.fFixedDepthBias = 0.005f;
    else
        m_LightAttribs.ShadowAttribs.fFixedDepthBias = 0.0075f;
    */

    DiligentShadowMapManager::InitInfo SMMgrInitInfo;
    SMMgrInitInfo.Format = m_ShadowSettings.Format;
    SMMgrInitInfo.Resolution = m_ShadowSettings.Resolution;
    SMMgrInitInfo.NumCascades = static_cast<Uint32>(m_LightAttribs.ShadowAttribs.iNumCascades);
    SMMgrInitInfo.ShadowMode = m_ShadowSettings.iShadowMode;
    SMMgrInitInfo.Is32BitFilterableFmt = m_ShadowSettings.Is32BitFilterableFmt;

    if (!m_pComparisonSampler)
    {
        SamplerDesc ComparsionSampler;
        ComparsionSampler.ComparisonFunc = COMPARISON_FUNC_LESS;
        // Note: anisotropic filtering requires SampleGrad to fix artifacts at
        // cascade boundaries
        ComparsionSampler.MinFilter = FILTER_TYPE_COMPARISON_LINEAR;
        ComparsionSampler.MagFilter = FILTER_TYPE_COMPARISON_LINEAR;
        ComparsionSampler.MipFilter = FILTER_TYPE_COMPARISON_LINEAR;
        s_gc.m_pDevice->CreateSampler(ComparsionSampler, &m_pComparisonSampler);
    }
    SMMgrInitInfo.pComparisonSampler = m_pComparisonSampler;

    if (!m_pFilterableShadowMapSampler)
    {
        SamplerDesc SamplerDesc;
        SamplerDesc.MinFilter = FILTER_TYPE_ANISOTROPIC;
        SamplerDesc.MagFilter = FILTER_TYPE_ANISOTROPIC;
        SamplerDesc.MipFilter = FILTER_TYPE_ANISOTROPIC;
        SamplerDesc.MaxAnisotropy = m_LightAttribs.ShadowAttribs.iMaxAnisotropy;
        s_gc.m_pDevice->CreateSampler(SamplerDesc, &m_pFilterableShadowMapSampler);
    }
    SMMgrInitInfo.pFilterableShadowMapSampler = m_pFilterableShadowMapSampler;

    m_ShadowMapMgr.Initialize(s_gc.m_pDevice, nullptr, SMMgrInitInfo);
}

static float3 Vec3(const float4& v)
{
    return float3(v.x, v.y, v.z);
}

static float4x4 Orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    float4x4 orthoMatrix;
    orthoMatrix._11 = 2.0f / (right - left);
    orthoMatrix._22 = 2.0f / (top - bottom);
    orthoMatrix._33 = 1.0f / (farPlane - nearPlane);
    orthoMatrix._41 = -(right + left) / (right - left);
    orthoMatrix._42 = -(top + bottom) / (top - bottom);
    orthoMatrix._43 = -nearPlane / (farPlane - nearPlane);
    orthoMatrix._44 = 1.0f;
    return orthoMatrix;
}

static float4x4 LookAt(const float3& eye, const float3& at, const float3& up)
{
    // Ensure we have a valid direction vector
    float3 zAxis = at - eye;
    float zLength = length(zAxis);
    if (zLength < 1e-6f) {
        // Return identity matrix if eye and at are too close
        return float4x4::Identity();
    }
    zAxis /= zLength;

    // Handle case where up and zAxis are parallel
    float upDotZ = dot(up, zAxis);
    float3 fixedUp = (abs(upDotZ) > 0.9999f) ? 
        float3(zAxis.z, zAxis.x, zAxis.y) : // Choose different up vector if parallel
        up;

    // Compute coordinate frame
    float3 xAxis = normalize(cross(fixedUp, zAxis));
    float3 yAxis = cross(zAxis, xAxis); // Note: no need to normalize since xAxis and zAxis are perpendicular

    // Build view matrix
    float4x4 viewMatrix;
    viewMatrix._11 = xAxis.x;
    viewMatrix._12 = yAxis.x;
    viewMatrix._13 = zAxis.x;
    viewMatrix._14 = 0.0f;

    viewMatrix._21 = xAxis.y;
    viewMatrix._22 = yAxis.y;
    viewMatrix._23 = zAxis.y;
    viewMatrix._24 = 0.0f;

    viewMatrix._31 = xAxis.z;
    viewMatrix._32 = yAxis.z;
    viewMatrix._33 = zAxis.z;
    viewMatrix._34 = 0.0f;

    viewMatrix._41 = -dot(xAxis, eye);
    viewMatrix._42 = -dot(yAxis, eye);
    viewMatrix._43 = -dot(zAxis, eye);
    viewMatrix._44 = 1.0f;

    return viewMatrix;
}

struct CascadeMatrices {
    float4x4 WorldToLightView;
    float4x4 ViewProj;
    float4x4 Orthographic;
};

 BoundBox GetBoxInsideFrustum(const ViewFrustum& Frustum, const BoundBox& Box)
 {
     // First check if there's any intersection at all
     auto Visibility = GetBoxVisibility(Frustum, Box);
     if (Visibility == BoxVisibility::Invisible)
         return BoundBox::Invalid();
     if (Visibility == BoxVisibility::FullyVisible)
         return Box;
 
     // Start with the original box
     BoundBox Result = Box;
 
     // Iterate over each plane of the frustum
     for (Uint32 plane_idx = 0; plane_idx < ViewFrustum::NUM_PLANES; ++plane_idx)
     {
         const Plane3D& CurrPlane = Frustum.GetPlane(static_cast<ViewFrustum::PLANE_IDX>(plane_idx));
 
         // Check each corner of the box
         for (int i = 0; i < 8; ++i)
         {
             float3 Corner = Result.GetCorner(i);
             float Distance = dot(Corner, CurrPlane.Normal) + CurrPlane.Distance;
 
             // If the corner is outside the plane, adjust it
             if (Distance < 0)
             {
                 float3 Adjustment = CurrPlane.Normal * -Distance;
                 Corner += Adjustment;
 
                 // Update the box with the adjusted corner
                 Result = Result.Enclose(Corner);
             }
         }
     }
 
     return Result;
 }

CascadeMatrices ComputeCascadeViewProj(const HLSL::CameraAttribs& cameraAttribs,const float3& lightDirection, const BoundBox& aabb, const ShadowMap::ShadowSettings& shadowSettings)
{
    float4x4 WorldToLightView = LookAt(float3(0), -lightDirection, float3(0.0f, 1.0f, 0.0f));

    //float3 cameraPos = float3(-cameraAttribs.f4Position.x, 0.0f, -cameraAttribs.f4Position.z); 
    //BoundBox box = aabb.Transform(float4x4::Translation(cameraPos));
    BoundBox box = aabb;

    // Get the corners of the model's bounding box
    std::array<float3, 8> corners = {
        float3(box.Min.x, box.Min.y, box.Min.z),
        float3(box.Max.x, box.Min.y, box.Min.z),
        float3(box.Min.x, box.Max.y, box.Min.z),
        float3(box.Max.x, box.Max.y, box.Min.z),
        float3(box.Min.x, box.Min.y, box.Max.z),
        float3(box.Max.x, box.Min.y, box.Max.z),
        float3(box.Min.x, box.Max.y, box.Max.z),
        float3(box.Max.x, box.Max.y, box.Max.z)
    };

    // Transform the corners to light space
    float3 minCorner = float3(std::numeric_limits<float>::max());
    float3 maxCorner = float3(std::numeric_limits<float>::lowest());

    for (const auto& corner : corners)
    {
        float4 transformed = float4(corner, 1.0f) * WorldToLightView;
        minCorner = min(minCorner, float3(transformed));
        maxCorner = max(maxCorner, float3(transformed));
    }

    // Add padding to avoid edge cases (using similar scale as DistributeCascades)
    float width = maxCorner.x - minCorner.x;
    float height = maxCorner.y - minCorner.y;
    float depth = maxCorner.z - minCorner.z;
    float largestDimension = std::max(width, height);
    
    // Center the projection
    float centerX = (minCorner.x + maxCorner.x) * 0.5f;
    float centerY = (minCorner.y + maxCorner.y) * 0.5f;
    
    // Compute extended bounds with fixed margin
    float shadowMapSize = static_cast<float>(shadowSettings.Resolution);
    float fixedMargin = 0.5f; // Matches DistributeCascades fixed margin
    float marginScale = shadowMapSize / (shadowMapSize - 2.0f * fixedMargin);
    
    largestDimension *= marginScale;
    
    minCorner.x = centerX - largestDimension * 0.5f;
    maxCorner.x = centerX + largestDimension * 0.5f;
    minCorner.y = centerY - largestDimension * 0.5f;
    maxCorner.y = centerY + largestDimension * 0.5f;

    // Extend Z range proportionally and add bias
    float zExtension = 0.1f; // Similar to DistributeCascades z extension
    float zRange = maxCorner.z - minCorner.z;
    zRange *= (1.0f / (1.0f - zExtension * 2.0f));
    float zCenter = (maxCorner.z + minCorner.z) * 0.5f;
    minCorner.z = zCenter - zRange;
    maxCorner.z = zCenter + zRange;

    // Compute the projection matrix for the light
    // Note: Using negative Z range to match DistributeCascades convention
    float4x4 lightProj = Orthographic(minCorner.x, maxCorner.x, minCorner.y, maxCorner.y, maxCorner.z, minCorner.z);

    // Return the combined view-projection matrix
    CascadeMatrices res = {WorldToLightView, WorldToLightView * lightProj, lightProj};

    {
        // Test that the center of the bounding box projects to the center of the depth buffer
        float3 boxCenter = (box.Min + box.Max) * 0.5f;
        float4 centerViewSpace = float4(boxCenter, 1.0f) * WorldToLightView;
        float4 centerLightSpace = centerViewSpace * lightProj;

        // Perspective divide to get NDC coordinates
        float3 centerNDC = float3(centerLightSpace) / centerLightSpace.w;
        
        // NDC coordinates should be very close to (0,0,z) since we centered the projection
        const float epsilon = 0.001f;
        assert(std::abs(centerNDC.x) < epsilon && "Box center X not at depth buffer center");
        assert(std::abs(centerNDC.y) < epsilon && "Box center Y not at depth buffer center");
    }

    return res;
}

CascadeMatrices ComputeCameraFrustumCascade(const HLSL::CameraAttribs& cameraAttribs, const float4x4& modelTransform, const BoundBox& aabb, const float3& lightDirection, 
    const ShadowMap::ShadowSettings& shadowSettings)
{
    //ViewFrustum Frustum;

    //ExtractViewFrustumPlanesFromMatrix(cameraAttribs.mViewProj.Transpose(), Frustum, false);
    //auto box = GetBoxInsideFrustum(Frustum, aabb);

    float3 cameraPos = float3(cameraAttribs.f4Position.x, 0.0f, cameraAttribs.f4Position.z); 
    //BoundBox box = aabb.Transform(float4x4::Translation(cameraPos));

    BoundBox box = aabb;
    box.Min.x = -18.0f;
    box.Max.x = 18.0f;
    box.Min.z = -18.0f;
    box.Max.z = 18.0f;

    box = box.Transform(modelTransform);
    box = box.Transform(float4x4::Translation(cameraPos));

    CascadeMatrices res = ComputeCascadeViewProj(cameraAttribs, lightDirection, box, shadowSettings);

    return res;
}

void ShadowMap::RenderShadowMap(const HLSL::CameraAttribs& CurrCamAttribs, float3 Direction, VulkanContext::frame_id_t inFlightIndex, const SF_GLTF_PBR_Renderer::RenderInfo& RenderParams, HLSL::PBRShadowMapInfo* shadowInfo, GraphicsContext::SFModel& model)
{
    DiligentShadowMapManager::DistributeCascadeInfo DistrInfo;
    auto camWorld = CurrCamAttribs.f4Position;
    auto view = CurrCamAttribs.mView.Transpose();
    auto proj = CurrCamAttribs.mProj.Transpose();
    auto viewProj = CurrCamAttribs.mViewProj.Transpose();
    DistrInfo.pCameraView = &view;
    DistrInfo.pCameraProj = &proj;
    DistrInfo.pLightDir = &Direction;
    DistrInfo.fPartitioningFactor = 0.95f;
    DistrInfo.UseRightHandedLightViewTransform = true;

    CascadeMatrices cascadeViewProj{};

    DistrInfo.AdjustCascadeRange = [view, proj, viewProj, &model, inFlightIndex](int CascadeIdx, float& MinZ, float& MaxZ) {

        // Adjust the whole range only
        if(CascadeIdx == -1)
        {
#if 0
            // Get the corners of the model's bounding box
            const auto& aabb = model.aabb;
            std::array<float3, 8> corners = {
                float3(aabb.Min.x, aabb.Min.y, aabb.Min.z),
                float3(aabb.Max.x, aabb.Min.y, aabb.Min.z),
                float3(aabb.Min.x, aabb.Max.y, aabb.Min.z),
                float3(aabb.Max.x, aabb.Max.y, aabb.Min.z),
                float3(aabb.Min.x, aabb.Min.y, aabb.Max.z),
                float3(aabb.Max.x, aabb.Min.y, aabb.Max.z),
                float3(aabb.Min.x, aabb.Max.y, aabb.Max.z),
                float3(aabb.Max.x, aabb.Max.y, aabb.Max.z)
            };

            // Transform the corners to light space
            float minZ = std::numeric_limits<float>::max();
            float maxZ = std::numeric_limits<float>::lowest();

            //OutputDebugString(("AABB Min: " + std::to_string(aabb.Min.z) + "\n").c_str());
            //OutputDebugString(("AABB Max: " + std::to_string(aabb.Max.z) + "\n").c_str());

#if 0
            for (const auto& corner : corners)
            {
                float4 transformed = float4(corner, 1.0f) * viewProj;
                float z = transformed.z / transformed.w; // Perspective divide

                if (z < minZ) minZ = z;
                if (z > maxZ) maxZ = z;
            }

            if (minZ < 0.10f)
            {
                minZ = 0.10f;
            }

            MinZ = minZ;
            MaxZ = maxZ;
#else
            for (const auto& corner : corners)
            {
                float4 test = { -122.0f, -2.0f, -122.0f, 1.0f };
                float4 testTrans = test * model.modelTransform;
                testTrans = testTrans * viewProj;

                float4 transformed = float4(corner, 1.0f);
                transformed = transformed * viewProj;
                transformed /= transformed.w;
                float z = transformed.z;

                if (z < minZ) minZ = z;
                if (z > maxZ) maxZ = z;
            }

            //OutputDebugString(("View Space MinZ: " + std::to_string(minZ) + "\n").c_str());
            //OutputDebugString(("View Space MaxZ: " + std::to_string(maxZ) + "\n").c_str());

            // Convert to positive distances
            float nearPlane = std::max(0.1f, minZ);  // Closest point (least negative becomes smallest positive)
            float farPlane = maxZ;   // Farthest point (most negative becomes largest positive)

            // Debug: Print converted distances
            //OutputDebugString(("Near Plane: " + std::to_string(nearPlane) + "\n").c_str());
            //OutputDebugString(("Far Plane: " + std::to_string(farPlane) + "\n").c_str());

            // Add padding
            MinZ = nearPlane;
            MaxZ = farPlane;

            //MinZ = 1.0f;
            //MaxZ = 12.0f;

            // Debug: Print final values
            //OutputDebugString(("Final MinZ: " + std::to_string(MinZ) + "\n").c_str());
            //OutputDebugString(("Final MaxZ: " + std::to_string(MaxZ) + "\n").c_str());
#endif
#endif

        }

    };

    m_ShadowMapMgr.DistributeCascades(DistrInfo, m_LightAttribs.ShadowAttribs);

    const int iNumShadowCascades = m_LightAttribs.ShadowAttribs.iNumCascades;
    for (int iCascade = 0; iCascade < iNumShadowCascades; ++iCascade)
    {
        const auto CascadeProjMatr = m_ShadowMapMgr.GetCascadeTranform(iCascade).Proj;

        auto WorldToLightViewSpaceMatr = m_LightAttribs.ShadowAttribs.mWorldToLightView.Transpose();
        auto WorldToLightProjSpaceMatr = WorldToLightViewSpaceMatr * CascadeProjMatr;

        HLSL::CameraAttribs ShadowCameraAttribs = {};

        ShadowCameraAttribs.mView = m_LightAttribs.ShadowAttribs.mWorldToLightView;
        ShadowCameraAttribs.mProj = CascadeProjMatr.Transpose();
        
        if(iCascade == 0)
        {
            cascadeViewProj = ComputeCameraFrustumCascade(CurrCamAttribs, model.modelTransform, model.worldspaceAABB, Direction, m_ShadowSettings);
        }
        else
        {
            cascadeViewProj = ComputeCascadeViewProj(CurrCamAttribs, Direction, model.aabb, m_ShadowSettings);
        }

        ShadowCameraAttribs.mViewProj = cascadeViewProj.ViewProj.Transpose();

        ShadowCameraAttribs.f4ViewportSize.x = static_cast<float>(m_ShadowSettings.Resolution);
        ShadowCameraAttribs.f4ViewportSize.y = static_cast<float>(m_ShadowSettings.Resolution);
        ShadowCameraAttribs.f4ViewportSize.z = 1.f / ShadowCameraAttribs.f4ViewportSize.x;
        ShadowCameraAttribs.f4ViewportSize.w = 1.f / ShadowCameraAttribs.f4ViewportSize.y;

        auto* pCascadeDSV = m_ShadowMapMgr.GetCascadeDSV(iCascade);
        s_gc.m_pImmediateContext->SetRenderTargets(0, nullptr, pCascadeDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        s_gc.m_pImmediateContext->ClearDepthStencil(pCascadeDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        if (model.dynamicMesh)
        {
            SF_GLTF_PBR_Renderer::RenderInfo ri = RenderParams;
            ri.Flags |= SF_GLTF_PBR_Renderer::PSO_FLAG_USE_HEIGHTMAP;
            ri.Flags |= SF_GLTF_PBR_Renderer::PSO_FLAG_USE_INSTANCING;
            ri.Flags |= SF_GLTF_PBR_Renderer::PSO_FLAG_USE_TERRAINING;
            ri.Flags |= SF_GLTF_PBR_Renderer::PSO_FLAG_USE_TEXCOORD1;

            DrawMesh(s_gc.m_pImmediateContext, *model.dynamicMesh, model.dynamicMeshTransforms[inFlightIndex & 0x01], ShadowCameraAttribs, ri);
        }
        else
        {
            DrawMesh(s_gc.m_pImmediateContext, *model.model, model.transforms[inFlightIndex & 0x01], ShadowCameraAttribs, RenderParams);
        }

        shadowInfo[iCascade].WorldToLightProjSpace = cascadeViewProj.ViewProj.Transpose();
        shadowInfo[iCascade].UVScale = { 1.0f, 1.0f };
        shadowInfo[iCascade].UVBias = { 0.0f, 0.0f };
        shadowInfo[iCascade].ShadowMapSlice = static_cast<float>(iCascade);
    }
}

template<std::size_t PLANES>
union TextureColor {
    std::array<float, PLANES> u;

    constexpr float& r() { return u[0]; }
    constexpr float& g() { return u[1]; }
    constexpr float& b() { return u[2]; }
    constexpr float& a() { return u[3]; }

    constexpr const float& r() const { return u[0]; }
    constexpr const float& g() const { return u[1]; }
    constexpr const float& b() const { return u[2]; }
    constexpr const float& a() const { return u[3]; }    
};

template<std::size_t PLANES>
uint32_t TextureColorToARGB(const TextureColor<PLANES>& color) {
    uint32_t a = static_cast<uint32_t>(color.a() * 255.0f);
    uint32_t r = static_cast<uint32_t>(color.r() * 255.0f);
    uint32_t g = static_cast<uint32_t>(color.g() * 255.0f);
    uint32_t b = static_cast<uint32_t>(color.b() * 255.0f);
    return (a << 24) | (r << 16) | (g << 8) | b;
}

template<std::size_t WIDTH, std::size_t HEIGHT, std::size_t PLANES>
struct Texture {
    std::array<std::array<TextureColor<PLANES>, WIDTH>, HEIGHT> data;
};

template<std::size_t WIDTH, std::size_t HEIGHT, std::size_t PLANES>
TextureColor<PLANES> bilinearSample(const Texture<WIDTH, HEIGHT, PLANES>& texture, float u, float v) {
    u *= WIDTH - 1;
    v *= HEIGHT - 1;

    int x = (int)u;
    int y = (int)v;
    float u_ratio = u - x;
    float v_ratio = v - y;
    float u_opposite = 1 - u_ratio;
    float v_opposite = 1 - v_ratio;

    TextureColor<PLANES> result;
    for (std::size_t plane = 0; plane < PLANES; ++plane) {
        result.u[plane] = (texture.data[y][x].u[plane] * u_opposite + texture.data[y][x+1].u[plane] * u_ratio) * v_opposite +
                          (texture.data[y+1][x].u[plane] * u_opposite  + texture.data[y+1][x+1].u[plane] * u_ratio) * v_ratio;
    }
    return result;
}

template<std::size_t WIDTH, std::size_t HEIGHT, std::size_t PLANES>
void fillTexture(Texture<WIDTH, HEIGHT, PLANES>& texture, const std::vector<uint8_t>& image) {
    for(int i = 0; i < WIDTH; ++i) {
        for(int j = 0; j < HEIGHT; ++j) {
            for(int k = 0; k < PLANES; ++k) {
                texture.data[j][i].u[k] = static_cast<float>(image[(j * WIDTH + i) * PLANES + k]) / 255.0f;
            }
        }
    }
}

template<std::size_t WIDTH, std::size_t HEIGHT, std::size_t PLANES>
void fillTextureLiteral(Texture<WIDTH, HEIGHT, PLANES>& texture, const std::vector<float>& image) {
    for (int i = 0; i < WIDTH; ++i) {
        for (int j = 0; j < HEIGHT; ++j) {
            for (int k = 0; k < PLANES; ++k) {
                texture.data[j][i].u[k] = image[(j * WIDTH + i) * PLANES + k];
            }
        }
    }
}

Texture<448, 160, 1> FONT1Texture;
Texture<840, 180, 1> FONT2Texture;
Texture<840, 220, 1> FONT3Texture;
Texture<2160, 392, 1> RaceDosPicTexture;

enum SFGraphicsMode
{
    Unset = -1,
    Text = 0,
    Graphics = 1,
};

SFGraphicsMode toSetGraphicsMode = Unset;
SFGraphicsMode graphicsMode = Unset;
std::counting_semaphore<700> modeChangeComplete{ 0 };

FrameSync frameSync{};

uint32_t graphicsDisplayOffset = 0;// 100 * 160;
std::mutex rotoscopePixelMutex;
std::vector<Rotoscope> rotoscopePixels;
std::vector<uint32_t> graphicsPixels;
std::vector<uint32_t> textPixels;

std::jthread graphicsThread{};
std::binary_semaphore stopSemaphore{0};
std::mutex graphicsRetrace{};

std::vector<uint8_t> serializedRotoscope;
std::vector<uint8_t> serializedSnapshot;

int cursorx = 0;
int cursory = 0;

std::jthread emulationThread;
std::atomic<bool> emulationThreadRunning = false;
std::atomic<bool> stopEmulationThread = false;
std::atomic<bool> pauseEmulationThread = false;

void StopEmulationThread() {
    stopEmulationThread = true;
    if (emulationThread.joinable()) {
        emulationThread.join(); 
    }
}

void StartEmulationThread(std::filesystem::path path)
{
    emulationThreadRunning = true;
    stopEmulationThread = false;

    emulationThread = std::jthread([path]() {

        SetCurrentThreadName("Emulation Thread");

        InitCPU();
        InitEmulator(path);
        enum RETURNCODE ret = OK;
        do
        {
            if(pauseEmulationThread)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(33));
            }
            else
            {
                ret = Step();
            }

            if (IsGraphicsShutdown())
                break;

            if (stopEmulationThread) {
                break;
            }
        } while (ret == OK || ret == EXIT);

        emulationThreadRunning = false;
    });
}

#if 0
class Spline2D {
public:
    template<typename Container>
        requires std::same_as<typename Container::value_type, TimePoint>
    Spline2D(const Container& points) {
        if (points.size() < 3) {
            throw std::runtime_error("Need at least 3 points for spline computation.");
        }

        for (const auto& point : points) {
            knots.push_back(point.frameTime);
            valuesX.push_back(point.position.x);
            valuesY.push_back(point.position.y);
        }
    }

    void addPoint(const TimePoint& point) {
        knots.push_back(point.frameTime);
        valuesX.push_back(point.position.x);
        valuesY.push_back(point.position.y);
    }

    void computeSplines() {
        splineX = std::make_unique<cubic_spline>(knots, valuesX, cubic_spline::natural);
        splineY = std::make_unique<cubic_spline>(knots, valuesY, cubic_spline::natural);
    }

    vec2<float> evaluate(double targetFrame) {
        if (!splineX || !splineY) {
            computeSplines();
        }
        return { (float)splineX->operator()(targetFrame), (float)splineY->operator()(targetFrame) };
    }

    double evaluateHeading(double targetFrame) {
        if (!splineX || !splineY) {
            computeSplines();
        }
        double tangentX = splineX->derivative(targetFrame);
        double tangentY = splineY->derivative(targetFrame);
        return atan2(tangentY, tangentX);
    }

    double evaluateVelocity(double targetFrame) {
        if (!splineX || !splineY) {
            computeSplines();
        }
        double velocityX = splineX->derivative(targetFrame);
        double velocityY = splineY->derivative(targetFrame);
        return sqrt(velocityX * velocityX + velocityY * velocityY);
    }

private:
    std::vector<double> knots, valuesX, valuesY;
    std::unique_ptr<cubic_spline> splineX, splineY;
};
#endif

uint32_t colortable[16] =
{
0x000000, // black
0x0000AA, // blue
0x00AA00, // green
0x00AAAA, // cyan
0xAA0000, // red
0xAA00AA, // magenta
0xAA5500, // brown
0xAAAAAA, // light gray
0x555555, // dark gray
0x5555FF,
0x55FF55,
0x55FFFF,
0xFF5555,
0xFF55FF,
0xFFFF55,
0xFFFFFF,
};

bool graphicsIsShutdown = false;

static uint8_t vgafont8[256*8] =
{
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x7e,0x81,0xa5,0x81,0xbd,0x99,0x81,0x7e,
0x7e,0xff,0xdb,0xff,0xc3,0xe7,0xff,0x7e,
0x6c,0xfe,0xfe,0xfe,0x7c,0x38,0x10,0x00,
0x10,0x38,0x7c,0xfe,0x7c,0x38,0x10,0x00,
0x38,0x7c,0x38,0xfe,0xfe,0x7c,0x38,0x7c,
0x10,0x10,0x38,0x7c,0xfe,0x7c,0x38,0x7c,
0x00,0x00,0x18,0x3c,0x3c,0x18,0x00,0x00,
0xff,0xff,0xe7,0xc3,0xc3,0xe7,0xff,0xff,
0x00,0x3c,0x66,0x42,0x42,0x66,0x3c,0x00,
0xff,0xc3,0x99,0xbd,0xbd,0x99,0xc3,0xff,
0x0f,0x07,0x0f,0x7d,0xcc,0xcc,0xcc,0x78,
0x3c,0x66,0x66,0x66,0x3c,0x18,0x7e,0x18,
0x3f,0x33,0x3f,0x30,0x30,0x70,0xf0,0xe0,
0x7f,0x63,0x7f,0x63,0x63,0x67,0xe6,0xc0,
0x99,0x5a,0x3c,0xe7,0xe7,0x3c,0x5a,0x99,
0x80,0xe0,0xf8,0xfe,0xf8,0xe0,0x80,0x00,
0x02,0x0e,0x3e,0xfe,0x3e,0x0e,0x02,0x00,
0x18,0x3c,0x7e,0x18,0x18,0x7e,0x3c,0x18,
0x66,0x66,0x66,0x66,0x66,0x00,0x66,0x00,
0x7f,0xdb,0xdb,0x7b,0x1b,0x1b,0x1b,0x00,
0x3e,0x63,0x38,0x6c,0x6c,0x38,0xcc,0x78,
0x00,0x00,0x00,0x00,0x7e,0x7e,0x7e,0x00,
0x18,0x3c,0x7e,0x18,0x7e,0x3c,0x18,0xff,
0x18,0x3c,0x7e,0x18,0x18,0x18,0x18,0x00,
0x18,0x18,0x18,0x18,0x7e,0x3c,0x18,0x00,
0x00,0x18,0x0c,0xfe,0x0c,0x18,0x00,0x00,
0x00,0x30,0x60,0xfe,0x60,0x30,0x00,0x00,
0x00,0x00,0xc0,0xc0,0xc0,0xfe,0x00,0x00,
0x00,0x24,0x66,0xff,0x66,0x24,0x00,0x00,
0x00,0x18,0x3c,0x7e,0xff,0xff,0x00,0x00,
0x00,0xff,0xff,0x7e,0x3c,0x18,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x30,0x78,0x78,0x30,0x30,0x00,0x30,0x00,
0x6c,0x6c,0x6c,0x00,0x00,0x00,0x00,0x00,
0x6c,0x6c,0xfe,0x6c,0xfe,0x6c,0x6c,0x00,
0x30,0x7c,0xc0,0x78,0x0c,0xf8,0x30,0x00,
0x00,0xc6,0xcc,0x18,0x30,0x66,0xc6,0x00,
0x38,0x6c,0x38,0x76,0xdc,0xcc,0x76,0x00,
0x60,0x60,0xc0,0x00,0x00,0x00,0x00,0x00,
0x18,0x30,0x60,0x60,0x60,0x30,0x18,0x00,
0x60,0x30,0x18,0x18,0x18,0x30,0x60,0x00,
0x00,0x66,0x3c,0xff,0x3c,0x66,0x00,0x00,
0x00,0x30,0x30,0xfc,0x30,0x30,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x60,
0x00,0x00,0x00,0xfc,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00,
0x06,0x0c,0x18,0x30,0x60,0xc0,0x80,0x00,
0x7c,0xc6,0xce,0xde,0xf6,0xe6,0x7c,0x00,
0x30,0x70,0x30,0x30,0x30,0x30,0xfc,0x00,
0x78,0xcc,0x0c,0x38,0x60,0xcc,0xfc,0x00,
0x78,0xcc,0x0c,0x38,0x0c,0xcc,0x78,0x00,
0x1c,0x3c,0x6c,0xcc,0xfe,0x0c,0x1e,0x00,
0xfc,0xc0,0xf8,0x0c,0x0c,0xcc,0x78,0x00,
0x38,0x60,0xc0,0xf8,0xcc,0xcc,0x78,0x00,
0xfc,0xcc,0x0c,0x18,0x30,0x30,0x30,0x00,
0x78,0xcc,0xcc,0x78,0xcc,0xcc,0x78,0x00,
0x78,0xcc,0xcc,0x7c,0x0c,0x18,0x70,0x00,
0x00,0x30,0x30,0x00,0x00,0x30,0x30,0x00,
0x00,0x30,0x30,0x00,0x00,0x30,0x30,0x60,
0x18,0x30,0x60,0xc0,0x60,0x30,0x18,0x00,
0x00,0x00,0xfc,0x00,0x00,0xfc,0x00,0x00,
0x60,0x30,0x18,0x0c,0x18,0x30,0x60,0x00,
0x78,0xcc,0x0c,0x18,0x30,0x00,0x30,0x00,
0x7c,0xc6,0xde,0xde,0xde,0xc0,0x78,0x00,
0x30,0x78,0xcc,0xcc,0xfc,0xcc,0xcc,0x00,
0xfc,0x66,0x66,0x7c,0x66,0x66,0xfc,0x00,
0x3c,0x66,0xc0,0xc0,0xc0,0x66,0x3c,0x00,
0xf8,0x6c,0x66,0x66,0x66,0x6c,0xf8,0x00,
0xfe,0x62,0x68,0x78,0x68,0x62,0xfe,0x00,
0xfe,0x62,0x68,0x78,0x68,0x60,0xf0,0x00,
0x3c,0x66,0xc0,0xc0,0xce,0x66,0x3e,0x00,
0xcc,0xcc,0xcc,0xfc,0xcc,0xcc,0xcc,0x00,
0x78,0x30,0x30,0x30,0x30,0x30,0x78,0x00,
0x1e,0x0c,0x0c,0x0c,0xcc,0xcc,0x78,0x00,
0xe6,0x66,0x6c,0x78,0x6c,0x66,0xe6,0x00,
0xf0,0x60,0x60,0x60,0x62,0x66,0xfe,0x00,
0xc6,0xee,0xfe,0xfe,0xd6,0xc6,0xc6,0x00,
0xc6,0xe6,0xf6,0xde,0xce,0xc6,0xc6,0x00,
0x38,0x6c,0xc6,0xc6,0xc6,0x6c,0x38,0x00,
0xfc,0x66,0x66,0x7c,0x60,0x60,0xf0,0x00,
0x78,0xcc,0xcc,0xcc,0xdc,0x78,0x1c,0x00,
0xfc,0x66,0x66,0x7c,0x6c,0x66,0xe6,0x00,
0x78,0xcc,0xe0,0x70,0x1c,0xcc,0x78,0x00,
0xfc,0xb4,0x30,0x30,0x30,0x30,0x78,0x00,
0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0xfc,0x00,
0xcc,0xcc,0xcc,0xcc,0xcc,0x78,0x30,0x00,
0xc6,0xc6,0xc6,0xd6,0xfe,0xee,0xc6,0x00,
0xc6,0xc6,0x6c,0x38,0x38,0x6c,0xc6,0x00,
0xcc,0xcc,0xcc,0x78,0x30,0x30,0x78,0x00,
0xfe,0xc6,0x8c,0x18,0x32,0x66,0xfe,0x00,
0x78,0x60,0x60,0x60,0x60,0x60,0x78,0x00,
0xc0,0x60,0x30,0x18,0x0c,0x06,0x02,0x00,
0x78,0x18,0x18,0x18,0x18,0x18,0x78,0x00,
0x10,0x38,0x6c,0xc6,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
0x30,0x30,0x18,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x78,0x0c,0x7c,0xcc,0x76,0x00,
0xe0,0x60,0x60,0x7c,0x66,0x66,0xdc,0x00,
0x00,0x00,0x78,0xcc,0xc0,0xcc,0x78,0x00,
0x1c,0x0c,0x0c,0x7c,0xcc,0xcc,0x76,0x00,
0x00,0x00,0x78,0xcc,0xfc,0xc0,0x78,0x00,
0x38,0x6c,0x60,0xf0,0x60,0x60,0xf0,0x00,
0x00,0x00,0x76,0xcc,0xcc,0x7c,0x0c,0xf8,
0xe0,0x60,0x6c,0x76,0x66,0x66,0xe6,0x00,
0x30,0x00,0x70,0x30,0x30,0x30,0x78,0x00,
0x0c,0x00,0x0c,0x0c,0x0c,0xcc,0xcc,0x78,
0xe0,0x60,0x66,0x6c,0x78,0x6c,0xe6,0x00,
0x70,0x30,0x30,0x30,0x30,0x30,0x78,0x00,
0x00,0x00,0xcc,0xfe,0xfe,0xd6,0xc6,0x00,
0x00,0x00,0xf8,0xcc,0xcc,0xcc,0xcc,0x00,
0x00,0x00,0x78,0xcc,0xcc,0xcc,0x78,0x00,
0x00,0x00,0xdc,0x66,0x66,0x7c,0x60,0xf0,
0x00,0x00,0x76,0xcc,0xcc,0x7c,0x0c,0x1e,
0x00,0x00,0xdc,0x76,0x66,0x60,0xf0,0x00,
0x00,0x00,0x7c,0xc0,0x78,0x0c,0xf8,0x00,
0x10,0x30,0x7c,0x30,0x30,0x34,0x18,0x00,
0x00,0x00,0xcc,0xcc,0xcc,0xcc,0x76,0x00,
0x00,0x00,0xcc,0xcc,0xcc,0x78,0x30,0x00,
0x00,0x00,0xc6,0xd6,0xfe,0xfe,0x6c,0x00,
0x00,0x00,0xc6,0x6c,0x38,0x6c,0xc6,0x00,
0x00,0x00,0xcc,0xcc,0xcc,0x7c,0x0c,0xf8,
0x00,0x00,0xfc,0x98,0x30,0x64,0xfc,0x00,
0x1c,0x30,0x30,0xe0,0x30,0x30,0x1c,0x00,
0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00,
0xe0,0x30,0x30,0x1c,0x30,0x30,0xe0,0x00,
0x76,0xdc,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x10,0x38,0x6c,0xc6,0xc6,0xfe,0x00,
0x78,0xcc,0xc0,0xcc,0x78,0x18,0x0c,0x78,
0x00,0xcc,0x00,0xcc,0xcc,0xcc,0x7e,0x00,
0x1c,0x00,0x78,0xcc,0xfc,0xc0,0x78,0x00,
0x7e,0xc3,0x3c,0x06,0x3e,0x66,0x3f,0x00,
0xcc,0x00,0x78,0x0c,0x7c,0xcc,0x7e,0x00,
0xe0,0x00,0x78,0x0c,0x7c,0xcc,0x7e,0x00,
0x30,0x30,0x78,0x0c,0x7c,0xcc,0x7e,0x00,
0x00,0x00,0x78,0xc0,0xc0,0x78,0x0c,0x38,
0x7e,0xc3,0x3c,0x66,0x7e,0x60,0x3c,0x00,
0xcc,0x00,0x78,0xcc,0xfc,0xc0,0x78,0x00,
0xe0,0x00,0x78,0xcc,0xfc,0xc0,0x78,0x00,
0xcc,0x00,0x70,0x30,0x30,0x30,0x78,0x00,
0x7c,0xc6,0x38,0x18,0x18,0x18,0x3c,0x00,
0xe0,0x00,0x70,0x30,0x30,0x30,0x78,0x00,
0xc6,0x38,0x6c,0xc6,0xfe,0xc6,0xc6,0x00,
0x30,0x30,0x00,0x78,0xcc,0xfc,0xcc,0x00,
0x1c,0x00,0xfc,0x60,0x78,0x60,0xfc,0x00,
0x00,0x00,0x7f,0x0c,0x7f,0xcc,0x7f,0x00,
0x3e,0x6c,0xcc,0xfe,0xcc,0xcc,0xce,0x00,
0x78,0xcc,0x00,0x78,0xcc,0xcc,0x78,0x00,
0x00,0xcc,0x00,0x78,0xcc,0xcc,0x78,0x00,
0x00,0xe0,0x00,0x78,0xcc,0xcc,0x78,0x00,
0x78,0xcc,0x00,0xcc,0xcc,0xcc,0x7e,0x00,
0x00,0xe0,0x00,0xcc,0xcc,0xcc,0x7e,0x00,
0x00,0xcc,0x00,0xcc,0xcc,0x7c,0x0c,0xf8,
0xc3,0x18,0x3c,0x66,0x66,0x3c,0x18,0x00,
0xcc,0x00,0xcc,0xcc,0xcc,0xcc,0x78,0x00,
0x18,0x18,0x7e,0xc0,0xc0,0x7e,0x18,0x18,
0x38,0x6c,0x64,0xf0,0x60,0xe6,0xfc,0x00,
0xcc,0xcc,0x78,0xfc,0x30,0xfc,0x30,0x30,
0xf8,0xcc,0xcc,0xfa,0xc6,0xcf,0xc6,0xc7,
0x0e,0x1b,0x18,0x3c,0x18,0x18,0xd8,0x70,
0x1c,0x00,0x78,0x0c,0x7c,0xcc,0x7e,0x00,
0x38,0x00,0x70,0x30,0x30,0x30,0x78,0x00,
0x00,0x1c,0x00,0x78,0xcc,0xcc,0x78,0x00,
0x00,0x1c,0x00,0xcc,0xcc,0xcc,0x7e,0x00,
0x00,0xf8,0x00,0xf8,0xcc,0xcc,0xcc,0x00,
0xfc,0x00,0xcc,0xec,0xfc,0xdc,0xcc,0x00,
0x3c,0x6c,0x6c,0x3e,0x00,0x7e,0x00,0x00,
0x38,0x6c,0x6c,0x38,0x00,0x7c,0x00,0x00,
0x30,0x00,0x30,0x60,0xc0,0xcc,0x78,0x00,
0x00,0x00,0x00,0xfc,0xc0,0xc0,0x00,0x00,
0x00,0x00,0x00,0xfc,0x0c,0x0c,0x00,0x00,
0xc3,0xc6,0xcc,0xde,0x33,0x66,0xcc,0x0f,
0xc3,0xc6,0xcc,0xdb,0x37,0x6f,0xcf,0x03,
0x18,0x18,0x00,0x18,0x18,0x18,0x18,0x00,
0x00,0x33,0x66,0xcc,0x66,0x33,0x00,0x00,
0x00,0xcc,0x66,0x33,0x66,0xcc,0x00,0x00,
0x22,0x88,0x22,0x88,0x22,0x88,0x22,0x88,
0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa,
0xdb,0x77,0xdb,0xee,0xdb,0x77,0xdb,0xee,
0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0xf8,0x18,0x18,0x18,
0x18,0x18,0xf8,0x18,0xf8,0x18,0x18,0x18,
0x36,0x36,0x36,0x36,0xf6,0x36,0x36,0x36,
0x00,0x00,0x00,0x00,0xfe,0x36,0x36,0x36,
0x00,0x00,0xf8,0x18,0xf8,0x18,0x18,0x18,
0x36,0x36,0xf6,0x06,0xf6,0x36,0x36,0x36,
0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36,
0x00,0x00,0xfe,0x06,0xf6,0x36,0x36,0x36,
0x36,0x36,0xf6,0x06,0xfe,0x00,0x00,0x00,
0x36,0x36,0x36,0x36,0xfe,0x00,0x00,0x00,
0x18,0x18,0xf8,0x18,0xf8,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xf8,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0x1f,0x00,0x00,0x00,
0x18,0x18,0x18,0x18,0xff,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0xff,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0x1f,0x18,0x18,0x18,
0x00,0x00,0x00,0x00,0xff,0x00,0x00,0x00,
0x18,0x18,0x18,0x18,0xff,0x18,0x18,0x18,
0x18,0x18,0x1f,0x18,0x1f,0x18,0x18,0x18,
0x36,0x36,0x36,0x36,0x37,0x36,0x36,0x36,
0x36,0x36,0x37,0x30,0x3f,0x00,0x00,0x00,
0x00,0x00,0x3f,0x30,0x37,0x36,0x36,0x36,
0x36,0x36,0xf7,0x00,0xff,0x00,0x00,0x00,
0x00,0x00,0xff,0x00,0xf7,0x36,0x36,0x36,
0x36,0x36,0x37,0x30,0x37,0x36,0x36,0x36,
0x00,0x00,0xff,0x00,0xff,0x00,0x00,0x00,
0x36,0x36,0xf7,0x00,0xf7,0x36,0x36,0x36,
0x18,0x18,0xff,0x00,0xff,0x00,0x00,0x00,
0x36,0x36,0x36,0x36,0xff,0x00,0x00,0x00,
0x00,0x00,0xff,0x00,0xff,0x18,0x18,0x18,
0x00,0x00,0x00,0x00,0xff,0x36,0x36,0x36,
0x36,0x36,0x36,0x36,0x3f,0x00,0x00,0x00,
0x18,0x18,0x1f,0x18,0x1f,0x00,0x00,0x00,
0x00,0x00,0x1f,0x18,0x1f,0x18,0x18,0x18,
0x00,0x00,0x00,0x00,0x3f,0x36,0x36,0x36,
0x36,0x36,0x36,0x36,0xff,0x36,0x36,0x36,
0x18,0x18,0xff,0x18,0xff,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0xf8,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x1f,0x18,0x18,0x18,
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,
0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,
0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,
0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,
0x00,0x00,0x76,0xdc,0xc8,0xdc,0x76,0x00,
0x00,0x78,0xcc,0xf8,0xcc,0xf8,0xc0,0xc0,
0x00,0xfc,0xcc,0xc0,0xc0,0xc0,0xc0,0x00,
0x00,0xfe,0x6c,0x6c,0x6c,0x6c,0x6c,0x00,
0xfc,0xcc,0x60,0x30,0x60,0xcc,0xfc,0x00,
0x00,0x00,0x7e,0xd8,0xd8,0xd8,0x70,0x00,
0x00,0x66,0x66,0x66,0x66,0x7c,0x60,0xc0,
0x00,0x76,0xdc,0x18,0x18,0x18,0x18,0x00,
0xfc,0x30,0x78,0xcc,0xcc,0x78,0x30,0xfc,
0x38,0x6c,0xc6,0xfe,0xc6,0x6c,0x38,0x00,
0x38,0x6c,0xc6,0xc6,0x6c,0x6c,0xee,0x00,
0x1c,0x30,0x18,0x7c,0xcc,0xcc,0x78,0x00,
0x00,0x00,0x7e,0xdb,0xdb,0x7e,0x00,0x00,
0x06,0x0c,0x7e,0xdb,0xdb,0x7e,0x60,0xc0,
0x38,0x60,0xc0,0xf8,0xc0,0x60,0x38,0x00,
0x78,0xcc,0xcc,0xcc,0xcc,0xcc,0xcc,0x00,
0x00,0xfc,0x00,0xfc,0x00,0xfc,0x00,0x00,
0x30,0x30,0xfc,0x30,0x30,0x00,0xfc,0x00,
0x60,0x30,0x18,0x30,0x60,0x00,0xfc,0x00,
0x18,0x30,0x60,0x30,0x18,0x00,0xfc,0x00,
0x0e,0x1b,0x1b,0x18,0x18,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0x18,0xd8,0xd8,0x70,
0x30,0x30,0x00,0xfc,0x00,0x30,0x30,0x00,
0x00,0x76,0xdc,0x00,0x76,0xdc,0x00,0x00,
0x38,0x6c,0x6c,0x38,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00,
0x0f,0x0c,0x0c,0x0c,0xec,0x6c,0x3c,0x1c,
0x78,0x6c,0x6c,0x6c,0x6c,0x00,0x00,0x00,
0x70,0x18,0x30,0x60,0x78,0x00,0x00,0x00,
0x00,0x00,0x3c,0x3c,0x3c,0x3c,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

void FrameSync::SetOrbitState(OrbitState state, std::optional<vec3<float>> optionalCamPos)
{
    orbitState = state;

    if(state == OrbitState::Insertion)
    {
        if (!optionalCamPos.has_value())
        {
            auto pos = vec3<float>(0.0f, 0.0f, 0.0f);
            optionalCamPos.emplace(pos);
        }

        auto pole = Magnum::Vector3( frameSync.staringPos.x, frameSync.staringPos.y, frameSync.staringPos.z );
        auto dest = Magnum::Vector3( optionalCamPos->x, optionalCamPos->y, optionalCamPos->z );

        positionTrack = Magnum::Animation::Track<Magnum::Float, Magnum::Vector3, Magnum::Math::Vector3<Magnum::Float>>{
            {{0.0f, pole}, {3.0f, dest}}, // Keyframe data
            Magnum::Math::lerp,           // Interpolator function
            Magnum::Animation::Extrapolation::Constant, // Extrapolation before
            Magnum::Animation::Extrapolation::Constant  // Extrapolation after
        };

        scaleTrack = Magnum::Animation::Track<Magnum::Float, Magnum::Float, Magnum::Float>{
            {{0.0f, 100.0f}, {3.0f, static_cast<float>(currentPlanetSphereSize)}}, // Keyframe data
            Magnum::Math::lerp,           // Interpolator function
            Magnum::Animation::Extrapolation::Constant, // Extrapolation before
            Magnum::Animation::Extrapolation::Constant  // Extrapolation after
        };
    }

    if(state == OrbitState::Landing)
    {
        if (!orbitCamPos.has_value())
        {
            auto pos = vec3<float>(0.0f, 0.0f, 0.0f);
            orbitCamPos.emplace(pos);
        }

        auto start = Magnum::Vector3( orbitCamPos->x, orbitCamPos->y, orbitCamPos->z );
        auto end = Magnum::Vector3(0.0f, 1.0f, 0.0f);

        positionTrack = Magnum::Animation::Track<Magnum::Float, Magnum::Vector3, Magnum::Math::Vector3<Magnum::Float>>{
            {{0.0f, start}, {3.0f, end}}, // Keyframe data
            Magnum::Math::lerp,           // Interpolator function
            Magnum::Animation::Extrapolation::Constant, // Extrapolation before
            Magnum::Animation::Extrapolation::Constant  // Extrapolation after
        };

        scaleTrack = Magnum::Animation::Track<Magnum::Float, Magnum::Float, Magnum::Float>{
            {{0.0f, static_cast<float>(currentPlanetSphereSize)}, {3.0f, 1000.0f}}, // Keyframe data
            Magnum::Math::lerp,           // Interpolator function
            Magnum::Animation::Extrapolation::Constant, // Extrapolation before
            Magnum::Animation::Extrapolation::Constant  // Extrapolation after
        };

    }

    orbitTimestamp = std::chrono::steady_clock::now();

    if (optionalCamPos.has_value())
    {
        orbitCamPos = optionalCamPos;
    }
}


OrbitStatus FrameSync::GetOrbitStatus()
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - orbitTimestamp).count();
    
    switch (orbitState)
    {
    case OrbitState::Holding:
        return { frameSync.staringPos, 100.0f };
    case OrbitState::Insertion:
        {
            float t = (float)elapsed / 1000.0f;

            auto pos = positionTrack.at(t);
            auto s = scaleTrack.at(t);

            if (t >= 3.0f) {
                SetOrbitState(OrbitState::Orbiting);
            }

            return { {pos.x(), pos.y(), pos.z() }, s};
        }
        break;
    case OrbitState::Landing:
        {
            float t = (float)elapsed / 1000.0f;

            auto pos = positionTrack.at(t);
            auto s = scaleTrack.at(t);

            if (t >= 3.0f) {
                SetOrbitState(OrbitState::Landed);

            }

            return { {pos.x(), pos.y(), pos.z() }, s };
        }
        break;
    case OrbitState::Landed:
        return { {0.0f, 1.0f, 0.0f}, 1000.0f };
    case OrbitState::Takeoff:
    case OrbitState::Orbiting:
        return { orbitCamPos.value(), (float)currentPlanetSphereSize };
    default:
        assert(false);
    }

    assert(false);
    return { vec3<float>(), 100.0f };
}


static uint8_t s_keyboardState[1024] = {};

class DOSKeyboard {
public:
    // Destructive read equivalent to Int 16 ah = 0
    virtual bool checkForKeyStroke() = 0;
    // Destructive read equivalent to Int 16 ah = 0
    virtual unsigned short getKeyStroke() = 0;

    virtual void pushKeyStroke(uint16_t key) {};

    virtual bool areArrowKeysDown() { return false; }

    virtual void update() = 0;

    virtual ~DOSKeyboard() = default;
};

class CLIKeyboard : public DOSKeyboard {
private:
    std::deque<uint16_t> queuedString{};

public:
    CLIKeyboard() = default;

    virtual ~CLIKeyboard() = default;

    bool checkForKeyStroke() override {
        return true;
    }

    unsigned short getKeyStroke() override {
        if(queuedString.empty())
        {
            printf("input: ");
            fflush(stdout);
            int c;
            do {
                c = getchar();
                queuedString.push_back(c);
            } while(c != '\n');
        }

        auto ret = queuedString.front();
        queuedString.pop_front();

        return ret;
    }

    void update() override {

    }
};

static nk_buttons sdl_button_to_nk(int button)
{
    switch (button)
    {
    default:
        /* ft */
    case SDL_BUTTON_LEFT:
        return NK_BUTTON_LEFT;
        break;
    case SDL_BUTTON_MIDDLE:
        return NK_BUTTON_MIDDLE;
        break;
    case SDL_BUTTON_RIGHT:
        return NK_BUTTON_RIGHT;
        break;

    }
}

namespace cereal {

template<class Archive>
void serialize(Archive& ar, NavigationData& data) {
    ar(data.window_x, data.window_y);
}

template<class Archive>
void serialize(Archive& ar, TextData& data) {
    ar(data.character, data.xormode, data.fontNum);
}

template<class Archive>
void serialize(Archive& ar, PicData& data) {
    ar(data.picID);
}

template<class Archive>
void serialize(Archive& ar, LineData& data) {
    ar(data.x0, data.y0, data.x1, data.y1, data.n, data.total);
}

template<class Archive>
void serialize(Archive& ar, RunBitData& data) {
    ar(data.tag);
}

template <class Archive>
void serialize(Archive & ar, Rotoscope & rotoscope) {
    ar(rotoscope.content, rotoscope.EGAcolor, rotoscope.argb,
       rotoscope.blt_x, rotoscope.blt_y, rotoscope.blt_w, rotoscope.blt_h,
       rotoscope.bgColor, rotoscope.fgColor);

    switch(rotoscope.content) {
        case NavigationalPixel:
            ar(rotoscope.navigationData);
            break;
        case TextPixel:
            ar(rotoscope.textData);
            break;
        case PicPixel:
            ar(rotoscope.picData);
            break;
        case LinePixel:
            ar(rotoscope.lineData);
            break;
        case RunBitPixel:
            ar(rotoscope.runBitData);
            break;
        default:
            break;
    }
}

}

class SDLKeyboard : public DOSKeyboard {
private:
    std::mutex eventQueueMutex;
    std::deque<SDL_Event> eventQueue{};
    std::counting_semaphore<1024> availableKeys{0};

    void pushEvent(const SDL_Event& event) {
        std::lock_guard<std::mutex> lock(eventQueueMutex);
        eventQueue.push_back(event);
        availableKeys.release(); // Increment the count of available keys
    }

    SDL_Event popEvent() {
        availableKeys.acquire(); // Wait until a key is available
        std::lock_guard<std::mutex> lock(eventQueueMutex);
        SDL_Event event = eventQueue.front();
        eventQueue.pop_front();
        return event;
    }

    bool keysAvailable() {
        std::lock_guard<std::mutex> lock(eventQueueMutex);
        return !eventQueue.empty();
    }

    static bool isArrowOrKeypad(const SDL_Event& event) {
        if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP) {
            return false;
        }

        switch (event.key.keysym.sym) {
        case SDLK_UP:
        case SDLK_DOWN:
        case SDLK_LEFT:
        case SDLK_RIGHT:
        case SDLK_KP_8:
        case SDLK_KP_2:
        case SDLK_KP_4:
        case SDLK_KP_6:
        case SDLK_KP_7:
        case SDLK_KP_9:
        case SDLK_KP_1:
        case SDLK_KP_3:
            return true;
        case SDLK_SPACE:
            if(frameSync.inCombatKey)
            {
                return true;
            }
            else
            {
                return false;
            }
            break;
        default:
            return false;
        }
    }

    unsigned short getArrowKeyDown() {
        const Uint8* state = (const Uint8*)s_keyboardState;
        bool up = state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_KP_8];
        bool down = state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_KP_2];
        bool left = state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_KP_4];
        bool right = state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_KP_6];

        if (state[SDL_SCANCODE_SPACE] && frameSync.inCombatKey)
        {
            // Set trigger
            Write16(0x5c7e, 0x1);
        }

        if (up && left) return 327; // Numpad 7 for up and left
        if (up && right) return 329; // Numpad 9 for up and right
        if (down && left) return 335; // Numpad 1 for down and left
        if (down && right) return 337; // Numpad 3 for down and right
        if (up) return 328;
        if (down) return 336;
        if (left) return 331;
        if (right) return 333;

        // Check for diagonal numpad keys directly
        if (state[SDL_SCANCODE_KP_7]) return 327;
        if (state[SDL_SCANCODE_KP_9]) return 329;
        if (state[SDL_SCANCODE_KP_1]) return 335;
        if (state[SDL_SCANCODE_KP_3]) return 337;

        return 0; // No arrow key down
    }

    static unsigned short GetKey(int sym)
    {
        if (sym == SDLK_LEFT)
        {
            return 331;
        }
        if (sym == SDLK_RIGHT)
        {
            return 333;
        }
        if (sym == SDLK_UP)
        {
            return 328;
        }
        if (sym == SDLK_DOWN)
        {
            return 336;
        }
        if (sym == SDLK_KP_8)
        {
            return 328;
        }
        if (sym == SDLK_KP_2)
        {
            return 336;
        }
        if (sym == SDLK_KP_4)
        {
            return 331;
        }
        if (sym == SDLK_KP_6)
        {
            return 333;
        }
        if (sym == SDLK_KP_7)
        {
            return 327;
        }
        if (sym == SDLK_KP_9)
        {
            return 329;
        }
        if (sym == SDLK_KP_1)
        {
            return 335;
        }
        if (sym == SDLK_KP_3)
        {
            return 337;
        }

        return sym;
    }

public:
    SDLKeyboard() {}    

    bool areArrowKeysDown() override {
        const Uint8* state = (const Uint8*)s_keyboardState;
        return state[SDL_SCANCODE_UP] || state[SDL_SCANCODE_DOWN] || state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_RIGHT] ||
            state[SDL_SCANCODE_KP_8] || state[SDL_SCANCODE_KP_2] || state[SDL_SCANCODE_KP_4] || state[SDL_SCANCODE_KP_6] ||
            state[SDL_SCANCODE_KP_7] || state[SDL_SCANCODE_KP_9] || state[SDL_SCANCODE_KP_1] || state[SDL_SCANCODE_KP_3];
    }

    void update() override {

        auto handleEvent = [&](SDL_Event event) -> bool {
            switch (event.type) {
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_F1)
                    {
                        s_shouldToggleMenu = true;
                    }
                    else if (event.key.keysym.sym == SDLK_F2)
                    {
                        s_useRotoscope = !s_useRotoscope;
                    }
                    else if(event.key.keysym.sym == SDLK_F3)
                    {
                        s_useEGA = !s_useEGA;
                    }
                    else if (event.key.keysym.sym == SDLK_F4)
                    {
                        s_adjust -= 1.0f;
                        printf("ADJUST %f\n", s_adjust);
                    }
                    else if (event.key.keysym.sym == SDLK_F5)
                    {
                        s_adjust += 1.00f;
                        printf("ADJUST %f\n", s_adjust);
                    }
                    else
                    {
                        if (emulationThreadRunning)
                        {
                            if (!isArrowOrKeypad(event))
                            {
                                pushEvent(event);
                            }
                        }
                        else
                        {
                            DoDemoKeys(event, s_gc.vc.in_flight_index_for_frame());
                        }
                    }
                    return true;
                    break;
                case SDL_KEYUP:
                    {
                        //std::lock_guard<std::mutex> lg(s_deadReckoningMutex);
                        //s_deadReckoning = { 0 , 0 };
                        if (!emulationThreadRunning)
                        {
                            DoDemoKeys(event, s_gc.vc.in_flight_index_for_frame());
                        }
                    }
                    break;
                case SDL_WINDOWEVENT:
                    {
                        if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                        {
                            if(SDL_GetWindowFromID(event.window.windowID) == window)
                            {
                                GraphicsQuit();
                                pushEvent(event);
                            }
                            #if defined(ENABLE_OFFSCREEN_VIDEO_RENDERER)
                            if(SDL_GetWindowFromID(event.window.windowID) == offscreenWindow)
                            {
                                SDL_DestroyWindow(offscreenWindow);
                                offscreenWindow = nullptr;
                            }
                            #endif
                        }
                    }
                    break;
                case SDL_QUIT:
                    GraphicsQuit();
                    break;
                case SDL_MOUSEMOTION:
                    DoDemoKeys(event, s_gc.vc.in_flight_index_for_frame());
                    nk_input_motion(&(nk_context->ctx), event.motion.x, event.motion.y);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    DoDemoKeys(event, s_gc.vc.in_flight_index_for_frame());
                    nk_input_button(&(nk_context->ctx), sdl_button_to_nk(event.button.button), event.button.x, event.button.y, 1);
                    break;
                case SDL_MOUSEBUTTONUP:
                    DoDemoKeys(event, s_gc.vc.in_flight_index_for_frame());
                    nk_input_button(&(nk_context->ctx), sdl_button_to_nk(event.button.button), event.button.x, event.button.y, 0);
                    break;
                case SDL_MOUSEWHEEL:
                    {
                        struct nk_vec2 vec;
                        vec.x = event.wheel.x;
                        vec.y = event.wheel.y;
                        nk_input_scroll(&(nk_context->ctx), vec);
                    }
                    break;
                default:
                    break;
            }

            return false;
        };

        nk_input_begin(&(nk_context->ctx));

        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            handleEvent(event);
        }

        nk_input_end(&(nk_context->ctx));
    }

    // Non-destructive read equivalent to Int 16 ah = 1
    bool checkForKeyStroke() override {
        if(areArrowKeysDown())
        {
            if (!frameSync.maneuvering)
            {
                auto now = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - frameSync.lastNonMovingArrowKey).count();
                if(duration < 100)
                {
                    return false;
                }
            }

            return true;
        }
       
        return keysAvailable();
    }

    // Destructive read equivalent to Int 16 ah = 0
    unsigned short getKeyStroke() override {
        // Prioritize non-arrow keys
        if (keysAvailable())
        {
            SDL_Event event = popEvent();
            return GetKey(event.key.keysym.sym);
        }

        auto arrow = getArrowKeyDown();
        if(arrow != 0)
        {
            if(!frameSync.maneuvering)
            {
                frameSync.lastNonMovingArrowKey = std::chrono::steady_clock::now();
            }

            return arrow;
        }

        SDL_Event event = popEvent();
        return GetKey(event.key.keysym.sym);
    }

    void pushKeyStroke(uint16_t key) override {
        SDL_Event newEvent;
        SDL_zero(newEvent); // Initialize the new event to zero
        newEvent.type = SDL_KEYDOWN; // Set the event type to key down
        newEvent.key.keysym.sym = key; // Assign the key value
        newEvent.key.state = SDL_PRESSED; // Set the key state to pressed
        newEvent.key.timestamp = SDL_GetTicks(); // Assign the current timestamp

        pushEvent(newEvent);
    }
};

static std::unique_ptr<DOSKeyboard> keyboard{};

static bool s_audioPlaying = false;

void play_buffer(void*, unsigned char*, int);

SDL_AudioSpec spec = {
	.freq = FREQUENCY, 
	.format = AUDIO_S16SYS, // Signed 16 bit integer format
	.channels = 1,
	.samples = 512, // The size of each "chunk"
	.callback = play_buffer, // user-defined function that provides the audio data
	.userdata = NULL // an argument to the callback function (we dont need any)
};

// Generate a sine wave
double tone(double hz, unsigned long time) {
	return sin(time * hz * M_PI * 2 / FREQUENCY);
}

// Generate a sawtooth wave
double saw(double hz, unsigned long time) {
	return fmod(time*hz/FREQUENCY, 1)*2-1;
}

// Generate a square wave
double square(double hz, unsigned long time) {
	double sine = tone(hz, time);
	return sine > 0.0 ? 1.0 : -1.0;
}

std::mutex audioSource1Mutex;
std::deque<Sint16> audioSource1;

// All speech is single track 22050 hz mono.
void queue_speech(int16_t* voiceAudio, uint64_t length)
{
    std::lock_guard<std::mutex> lock1(audioSource1Mutex);

    for(int i = 0; i < length; ++i)
    {
        // Double the playback rate as the playback rate is 44100 hz
        audioSource1.push_back((Sint16)voiceAudio[i]);
        audioSource1.push_back((Sint16)voiceAudio[i]);
    }
}

// This is the function that gets automatically called every time the audio device needs more data
void play_buffer(void* userdata, unsigned char* stream, int len) {
	SDL_memset(stream, spec.silence, len);

    static unsigned long time = 0;
    Sint16 *stream16 = (Sint16*)stream;

    std::lock_guard<std::mutex> lock1(audioSource1Mutex);

    int64_t alienSound = 0;

    for(int i = 0; i < len/2; i++, time++) {
        Sint16 outSample = 0;
        Sint16 squareWave = 0;
        Sint16 speech = 0;

        if(s_audioPlaying) {
            squareWave = (Sint16)(square(toneInHz, time) * 2000.0);
        }

        if (!audioSource1.empty()) {
            speech = audioSource1.front();
            audioSource1.pop_front();
            alienSound += speech;
        }

        outSample = squareWave + speech;

        // Avoid overflow
        if (outSample > INT16_MAX) {
            outSample = INT16_MAX;
        } else if (outSample < INT16_MIN) {
            outSample = INT16_MIN;
        }

        if(outSample == 0)
        {
            outSample = spec.silence;
        }

        stream16[i] = (Sint16)outSample;
    }

    if (len > 1)
    {
        alienSound /= (len / 2);
        s_alienVar1 = (float)s_alienVar1 * 0.9f + abs(float(alienSound)) * 0.1f;
    }
}

void BeepOn()
{
    SDL_PauseAudioDevice(audioDevice, 0);
    s_audioPlaying = true;
}

void BeepTone(uint16_t pitFreq)
{
    toneInHz = 1193182.0 / pitFreq;
}

void BeepOff()
{
    s_audioPlaying = false;
}

int GetFramesPerGameFrame()
{
    if (frameSync.gameContext != 4)
    {
        return 4;
    }
    else
    {
        return 16;
    }
}

void GraphicsSetDeadReckoning(int16_t deadX, int16_t deadY, 
    const std::vector<Icon>& iconList, 
    const std::vector<Icon>& system, 
    uint16_t orbitMask, 
    const StarMapSetup& starMap,
    const std::vector<MissileRecordUnique>& missiles,
    std::vector<LaserRecord>& lasers,
    std::vector<Explosion>& explosions)
{
    auto WLD_to_SCR = [](vec2<int16_t> input) {
        vec2<int16_t> output;

        output.y = input.y - static_cast<int16_t>(Read16(0x5B31)); // BVIS
        output.y *= static_cast<int16_t>(Read16(0x6221)); // YWLD:YPIX
        output.y /= static_cast<int16_t>(Read16(0x6223)); // YWLD:YPIX
        output.y += static_cast<int16_t>(Read16(0x596B)); // YLLDEST

        output.x = input.x - static_cast<int16_t>(Read16(0x5B3C)); // LVIS
        output.x *= static_cast<int16_t>(Read16(0x6211)); // XWLD:XPIX
        output.x /= static_cast<int16_t>(Read16(0x6213)); // XWLD:XPIX
        output.x += static_cast<int16_t>(Read16(0x595D)); // XLLDEST

        return output;
    };

    if(frameSync.maneuvering)
    {
        FrameToRender ftr{};

        ftr.deadReckoning = { deadX , deadY };
        ftr.iconList = iconList;
        ftr.solarSystem = system;
        ftr.starMap = starMap;
        ftr.renderCount = 0;
        ftr.orbitMask = orbitMask;
        ftr.worldCoord = { (int16_t)Read16(0x5dae), (int16_t)Read16(0x5db9) };
        ftr.screenCoord = WLD_to_SCR(ftr.worldCoord);
        ftr.heading = (int16_t)Read16(0x5dc7);
        ftr.missiles = missiles;

#if 0
        for (auto& missile : ftr.missiles) {
            vec2<int16_t> currPosWLD = { missile.currx, missile.curry };
            vec2<int16_t> destPosWLD = { missile.destx, missile.desty };

            vec2<int16_t> currPosSCR = WLD_to_SCR(currPosWLD);
            vec2<int16_t> destPosSCR = WLD_to_SCR(destPosWLD);

            missile.currx = currPosSCR.x;
            missile.curry = currPosSCR.y;
            missile.destx = destPosSCR.x;
            missile.desty = destPosSCR.y;
        }
#endif

        std::unique_lock<std::mutex> lock(frameSync.mutex);

        // This section updates the laser records, removing outdated ones and adding new ones with the current timestamp.
        // It first removes lasers older than 1 second, then adds new lasers from the provided list.
        auto now = std::chrono::steady_clock::now();
        frameSync.lasers.erase(std::remove_if(frameSync.lasers.begin(), frameSync.lasers.end(),
            [now](const LaserRecord& laser) {
                return std::chrono::duration_cast<std::chrono::milliseconds>(now - laser.timestamp).count() > 250;
            }), frameSync.lasers.end());

        for (auto& laser : lasers) {
            LaserRecord newLaser = laser;
            newLaser.timestamp = now;
            frameSync.lasers.push_back(newLaser);
        }

        lasers.clear();

        frameSync.explosions.erase(std::remove_if(frameSync.explosions.begin(), frameSync.explosions.end(),
            [now](const Explosion& explo) {
                return std::chrono::duration_cast<std::chrono::milliseconds>(now - explo.timestamp).count() > 1000;
            }), frameSync.explosions.end());        

        for (auto& explosion : explosions) {
            Explosion ex = explosion;
            ex.timestamp = now;
            frameSync.explosions.push_back(ex);
        }

        explosions.clear();

        for (auto& mi : missiles)
        {
            bool newShip = false;

            auto vesselIt = frameSync.combatTheatre.find(mi.nonce);
            if (vesselIt == frameSync.combatTheatre.end()) {
                frameSync.combatTheatre[mi.nonce] = HeadingAndThrust();
                newShip = true;
            }

            vesselIt = frameSync.combatTheatre.find(mi.nonce);
            auto& ship = vesselIt->second;

            if (newShip)
            {
                ship.interp = std::make_unique<Interpolator>(0.095f);
                ship.interp->addPointWithTime((float)frameSync.completedFrames, { (float)mi.mr.currx, (float)mi.mr.curry, 0.0f });
                ship.interp->addPoint((float)frameSync.completedFrames, { (float)mi.mr.destx, (float)mi.mr.desty, 0.0f });
            }
        }

        for (auto& icon : iconList)
        {
            if (icon.inst_type == SF_INSTANCE_VESSEL) {

                bool newShip = false;

                auto vesselIt = frameSync.combatTheatre.find(icon.iaddr);
                if (vesselIt == frameSync.combatTheatre.end()) {
                    frameSync.combatTheatre[icon.iaddr] = HeadingAndThrust();
                    newShip = true;
                }

                vesselIt = frameSync.combatTheatre.find(icon.iaddr);
                auto& ship = vesselIt->second;

                if(newShip)
                {
                    ship.interp = std::make_unique<Interpolator>();
                    ship.interp->addPointWithTime((float)frameSync.completedFrames, {icon.x, icon.y, 0.0f });
                }
                else
                {
                    ship.interp->queuePoint({ icon.x, icon.y, 0.0f }, (float)frameSync.completedFrames);
                }
            }
        }
  
        if (frameSync.framesToRender.size() < 2)
        {
            frameSync.framesToRender.push_back(ftr);
            uint64_t framesDrawn = frameSync.completedFramesPerGameFrame;
            frameSync.completedFramesPerGameFrame = 0;

            printf("GSDR Drew %d frames between one game frame GraphicsReportGameFrame - framesync cnt %d\n", framesDrawn, frameSync.framesToRender.size());

            frameSync.frameCompleted.notify_one();
        }
    }
    else
    {
        std::unique_lock<std::mutex> lock(frameSync.mutex);

        frameSync.stoppedFrame.starMap = starMap;

        frameSync.frameCompleted.notify_one();

        frameSync.combatTheatre.clear();
    }
}

void GraphicsDeleteMissile(uint64_t nonce, const MissileRecord& missile)
{
    std::unique_lock<std::mutex> lock(frameSync.mutex);

    auto vesselIt = frameSync.combatTheatre.find(nonce);

    if(vesselIt != frameSync.combatTheatre.end())
    {
#if 0
        auto& m = vesselIt->second;
        auto interp = m.interp->interpolate((float)frameSync.completedFrames);
        auto actualDistance = std::sqrt(std::pow(missile.currx - interp.position.x, 2) + std::pow(missile.curry - interp.position.y, 2));
        printf("Missile %llu Distance from end: %f\n", nonce, actualDistance);
        printf("Missile %llu interpolation start: (%f, %f) %f end: (%f, %f) %f - current (%f, %f) %f\n", nonce, 
            m.interp->ActivePoints()[0].x, m.interp->ActivePoints()[0].y, m.interp->ActiveTimes()[0],
            m.interp->ActivePoints()[1].x, m.interp->ActivePoints()[1].y, m.interp->ActiveTimes()[1],
            interp.position.x, interp.position.y, (float)frameSync.completedFrames);
#endif        
    }
}

void GraphicsSetOrbitState(OrbitState state, std::optional<vec3<float>> optionalCamPos)
{
    std::unique_lock<std::mutex> lock(frameSync.mutex);
    frameSync.SetOrbitState(state, optionalCamPos);
}

void GraphicsReportGameFrame()
{

}

RotoscopeShader& RotoscopeShader::operator=(const Rotoscope& other) {
    content = other.content;
    EGAcolor = other.EGAcolor;
    argb = other.argb;
    blt_x = other.blt_x;
    blt_y = other.blt_y;
    blt_w = other.blt_w;
    blt_h = other.blt_h;
    bgColor = other.bgColor;
    fgColor = other.fgColor;
    navMask = 0x00;

    switch(other.content)
    {
        case ClearPixel:
        case EllipsePixel:
        case BoxFillPixel:
        case PlotPixel:
        case PolyFillPixel:
        case TilePixel:
        case NavigationalPixel:
        case AuxSysPixel:
        case StarMapPixel:
        case SpaceManPixel:
            break;
        case TextPixel:
            textData = other.textData;
            break;
        case LinePixel:
            lineData = other.lineData;
            break;
        case PicPixel:
            runBitData.tag = other.picData.picID;
            break;
        case RunBitPixel:
            runBitData = other.runBitData;
            break;
        default:
            assert(false);
            break;
    }

    return *this;
}

static Texture<1536, 1152, 4> LOGO1Texture;
static Texture<1536, 1152, 4> LOGO2Texture;
static Texture<1536, 1152, 4> PORTPICTexture;
static Texture<2592, 2600, 4> BoxArtTexture;

avk::image imageFromData(const void* data, uint32_t width, uint32_t height, uint32_t bytesPerPixel, vk::Format format, avk::image_usage usage)
{
    uint32_t dataSize = width * height * bytesPerPixel;

    auto sb = s_gc.vc.create_buffer(
        AVK_STAGING_BUFFER_MEMORY_USAGE,
        vk::BufferUsageFlagBits::eTransferSrc,
        avk::generic_buffer_meta::create_from_size(dataSize)
    );

    auto image = s_gc.vc.create_image(width, height, format, 1, avk::memory_usage::device, usage);

    s_gc.vc.record_and_submit_with_fence({

        sb->fill(data, 0, 0, dataSize),

        avk::sync::buffer_memory_barrier(sb.as_reference(),
            avk::stage::auto_stage >> avk::stage::auto_stage,
            avk::access::auto_access >> avk::access::auto_access
            ),

        avk::sync::image_memory_barrier(image.as_reference(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({avk::layout::undefined, avk::layout::transfer_dst}),

        avk::copy_buffer_to_image(sb, image, avk::layout::transfer_dst),

        avk::sync::image_memory_barrier(image.as_reference(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({avk::layout::transfer_dst, avk::layout::shader_read_only_optimal})        

    }, *s_gc.mQueue)->wait_until_signalled();

    return image;
}

void LoadSplashImages()
{
    std::vector<uint8_t> image;
    unsigned width, height;

    struct ImageToLoad
    {
        std::string name;
        // Texture<1536, 1152, 4>& pic; // No longer used
        avk::image_sampler& vkPic;
        avk::border_handling_mode border{};

        ImageToLoad(const std::string& name, avk::image_sampler& vkPic, avk::border_handling_mode _border)
            : name(name), vkPic(vkPic), border(_border) {}

        
    };

    static const std::vector<ImageToLoad> images = {
        { "logo_1.png", /* LOGO1Texture, */ s_gc.LOGO1, avk::border_handling_mode::clamp_to_edge },
        { "logo_2.png", /* LOGO2Texture, */ s_gc.LOGO2, avk::border_handling_mode::clamp_to_edge },
        { "station.png", /* PORTPICTexture, */ s_gc.PORTPIC, avk::border_handling_mode::clamp_to_edge },
        { "boxart.png", /* BoxArtTexture, */ s_gc.boxArtImage, avk::border_handling_mode::clamp_to_border },
        { "noise.png", /* BoxArtTexture, */ s_gc.fourDeeNoise, avk::border_handling_mode::repeat },
    };

    for (auto& img : images)
    {
        unsigned error = lodepng::decode(image, width, height, img.name, LCT_RGBA, 8);
        if (error)
        {
            printf("decoder error %d, %s loading %s\n", error, lodepng_error_text(error), img.name.c_str());
            exit(-1);
        }

        //fillTexture(img.pic, image);
        img.vkPic = s_gc.vc.create_image_sampler(
            s_gc.vc.create_image_view(
                imageFromData(image.data(), width, height, 4, vk::Format::eR8G8B8A8Unorm, avk::image_usage::general_image)
            ),
            s_gc.vc.create_sampler(avk::filter_mode::bilinear, img.border)
        );
        image.clear();
    }

    // Load noise.png into diligentFourDeeNoise
    unsigned error = lodepng::decode(image, width, height, "noise.png", LCT_RGBA, 8);
    if (error)
    {
        printf("decoder error %d, %s loading noise.png\n", error, lodepng_error_text(error));
        exit(-1);
    }

    TextureDesc texDesc{};
    texDesc.Type = RESOURCE_DIM_TEX_2D;
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.Format = TEX_FORMAT_RGBA8_UNORM;
    texDesc.MipLevels = 1;
    texDesc.SampleCount = 1;
    texDesc.Usage = USAGE_DEFAULT;
    texDesc.BindFlags = BIND_SHADER_RESOURCE;
    TextureSubResData Level0Data{image.data(), width * 4};
    TextureData InitData{&Level0Data, 1};
    ITexture* dcb{};
    s_gc.m_pDevice->CreateTexture(texDesc, &InitData, &dcb);
    s_gc.diligentFourDeeNoise = dcb->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    image.clear();
}

void LoadSDFImages()
{
    std::ifstream file("atlas.raw.zst", std::ios::binary | std::ios::ate);
    if (!file)
    {
        printf("Error opening atlas.raw.zst\n");
        exit(-1);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> compressedData(size);
    if (!file.read(compressedData.data(), size))
    {
        printf("Error reading atlas.raw.zst\n");
        exit(-1);
    }

    unsigned long long const rSize = ZSTD_getFrameContentSize(compressedData.data(), size);
    if (rSize == ZSTD_CONTENTSIZE_ERROR || rSize == ZSTD_CONTENTSIZE_UNKNOWN)
    {
        printf("Error determining decompressed size\n");
        exit(-1);
    }

    std::vector<float> buffer(rSize / sizeof(float));
    size_t const dSize = ZSTD_decompress(buffer.data(), rSize, compressedData.data(), size);
    if (ZSTD_isError(dSize))
    {
        printf("Error decompressing atlas.raw.zst: %s\n", ZSTD_getErrorName(dSize));
        exit(-1);
    }

    fillTextureLiteral(RaceDosPicTexture, buffer);

    auto image = imageFromData(buffer.data(), RaceDosPicTexture.data[0].size(), RaceDosPicTexture.data.size(), 4, vk::Format::eR32Sfloat, avk::image_usage::general_image);
    s_gc.RACEDOSATLAS = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(image), 
        s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge)
    );
}

void LoadFonts()
{
    std::vector<uint8_t> image;
    unsigned width, height;

    unsigned error = lodepng::decode(image, width, height, "FONT1_sdf.png", LCT_GREY, 8);
    if(error) 
    {
        printf("decoder error %d, %s\n", error, lodepng_error_text(error));
        exit(-1);
    }

    fillTexture(FONT1Texture, image);
    s_gc.FONT1 = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(
            imageFromData(image.data(), width, height, 1, vk::Format::eR8Unorm, avk::image_usage::general_image)
        ), 
        s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge)
    );
    image.clear();

    error = lodepng::decode(image, width, height, "FONT2_sdf.png", LCT_GREY, 8);
    if(error) 
    {
        printf("decoder error %d, %s\n", error, lodepng_error_text(error));
        exit(-1);
    }

    fillTexture(FONT2Texture, image);
    s_gc.FONT2 = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(
            imageFromData(image.data(), width, height, 1, vk::Format::eR8Unorm, avk::image_usage::general_image)
        ), 
        s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge)
    );
    image.clear();

    error = lodepng::decode(image, width, height, "FONT3_sdf.png", LCT_GREY, 8);
    if(error) 
    {
        printf("decoder error %d, %s\n", error, lodepng_error_text(error));
        exit(-1);
    }

    fillTexture(FONT3Texture, image);
    s_gc.FONT3 = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(
            imageFromData(image.data(), width, height, 1, vk::Format::eR8Unorm, avk::image_usage::general_image)
        ), 
        s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge)
    );    
    image.clear();
}

static const std::vector<std::pair<std::string, vk::Format>> shipTextures = {
    {"ship.png", vk::Format::eR8G8B8A8Unorm},
    {"mechan-9.png", vk::Format::eR8G8B8A8Unorm},
    {"debris.png", vk::Format::eR8G8B8A8Unorm},
    {"missile.png", vk::Format::eR8G8B8A8Unorm},
};

void LoadAssets()
{
    std::vector<uint8_t> image;
    unsigned width, height;

    //unsigned error = lodepng::decode(image, width, height, "ship.png", LCT_RGBA, 8);
    unsigned error = lodepng::decode(image, width, height, "mechan-9.png", LCT_RGBA, 8);
    if(error) 
    {
        printf("decoder error %d, %s\n", error, lodepng_error_text(error));
        exit(-1);
    }

    s_gc.shipImage = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(
            s_gc.vc.create_image(512, 512, vk::Format::eR8G8B8A8Unorm, shipTextures.size(), avk::memory_usage::device, avk::image_usage::general_image)
        ), 
        s_gc.vc.create_sampler(avk::filter_mode::trilinear, avk::border_handling_mode::clamp_to_edge)
    );
    image.clear();

#if 0
    unsigned fourDeeNoiseWidth = 256, fourDeeNoiseHeight = 256;

    // Generate RGBA fourDeeNoise
    image.resize(fourDeeNoiseWidth * fourDeeNoiseHeight * 4); // 4 bytes per pixel (RGBA)
    for (unsigned i = 0; i < image.size(); i += 4) {
        image[i] = rand() % 256; // R
        image[i + 1] = rand() % 256; // G
        image[i + 2] = rand() % 256; // B
        image[i + 3] = rand() % 256; // A
    }

    // Initialize the fourDeeNoise image sampler
    s_gc.fourDeeNoise = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(
            imageFromData(image.data(), fourDeeNoiseWidth, fourDeeNoiseHeight, 4, vk::Format::eR8G8B8A8Unorm, avk::image_usage::general_image)
        ),
        s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::repeat)
    );
#endif

    s_gc.planetAlbedoImages = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(
            s_gc.vc.create_image(48, 24, vk::Format::eR8G8B8A8Unorm, 811, avk::memory_usage::device, avk::image_usage::general_image)
        ), 
        s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::repeat)
    );

    s_gc.alienColorImage = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(
            s_gc.vc.create_image(512, 512, vk::Format::eR8G8B8A8Unorm, 1, avk::memory_usage::device, avk::image_usage::general_image)
        ), 
        s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::repeat)
    );

    s_gc.alienBackgroundImage = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(
            s_gc.vc.create_image(512, 512, vk::Format::eR8G8B8A8Unorm, 1, avk::memory_usage::device, avk::image_usage::general_image)
        ), 
        s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::repeat)
    );

    s_gc.alienDepthImage = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(
            s_gc.vc.create_image(512, 512, vk::Format::eR8G8B8A8Unorm, 1, avk::memory_usage::device, avk::image_usage::general_image)
        ), 
        s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::repeat)
    );

    s_gc.vc.record_and_submit_with_fence({
        avk::sync::image_memory_barrier(s_gc.shipImage->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::shader_read_only_optimal }),

        avk::sync::image_memory_barrier(s_gc.planetAlbedoImages->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::shader_read_only_optimal }),

            avk::sync::image_memory_barrier(s_gc.alienColorImage->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::shader_read_only_optimal }),

            avk::sync::image_memory_barrier(s_gc.alienDepthImage->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::shader_read_only_optimal }),

            avk::sync::image_memory_barrier(s_gc.alienBackgroundImage->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::shader_read_only_optimal }),

            avk::sync::image_memory_barrier(s_gc.fourDeeNoise->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::shader_read_only_optimal }),

    }, *s_gc.mQueue)->wait_until_signalled();
}

void GraphicsInitPlanets(std::unordered_map<uint32_t, PlanetSurface> surfaces)
{
    s_gc.surfaceData = surfaces;

    s_gc.shouldInitPlanets = true;

    s_gc.planetsDone.acquire();
}

static constexpr char EnvMapPSMain[] = R"(
void main(in  float4 Pos          : SV_Position,
          in  float4 ClipPos      : CLIP_POS,
          out float4 Color        : SV_Target0,
          out float4 MotionVec    : SV_Target4)
{
    SampleEnvMapOutput EnvMap = SampleEnvMap(ClipPos);
    Color     = EnvMap.Color;
    MotionVec = float4(EnvMap.MotionVector, 0.0, 1.0);
}
)";


static void LoadEnvironmentMap(const char* Path, GraphicsContext::SFModel& model, bool blowUp)
{

    EnvMapRenderer::CreateInfo EnvMapRendererCI;
    EnvMapRendererCI.pDevice          = s_gc.m_pDevice;
    EnvMapRendererCI.pCameraAttribsCB = s_gc.frameAttribsCB;

    EnvMapRendererCI.NumRenderTargets = GBUFFER_RT_NUM_COLOR_TARGETS;
    for (Uint32 i = 0; i < EnvMapRendererCI.NumRenderTargets; ++i)
        EnvMapRendererCI.RTVFormats[i] = s_gc.gBuffer->GetElementDesc(i).Format;
    EnvMapRendererCI.DSVFormat = s_gc.gBuffer->GetElementDesc(GBUFFER_RT_DEPTH0).Format;

    EnvMapRendererCI.PSMainSource = EnvMapPSMain;
    s_gc.envMapRenderer = std::make_unique<EnvMapRenderer>(EnvMapRendererCI);

    RefCntAutoPtr<ITexture> pEnvironmentMap;

    TextureLoadInfo tli = {};
    tli.Name = "Environment map";
    tli.Usage = USAGE_DEFAULT;
    tli.BindFlags = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;

    CreateTextureFromFile(Path, tli, s_gc.m_pDevice, &pEnvironmentMap);
    VERIFY_EXPR(pEnvironmentMap);

    if (blowUp)
    {
        const char* csSource = R"(
        RWTexture2DArray<float4> g_Texture : register(u0);

        [numthreads(16, 16, 1)]
        void CSMain(uint3 DTid : SV_DispatchThreadID)
        {
            float4 color = g_Texture[DTid.xyz];
            float expFactor = 2.0;
            float maxIntensity = 511.0; // Maximum intensity for bright stars

            // Apply the power function to scale the color
            color.rgb = maxIntensity * pow(color.rgb, expFactor) + color.rgb;

            g_Texture[DTid.xyz] = color;
        }
        )";

        ShaderCreateInfo ShaderCI;
        ShaderCI.Source = csSource;
        ShaderCI.EntryPoint = "CSMain";
        ShaderCI.Desc.ShaderType = SHADER_TYPE_COMPUTE;
        ShaderCI.Desc.Name = "Multiply Pixels Compute Shader";
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

        RefCntAutoPtr<IShader> pShader;
        s_gc.m_pDevice->CreateShader(ShaderCI, &pShader);
        VERIFY_EXPR(pShader);

        PipelineResourceSignatureDescX SignatureDesc{ "Multiply Pixels Compute PSO" };
        SignatureDesc
            .AddResource(SHADER_TYPE_COMPUTE, "g_Texture", SHADER_RESOURCE_TYPE_TEXTURE_UAV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE);

        RefCntAutoPtr<IPipelineResourceSignature> pResSig;
        s_gc.m_pDevice->CreatePipelineResourceSignature(SignatureDesc, &pResSig);
        VERIFY_EXPR(pResSig);

        ComputePipelineStateCreateInfoX CPSOCreateInfo{ "Multiply Pixels Compute PSO" };
        CPSOCreateInfo.AddShader(pShader);
        CPSOCreateInfo.AddSignature(pResSig);

        RefCntAutoPtr<IPipelineState> pPSO;
        s_gc.m_pDevice->CreatePipelineState(CPSOCreateInfo, &pPSO);
        VERIFY_EXPR(pPSO);

        RefCntAutoPtr<IShaderResourceBinding> pSRB;
        pResSig->CreateShaderResourceBinding(&pSRB, true);
        VERIFY_EXPR(pSRB);

        pSRB->GetVariableByName(SHADER_TYPE_COMPUTE, "g_Texture")->Set(pEnvironmentMap->GetDefaultView(TEXTURE_VIEW_UNORDERED_ACCESS));

        DispatchComputeAttribs DispatchAttrs;
        DispatchAttrs.ThreadGroupCountX = (pEnvironmentMap->GetDesc().Width + 15) / 16;
        DispatchAttrs.ThreadGroupCountY = (pEnvironmentMap->GetDesc().Height + 15) / 16;
        DispatchAttrs.ThreadGroupCountZ = pEnvironmentMap->GetDesc().Depth;

        s_gc.m_pImmediateContext->SetPipelineState(pPSO);
        s_gc.m_pImmediateContext->CommitShaderResources(pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        s_gc.m_pImmediateContext->DispatchCompute(DispatchAttrs);
        s_gc.m_pImmediateContext->Flush();
    }
    
    s_gc.pbrRenderer->PrecomputeCubemaps(s_gc.m_pImmediateContext, pEnvironmentMap->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE), 8192, 1024, true);

    StateTransitionDesc Barriers[] = {
        {pEnvironmentMap, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_SHADER_RESOURCE, STATE_TRANSITION_FLAG_UPDATE_STATE},
    };
    s_gc.m_pImmediateContext->TransitionResourceStates(_countof(Barriers), Barriers);

    model.env = pEnvironmentMap->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

    s_gc.m_pImmediateContext->Flush();
}

static SF_PBR_Renderer::CreateInfo::PSMainSourceInfo GetPbrPSMainSource(SF_PBR_Renderer::PSO_FLAGS PSOFlags)
{
    SF_PBR_Renderer::CreateInfo::PSMainSourceInfo PSMainInfo;

    PSMainInfo.OutputStruct = R"(
struct PSOutput
{
    float4 Color        : SV_Target0;
    float4 Normal       : SV_Target1;
    float4 BaseColor    : SV_Target2;
    float4 MaterialData : SV_Target3;
    float4 MotionVec    : SV_Target4;
    float4 SpecularIBL  : SV_Target5;
};
)";

    PSMainInfo.Footer = R"(
    PSOutput PSOut;
#   if UNSHADED
    {
        PSOut.Color        = g_Frame.Renderer.UnshadedColor + g_Frame.Renderer.HighlightColor;
        PSOut.Normal       = float4(0.0, 0.0, 0.0, 0.0);
        PSOut.MaterialData = float4(0.0, 0.0, 0.0, 0.0);
        PSOut.BaseColor    = float4(0.0, 0.0, 0.0, 0.0);
        PSOut.SpecularIBL  = float4(0.0, 0.0, 0.0, 0.0);
    }
#   else
    {
        PSOut.Color            = OutColor;
        PSOut.Normal.xyz       = Shading.BaseLayer.Normal.xyz;
        PSOut.MaterialData.xyz = float3(Shading.BaseLayer.Srf.PerceptualRoughness, Shading.BaseLayer.Metallic, 0.0);
        PSOut.BaseColor.xyz    = BaseColor.xyz;
        PSOut.SpecularIBL.xyz  = GetBaseLayerSpecularIBL(Shading, SrfLighting);

#       if ENABLE_CLEAR_COAT
	    {
            // We clearly can't do SSR for both base layer and clear coat, so we
            // blend the base layer properties with the clearcoat using the clearcoat factor.
            // This way when the factor is 0.0, we get the base layer, when it is 1.0,
            // we get the clear coat, and something in between otherwise.

            PSOut.Normal.xyz      = normalize(lerp(PSOut.Normal.xyz, Shading.Clearcoat.Normal, Shading.Clearcoat.Factor));
            PSOut.MaterialData.xy = lerp(PSOut.MaterialData.xy, float2(Shading.Clearcoat.Srf.PerceptualRoughness, 0.0), Shading.Clearcoat.Factor);
            PSOut.BaseColor.xyz   = lerp(PSOut.BaseColor.xyz, float3(1.0, 1.0, 1.0), Shading.Clearcoat.Factor);

            // Note that the base layer IBL is weighted by (1.0 - Shading.Clearcoat.Factor * ClearcoatFresnel).
            // Here we are weighting it by (1.0 - Shading.Clearcoat.Factor), which is always smaller,
            // so when we subtract the IBL, it can never be negative.
            PSOut.SpecularIBL.xyz = lerp(
                PSOut.SpecularIBL.xyz,
                GetClearcoatIBL(Shading, SrfLighting),
                Shading.Clearcoat.Factor);
        }
#       endif
    
        // Blend material data and IBL with background
	    PSOut.BaseColor    = float4(PSOut.BaseColor.xyz    * BaseColor.a, BaseColor.a);
        PSOut.MaterialData = float4(PSOut.MaterialData.xyz * BaseColor.a, BaseColor.a);
        PSOut.SpecularIBL  = float4(PSOut.SpecularIBL.xyz  * BaseColor.a, BaseColor.a);
    
        // Do not blend motion vectors as it does not make sense
        PSOut.MotionVec = float4(MotionVector, 0.0, 1.0);

        // Also do not blend normal - we want normal of the top layer
        PSOut.Normal.a = 1.0;
	}
#   endif

    return PSOut;
)";

    return PSMainInfo;
}

static float2 InitHeightmap()
{
    /*
    const int8_t image[9][9] = {
        {-16, -16, 16, 16, 32, 48, 48, 32, 32},
        {-16, -16, 16, 16, 32, 48, 32, 32, 48},
        {-16, -16, 16, 16, 32, 32, 32, 48, 48},
        {-16, -16, -16, 16, 32, 32, 32, 32, 48},
        {-16, -16, 16, 16, 32, 32, 32, 48, 32},
        {-16, -16, 16, 16, 32, 32, 32, 32, 48},
        {-16, -16, 16, 16, 32, 32, 32, 32, 32},
        {-16, -16, 16, 16, 32, 32, 32, 32, 32},
        {-16, 16, 16, 16, 16, 32, 32, 32, 32},
    };
    static constexpr MapWidth = 9;
    static constexpr MapHeight = 9;
    */

#define USE_LOFI_EARTH 1
//#define USE_HEAVEN 1

#if !defined(USE_LOFI_EARTH) && !defined(USE_HEAVEN)
    #error "Must define either USE_LOFI_EARTH or USE_HEAVEN"
#endif

    std::vector<unsigned char> image;
    unsigned MapWidth, MapHeight;
#if defined(USE_LOFI_EARTH)
    unsigned error = lodepng::decode(image, MapWidth, MapHeight, "lofi_earth.png", LCT_GREY, 8);
#else
    unsigned error = lodepng::decode(image, MapWidth, MapHeight, "heaven_output.png", LCT_GREY, 8);
#endif
    if (error)
    {
        printf("decoder error %d, %s loading lofi_earth.png\n", error, lodepng_error_text(error));
        exit(-1);
    }

#if defined(USE_LOFI_EARTH)
    // For lofi_earth.png
    auto convertToHeightValue = [](unsigned char val) {
        if (val == 0) {
            return (unsigned char)0xf0; // Water
        }
        
        float normalized_val = static_cast<float>(val) / 255.0f;
        int height_val = static_cast<int>(normalized_val * 8.0f);
        
        if (height_val > 6) {
            height_val = 6;
        }
        
        height_val += 1;
        height_val <<= 4;
        
        return (unsigned char)height_val;
    };

#else
    auto convertToHeightValue = [](unsigned char val) {
        return (unsigned char)(val);
    };
#endif

    std::transform(image.begin(), image.end(), image.begin(), convertToHeightValue);

    int8_t* ptr = (int8_t*)image.data();

    s_gc.heightmapSize = int2(MapWidth, MapHeight);
    s_gc.heightmapData.resize(MapWidth * MapHeight);

    for (int y = 0; y < MapHeight; ++y)
    {
        for (int x = 0; x < MapWidth; ++x)
        {
            s_gc.heightmapData[y * MapWidth + x] = static_cast<float>(ptr[y * MapWidth + x] + 16) / 8.0f;
        }
    }

    TextureDesc HeightmapTexDesc;
    HeightmapTexDesc.Name = "Dummy Heightmap Texture";
    HeightmapTexDesc.Type = RESOURCE_DIM_TEX_2D;
    HeightmapTexDesc.Width = MapWidth;
    HeightmapTexDesc.Height = MapHeight;
    HeightmapTexDesc.Format = TEX_FORMAT_R32_FLOAT;
    HeightmapTexDesc.BindFlags = BIND_SHADER_RESOURCE;

    s_gc.m_pDevice->CreateTexture(HeightmapTexDesc, nullptr, &s_gc.heightmap);

    s_gc.heightmapView = s_gc.heightmap->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

    Box UpdateBox;
    UpdateBox.MinX = 0;
    UpdateBox.MinY = 0;
    UpdateBox.MinZ = 0;
    UpdateBox.MaxX = HeightmapTexDesc.Width;
    UpdateBox.MaxY = HeightmapTexDesc.Height;
    UpdateBox.MaxZ = 1;

    TextureSubResData SubresData;
    SubresData.pData = s_gc.heightmapData.data();
    SubresData.Stride = HeightmapTexDesc.Width * sizeof(float);

    s_gc.m_pImmediateContext->UpdateTexture(s_gc.heightmap, 0, 0, UpdateBox, SubresData, RESOURCE_STATE_TRANSITION_MODE_NONE, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    StateTransitionDesc HeightmapTransitionDesc = {s_gc.heightmap, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_SHADER_RESOURCE, STATE_TRANSITION_FLAG_UPDATE_STATE};
    s_gc.m_pImmediateContext->TransitionResourceStates(1, &HeightmapTransitionDesc);

    return float2((float)MapWidth, (float)MapHeight);
}   

static void InitPBRRenderer(ITextureView* shadowMap)
{
    GBuffer::ElementDesc GBufferElems[GBUFFER_RT_COUNT];
    GBufferElems[GBUFFER_RT_RADIANCE]       = {TEX_FORMAT_RGBA16_FLOAT};
    GBufferElems[GBUFFER_RT_NORMAL]         = {TEX_FORMAT_RGBA16_FLOAT};
    GBufferElems[GBUFFER_RT_BASE_COLOR]     = {TEX_FORMAT_RGBA8_UNORM};
    GBufferElems[GBUFFER_RT_MATERIAL_DATA]  = {TEX_FORMAT_RG8_UNORM};
    GBufferElems[GBUFFER_RT_MOTION_VECTORS] = {TEX_FORMAT_RG16_FLOAT};
    GBufferElems[GBUFFER_RT_SPECULAR_IBL]   = {TEX_FORMAT_RGBA16_FLOAT};
    GBufferElems[GBUFFER_RT_DEPTH0]         = {TEX_FORMAT_D32_FLOAT};
    GBufferElems[GBUFFER_RT_DEPTH1]         = {TEX_FORMAT_D32_FLOAT};
    static_assert(GBUFFER_RT_COUNT == 8, "Not all G-buffer elements are initialized");

    s_gc.gBuffer = std::make_unique<GBuffer>(GBufferElems, _countof(GBufferElems));

    SF_GLTF_PBR_Renderer::CreateInfo RendererCI{};

    RendererCI.EnableClearCoat = true;
    RendererCI.EnableSheen = true;
    RendererCI.EnableIridescence = true;
    RendererCI.EnableTransmission = true;
    RendererCI.EnableAnisotropy = true;
    RendererCI.FrontCounterClockwise = true;
    RendererCI.EnableShadows = true;

    RendererCI.GetPSMainSource = GetPbrPSMainSource;

    RendererCI.pHeightmapAttribsCB = s_gc.heightmapAttribsCB;
    RendererCI.pPrimitiveAttribsCB = s_gc.PBRPrimitiveAttribsCB;
    RendererCI.pJointsBuffer = s_gc.jointsBuffer;
    RendererCI.pTerrainAttribsCB = s_gc.terrainAttribsCB;

    RendererCI.SheenAlbedoScalingLUTPath    = "sheen_albedo_scaling.jpg";
    RendererCI.PreintegratedCharlieBRDFPath = "charlie_preintegrated.jpg";

    s_gc.renderParams = {};

    s_gc.renderParams.Flags =
        SF_GLTF_PBR_Renderer::PSO_FLAG_DEFAULT |
        SF_GLTF_PBR_Renderer::PSO_FLAG_ENABLE_CLEAR_COAT |
        SF_GLTF_PBR_Renderer::PSO_FLAG_ALL_TEXTURES |
        SF_GLTF_PBR_Renderer::PSO_FLAG_ENABLE_SHEEN |
        SF_GLTF_PBR_Renderer::PSO_FLAG_ENABLE_ANISOTROPY |
        SF_GLTF_PBR_Renderer::PSO_FLAG_ENABLE_IRIDESCENCE |
        SF_GLTF_PBR_Renderer::PSO_FLAG_ENABLE_TRANSMISSION |
        SF_GLTF_PBR_Renderer::PSO_FLAG_ENABLE_VOLUME |
        SF_GLTF_PBR_Renderer::PSO_FLAG_ENABLE_TEXCOORD_TRANSFORM |
        SF_GLTF_PBR_Renderer::PSO_FLAG_ENABLE_SHADOWS;

    s_gc.renderParams.Flags &= ~SF_GLTF_PBR_Renderer::PSO_FLAG_ENABLE_TONE_MAPPING;
    s_gc.renderParams.Flags |= SF_GLTF_PBR_Renderer::PSO_FLAG_COMPUTE_MOTION_VECTORS;
    s_gc.renderParams.Flags |= SF_GLTF_PBR_Renderer::PSO_FLAG_USE_AO_MAP;

    RendererCI.NumRenderTargets = GBUFFER_RT_NUM_COLOR_TARGETS;
    for (Uint32 i = 0; i < RendererCI.NumRenderTargets; ++i)
        RendererCI.RTVFormats[i] = s_gc.gBuffer->GetElementDesc(i).Format;
    RendererCI.DSVFormat = s_gc.gBuffer->GetElementDesc(GBUFFER_RT_DEPTH0).Format;

    s_gc.pbrRenderer = std::make_unique<SF_GLTF_PBR_Renderer>(s_gc.m_pDevice, nullptr, s_gc.m_pImmediateContext, RendererCI);

    PostFXContext::CreateInfo pfxcci{};
    s_gc.postFXContext = std::make_unique<PostFXContext>(s_gc.m_pDevice, pfxcci);
    
    Bloom::CreateInfo bci{};

    s_gc.bloom = std::make_unique<Bloom>(s_gc.m_pDevice, bci);

    ScreenSpaceAmbientOcclusion::CreateInfo ssaoci{};
    s_gc.ssao = std::make_unique<ScreenSpaceAmbientOcclusion>(s_gc.m_pDevice, ssaoci);

    #if defined(DE_SSR)
    ScreenSpaceReflection::CreateInfo ssrci{};
    s_gc.ssr = std::make_unique<ScreenSpaceReflection>(s_gc.m_pDevice, ssrci);
    #endif

    s_gc.station.bindings = s_gc.pbrRenderer->CreateResourceBindings(*s_gc.station.model, s_gc.frameAttribsCB, shadowMap, nullptr, nullptr);
    if (frameSync.demoMode == 1)
    {
        s_gc.planet.bindings = s_gc.pbrRenderer->CreateResourceBindings(*s_gc.planet.model, s_gc.frameAttribsCB, shadowMap, nullptr, nullptr);
    }
    else if (frameSync.demoMode == 2)
    {
        s_gc.terrain.bindings = s_gc.pbrRenderer->CreateResourceBindings(*s_gc.terrain.model, 
                                                                          s_gc.frameAttribsCB, 
                                                                          shadowMap, 
                                                                          s_gc.heightmapAttribsCB, 
                                                                          s_gc.heightmapView);
    }

    StateTransitionDesc Barriers[] = {
        {s_gc.frameAttribsCB, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, STATE_TRANSITION_FLAG_UPDATE_STATE},
        {s_gc.heightmapAttribsCB, RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_CONSTANT_BUFFER, STATE_TRANSITION_FLAG_UPDATE_STATE},
        {s_gc.pbrRenderer->GetInstanceAttribsSB(), RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_SHADER_RESOURCE, STATE_TRANSITION_FLAG_UPDATE_STATE},
        {s_gc.heightmap, RESOURCE_STATE_COPY_DEST, RESOURCE_STATE_SHADER_RESOURCE, STATE_TRANSITION_FLAG_UPDATE_STATE},
    };
    s_gc.m_pImmediateContext->TransitionResourceStates(_countof(Barriers), Barriers);

    LoadEnvironmentMap("starfield.ktx", s_gc.station, true);
    if (frameSync.demoMode == 1)
    {
        LoadEnvironmentMap("starfield.ktx", s_gc.planet, false);
    }
    else if(frameSync.demoMode == 2)
    {
        LoadEnvironmentMap("starfield.ktx", s_gc.terrain, false);
    }

}

static int GraphicsInitThread()
{
    if(false)
    {
        Interpolator interpolator;
        std::ofstream outFile("interpolated_points_with_heading.txt");

        // Lambda function to add the first point with a specified time and mark it as a key point in the file
        auto addFirstPointAndMark = [&interpolator, &outFile](float time, vec3<float> point) {
            interpolator.addPointWithTime(time, point);
            outFile << point.x << " " << point.y << " 0.0 1" << std::endl; // 1 marks this as a key point
        };

        // Lambda function for adding subsequent points without specifying time, marking them as key points in the file
        auto addSubsequentPointAndMark = [&interpolator, &outFile](vec3<float> point) {
            interpolator.queuePoint(point, 0.0f); // Assuming this method exists and computes time automatically
            outFile << point.x << " " << point.y << " 0.0 1" << std::endl; // 1 marks this as a key point
        };

        // Use the first lambda function to add the first point with a specified time
        addFirstPointAndMark(0, {0, 0, 0});

        // Use the second lambda function to add all subsequent points, letting their times be computed automatically
        addSubsequentPointAndMark({10, 10, 0});
        addSubsequentPointAndMark({9, 10, 0 });
        addSubsequentPointAndMark({ 8, 10, 0 });
        addSubsequentPointAndMark({ 7, 9, 0 });
        addSubsequentPointAndMark({ 6, 10, 0 });
        addSubsequentPointAndMark({ 0, 0, 0 });

        float t = 0.0f;

        for(;;)
        {
            auto point = interpolator.interpolate(t);
            outFile << point.position.x << " " << point.position.y << " " << point.heading << " 0" << std::endl; // 0 for interpolated points

            if(interpolator.isExtrapolating(t))
            {
                break;
            }

            t += 8.0;
        }

        outFile.close();
        exit(0);
    }


    SetCurrentThreadName("Graphics Thread");

    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);

    SDL_DisplayMode dm;
    if (SDL_GetDesktopDisplayMode(0, &dm) != 0)
    {
        SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
    }
    else
    {
        SDL_Log("Desktop display mode: %dx%dpx @ %dhz", dm.w, dm.h, dm.refresh_rate);
    }

    WINDOW_WIDTH = (WINDOW_WIDTH * dm.w) / TARGET_RESOLUTION_WIDTH;
    WINDOW_HEIGHT = (WINDOW_HEIGHT * dm.h) / TARGET_RESOLUTION_HEIGHT;
    OFFSCREEN_WINDOW_WIDTH = (OFFSCREEN_WINDOW_WIDTH * dm.w) / TARGET_RESOLUTION_WIDTH;
    OFFSCREEN_WINDOW_HEIGHT = (OFFSCREEN_WINDOW_HEIGHT * dm.h) / TARGET_RESOLUTION_HEIGHT;

    audioDevice = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);

    window = SDL_CreateWindow("Starflight", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        printf("SDL_CreateWindow Error: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }  

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL)
    {
        printf("SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    nk_surface = SDL_CreateRGBSurfaceWithFormat(0, WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_PIXELFORMAT_ARGB8888);

    nk_context = nk_sdlsurface_init(nk_surface, 13.0f);

    graphicsTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, GRAPHICS_MODE_WIDTH, GRAPHICS_MODE_HEIGHT);
    if (graphicsTexture == NULL)
    {
        printf("SDL_CreateTexture Error: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    windowTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (windowTexture == NULL)
    {
        printf("SDL_CreateTexture Error: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    // Create the text mode texture
    textTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, TEXT_MODE_WIDTH, TEXT_MODE_HEIGHT);
    if (textTexture == NULL)
    {
        printf("SDL_CreateTexture Error: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }

#if defined(ENABLE_OFFSCREEN_VIDEO_RENDERER)

    offscreenWindow = SDL_CreateWindow("Off Screen Starflight", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, OFFSCREEN_WINDOW_WIDTH, OFFSCREEN_WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (offscreenWindow == NULL)
    {
        printf("SDL_CreateWindow Error: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    offscreenRenderer = SDL_CreateRenderer(offscreenWindow, -1, 0);
    if (offscreenRenderer == NULL)
    {
        printf("SDL_CreateRenderer Error: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    offscreenTexture = SDL_CreateTexture(offscreenRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, GRAPHICS_MODE_WIDTH, GRAPHICS_MODE_HEIGHT);
    if (offscreenTexture == NULL)
    {
        printf("SDL_CreateTexture Error: %s", SDL_GetError());
        SDL_Quit();
        return 0;
    }

#endif

    EngineVkCreateInfo EngineCI;
    EngineCI.DynamicHeapSize = 8 << 24;
    EngineCI.DeviceExtensionCount = 4;
    const char* deviceExtensions[] = { 
        "VK_KHR_create_renderpass2", 
        "VK_KHR_synchronization2", 
        "VK_KHR_maintenance4",
        "VK_KHR_get_memory_requirements2"
    };
    EngineCI.ppDeviceExtensionNames = deviceExtensions;

    // Initialize Vulkan synchronization features
    VkPhysicalDeviceSynchronization2FeaturesKHR sync2Features = {};
    sync2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR;
    sync2Features.synchronization2 = VK_TRUE;

    VkPhysicalDeviceMaintenance4FeaturesKHR maintenance4Features = {};
    maintenance4Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES_KHR;
    maintenance4Features.maintenance4 = VK_TRUE;
    sync2Features.pNext = &maintenance4Features;

    EngineCI.pDeviceExtensionFeatures = &sync2Features;

    EngineCI.EnableValidation = true;

    auto* pFactoryVk = GetEngineFactoryVk();
    pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &s_gc.m_pDevice, &s_gc.m_pImmediateContext);

    RefCntAutoPtr<IRenderDeviceVk> pDeviceVk(s_gc.m_pDevice, IID_RenderDeviceVk);
    s_gc.m_pDeviceVk = pDeviceVk;

    RefCntAutoPtr<IDeviceContextVk> pDeviceContextVk(s_gc.m_pImmediateContext, IID_DeviceContextVk);
    s_gc.m_pImmediateContextVk = pDeviceContextVk;

    s_gc.vc.vulkan_instance(s_gc.m_pDeviceVk->GetVkInstance());
    s_gc.vc.physical_device(s_gc.m_pDeviceVk->GetVkPhysicalDevice());
    s_gc.vc.device(s_gc.m_pDeviceVk->GetVkDevice());
    s_gc.vc.create_swap_chain(VulkanContext::swapchain_creation_mode::create_new_swapchain, window, WINDOW_WIDTH, WINDOW_HEIGHT);

    s_gc.mQueue = s_gc.vc.getAVKQueue();

    s_gc.spaceMan = Interpolator(0.09f, true);
    s_gc.spaceMan.addPointWithTime(0.0f, { 0.912174284f, -0.268656492f, -0.013f });

    auto& commandPool = s_gc.vc.get_command_pool_for_resettable_command_buffers(*s_gc.mQueue);
#if defined(FX_SSR)
    size_t scratchBufferSize = ffxGetScratchMemorySizeVK(pDeviceVk->GetVkPhysicalDevice(), FFX_SSSR_CONTEXT_COUNT);
    void* scratchBuffer = calloc(1, scratchBufferSize);

    VkDeviceContext device_context = {};
    device_context.vkDevice = pDeviceVk->GetVkDevice();
    device_context.vkPhysicalDevice = pDeviceVk->GetVkPhysicalDevice();

    auto ffxResult = ffxGetInterfaceVK(&s_gc.ffxInterface, ffxGetDeviceVK(&device_context), scratchBuffer, scratchBufferSize, FFX_SSSR_CONTEXT_COUNT);
    assert(ffxResult == FFX_OK);

    FfxSssrContextDescription sssrDesc = {};
    sssrDesc.flags = 0;
    sssrDesc.renderSize.width = WINDOW_WIDTH;
    sssrDesc.renderSize.height = WINDOW_HEIGHT;
    sssrDesc.normalsHistoryBufferFormat = FFX_SURFACE_FORMAT_R16G16B16A16_FLOAT;
    sssrDesc.backendInterface = s_gc.ffxInterface;
    ffxResult = ffxSssrContextCreate(&s_gc.sssrContext, &sssrDesc);
    assert(ffxResult == FFX_OK);
#endif

    InitStation();
    switch (frameSync.demoMode)
    {
        case 0:
            break;
        case 1:
            InitPlanet();
            break;
        case 2:
            InitTerrain();
            break;
        default:
            break;
    }

    s_gc.cameraAttribs = std::make_unique<HLSL::CameraAttribs[]>(2);

    InitializeCommonResources();

    s_gc.shadowMap = std::make_unique<ShadowMap>();
    s_gc.shadowMap->Initialize();
    s_gc.shadowMap->InitializeResourceBindings(s_gc.terrain.model);

    s_gc.epipolarLightScattering = std::make_unique<EpipolarLightScattering>(EpipolarLightScattering::CreateInfo{
    s_gc.m_pDevice,
    nullptr,
    s_gc.m_pImmediateContext,
    TEX_FORMAT_RGBA8_UNORM_SRGB, /* SCDesc.ColorBufferFormat, */
    TEX_FORMAT_D32_FLOAT, /* SCDesc.DepthBufferFormat, */
    TEX_FORMAT_R11G11B10_FLOAT,
    true, /* m_PackMatrixRowMajor, */
        });

    InitPBRRenderer(s_gc.shadowMap->GetShadowMap());

    // Navigation window is 72 x 120 pixels.
    uint32_t navWidth = WINDOW_WIDTH; // (uint32_t)ceilf((float(NagivationWindowWidth) / 160.0f) * (float)WINDOW_WIDTH);
    uint32_t navHeight = WINDOW_HEIGHT; // (uint32_t)ceilf((float(NagivationWindowHeight) / 200.0f) * (float)WINDOW_HEIGHT);

    for (int i = 0; i < s_gc.vc.number_of_frames_in_flight(); ++i)
    {
        GraphicsContext::BufferData bd{};

        bd.frameStaging = 
            s_gc.vc.create_buffer(
                avk::memory_usage::host_coherent,
                vk::BufferUsageFlagBits::eTransferSrc,
                avk::generic_buffer_meta::create_from_size(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t)));

        bd.rotoscope = 
            s_gc.vc.create_buffer(
                avk::memory_usage::host_coherent,
                vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer,
                avk::storage_buffer_meta::create_from_size(GRAPHICS_MODE_WIDTH * GRAPHICS_MODE_HEIGHT * sizeof(RotoscopeShader)));

        bd.uniform = 
            s_gc.vc.create_buffer(
                avk::memory_usage::host_coherent,
                vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
                avk::uniform_buffer_meta::create_from_size(sizeof(UniformBlock)));

        bd.command = 
            commandPool->alloc_command_buffer(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

        bd.navigation = s_gc.vc.create_image_sampler(
            s_gc.vc.create_image_view(
                s_gc.vc.create_image(navWidth, navHeight, vk::Format::eR8G8B8A8Unorm, 1, avk::memory_usage::device, avk::image_usage::general_image | avk::image_usage::shader_storage)
            ),
            s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge));

        bd.gameOutput = s_gc.vc.create_image_sampler(
            s_gc.vc.create_image_view(
                s_gc.vc.create_image(navWidth, navHeight, vk::Format::eR8G8B8A8Unorm, 1, avk::memory_usage::device, avk::image_usage::general_image | avk::image_usage::shader_storage)
            ),
            s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge));

        bd.orrery = s_gc.vc.create_image_sampler(
            s_gc.vc.create_image_view(
                s_gc.vc.create_image(920, 718, vk::Format::eR8G8B8A8Unorm, 1, avk::memory_usage::device, avk::image_usage::general_image | avk::image_usage::shader_storage)
            ),
            s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge));

        bd.starmap = s_gc.vc.create_image_sampler(
            s_gc.vc.create_image_view(
                s_gc.vc.create_image(1112, 664, vk::Format::eR8G8B8A8Unorm, 1, avk::memory_usage::device, avk::image_usage::general_image | avk::image_usage::shader_storage)
            ),
            s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge));

        bd.ui = s_gc.vc.create_image_sampler(
            s_gc.vc.create_image_view(
                s_gc.vc.create_image(navWidth, navHeight, vk::Format::eR8G8B8A8Unorm, 1, avk::memory_usage::device, avk::image_usage::general_image | avk::image_usage::shader_storage)
            ),
            s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge));            

        bd.iconUniform =
            s_gc.vc.create_buffer(
                avk::memory_usage::host_coherent,
                vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
                avk::uniform_buffer_meta::create_from_size(sizeof(IconUniform<64>)));

        bd.orreryUniform =
            s_gc.vc.create_buffer(
                avk::memory_usage::host_coherent,
                vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
                avk::uniform_buffer_meta::create_from_size(sizeof(IconUniform<64>))); 

        bd.starmapUniform =
            s_gc.vc.create_buffer(
                avk::memory_usage::host_coherent,
                vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
                avk::uniform_buffer_meta::create_from_size(sizeof(IconUniform<700>)));

        bd.avkdColorBuffer = s_gc.vc.create_image_sampler(
            s_gc.vc.create_image_view(
                s_gc.vc.create_image(WINDOW_WIDTH, WINDOW_HEIGHT, vk::Format::eR8G8B8A8Srgb, 1, avk::memory_usage::device, avk::image_usage::general_color_attachment),
                vk::Format::eR8G8B8A8Srgb
            ),
            s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge)
        );

        bd.avkdSsrBuffer = s_gc.vc.create_image_sampler(
            s_gc.vc.create_image_view(
                s_gc.vc.create_image(WINDOW_WIDTH, WINDOW_HEIGHT, vk::Format::eR16G16B16A16Sfloat, 1, avk::memory_usage::device, avk::image_usage::general_color_attachment)
            ),
            s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge)
        );

        TextureDesc cbDesc{};
        cbDesc.Type = RESOURCE_DIM_TEX_2D;
        cbDesc.Width = WINDOW_WIDTH;
        cbDesc.Height = WINDOW_HEIGHT;
        cbDesc.Format = TEX_FORMAT_RGBA8_UNORM_SRGB;
        cbDesc.MipLevels = 1;
        cbDesc.SampleCount = 1;
        cbDesc.Usage = USAGE_DEFAULT;
        cbDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
        ITexture* dcb{};
        pDeviceVk->CreateTextureFromVulkanImage(bd.avkdColorBuffer.get().get_image().handle(), cbDesc, RESOURCE_STATE_UNDEFINED, &dcb);
        bd.diligentColorBuffer = dcb->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);

        cbDesc.Type = RESOURCE_DIM_TEX_2D;
        cbDesc.Width = WINDOW_WIDTH;
        cbDesc.Height = WINDOW_HEIGHT;
        cbDesc.Format = TEX_FORMAT_RGBA16_FLOAT;
        cbDesc.MipLevels = 1;
        cbDesc.SampleCount = 1;
        cbDesc.Usage = USAGE_DEFAULT;
        cbDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
        dcb = {};
        pDeviceVk->CreateTextureFromVulkanImage(bd.avkdSsrBuffer.get().get_image().handle(), cbDesc, RESOURCE_STATE_UNDEFINED, &dcb);
        bd.diligentSsrBuffer = dcb->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        bd.diligentSsrBufferSrv = dcb->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        bd.avkdDepthBuffer = s_gc.vc.create_image(WINDOW_WIDTH, WINDOW_HEIGHT, vk::Format::eD32Sfloat, 1, avk::memory_usage::device, avk::image_usage::general_depth_stencil_attachment);
        cbDesc = {};
        cbDesc.Type = RESOURCE_DIM_TEX_2D;
        cbDesc.Width = WINDOW_WIDTH;
        cbDesc.Height = WINDOW_HEIGHT;
        cbDesc.Format = TEX_FORMAT_D32_FLOAT;
        cbDesc.MipLevels = 1;
        cbDesc.SampleCount = 1;
        cbDesc.Usage = USAGE_DEFAULT;
        cbDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_DEPTH_STENCIL;
        ITexture* dd{};
        pDeviceVk->CreateTextureFromVulkanImage(bd.avkdDepthBuffer.get().handle(), cbDesc, RESOURCE_STATE_UNDEFINED, &dd);
        bd.diligentDepthBuffer = dd->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL);

        bd.avkdShadowDepthBuffer = s_gc.vc.create_image(2048, 2048, vk::Format::eD32Sfloat, 1, avk::memory_usage::device, avk::image_usage::general_depth_stencil_attachment);
        cbDesc = {};
        cbDesc.Type = RESOURCE_DIM_TEX_2D;
        cbDesc.Width = 2048;
        cbDesc.Height = 2048;
        cbDesc.Format = TEX_FORMAT_D32_FLOAT;
        cbDesc.MipLevels = 1;
        cbDesc.SampleCount = 1;
        cbDesc.Usage = USAGE_DEFAULT;
        cbDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_DEPTH_STENCIL;
        dd = {};
        pDeviceVk->CreateTextureFromVulkanImage(bd.avkdShadowDepthBuffer.get().handle(), cbDesc, RESOURCE_STATE_UNDEFINED, &dd);
        bd.diligentShadowDepthBuffer = dd->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL);

        cbDesc = {};
        cbDesc.Type = RESOURCE_DIM_TEX_2D;
        cbDesc.Width = WINDOW_WIDTH;
        cbDesc.Height = WINDOW_HEIGHT;
        cbDesc.Format = TEX_FORMAT_RGBA8_UNORM_SRGB;
        cbDesc.MipLevels = 1;
        cbDesc.SampleCount = 1;
        cbDesc.Usage = USAGE_DEFAULT;
        cbDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
        pDeviceVk->CreateTexture(cbDesc, nullptr, &bd.offscreenColorBuffer);

        cbDesc = {};
        cbDesc.Type = RESOURCE_DIM_TEX_2D;
        cbDesc.Width = WINDOW_WIDTH;
        cbDesc.Height = WINDOW_HEIGHT;
        cbDesc.Format = TEX_FORMAT_D32_FLOAT;
        cbDesc.MipLevels = 1;
        cbDesc.SampleCount = 1;
        cbDesc.Usage = USAGE_DEFAULT;
        cbDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_DEPTH_STENCIL;
        pDeviceVk->CreateTexture(cbDesc, nullptr, &bd.offscreenDepthBuffer);

        cbDesc = {};
        cbDesc.Type = RESOURCE_DIM_TEX_2D;
        cbDesc.Width = GraphicsContext::BufferData::waterHeightMapSize;
        cbDesc.Height = GraphicsContext::BufferData::waterHeightMapSize;
        cbDesc.Format = TEX_FORMAT_R32_FLOAT;
        cbDesc.MipLevels = 1;
        cbDesc.SampleCount = 1;
        cbDesc.Usage = USAGE_DEFAULT;
        cbDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET | BIND_UNORDERED_ACCESS;
        pDeviceVk->CreateTexture(cbDesc, nullptr, &bd.waterHeightMap);
     
        s_gc.vc.record_and_submit_with_fence({
            avk::sync::image_memory_barrier(bd.gameOutput->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({avk::layout::undefined, avk::layout::shader_read_only_optimal})
        }, * s_gc.mQueue)->wait_until_signalled();

        s_gc.buffers.push_back(std::move(bd));
    }

/*
std::array<vk::DescriptorSetLayoutBinding, 8> bindings = {
    vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eCompute), // imgOutput
    vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), // RotoBuffer
    vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute), // FONT1Texture
    vk::DescriptorSetLayoutBinding(3, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute), // FONT2Texture
    vk::DescriptorSetLayoutBinding(4, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute), // FONT3Texture
    vk::DescriptorSetLayoutBinding(5, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute), // LOGO1Texture
    vk::DescriptorSetLayoutBinding(6, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eCompute), // LOGO2Texture
    vk::DescriptorSetLayoutBinding(7, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute) // uniforms
};
*/

    s_gc.rotoscopePipeline = s_gc.vc.create_compute_pipeline_for(
        "rotoscope.comp",
        avk::descriptor_binding<avk::image_view_as_storage_image>(0, 0, 1u),
        avk::descriptor_binding<avk::buffer>(0, 1, s_gc.buffers[0].rotoscope),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 2, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 3, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 4, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 5, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 6, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 7, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 8, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 9, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 10, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 11, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 12, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 13, 1u),
        avk::descriptor_binding<avk::buffer>(0, 14, s_gc.buffers[0].uniform),
        avk::descriptor_binding<avk::buffer>(0, 15, s_gc.buffers[0].iconUniform),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 16, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 17, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 18, 1u)
    );

    s_gc.navigationPipeline = s_gc.vc.create_compute_pipeline_for(
        "navigation.comp",
        avk::descriptor_binding<avk::image_view_as_storage_image>(0, 0, 1u),
        avk::descriptor_binding<avk::buffer>(0, 1, s_gc.buffers[0].rotoscope),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 2, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 3, 1u),
        avk::descriptor_binding<avk::buffer>(0, 4, s_gc.buffers[0].uniform),
        avk::descriptor_binding<avk::buffer>(0, 5, s_gc.buffers[0].iconUniform),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 6, 1u)
    );

    s_gc.compositorPipeline = s_gc.vc.create_compute_pipeline_for(
        "compositor.comp",
        avk::descriptor_binding<avk::image_view_as_storage_image>(0, 0, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 1, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 2, 1u),
        avk::descriptor_binding<avk::buffer>(0, 3, s_gc.buffers[0].uniform)
    );    

    s_gc.orbitPipeline = s_gc.vc.create_compute_pipeline_for(
        "orbit.comp",
        avk::descriptor_binding<avk::image_view_as_storage_image>(0, 0, 1u),
        avk::descriptor_binding<avk::buffer>(0, 1, s_gc.buffers[0].rotoscope),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 2, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 3, 1u),
        avk::descriptor_binding<avk::buffer>(0, 4, s_gc.buffers[0].uniform),
        avk::descriptor_binding<avk::buffer>(0, 5, s_gc.buffers[0].iconUniform),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 6, 1u)
    );

    s_gc.orreryPipeline = s_gc.vc.create_compute_pipeline_for(
        "orrery.comp",
        avk::descriptor_binding<avk::image_view_as_storage_image>(0, 0, 1u),
        avk::descriptor_binding<avk::buffer>(0, 1, s_gc.buffers[0].rotoscope),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 2, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 3, 1u),
        avk::descriptor_binding<avk::buffer>(0, 4, s_gc.buffers[0].uniform),
        avk::descriptor_binding<avk::buffer>(0, 5, s_gc.buffers[0].orreryUniform)
    );

    s_gc.starmapPipeline = s_gc.vc.create_compute_pipeline_for(
        "starmap.comp",
        avk::descriptor_binding<avk::image_view_as_storage_image>(0, 0, 1u),
        avk::descriptor_binding<avk::buffer>(0, 1, s_gc.buffers[0].rotoscope),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 2, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 3, 1u),
        avk::descriptor_binding<avk::buffer>(0, 4, s_gc.buffers[0].uniform),
        avk::descriptor_binding<avk::buffer>(0, 5, s_gc.buffers[0].starmapUniform)
    );

    s_gc.encounterPipeline = s_gc.vc.create_compute_pipeline_for(
        "encounter.comp",
        avk::descriptor_binding<avk::image_view_as_storage_image>(0, 0, 1u),
        avk::descriptor_binding<avk::buffer>(0, 1, s_gc.buffers[0].rotoscope),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 2, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 3, 1u),
        avk::descriptor_binding<avk::buffer>(0, 4, s_gc.buffers[0].uniform),
        avk::descriptor_binding<avk::buffer>(0, 5, s_gc.buffers[0].iconUniform),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 6, 1u)
    );

    s_gc.textPipeline = s_gc.vc.create_compute_pipeline_for(
        "text.comp",
        avk::descriptor_binding<avk::image_view_as_storage_image>(0, 0, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 1, 1u),
        avk::descriptor_binding<avk::buffer>(0, 2, s_gc.buffers[0].uniform)
    );

    s_gc.titlePipeline = s_gc.vc.create_compute_pipeline_for(
        "title.comp",
        avk::descriptor_binding<avk::image_view_as_storage_image>(0, 0, 1u),
        avk::descriptor_binding<avk::buffer>(0, 1, s_gc.buffers[0].uniform),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 2, 1u),
        avk::descriptor_binding<avk::combined_image_sampler_descriptor_info>(0, 3, 1u)
    );

    s_gc.descriptorCache = s_gc.vc.create_descriptor_cache();

    LoadFonts();
    LoadSplashImages();
    LoadSDFImages();
    LoadAssets();

    s_gc.textImage = s_gc.vc.create_image_sampler(
        s_gc.vc.create_image_view(
            s_gc.vc.create_image(TEXT_MODE_WIDTH, TEXT_MODE_HEIGHT, vk::Format::eR8G8B8A8Unorm, 1, avk::memory_usage::device, avk::image_usage::general_image)
        ),
        s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge)
    );

    s_gc.epoch = std::chrono::steady_clock::now();

    keyboard = std::make_unique<SDLKeyboard>();

    graphicsPixels = std::vector<uint32_t>();
    graphicsPixels.resize(GRAPHICS_MEMORY_ALLOC);

    rotoscopePixels = std::vector<Rotoscope>();
    rotoscopePixels.resize(GRAPHICS_MEMORY_ALLOC);

    textPixels = std::vector<uint32_t>();
    textPixels.resize(TEXT_MODE_WIDTH * TEXT_MODE_HEIGHT);

    s_gc.m_pImmediateContext->Flush();

    initPromise.set_value();

    return 0;
}

void TranslateMan(InterpolatorPoint manPoint)
{
    static const std::vector<std::string> targetNodeNames = { "Astronaut" };

    static std::once_flag initFlag;
    static std::unordered_map<std::string, QuaternionF> initialRotations;

    std::call_once(initFlag, [&]() {
        auto& nodes = s_gc.station.model->GetNodes();
        for (auto& node : nodes) {
            if (std::find(targetNodeNames.begin(), targetNodeNames.end(), node.Name) != targetNodeNames.end()) {
                initialRotations[node.Name] = node.Rotation;
            }
        }
    });

    float angle = manPoint.heading;
    angle -= PI / 2;
    QuaternionF newHeading = QuaternionF::RotationFromAxisAngle({ 0.f, 0.f, 1.f }, angle);

    {
        float a = 0.38f;
        float b = (0.25f + 0.21f) / 2.0f;
        float centerX = 0.0f;
        float centerY = -0.02f; // Adjusted for asymmetry in original y bounds

        float dX = (manPoint.position.x - centerX);
        float dY = (manPoint.position.y - centerY) * (a / b);

        if (dX * dX + dY * dY <= (a * a)) {
            manPoint.position.z += 0.027f;
        }
    }

    auto& nodes = s_gc.station.model->GetNodes();
    for (auto& node : nodes) {
        if (std::find(targetNodeNames.begin(), targetNodeNames.end(), node.Name) != targetNodeNames.end()) {

            node.Translation = { manPoint.position.x, manPoint.position.y, manPoint.position.z };
            node.Rotation = initialRotations[node.Name] * newHeading;
        }
    }
}

void GraphicsMoveSpaceMan(uint16_t x, uint16_t y)
{
    float4 easyCameraf4Position = float4(0.0f, -0.217814416f, 0.159155563f, 1.0f);
    float4x4 easyViewProjInvT = Matrix4x4(
        -0.0254427418f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.00776134850f, 2.17596197f, -2.10976076f,
        0.0f, 0.0174322892f, -1.58996093f, 1.56110823f,
        0.0f, 0.0f, -9.98998070f, 9.99998188f
    );

    // Retrieve the current camera attributes
    auto& camAttribs = s_gc.cameraAttribs[0]; // Assuming index 0 for simplicity, adjust as necessary

    float3 cameraPosition = float3(easyCameraf4Position.x, easyCameraf4Position.y, easyCameraf4Position.z);
    float distanceToOrigin = length(cameraPosition);

    // Convert screen space coordinates to normalized device coordinates (NDC)
    float ndcX = (2.0f * x / GRAPHICS_MODE_WIDTH) - 1.0f;
    float ndcY = 1.0f - (2.0f * y / GRAPHICS_MODE_HEIGHT);

    // Create clip space position
    float4 clipSpacePos = float4(ndcX, ndcY, 1.0, 1.0f); // z = 1 for forward direction, w = 1 for perspective division

    // Transform clip space position to world space using the precomputed inverse view-projection matrix
    float4 worldSpacePos = clipSpacePos * easyViewProjInvT.Transpose() * s_gc.station.modelTransform.Inverse();

    // Perspective divide
    worldSpacePos /= worldSpacePos.w;

    // Define the plane in world space where the spaceman moves (e.g., ground plane)
    float3 planeNormal = float3(0.0f, -0.05f, 1.00f); // Adjusted plane normal to be tilted
    float planeD = 0.0f; // Plane is at z = 0

    float4 worldCam = easyCameraf4Position * s_gc.station.modelTransform.Inverse();

    worldCam /= worldCam.w;

    // Calculate intersection of ray from camera to worldSpacePos with the plane
    float3 rayOrigin = float3(worldCam.x, worldCam.y, worldCam.z);
    float3 rayDirection = normalize(float3(worldSpacePos.x, worldSpacePos.y, worldSpacePos.z) - rayOrigin);
    float denom = dot(planeNormal, rayDirection);

    if (fabs(denom) > 1e-6) { // Ensure the denominator is not too small
        float t = -dot(planeNormal, rayOrigin) / denom;
        if (t >= 0) { // Check if the intersection is in front of the camera
            std::lock_guard<std::mutex> lg(s_gc.spaceManMutex);

            double currentTimeInSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - s_gc.epoch).count();
            double nextTime = currentTimeInSeconds + 0.15f;

            float3 intersectionPoint = rayOrigin + t * rayDirection;

            if (s_gc.spaceMan.isExtrapolating(currentTimeInSeconds))
            {
                auto point = s_gc.spaceMan.interpolate(currentTimeInSeconds);
                s_gc.spaceMan.addPointWithTime(currentTimeInSeconds, point.position);
                s_gc.spaceMan.addPointWithTime(nextTime, { intersectionPoint.x, intersectionPoint.y, intersectionPoint.z });
            }
            else
            {
                s_gc.spaceMan.addPoint(nextTime, { intersectionPoint.x, intersectionPoint.y, intersectionPoint.z });
            }
        }
    }
}

void GraphicsSplash(uint16_t seg, uint16_t fileNum)
{
    s_gc.epoch = std::chrono::steady_clock::now();
}

static std::binary_semaphore s_graphicsShutdown{0};

std::binary_semaphore& GraphicsInit()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());

        s_graphicsShutdown.release();
        return s_graphicsShutdown;
    }

    /*
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0)
        printf("Failed to initialize SDL_image with PNG support: %s\n", IMG_GetError());
        return;
    }*/

    FILE* file;
    file = freopen("stdout", "w", stdout); // redirects stdout
    file = freopen("stderr", "w", stderr); // redirects stderr

    graphicsThread = std::jthread([] {
        GraphicsInitThread();

#if defined(_WIN32) && (defined(_DEBUG) || defined(DEBUG))
            // Disable memory leak detection and allocation debug flags
            int dbgFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
            dbgFlags &= ~(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // Clear the flags
            _CrtSetDbgFlag(dbgFlags);
#endif

        // Uncomment to start the game immediately
        //StartEmulationThread("");

        while (!stopSemaphore.try_acquire()) {
            constexpr std::chrono::nanoseconds scanout_duration = std::chrono::nanoseconds(13340000); // 80% of 1/60th of a second
            constexpr std::chrono::nanoseconds retrace_duration = std::chrono::nanoseconds(3330000); // 20% of 1/60th of a second

            {
                std::lock_guard<std::mutex> lg(graphicsRetrace);
                GraphicsUpdate();
                if (graphicsIsShutdown)
                    return;

#if defined(USE_CPU_RASTERIZATION)
                std::this_thread::sleep_for(scanout_duration);
#endif
            }

#if defined(USE_CPU_RASTERIZATION)
            std::this_thread::sleep_for(retrace_duration);
#endif
        }
    });

    InitTextToSpeech();

    return s_graphicsShutdown;
}

uint32_t DrawLinePixel(const Rotoscope& roto, vec2<float> uv, float polygonWidth)
{
    uint32_t pixel = 0;

    float lineX1 = ((float)roto.lineData.x0 + 0.10f)  / (float)GRAPHICS_MODE_WIDTH;
    float lineY1 = ((float)roto.lineData.y0 + 0.10f) / (float)GRAPHICS_MODE_HEIGHT;
    float lineX2 = ((float)roto.lineData.x1 + 0.90f) / (float)GRAPHICS_MODE_WIDTH;
    float lineY2 = ((float)roto.lineData.y1 + 0.90f) / (float)GRAPHICS_MODE_HEIGHT;

    float a = polygonWidth;
    float one_px = 1.0f / WINDOW_WIDTH;
    vec2<float> p1 = {lineX1, lineY1};
    vec2<float> p2 = {lineX2, lineY2};
    auto mix = [](float a, float b, float t) { return a * (1 - t) + b * t; };
    auto distance = [](vec2<float> a, vec2<float> b) { return sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2)); };

    float d = distance(p1, p2);
    float duv = distance(p1, uv);

    float r = 1.0f - floor(1.0f - (a * one_px) + distance(vec2<float>{mix(p1.x, p2.x, std::clamp(duv / d, 0.0f, 1.0f)), mix(p1.y, p2.y, std::clamp(duv / d, 0.0f, 1.0f))}, uv));

    if (r > 0.0f)
    {
        pixel = colortable[roto.fgColor & 0xf];
    }
    else
    {
        pixel = colortable[roto.bgColor & 0xf];
    }

    return pixel;
}

uint32_t DrawFontPixel(const Rotoscope& roto, vec2<float> uv, vec2<float> subUv)
{
    uint32_t pixel = 0;

    float fontX = ((float)roto.blt_x + subUv.x) / (float)roto.blt_w;
    float fontY = ((float)roto.blt_y + subUv.v) / (float)roto.blt_h;

    if(roto.textData.fontNum == 1)
    {
        // Find the character in our atlas.
        constexpr float fontSpaceWidth = 8.0f * 4.0f;
        constexpr float fontSpaceHeight = 8.0f * 4.0f;

        constexpr float atlasWidth = 448.0f;
        constexpr float atlasHeight = 160.0f;

        uint32_t c = roto.textData.character - 32;
        uint32_t fontsPerRow = 448 / (int)fontSpaceWidth;
        uint32_t fontRow = c / fontsPerRow;
        uint32_t fontCol = c % fontsPerRow;

        float u = fontCol * fontSpaceWidth / atlasWidth;
        float v = fontRow * fontSpaceHeight / atlasHeight;

        u += fontX * (fontSpaceWidth / atlasWidth);
        v += fontY * (fontSpaceHeight / atlasHeight);

        auto glyph = bilinearSample(FONT1Texture, u, v);
        pixel = colortable[roto.bgColor & 0xf];
        if(glyph.r() > 0.80f)
        {
            pixel = colortable[roto.fgColor & 0xf];
        }
        #if 0
        else
        {
            #if 1
            uint32_t r = static_cast<uint32_t>(fontX * 255);
            uint32_t g = static_cast<uint32_t>(fontY * 255);
            uint32_t b = 0;
            uint32_t a = 255;

            pixel = (a << 24) | (r << 16) | (g << 8) | b;
            #endif
        }
        #endif
    } else if (roto.textData.fontNum == 2)
    {
        // Find the character in our atlas.
        constexpr float fontSpaceWidth = 15.0f * 4.0f;
        constexpr float fontSpaceHeight = 11.0f * 4.0f;

        constexpr float atlasWidth = 840.0f;
        constexpr float atlasHeight = 220.0f;

        uint32_t c = roto.textData.character - 32;
        uint32_t fontsPerRow = 840 / (int)fontSpaceWidth;
        uint32_t fontRow = c / fontsPerRow;
        uint32_t fontCol = c % fontsPerRow;

        float u = fontCol * fontSpaceWidth / atlasWidth;
        float v = fontRow * fontSpaceHeight / atlasHeight;

        u += fontX * (fontSpaceWidth / atlasWidth);
        v += fontY * (fontSpaceHeight / atlasHeight);

        auto glyph = bilinearSample(FONT2Texture, u, v);
        pixel = colortable[roto.bgColor & 0xf];
        if(glyph.r() > 0.9f)
        {
            pixel = colortable[roto.fgColor & 0xf];
        }
    } else if (roto.textData.fontNum == 3)
    {
                            // Find the character in our atlas.
        constexpr float fontSpaceWidth = 15.0f * 4.0f;
        constexpr float fontSpaceHeight = 11.0f * 4.0f;

        constexpr float atlasWidth = 840.0f;
        constexpr float atlasHeight = 220.0f;

        uint32_t c = roto.textData.character - 32;
        uint32_t fontsPerRow = 840 / (int)fontSpaceWidth;
        uint32_t fontRow = c / fontsPerRow;
        uint32_t fontCol = c % fontsPerRow;

        float u = fontCol * fontSpaceWidth / atlasWidth;
        float v = fontRow * fontSpaceHeight / atlasHeight;

        u += fontX * (fontSpaceWidth / atlasWidth);
        v += fontY * (fontSpaceHeight / atlasHeight);

        auto glyph = bilinearSample(FONT3Texture, u, v);
        pixel = colortable[roto.bgColor & 0xf];
        if(glyph.r() > 0.9f)
        {
            pixel = colortable[roto.fgColor & 0xf];
        }
        #if 0
        else
        {
            uint32_t r = static_cast<uint32_t>(fontX * 255);
            uint32_t g = static_cast<uint32_t>(fontY * 255);
            uint32_t b = 0;
            uint32_t a = 255;

            pixel = (a << 24) | (r << 16) | (g << 8) | b;
        }
        #endif
    }
    #if 0
    else
    {
        uint32_t r = static_cast<uint32_t>(fontX * 255);
        uint32_t g = static_cast<uint32_t>(fontY * 255);
        uint32_t b = 0;
        uint32_t a = 255;

        pixel = (a << 24) | (r << 16) | (g << 8) | b;
    }
    #endif

    return pixel;
}

uint32_t DrawSDFSilhouette(const Rotoscope& roto, vec2<float> subUv) {
    auto sdf = bilinearSample(RaceDosPicTexture, subUv.x, subUv.y);

    if (sdf.r() < 0.5)
    {
        return colortable[roto.bgColor & 0xf];
    }
    else
    {
        return colortable[roto.fgColor & 0xf];
    }
}

float calculateDistance(vec2<float> point1, vec2<float> point2) {
    float xDist = point2.x - point1.x;
    float yDist = 0.60f * (point2.y - point1.y);
    return sqrt((xDist * xDist) + (yDist * yDist));
}

float calculateBoundingBoxDistance(vec2<float> point1, vec2<float> point2) {
    float xDist = abs(point2.x - point1.x);
    float yDist = abs(point2.y - point1.y);
    return std::max(xDist, yDist);
}

float calculateBoundingBoxDistanceIcon(vec2<float> point1, vec2<float> point2) {
    float xDist = abs(point2.x - point1.x);
    float yDist = 0.60f * abs(point2.y - point1.y);
    return std::max(xDist, yDist);
}

uint32_t DrawNavigationPixel(const Rotoscope& roto, vec2<float> uv, vec2<float> subUv, const std::vector<Icon>& icons)
{
    uint32_t pixel = 0;
    vec2<float> sub{};

    float subX = ((float)roto.blt_x + subUv.x) / (float)roto.blt_w;
    float subY = ((float)roto.blt_y + subUv.y) / (float)roto.blt_h;

    /* 
    50     C= NULL-ICON    ( 0 radius circle icon)
    253    C= SYS-ICON     ( star icon)                             
    254    C= INVIS-ICON   ( invisible icon - may collide)          
    255    C= FLUX-ICON    ( flux icon identifier)   
    */

/*
 @IX @IY ?INVIS               \ in the vis area?                
  @ID 51 91 WITHIN OR           \ or a filled circle            
  IF @ID NULL-ICON <                                            
    IF .8X8ICON ELSE @ID .ICONCASES THEN                        
  THEN ;                                                        
  */

    auto pixelPos = vec2<float>((float)roto.blt_x + subUv.x, (float)roto.blt_y + subUv.y);

    for(auto icon : icons)
    {
        auto iconPos = vec2<float>((float)icon.screenX, (float)icon.screenY);
        auto bltPos = vec2<float>((float)icon.bltX + 3.0f, (float)icon.bltY + 3.0f);

        if(icon.id >= 51 && icon.id <= 91)
        {
            float basesize = 29.0f * (float)(icon.id - 50);

            if(calculateDistance(iconPos, pixelPos) < basesize)
            {
                return colortable[icon.clr & 0xf];
            }
        }
        else if(icon.id < 50)
        {
            // Draw 8x8 icon. #29 is our spacecraft
            if(calculateBoundingBoxDistance(bltPos, pixelPos) < 4.0f)
            {
                return colortable[icon.clr & 0xf];
            }
        }
        else
        {
            float distance = 1.0;

            switch(icon.id)
            {
                case 253: // Star icon
                    /*
                    CASE SYSCASES ( color -- *bltseg-pfa )                          
                    RED    IS SSYSEG   \ small starsystem blt                     
                    ORANGE IS SSYSEG   \ medium starsystem blt                    
                    WHITE  IS MSYSEG   \ medium starsystem blt                    
                    YELLOW IS MSYSEG   \ medium starsystem blt                    
                    OTHERS LSYSEG        \ large starsystem blt  
                    */
                    
                    switch(icon.clr & 0xf)
                    {
                        case 0x4: // RED
                        case 0x6: // ORANGE
                            distance = 7.0f;
                            break;
                        case 0xE: // YELLOW
                        case 0xF: // WHITE
                            distance = 10.0f;
                            break;
                        default: // OTHERS
                            distance = 14.0f;
                            break;
                    }
                    if(calculateBoundingBoxDistanceIcon(iconPos, pixelPos) < distance)
                    {
                        return colortable[icon.clr & 0xf];
                    }
                    break;
                case 254: // Invisible icon (Encounter hit testing?)
                    if(roto.blt_x == icon.screenX && roto.blt_y == icon.screenY)
                    {
                        return colortable[icon.clr & 0xf];
                    }
                    break;
                case 255: // Flux icon
                    if(roto.blt_x == icon.screenX && roto.blt_y == icon.screenY)
                    {
                        return colortable[icon.clr & 0xf];
                    }
                    break;
                default:
                    assert(false);
                    break;
            }
        }

    }
    
    return 0x0;
}

uint32_t DrawRunBit(const Rotoscope& roto, vec2<float> uv, vec2<float> subUv)
{
    uint32_t pixel = 0;
    vec2<float> sub{};

    float subX = ((float)roto.blt_x + subUv.x) / (float)roto.blt_w;
    float subY = ((float)roto.blt_y + subUv.y) / (float)roto.blt_h;

    switch (roto.runBitData.tag)
    {
        case 44: // Port-Pic Top 100 pixels
            pixel = TextureColorToARGB(bilinearSample(PORTPICTexture, subX, subY * 0.5f));
            break;
        case 49: // Port-Pic Botton 100 pixels
            pixel = TextureColorToARGB(bilinearSample(PORTPICTexture, subX, (subY * 0.5f) + 0.5f));
            break;
        case 141: // First splash
            pixel = TextureColorToARGB(bilinearSample(LOGO1Texture, uv.u, uv.v));
            break;
        case 54:
            pixel = TextureColorToARGB(bilinearSample(LOGO2Texture, uv.u, uv.v));
            break;
        case 125:
            sub = vec2<float>(subX / 5.0f, subY);
            pixel = DrawSDFSilhouette(roto, sub);
            break;
        case 126:
            sub = vec2<float>(subX / 5.0f + (1.0f / 5.0f), subY);
            pixel = DrawSDFSilhouette(roto, sub);
            break;
        case 127:
            sub = vec2<float>(subX / 5.0f + (2.0f / 5.0f), subY);
            pixel = DrawSDFSilhouette(roto, sub);
            break;
        case 128:
            sub = vec2<float>(subX / 5.0f + (3.0f / 5.0f), subY);
            pixel = DrawSDFSilhouette(roto, sub);
            break;
        case 129:
            sub = vec2<float>(subX / 5.0f + (4.0f / 5.0f), subY);
            pixel = DrawSDFSilhouette(roto, sub);
            break;
        default:
            //assert(false);
            pixel = roto.argb;
            break;
    }

    return pixel;
}

static void DoRotoscope(std::vector<uint32_t>& windowData, const std::vector<Rotoscope>& rotoPixels, const std::vector<Icon>& icons)
{
    uint32_t index = 0;
    const float polygonWidth = (float)WINDOW_WIDTH / (float)GRAPHICS_MODE_WIDTH;

    for(uint32_t y = 0; y < WINDOW_HEIGHT; ++y)
    {
        for(uint32_t x = 0; x < WINDOW_WIDTH; ++x)
        {
            // Calculate the corresponding position in the smaller texture
            uint32_t srcX = x * GRAPHICS_MODE_WIDTH / WINDOW_WIDTH;
            uint32_t srcY = y * GRAPHICS_MODE_HEIGHT / WINDOW_HEIGHT;

            // Calculate the index in the smaller texture
            uint32_t srcIndex = srcY * GRAPHICS_MODE_WIDTH + srcX;

            auto& roto = rotoPixels[srcIndex];

            float xcoord = (float)x / (float)WINDOW_WIDTH;
            float ycoord = (float)y / (float)WINDOW_HEIGHT;
            vec2<float> uv = {xcoord, ycoord};

            float subPixelXOffset = (xcoord * GRAPHICS_MODE_WIDTH) - srcX;
            float subPixelYOffset = (ycoord * GRAPHICS_MODE_HEIGHT) - srcY;
            vec2<float> subUv = {subPixelXOffset, subPixelYOffset};

            // Pull the pixel from the smaller texture
            uint32_t pixel = roto.argb;

            if(s_useRotoscope)
            {
                switch(roto.content)
                {
                    case LinePixel:
                        //pixel = DrawLinePixel(roto, uv, polygonWidth);
                        break;
                    case TextPixel:
                        pixel = DrawFontPixel(roto, uv, subUv);    
                        break;
                    case RunBitPixel:
                        pixel = DrawRunBit(roto, uv, subUv);
                        break;
                    case NavigationalPixel:
                        pixel = DrawNavigationPixel(roto, uv, subUv, icons);
                        break;
                    default:
                        //pixel = 0xffff0000;
                        pixel = colortable[(int)roto.content];
                        break;
                }
            }
            else
            {
                if(!s_useEGA)
                {
                    static uint32_t CGAPalette[4] = {0xff000000, 0xff00aaaa, 0xffaa00aa, 0xffaaaaaa};

                    uint8_t c = EGAToCGA[roto.EGAcolor];
                    if(subUv.u < 0.5f)
                    {
                        pixel = CGAPalette[(c >> 2) & 3];
                    }
                    else
                    {
                        pixel = CGAPalette[c & 3];
                    }
                }
            }

            // Place the pixel in the larger surface
            windowData[index] = pixel;
            ++index;
        }
    }
}

std::vector<avk::recorded_commands_t> TextPass(VulkanContext::frame_id_t inFlightIndex, UniformBlock& uniform, const void* data, uint64_t dataSize)
{
    auto sb = s_gc.vc.create_buffer(
        AVK_STAGING_BUFFER_MEMORY_USAGE,
        vk::BufferUsageFlagBits::eTransferSrc,
        avk::generic_buffer_meta::create_from_size(dataSize)
    );

    return {
        avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].gameOutput->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::general }),

        sb->fill(data,0, 0, dataSize),

        avk::sync::buffer_memory_barrier(sb.as_reference(),
            avk::stage::auto_stage >> avk::stage::auto_stage,
            avk::access::auto_access >> avk::access::auto_access
        ),

        avk::sync::image_memory_barrier(s_gc.textImage->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::transfer_dst }),

        avk::copy_buffer_to_image(sb, s_gc.textImage->get_image(), avk::layout::transfer_dst),

        avk::sync::image_memory_barrier(s_gc.textImage->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::transfer_dst, avk::layout::shader_read_only_optimal }),

        s_gc.buffers[inFlightIndex].uniform->fill(&uniform, 0, 0, sizeof(UniformBlock)),

        avk::sync::buffer_memory_barrier(s_gc.buffers[inFlightIndex].uniform.as_reference(),
            avk::stage::auto_stage >> avk::stage::auto_stage,
            avk::access::auto_access >> avk::access::auto_access
        ),

        avk::command::bind_pipeline(s_gc.textPipeline.as_reference()),
        avk::command::bind_descriptors(s_gc.textPipeline->layout(), s_gc.descriptorCache->get_or_create_descriptor_sets({
                avk::descriptor_binding(0, 0, s_gc.buffers[inFlightIndex].gameOutput->get_image_view()->as_storage_image(avk::layout::general)),
                avk::descriptor_binding(0, 1, s_gc.textImage->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 2, s_gc.buffers[inFlightIndex].uniform->as_uniform_buffer())
            })),
        avk::command::dispatch((WINDOW_WIDTH + 31u) / 32u, (WINDOW_HEIGHT + 31u) / 32u, 1),

        avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].gameOutput->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::general, avk::layout::shader_read_only_optimal })
    };
}

static float4x4 GetAdjustedProjectionMatrix(float FOV, float NearPlane, float FarPlane)
{
    float AspectRatio = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);
    float XScale, YScale;

    YScale = 1.f / std::tan(FOV / 2.f);
    XScale = YScale / AspectRatio;

    float4x4 Proj;
    Proj._11 = XScale;
    Proj._22 = YScale;
    Proj.SetNearFarClipPlanes(NearPlane, FarPlane, s_gc.m_pDevice->GetDeviceInfo().NDC.MinZ == -1);
    return Proj;
}

static float4x4 GetSurfacePretransformMatrix(const float3& f3CameraViewAxis)
{
    return float4x4::Identity();
}

void InitModel(std::string modelPath, GraphicsContext::SFModel& model, int defaultCameraIndex)
{
    SF_GLTF::ModelCreateInfo ModelCI;
    ModelCI.FileName = modelPath.c_str();

    try
    {
        model.model = std::make_shared<SF_GLTF::Model>(s_gc.m_pDevice, s_gc.m_pImmediateContext, ModelCI);
    }
    catch (const std::exception& e)
    {
        char cwd[1024];
        #ifdef _WIN32
        if (_getcwd(cwd, sizeof(cwd)) != nullptr) // Use _getcwd on Windows
        #else
        if (getcwd(cwd, sizeof(cwd)) != nullptr) // Use getcwd on other platforms
        #endif
        {
            printf("Current working directory: %s\n", cwd);
        }
        else
        {
            perror("getcwd() error");
        }
        printf("Failed to load model from path: %s\n", modelPath.c_str());
        throw;
    }

    std::vector<const Diligent::SF_GLTF::Node*> cameras;

    for (const auto* node : model.model->GetScenes()[0].LinearNodes)
    {
        if (node->pCamera != nullptr && node->pCamera->Type == SF_GLTF::Camera::Projection::Perspective)
        {
            cameras.push_back(node);
        }

        if (node->pLight != nullptr)
        {
            model.lights.push_back(node);
        }
    }

    if (!cameras.empty() && defaultCameraIndex < cameras.size())
    {
        model.camera = cameras[defaultCameraIndex];
    }
}

void InitPlanet()
{
    InitModel("C:/Users/Dean/Downloads/earth.glb", s_gc.planet);
}

void UpdatePlanet(VulkanContext::frame_id_t inFlightIndex)
{
    VulkanContext::frame_id_t frameCount = s_gc.vc.current_frame();

    double currentTimeInSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - s_gc.epoch).count();

    float rotationAngle = currentTimeInSeconds * 0.1f;
    //float rotationAngle = (float)frameCount * 0.01f;

    float axisOne = rotationAngle;

    float4x4 RotationMatrixCam = float4x4::Identity();
    float4x4 RotationMatrixModel = float4x4::RotationY(axisOne);

    s_gc.planet.model->ComputeTransforms(s_gc.renderParams.SceneIndex, s_gc.planet.transforms[0]);
    s_gc.planet.aabb = s_gc.planet.model->ComputeBoundingBox(s_gc.renderParams.SceneIndex, s_gc.planet.transforms[0], nullptr);
    s_gc.planet.worldspaceAABB = s_gc.planet.aabb;

    // Center and scale model
    float  MaxDim = 0;
    float3 ModelDim{ s_gc.planet.aabb.Max - s_gc.planet.aabb.Min };
    MaxDim = std::max(MaxDim, ModelDim.x);
    MaxDim = std::max(MaxDim, ModelDim.y);
    MaxDim = std::max(MaxDim, ModelDim.z);

    float4x4 InvYAxis = float4x4::Identity();
    InvYAxis._22 = -1;

#if 1
    s_gc.planet.scale = (1.0f / std::max(MaxDim, 0.01f)) * 0.5f;
    auto     Translate = -s_gc.planet.aabb.Min - 0.5f * ModelDim;
    InvYAxis._22 = -1;
#else
    s_gc.planet.scale = 1.0f;
    float3 Translate = { 0.f, 0.f, 0.f };
#endif

    s_gc.planet.modelTransform = float4x4::Translation(Translate) * float4x4::Scale(s_gc.planet.scale) * InvYAxis;
    s_gc.planet.scaleAndTransform = float4x4::Translation(Translate) * float4x4::Scale(s_gc.planet.scale);

    s_gc.planet.model->ComputeTransforms(s_gc.renderParams.SceneIndex, s_gc.planet.transforms[0], s_gc.planet.modelTransform);
        
    s_gc.planet.aabb = s_gc.planet.model->ComputeBoundingBox(s_gc.renderParams.SceneIndex, s_gc.planet.transforms[0], nullptr);
    s_gc.planet.transforms[1] = s_gc.planet.transforms[0];

    float YFov = PI_F / 4.0f;
    float ZNear = 0.1f;
    float ZFar = 100.f;

    float4x4 CameraView;

    const auto* pCameraNode = s_gc.planet.camera;
    const auto* pCamera = pCameraNode->pCamera;
    const auto& CameraGlobalTransform = s_gc.planet.transforms[inFlightIndex & 0x01].NodeGlobalMatrices[pCameraNode->Index];

    // GLTF camera is defined such that the local +X axis is to the right,
    // the lens looks towards the local -Z axis, and the top of the camera
    // is aligned with the local +Y axis.
    // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#cameras
    // We need to inverse the Z axis as our camera looks towards +Z.
    float4x4 InvZAxis = float4x4::Identity();
    InvZAxis._33 = -1;

#if 1
    CameraView = CameraGlobalTransform.Inverse() * InvZAxis;
    s_gc.renderParams.ModelTransform = RotationMatrixModel;
#else
    auto trans = float4x4::Translation(0.0f, -0.057f, 0.264f);
    auto cam = QuaternionF(0.0f, 0.2079117f, 0.9781476f, 0.0f);

    auto modelTransform = QuaternionF::RotationFromAxisAngle(float3{ -1.f, 0.0f, 0.0f }, -PI_F / 2.f).ToMatrix();

    //auto invModelTransform = modelTransform.Inverse();
    
    CameraView = RotationMatrixCam * cam.ToMatrix() * trans;
    s_gc.renderParams.ModelTransform = RotationMatrixModel * modelTransform; // QuaternionF::RotationFromAxisAngle(float3{ -1.f, 0.0f, 0.0f }, -PI_F / 2.f).ToMatrix();
    //s_gc.renderParams.ModelTransform = float4x4::Identity();
#endif

    YFov = pCamera->Perspective.YFov;
    ZNear = pCamera->Perspective.ZNear;
    ZFar = pCamera->Perspective.ZFar;

    // Apply pretransform matrix that rotates the scene according the surface orientation
    CameraView *= GetSurfacePretransformMatrix(float3{ 0, 0, 1 });

    // Rotate the camera up by 15 degrees around the y-axis
    //float angle = 45.0f * (M_PI / 180.0f); // Convert degrees to radians
    //float4x4 RotateX = float4x4::RotationX(angle);
    //CameraView = RotateX * CameraView;    

    float4x4 CameraWorld = CameraView.Inverse();

    // Get projection matrix adjusted to the current screen orientation
    const auto CameraProj = GetAdjustedProjectionMatrix(YFov, ZNear, ZFar);
    const auto CameraViewProj = CameraView * CameraProj;

    float3 CameraWorldPos = float3::MakeVector(CameraWorld[3]);

    auto& CurrCamAttribs = s_gc.cameraAttribs[inFlightIndex & 0x01];

    CurrCamAttribs.f4ViewportSize = float4{ static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT), 1.f / (float)WINDOW_WIDTH, 1.f / (float)WINDOW_HEIGHT };
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

    s_gc.cameraAttribs[(inFlightIndex + 1) & 0x01] = CurrCamAttribs;    
}

void RenderPlanet(VulkanContext::frame_id_t inFlightIndex)
{
    RenderSFModel(inFlightIndex, s_gc.planet);
}

float bicubicOffset = -0.5f;

void DoDemoKeys(SDL_Event event, VulkanContext::frame_id_t inFlightIndex)
{
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration<float>(now - frameSync.uiTriggerTimestamp).count();
    bool menuVisible = frameSync.uiTrigger.at(static_cast<Magnum::Float>(duration)) > 0.0f;

    float3 MoveDirection = float3(0, 0, 0);

    std::vector<const Diligent::SF_GLTF::Node*> cameras;

    for (const auto* node : s_gc.terrain.model->GetScenes()[0].LinearNodes)
    {
        if (node->pCamera != nullptr && node->pCamera->Type == SF_GLTF::Camera::Projection::Perspective)
        {
            cameras.push_back(node);
        }
    }

    int currentCameraIndex = -1;
    for (size_t i = 0; i < cameras.size(); ++i)
    {
        if (cameras[i] == s_gc.terrain.camera)
        {
            currentCameraIndex = static_cast<int>(i);
            break;
        }
    }

    const float rotationSpeed = 0.005f;
    const float handness = s_gc.cameraAttribs[inFlightIndex & 0x01].fHandness;

    const float3 referenceRightAxis = float3{ 1, 0, 0 };
    const float3 referenceUpAxis = float3{ 0, 1, 0 };
    static bool shiftDown = false;

    switch (event.type) {
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
                case SDLK_LSHIFT:
                    shiftDown = true;
                    break;
                case SDLK_w:
                    if (currentCameraIndex == 0)
                    {
                        MoveDirection.y += 1.0f;
                        //const float delta = 1.0f / 61.0f;
                        //s_gc.terrainTextureOffset.y -= delta;
                    }
                    else
                    {
                        MoveDirection.z -= 1.0f;
                    }
                    break;
                case SDLK_s:
                    if (currentCameraIndex == 0)
                    {
                         MoveDirection.y -= 1.0f;
                        //const float delta = 1.0f / 61.0f;
                        //s_gc.terrainTextureOffset.y += delta;
                    }
                    else
                    {
                        MoveDirection.z += 1.0f;
                    }
                    break;
                case SDLK_q:
                    if (currentCameraIndex == 0)
                    {
                        MoveDirection.z -= 1.0f;
                    }
                    break;
                case SDLK_e:
                    if (currentCameraIndex == 0)
                    {
                        MoveDirection.z += 1.0f;
                    }
                    break;
                case SDLK_a:
                    MoveDirection.x -= 1.0f;
                    {
                        //const float delta = 1.0f / 61.0f;
                        //s_gc.terrainTextureOffset.x -= delta;
                    }
                    break;
                case SDLK_d:
                    MoveDirection.x += 1.0f;
                    {
                        //const float delta = 1.0f / 61.0f;
                        //s_gc.terrainTextureOffset.x += delta;
                    }
                    break;
                case SDLK_c:
                    {
                        s_gc.terrainMovement = { 388.0f * GraphicsContext::TileSize, -40.0f, 245.0f * GraphicsContext::TileSize };
                        s_gc.terrainFPVRotation = float4x4::Identity();
                        s_gc.FPVyawAngle = 0.0f;
                        s_gc.FPVpitchAngle = 0.0f;
                        s_gc.mouseState = {};
                        currentCameraIndex = (currentCameraIndex + 1) % cameras.size();
                        s_gc.terrain.camera = cameras[currentCameraIndex];
                    }
                    break;
                case SDLK_m:
                    {
                        int64_t materialCount = s_gc.terrain.model->GetMaterials().size();
                        int64_t materialIndex = s_gc.terrainMaterialIndex;
                        materialIndex += shiftDown ? -1 : 1;
                        if (materialIndex < 0)
                        {
                            materialIndex += materialCount;
                        }
                        else if (materialIndex >= materialCount)
                        {
                            materialIndex -= materialCount;
                        }
                        s_gc.terrainMaterialIndex = static_cast<Uint32>(materialIndex);
                        for (const auto& node : s_gc.terrain.dynamicMesh->GetNodes())
                        {
                            if (node.pMesh)
                            {
                                for (auto& primitive : node.pMesh->Primitives)
                                {
                                    primitive.MaterialId = s_gc.terrainMaterialIndex;
                                }
                            }
                        }
                    }
                    break;
                case SDLK_t:
                    {
                        s_gc.tvDelta.y = -0.05f;
                        s_gc.tvNudge = Quaternion<float>::RotationFromAxisAngle(float3(0.0f, 1.0f, 0.0f), PI);
                    }
                    break;
                case SDLK_g:
                    {
                        s_gc.tvDelta.y = 0.05f;
                        s_gc.tvNudge = {};
                    }
                    break;
                case SDLK_f:
                    {
                        s_gc.tvDelta.x = -0.05f;
                        s_gc.tvNudge = Quaternion<float>::RotationFromAxisAngle(float3(0.0f, 1.0f, 0.0f), -PI_F / 2.0f);
                    }
                    break;
                case SDLK_h:
                    {
                        s_gc.tvDelta.x = 0.05f;
                        s_gc.tvNudge = Quaternion<float>::RotationFromAxisAngle(float3(0.0f, 1.0f, 0.0f), PI_F / 2.0f);
                    }
                    break;
                case SDLK_o:
                    {
                        bicubicOffset += 0.01f; 
                        char debugStr[256];
                        sprintf_s(debugStr, "Bicubic offset: %.3f\n", bicubicOffset);
                        OutputDebugStringA(debugStr);
                    }
                    break;
                case SDLK_p:
                    {
                        bicubicOffset -= 0.01f;
                        char debugStr[256];
                        sprintf_s(debugStr, "Bicubic offset: %.3f\n", bicubicOffset);
                        OutputDebugStringA(debugStr);
                    }
                    break;
                default:
                    break;
            }
            break;
        case SDL_KEYUP:
            {
                if (event.key.keysym.sym == SDLK_LSHIFT)
                {
                    shiftDown = false;
                }
                s_gc.terrainDelta = {};
                s_gc.tvDelta = {};
            }
            break;
        case SDL_MOUSEMOTION:
            {
                if (!menuVisible)
                {
                    GraphicsContext::MouseState mouseState{};
                    mouseState.pos.x = event.motion.x;
                    mouseState.pos.y = event.motion.y;
                    mouseState.leftDown = (event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
                    mouseState.rightDown = (event.motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;

                    float MouseDeltaX = 0;
                    float MouseDeltaY = 0;
                    if (s_gc.mouseState.pos.x >= 0 && s_gc.mouseState.pos.x >= 0 &&
                        s_gc.mouseState.leftDown)
                    {
                        MouseDeltaX = mouseState.pos.x - s_gc.mouseState.pos.x;
                        MouseDeltaY = mouseState.pos.y - s_gc.mouseState.pos.y;
                    }
                    s_gc.mouseState = mouseState;

                    float fYawDelta = -MouseDeltaX * rotationSpeed;
                    float fPitchDelta = -MouseDeltaY * rotationSpeed;
                    if (mouseState.leftDown)
                    {
                        s_gc.FPVyawAngle += fYawDelta * -handness;
                        s_gc.FPVpitchAngle += fPitchDelta * -handness;
                        s_gc.FPVpitchAngle = std::max(s_gc.FPVpitchAngle, -PI_F / 2.f);
                        s_gc.FPVpitchAngle = std::min(s_gc.FPVpitchAngle, +PI_F / 2.f);
                    }
                    else if (mouseState.rightDown)
                    {
                        s_gc.NormalizedXCoordForSunRotation = ((s_gc.mouseState.pos.x / WINDOW_WIDTH) - 0.5f) * 2.0f;
                    }

                    auto yaw = float4x4::RotationArbitrary(referenceUpAxis, s_gc.FPVyawAngle);
                    auto pitch = float4x4::RotationArbitrary(referenceRightAxis, s_gc.FPVpitchAngle);
                    
#if 0
                    auto camRotation =
                        QuaternionF::RotationFromAxisAngle(float3{1, 0, 0}, s_gc.FPVpitchAngle) *
                        QuaternionF::RotationFromAxisAngle(float3{0, 1, 0}, -s_gc.FPVyawAngle);
                     
                    auto CameraRotationMatrix = camRotation.ToMatrix();                    

                    s_gc.terrainFPVRotation = yaw * pitch;
                    s_gc.terrainFPVRotation = CameraRotationMatrix;
#endif
                }
            }
            break;
        default:
            break;
    }

    if (MoveDirection.x != 0.0f || MoveDirection.y != 0.0f || MoveDirection.z != 0.0f)
    {
        auto view = s_gc.cameraAttribs[inFlightIndex & 0x01].mView.Transpose();
        const auto& cam = view.Inverse();

        s_gc.terrainDelta = float3(
            MoveDirection.x * cam._11 + MoveDirection.y * cam._21 + MoveDirection.z * cam._31,
            MoveDirection.x * cam._12 + MoveDirection.y * cam._22 + MoveDirection.z * cam._32,
            MoveDirection.x * cam._13 + MoveDirection.y * cam._23 + MoveDirection.z * cam._33
        );
    }
   
}

void InitTerrain()
{
    InitModel("61x61plane.glb", s_gc.terrain, 0);

    s_gc.terrainSize = InitHeightmap();

    s_gc.terrain.dynamicMesh = std::make_unique<SF_GLTF::DynamicMesh>(s_gc.m_pDevice, s_gc.m_pImmediateContext, s_gc.terrain.model);
    s_gc.terrain.dynamicMesh->GeneratePlanes(4.0f, 4.0f, 0.0f, s_gc.terrainSize );

    s_gc.terrain.planetTypes = {
        { "Earth-like", { { "Water", -15.0f }, { "Beach", 2.01f }, { "Grass2", 3.01f }, { "HighGrass", 6.01f}, { "Rock", 8.51f }, {"Ice", 10.01f}}},
        { "Earth-dead", { { "Water", -15.0f }, { "Beach", 2.01f }, { "Barren", 3.01f }, { "HighBarren", 6.01f}, { "Rock", 8.51f }, {"Ice", 10.01f}}},
        { "Moon", { { "Moon", -15.0f } } },
        { "Sulfur", { { "Sulfur", -15.0f } } },
        { "Lava", { { "Lava", -15.0f } } },
    };

    for (const auto& planetType : s_gc.terrain.planetTypes)
    {
        for (const auto& biomBoundary : planetType.boundaries)
        {
            if(s_gc.terrain.biomMaterialIndex.find(biomBoundary.name) != s_gc.terrain.biomMaterialIndex.end())
                break;

            auto& materials = s_gc.terrain.model->GetMaterials();
            for (size_t i = 0; i < materials.size(); ++i) {
                if (materials[i].Name == biomBoundary.name) {
                    s_gc.terrain.biomMaterialIndex[biomBoundary.name] = static_cast<Uint32>(i);
                    break;
                }
            }

            assert(s_gc.terrain.biomMaterialIndex.find(biomBoundary.name) != s_gc.terrain.biomMaterialIndex.end());
        }
    }
}

void UpdateTerrain(VulkanContext::frame_id_t inFlightIndex)
{
    VulkanContext::frame_id_t frameCount = s_gc.vc.current_frame();

    double currentTimeInSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - s_gc.epoch).count();

    s_gc.terrainMovement += s_gc.terrainDelta/ 15.0f;
    s_gc.tvLocation += s_gc.tvDelta;

    // Smoothly interpolate tvRotation towards tvNudge using a low-pass filter
    const float rotationLerpFactor = 0.1f;
    s_gc.tvRotation = slerp(s_gc.tvRotation, s_gc.tvNudge, rotationLerpFactor);

    //s_gc.terrainTextureOffset.x = s_gc.terrainMovement.x / s_gc.terrainSize.x;
    //s_gc.terrainTextureOffset.y = s_gc.terrainMovement.z / s_gc.terrainSize.y;

    SF_GLTF::TerrainItem rover{ "Rover", s_gc.tvLocation, float2{ 0.0f, 0.0f }, s_gc.tvRotation, true };
    SF_GLTF::TerrainItem ruin{ "AncientRuin", float2{388.0f, 245.0f}, float2{ 0.0f, 0.0f }, Quaternion<float>{}, true };
    SF_GLTF::TerrainItem endurium{ "Endurium", float2{389.0f, 246.0f}, float2{ 0.0f, 0.0f }, Quaternion<float>{}, true };
    SF_GLTF::TerrainItem recentRuin{ "RecentRuin", float2{389.0f, 249.0f}, float2{ 0.0f, 0.0f }, Quaternion<float>{}, true };
    // tree-stylized-01
    SF_GLTF::TerrainItem tree{ "tree-stylized-01", float2{389.0f, 248.0f}, float2{ 0.0f, 0.0f }, Quaternion<float>{}, false };

#if 1
    // Debug balls
    SF_GLTF::TerrainItem ball{ "Ball", float2{389.0f, 248.0f}, float2{ 0.0f, 0.0f }, Quaternion<float>{}, true };
    SF_GLTF::TerrainItem ball1{ "Ball", float2{389.0f - 0.25f, 248.0f - 0.25f}, float2{ 0.0f, 0.0f }, Quaternion<float>{}, true };
    SF_GLTF::TerrainItem ball2{ "Ball", float2{389.0f + 0.25f, 248.0f - 0.25f}, float2{ 0.0f, 0.0f }, Quaternion<float>{}, true };
    SF_GLTF::TerrainItem ball3{ "Ball", float2{389.0f - 0.25f, 248.0f + 0.25f}, float2{ 0.0f, 0.0f }, Quaternion<float>{}, true };
    SF_GLTF::TerrainItem ball4{ "Ball", float2{389.0f + 0.25f, 248.0f + 0.25f}, float2{ 0.0f, 0.0f }, Quaternion<float>{}, true };
#endif

    std::vector<SF_GLTF::TerrainItem> terrainItems = { rover, ruin, endurium, recentRuin, tree, ball, ball1, ball2, ball3, ball4 };

    s_gc.terrain.dynamicMesh->ReplaceTerrain(s_gc.terrainMovement);

    SF_GLTF::TerrainData terrainData{ s_gc.heightmapData, s_gc.heightmapSize, {1.0f, 1.0f} };

    s_gc.terrain.dynamicMesh->SetTerrainItems(terrainItems, terrainData);

    float4x4 RotationMatrixCam = float4x4::Identity();
    float4x4 RotationMatrixModel = float4x4::Identity();

    s_gc.terrain.model->ComputeTransforms(s_gc.renderParams.SceneIndex, s_gc.terrain.transforms[0]);
    s_gc.terrain.dynamicMesh->ComputeTransforms(s_gc.renderParams.SceneIndex, s_gc.terrain.dynamicMeshTransforms[0]);
    s_gc.terrain.aabb = s_gc.terrain.dynamicMesh->ComputeBoundingBox(s_gc.renderParams.SceneIndex, s_gc.terrain.transforms[0], &s_gc.terrain.dynamicMeshTransforms[0]);
    s_gc.terrain.worldspaceAABB = s_gc.terrain.aabb;

    // Center and scale model
    float  MaxDim = 0;
    float3 ModelDim{ s_gc.terrain.aabb.Max - s_gc.terrain.aabb.Min };
    MaxDim = std::max(MaxDim, ModelDim.x);
    MaxDim = std::max(MaxDim, ModelDim.y);
    MaxDim = std::max(MaxDim, ModelDim.z);

    float4x4 InvYAxis = float4x4::Identity();
    //InvYAxis._22 = -1;

#if 0
    s_gc.terrain.scale = (1.0f / std::max(MaxDim, 0.01f)) * 0.5f;
    auto     Translate = -s_gc.terrain.aabb.Min - 0.5f * ModelDim;
    InvYAxis._22 = -1;
#else
    s_gc.terrain.scale = 1.0f;
    float3 Translate = { 0.f, 0.f, 0.f };
#endif

    s_gc.terrain.modelTransform = float4x4::Translation(Translate) * float4x4::Scale(s_gc.terrain.scale) * InvYAxis;
    s_gc.terrain.scaleAndTransform = float4x4::Translation(Translate) * float4x4::Scale(s_gc.terrain.scale);

    s_gc.terrain.model->ComputeTransforms(s_gc.renderParams.SceneIndex, s_gc.terrain.transforms[0], s_gc.terrain.modelTransform);
    s_gc.terrain.dynamicMesh->ComputeTransforms(s_gc.renderParams.SceneIndex, s_gc.terrain.dynamicMeshTransforms[0], s_gc.terrain.modelTransform);

    s_gc.terrain.aabb = s_gc.terrain.dynamicMesh->ComputeBoundingBox(s_gc.renderParams.SceneIndex, s_gc.terrain.transforms[0], &s_gc.terrain.dynamicMeshTransforms[0]);
    s_gc.terrain.transforms[1] = s_gc.terrain.transforms[0];
    s_gc.terrain.dynamicMeshTransforms[1] = s_gc.terrain.dynamicMeshTransforms[0];

    float YFov = PI_F / 4.0f;
    float ZNear = 0.1f;
    float ZFar = 100.f;

    float4x4 CameraView;

    const auto* pCameraNode = s_gc.terrain.camera;
    const auto* pCamera = pCameraNode->pCamera;
    const auto& CameraGlobalTransform = s_gc.terrain.transforms[inFlightIndex & 0x01].NodeGlobalMatrices[pCameraNode->Index];

    auto camRotation =
        QuaternionF::RotationFromAxisAngle(float3{ 1, 0, 0 }, -s_gc.FPVpitchAngle) *
        QuaternionF::RotationFromAxisAngle(float3{ 0, 1, 0 }, -s_gc.FPVyawAngle);

    auto CameraRotationMatrix = camRotation.ToMatrix();

    // GLTF camera is defined such that the local +X axis is to the right,
    // the lens looks towards the local -Z axis, and the top of the camera
    // is aligned with the local +Y axis.
    // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#cameras
    // We need to inverse the Z axis as our camera looks towards +Z.
    float4x4 InvZAxis = float4x4::Identity();
    InvZAxis._33 = -1;

    auto trans = float4x4::Translation(-s_gc.terrainMovement.x, s_gc.terrainMovement.y, -s_gc.terrainMovement.z);
    //CameraView = trans * s_gc.terrainFPVRotation * CameraGlobalTransform.Inverse() * InvZAxis;
    CameraView = trans * CameraRotationMatrix * InvZAxis;
    //CameraView = trans * CameraRotationMatrix;
    s_gc.renderParams.ModelTransform = RotationMatrixModel;

    YFov = pCamera->Perspective.YFov;
    ZNear = pCamera->Perspective.ZNear;
    ZFar = pCamera->Perspective.ZFar;
    ZNear = 1.0f;
    ZFar = 400.0f;
    //ZFar = 10000.0f;

    // Apply pretransform matrix that rotates the scene according the surface orientation
    CameraView *= GetSurfacePretransformMatrix(float3{ 0, 0, 1 });

    // Rotate the camera up by 15 degrees around the y-axis
    //float angle = 45.0f * (M_PI / 180.0f); // Convert degrees to radians
    //float4x4 RotateX = float4x4::RotationX(angle);
    //CameraView = RotateX * CameraView;    

    float4x4 CameraWorld = CameraView.Inverse();

    // Get projection matrix adjusted to the current screen orientation
    const auto CameraProj = GetAdjustedProjectionMatrix(YFov, ZNear, ZFar);
    const auto CameraViewProj = CameraView * CameraProj;

    float3 CameraWorldPos = float3::MakeVector(CameraWorld[3]);

    auto& CurrCamAttribs = s_gc.cameraAttribs[inFlightIndex & 0x01];

    CurrCamAttribs.f4ViewportSize = float4{ static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT), 1.f / (float)WINDOW_WIDTH, 1.f / (float)WINDOW_HEIGHT };
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
    CurrCamAttribs.f4ExtraData[0].x = pCamera->Perspective.YFov;

    s_gc.cameraAttribs[(inFlightIndex + 1) & 0x01] = CurrCamAttribs;
}

void RenderWaterHeightMap(VulkanContext::frame_id_t inFlightIndex)
{
    // Initialize compute shader and bindings if not already done
    if (!s_gc.waterComputeShader.pWaterComputePSO)
    {
        const char* csSource = R"(
        // Wave parameters
        static const float PI = 3.141592;
        static const float SEA_HEIGHT = 0.3;
        static const float SEA_CHOPPY = 4.0;
        static const float SEA_SPEED = 0.8;
        static const float SEA_FREQ = 0.16;
        static const int ITER_FRAGMENT = 5;

        // Time input (should be provided as a constant buffer)
        cbuffer TimeConstants : register(b0)
        {
            float Time;
        }

        // Noise texture
        Texture2D NoiseTexture : register(t0);
        SamplerState NoiseSampler : register(s0);

        // Output texture
        RWTexture2D<float> OutputHeightMap : register(u0);

        // Helper matrix for octaves
        static const float2x2 octave_m = float2x2(1.6, 1.2, -1.2, 1.6);

        #define SEA_TIME (1.0 + Time * SEA_SPEED)

        float noise(float2 p)
        {
            return NoiseTexture.SampleLevel(NoiseSampler, p * 0.01, 0).r * 2.0 - 1.0;
        }

        float sea_octave(float2 uv, float choppy)
        {
            uv += noise(uv);
            float2 wv = 1.0 - abs(sin(uv));
            float2 swv = abs(cos(uv));
            wv = lerp(wv, swv, wv);
            return pow(1.0 - pow(wv.x * wv.y, 0.65), choppy);
        }

        float get_height(float2 p)
        {
            float freq = SEA_FREQ;
            float amp = SEA_HEIGHT;
            float choppy = SEA_CHOPPY;
            float2 uv = p;
            
            float d, h = 0.0;
            
            [unroll]
            for (int i = 0; i < ITER_FRAGMENT; i++)
            {
                d = sea_octave((uv + SEA_TIME) * freq, choppy);
                d += sea_octave((uv - SEA_TIME) * freq, choppy);
                h += d * amp;
                
                uv = mul(uv, octave_m);
                freq *= 1.9;
                amp *= 0.22;
                choppy = lerp(choppy, 1.0, 0.2);
            }
            
            return h;
        }

        [numthreads(8, 8, 1)]
        void CSMain(uint3 DTid : SV_DispatchThreadID)
        {
            // Get dimensions of the output texture
            uint width, height;
            OutputHeightMap.GetDimensions(width, height);
            
            // Early exit if we're outside the texture bounds
            if (DTid.x >= width || DTid.y >= height)
                return;
            
            // Convert pixel coordinates to UV space
            float2 origUV = float2(DTid.xy) / float2(width, height);
            float2 uv = origUV * 2.0 - 1.0;
            
            uv *= 8.0; // Scale factor for wave size (adjust as needed)
            
            float wave_height = get_height(uv);

#if 0
            // Create a border region with height 1.0
            if (origUV.x < 0.025f || origUV.x > 0.975f || 
                origUV.y < 0.025f || origUV.y > 0.975f)
            {
                wave_height = 1.0f;
            }
#endif

            wave_height = wave_height * 0.5 + 0.5;

            OutputHeightMap[DTid.xy] = wave_height;
        }
        )";

        ShaderCreateInfo ShaderCI;
        ShaderCI.Source = csSource;
        ShaderCI.EntryPoint = "CSMain";
        ShaderCI.Desc.ShaderType = SHADER_TYPE_COMPUTE;
        ShaderCI.Desc.Name = "Water Height Map Generator";
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

        RefCntAutoPtr<IShader> pShader;
        s_gc.m_pDevice->CreateShader(ShaderCI, &pShader);
        VERIFY_EXPR(pShader);
        PipelineResourceSignatureDescX SignatureDesc{ "Water Height Map Generator PSO" };
        SignatureDesc
            .AddResource(SHADER_TYPE_COMPUTE, "TimeConstants", SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE)
            .AddResource(SHADER_TYPE_COMPUTE, "NoiseTexture", SHADER_RESOURCE_TYPE_TEXTURE_SRV, SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE)
            .AddResource(SHADER_TYPE_COMPUTE, "OutputHeightMap", SHADER_RESOURCE_TYPE_TEXTURE_UAV, SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC);

        SignatureDesc.AddImmutableSampler(SHADER_TYPE_COMPUTE, "NoiseSampler", Sam_LinearWrap);

        RefCntAutoPtr<IPipelineResourceSignature> pResSig;
        s_gc.m_pDevice->CreatePipelineResourceSignature(SignatureDesc, &pResSig);
        VERIFY_EXPR(pResSig);

        ComputePipelineStateCreateInfoX CPSOCreateInfo{ "Water Height Map Generator PSO" };
        CPSOCreateInfo.AddShader(pShader);
        CPSOCreateInfo.AddSignature(pResSig);

        s_gc.m_pDevice->CreatePipelineState(CPSOCreateInfo, &s_gc.waterComputeShader.pWaterComputePSO);
        VERIFY_EXPR(s_gc.waterComputeShader.pWaterComputePSO);

        pResSig->CreateShaderResourceBinding(&s_gc.waterComputeShader.pWaterComputeSRB, true);
        VERIFY_EXPR(s_gc.waterComputeShader.pWaterComputeSRB);

        // Create time constant buffer
        BufferDesc CBDesc;
        CBDesc.Name = "Time Constants CB";
        CBDesc.Size = sizeof(float);
        CBDesc.Usage = USAGE_DYNAMIC;
        CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
        CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;

        s_gc.m_pDevice->CreateBuffer(CBDesc, nullptr, &s_gc.waterComputeShader.pTimeConstantBuffer);
        VERIFY_EXPR(s_gc.waterComputeShader.pTimeConstantBuffer);
    }

    // Set the output heightmap texture for the current in-flight frame
    s_gc.waterComputeShader.pWaterComputeSRB->GetVariableByName(SHADER_TYPE_COMPUTE, "OutputHeightMap")->Set(s_gc.buffers[inFlightIndex].waterHeightMap->GetDefaultView(TEXTURE_VIEW_UNORDERED_ACCESS));

    // Set the noise texture
    s_gc.waterComputeShader.pWaterComputeSRB->GetVariableByName(SHADER_TYPE_COMPUTE, "NoiseTexture")->Set(s_gc.diligentFourDeeNoise);

    // Update time value
    {
        MapHelper<float> TimeData(s_gc.m_pImmediateContext, s_gc.waterComputeShader.pTimeConstantBuffer, MAP_WRITE, MAP_FLAG_DISCARD);
        double currentTimeInSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - s_gc.epoch).count();
        *TimeData = static_cast<float>(currentTimeInSeconds);
    }

    // Set the constant buffer
    s_gc.waterComputeShader.pWaterComputeSRB->GetVariableByName(SHADER_TYPE_COMPUTE, "TimeConstants")->Set(s_gc.waterComputeShader.pTimeConstantBuffer);

    // Calculate dispatch dimensions based on heightmap size
    DispatchComputeAttribs DispatchAttrs;
    DispatchAttrs.ThreadGroupCountX = (GraphicsContext::BufferData::waterHeightMapSize + 7) / 8;
    DispatchAttrs.ThreadGroupCountY = (GraphicsContext::BufferData::waterHeightMapSize + 7) / 8;
    DispatchAttrs.ThreadGroupCountZ = 1;

    s_gc.m_pImmediateContext->SetPipelineState(s_gc.waterComputeShader.pWaterComputePSO);
    s_gc.m_pImmediateContext->CommitShaderResources(s_gc.waterComputeShader.pWaterComputeSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    s_gc.m_pImmediateContext->DispatchCompute(DispatchAttrs);
}

void RenderTerrain(VulkanContext::frame_id_t inFlightIndex)
{
    if(s_gc.currentScene != SCENE_TERRAIN)
    {
        s_gc.shadowMap->InitializeResourceBindings(s_gc.terrain.model);
        s_gc.currentScene = SCENE_TERRAIN;
    }

    auto sunBehavior = [](const float4x4& lightGlobalTransform, double currentTimeInSeconds) {

        // 1,0,0, points towards the horizon
        float3 lightDir = float3{ 1.0f, 0.1f, 0.0f };

        // Calculate the angle of the sun based on the current time
        //float angle = static_cast<float>(fmod(currentTimeInSeconds, 60.0) / 60.0 * M_PI); // Full rotation in a minute

        // Calculate the angle of the sun based on the NormalizedXCoordForSunRotation
        float angle = (s_gc.NormalizedXCoordForSunRotation + 1.0f) * (M_PI / 2.0f);

        //if(angle > M_PI)
        //{
        //    angle = 1.5f * M_PI;
        //}

        //char buffer[50];
        //sprintf(buffer, "angle: %f\n", angle);
        //OutputDebugString(buffer);
       
        // Set this to high noon given that lightDir starts straight east
        //float angle = static_cast<float>(M_PI / 2.0); // High noon, 90 degrees

        // Rotate the light direction around the y-axis to simulate rising from east to west
        float4x4 rotationMatrix = float4x4::RotationZ(angle);
        lightDir = rotationMatrix * float4(lightDir, 0.0f);

        float3 Direction = normalize(lightDir);

        //float3 Direction = float3(-0.554699242f, -0.0599640049f, -0.829887390f);
        //float3 Direction = float3(-0.5f, -0.5f, 0.0f);

        return Direction;
    };

    RenderWaterHeightMap(inFlightIndex);

    RenderSFModel(inFlightIndex, s_gc.terrain, sunBehavior);
}


std::vector<avk::recorded_commands_t> DemoPass(VulkanContext::frame_id_t inFlightIndex, UniformBlock& uniform)
{
    if (frameSync.demoMode == 1)
    {
        UpdatePlanet(inFlightIndex);
        RenderPlanet(inFlightIndex);
    }
    else if (frameSync.demoMode == 2)
    {
        UpdateTerrain(inFlightIndex);
        RenderTerrain(inFlightIndex);
    }

    s_gc.m_pImmediateContext->Flush();

    return {

        s_gc.buffers[inFlightIndex].uniform->fill(&uniform, 0, 0, sizeof(UniformBlock)),

        avk::sync::buffer_memory_barrier(s_gc.buffers[inFlightIndex].uniform.as_reference(),
            avk::stage::auto_stage >> avk::stage::auto_stage,
            avk::access::auto_access >> avk::access::auto_access
        ),

        avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].gameOutput->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::transfer_dst }),


        avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].avkdColorBuffer->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::color_attachment_optimal, avk::layout::transfer_src }),

        avk::copy_image_to_another(s_gc.buffers[inFlightIndex].avkdColorBuffer->get_image(), avk::layout::transfer_src, s_gc.buffers[inFlightIndex].gameOutput->get_image(), avk::layout::transfer_dst, vk::ImageAspectFlagBits::eColor),

        avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].avkdColorBuffer->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::transfer_src, avk::layout::color_attachment_optimal }),

        avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].gameOutput->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::transfer_dst, avk::layout::shader_read_only_optimal }),

    };
}

std::vector<avk::recorded_commands_t> TitlePass(VulkanContext::frame_id_t inFlightIndex, UniformBlock& uniform)
{
    return {
        avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].gameOutput->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::general }),

        s_gc.buffers[inFlightIndex].uniform->fill(&uniform, 0, 0, sizeof(UniformBlock)),

        avk::sync::buffer_memory_barrier(s_gc.buffers[inFlightIndex].uniform.as_reference(),
            avk::stage::auto_stage >> avk::stage::auto_stage,
            avk::access::auto_access >> avk::access::auto_access
        ),

        avk::command::bind_pipeline(s_gc.titlePipeline.as_reference()),
        avk::command::bind_descriptors(s_gc.titlePipeline->layout(), s_gc.descriptorCache->get_or_create_descriptor_sets({
                avk::descriptor_binding(0, 0, s_gc.buffers[inFlightIndex].gameOutput->get_image_view()->as_storage_image(avk::layout::general)),
                avk::descriptor_binding(0, 1, s_gc.buffers[inFlightIndex].uniform->as_uniform_buffer()),
                avk::descriptor_binding(0, 2, s_gc.fourDeeNoise->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 3, s_gc.boxArtImage->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
            })),
        avk::command::dispatch((WINDOW_WIDTH + 15u) / 16u, (WINDOW_HEIGHT + 15u) / 16u, 1),

        avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].gameOutput->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::general, avk::layout::shader_read_only_optimal })
    };
}

std::vector<avk::recorded_commands_t> CPUCopyPass(VulkanContext::frame_id_t inFlightIndex, const void* data, uint64_t dataSize)
{
    return {
        s_gc.buffers[inFlightIndex].frameStaging->fill(data, 0, 0, dataSize),

        avk::sync::buffer_memory_barrier(s_gc.buffers[inFlightIndex].frameStaging.as_reference(),
            avk::stage::auto_stage >> avk::stage::auto_stage,
            avk::access::auto_access >> avk::access::auto_access
            ),

        avk::sync::image_memory_barrier(s_gc.vc.current_backbuffer()->image_at(0),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({avk::layout::undefined, avk::layout::transfer_dst}),

        avk::copy_buffer_to_image(s_gc.buffers[inFlightIndex].frameStaging, s_gc.vc.current_backbuffer()->image_at(0), avk::layout::transfer_dst),

        avk::sync::image_memory_barrier(s_gc.vc.current_backbuffer()->image_at(0),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({avk::layout::transfer_dst, avk::layout::present_src})
    };
}

std::vector<avk::recorded_commands_t> GPUCompositor(VulkanContext::frame_id_t inFlightIndex,
    UniformBlock& uniform,
    const void* data,
    uint64_t dataSize)
{
    std::vector<avk::recorded_commands_t> res{};

    res.push_back(
        avk::sync::image_memory_barrier(s_gc.vc.current_backbuffer()->image_at(0),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::general }));

    res.push_back(
        avk::sync::buffer_memory_barrier(s_gc.buffers[inFlightIndex].frameStaging.as_reference(),
            avk::stage::auto_stage >> avk::stage::auto_stage,
            avk::access::auto_access >> avk::access::auto_access
        ));

    if (uniform.menuVisibility > 0.0f)
    {
        s_gc.buffers[inFlightIndex].frameStaging->fill(data, 0, 0, dataSize);

        res.push_back(
            avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].ui->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::transfer_dst }));

        res.push_back(
            avk::copy_buffer_to_image(s_gc.buffers[inFlightIndex].frameStaging, s_gc.buffers[inFlightIndex].ui->get_image(), avk::layout::transfer_dst));

        res.push_back(
            avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].ui->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::transfer_dst, avk::layout::shader_read_only_optimal }));
    }
    else
    {
        res.push_back(
            avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].ui->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::shader_read_only_optimal }));
    }

    res.push_back(
        avk::command::bind_pipeline(s_gc.compositorPipeline.as_reference()));

    res.push_back(
        avk::command::bind_descriptors(s_gc.compositorPipeline->layout(), s_gc.descriptorCache->get_or_create_descriptor_sets({
                avk::descriptor_binding(0, 0, s_gc.vc.current_backbuffer_reference().image_view_at(0)->as_storage_image(avk::layout::general)),
                avk::descriptor_binding(0, 1, s_gc.buffers[inFlightIndex].gameOutput->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 2, s_gc.buffers[inFlightIndex].ui->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 3, s_gc.buffers[inFlightIndex].uniform->as_uniform_buffer())
            })));

    res.push_back(
        avk::command::dispatch((WINDOW_WIDTH + 31) / 32, (WINDOW_HEIGHT + 31) / 32, 1));

    res.push_back(
        avk::sync::image_memory_barrier(s_gc.vc.current_backbuffer()->image_at(0),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::general, avk::layout::present_src }));

    return res;
}

std::vector<avk::recorded_commands_t> GPURotoscope(VulkanContext::frame_id_t inFlightIndex, 
    avk::image_sampler outputImage,
    UniformBlock& uniform, 
    IconUniform<64>& iconUniform,
    IconUniform<64>& orreryUniform,
    IconUniform<700>& starmapUniform,
    const std::vector<RotoscopeShader>& shaderBackBuffer, 
    avk::compute_pipeline navPipeline,
    avk::compute_pipeline orreryPipeline,
    avk::compute_pipeline starmapPipeline)
{
    std::vector<avk::recorded_commands_t> res{};

    res.push_back(
        avk::sync::image_memory_barrier(outputImage->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::general }));

    res.push_back(
        s_gc.buffers[inFlightIndex].rotoscope->fill(shaderBackBuffer.data(), 0, 0, shaderBackBuffer.size() * sizeof(RotoscopeShader)));

    res.push_back(
        avk::sync::buffer_memory_barrier(s_gc.buffers[inFlightIndex].rotoscope.as_reference(),
            avk::stage::auto_stage >> avk::stage::auto_stage,
            avk::access::auto_access >> avk::access::auto_access
        ));

    res.push_back(
        s_gc.buffers[inFlightIndex].uniform->fill(&uniform, 0, 0, sizeof(UniformBlock)));

    res.push_back(
        avk::sync::buffer_memory_barrier(s_gc.buffers[inFlightIndex].rotoscope.as_reference(),
            avk::stage::auto_stage >> avk::stage::auto_stage,
            avk::access::auto_access >> avk::access::auto_access
        ));

    res.push_back(
        s_gc.buffers[inFlightIndex].iconUniform->fill(&iconUniform, 0, 0, sizeof(IconUniform<64>)));

    res.push_back(
        s_gc.buffers[inFlightIndex].orreryUniform->fill(&orreryUniform, 0, 0, sizeof(IconUniform<64>)));        

    res.push_back(
        s_gc.buffers[inFlightIndex].starmapUniform->fill(&starmapUniform, 0, 0, sizeof(IconUniform<700>)));     

    if (starmapPipeline.has_value())
    {
        res.push_back(
            avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].starmap->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::general }));

        res.push_back(
            avk::command::bind_pipeline(starmapPipeline.as_reference()));

        res.push_back(
            avk::command::bind_descriptors(starmapPipeline->layout(), s_gc.descriptorCache->get_or_create_descriptor_sets({
                    avk::descriptor_binding(0, 0, s_gc.buffers[inFlightIndex].starmap->get_image_view()->as_storage_image(avk::layout::general)),
                    avk::descriptor_binding(0, 1, s_gc.buffers[inFlightIndex].rotoscope->as_storage_buffer()),
                    avk::descriptor_binding(0, 2, s_gc.shipImage->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                    avk::descriptor_binding(0, 3, s_gc.planetAlbedoImages->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                    avk::descriptor_binding(0, 4, s_gc.buffers[inFlightIndex].uniform->as_uniform_buffer()),
                    avk::descriptor_binding(0, 5, s_gc.buffers[inFlightIndex].starmapUniform->as_uniform_buffer())
                })));

        res.push_back(
            avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].starmap->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::general }));

        res.push_back(
            avk::command::dispatch((1112 + 31) / 32u, (664 + 31) / 32u, 1));

        res.push_back(
            avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].starmap->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::general, avk::layout::shader_read_only_optimal }));
    }
    else
    {
        res.push_back(
            avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].starmap->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::shader_read_only_optimal }));
    }

    if (orreryPipeline.has_value())
    {
        res.push_back(
            avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].orrery->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::general }));

        res.push_back(
            avk::command::bind_pipeline(orreryPipeline.as_reference()));

        res.push_back(
            avk::command::bind_descriptors(orreryPipeline->layout(), s_gc.descriptorCache->get_or_create_descriptor_sets({
                    avk::descriptor_binding(0, 0, s_gc.buffers[inFlightIndex].orrery->get_image_view()->as_storage_image(avk::layout::general)),
                    avk::descriptor_binding(0, 1, s_gc.buffers[inFlightIndex].rotoscope->as_storage_buffer()),
                    avk::descriptor_binding(0, 2, s_gc.shipImage->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                    avk::descriptor_binding(0, 3, s_gc.planetAlbedoImages->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                    avk::descriptor_binding(0, 4, s_gc.buffers[inFlightIndex].uniform->as_uniform_buffer()),
                    avk::descriptor_binding(0, 5, s_gc.buffers[inFlightIndex].orreryUniform->as_uniform_buffer())
                })));

        res.push_back(
            avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].orrery->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::general }));

        res.push_back(
            avk::command::dispatch((920 + 3) / 4u, (718 + 3) / 4u, 1));

        res.push_back(
            avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].orrery->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::general, avk::layout::shader_read_only_optimal }));
    }
    else
    {
        res.push_back(
            avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].orrery->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::shader_read_only_optimal }));
    }

    if(navPipeline.has_value())
    {
        res.push_back(
            avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].navigation->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::general }));

        res.push_back(
            avk::command::bind_pipeline(navPipeline.as_reference()));

        res.push_back(
            avk::command::bind_descriptors(navPipeline->layout(), s_gc.descriptorCache->get_or_create_descriptor_sets({
                    avk::descriptor_binding(0, 0, s_gc.buffers[inFlightIndex].navigation->get_image_view()->as_storage_image(avk::layout::general)),
                    avk::descriptor_binding(0, 1, s_gc.buffers[inFlightIndex].rotoscope->as_storage_buffer()),
                    avk::descriptor_binding(0, 2, s_gc.shipImage->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                    avk::descriptor_binding(0, 3, s_gc.planetAlbedoImages->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                    avk::descriptor_binding(0, 4, s_gc.buffers[inFlightIndex].uniform->as_uniform_buffer()),
                    avk::descriptor_binding(0, 5, s_gc.buffers[inFlightIndex].iconUniform->as_uniform_buffer()),
                    avk::descriptor_binding(0, 6, s_gc.fourDeeNoise->as_combined_image_sampler(avk::layout::shader_read_only_optimal))
                })));

        res.push_back(
            avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].navigation->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::general }));     

        uint32_t navWidth = (uint32_t)ceilf((float(NagivationWindowWidth) / 160.0f) * (float)WINDOW_WIDTH);
        uint32_t navHeight = (uint32_t)ceilf((float(NagivationWindowHeight) / 200.0f) * (float)WINDOW_HEIGHT);                

        res.push_back(
            avk::command::dispatch((WINDOW_WIDTH + 3) / 4u, (WINDOW_HEIGHT + 3) / 4u, 1));

        res.push_back(
            avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].navigation->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::general, avk::layout::shader_read_only_optimal }));                        
    }
    else
    {
        res.push_back(
            avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].navigation->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::shader_read_only_optimal }));
    }

    res.push_back(
        avk::command::bind_pipeline(s_gc.rotoscopePipeline.as_reference()));

    res.push_back(
        avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].avkdColorBuffer->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::color_attachment_optimal, avk::layout::shader_read_only_optimal }));

    res.push_back(
        avk::command::bind_descriptors(s_gc.rotoscopePipeline->layout(), s_gc.descriptorCache->get_or_create_descriptor_sets({
                avk::descriptor_binding(0, 0, outputImage->get_image_view()->as_storage_image(avk::layout::general)),
                avk::descriptor_binding(0, 1, s_gc.buffers[inFlightIndex].rotoscope->as_storage_buffer()),
                avk::descriptor_binding(0, 2, s_gc.FONT1->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 3, s_gc.FONT2->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 4, s_gc.FONT3->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 5, s_gc.LOGO1->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 6, s_gc.LOGO2->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                //avk::descriptor_binding(0, 7, s_gc.PORTPIC->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 7, s_gc.buffers[inFlightIndex].avkdColorBuffer->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 8, s_gc.RACEDOSATLAS->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 9, s_gc.shipImage->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 10, s_gc.planetAlbedoImages->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 11, s_gc.alienColorImage->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 12, s_gc.alienDepthImage->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 13, s_gc.alienBackgroundImage->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 14, s_gc.buffers[inFlightIndex].uniform->as_uniform_buffer()),
                avk::descriptor_binding(0, 15, s_gc.buffers[inFlightIndex].iconUniform->as_uniform_buffer()),
                avk::descriptor_binding(0, 16, s_gc.buffers[inFlightIndex].navigation->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 17, s_gc.buffers[inFlightIndex].orrery->as_combined_image_sampler(avk::layout::shader_read_only_optimal)),
                avk::descriptor_binding(0, 18, s_gc.buffers[inFlightIndex].starmap->as_combined_image_sampler(avk::layout::shader_read_only_optimal))
            })));

    res.push_back(
        avk::command::dispatch((WINDOW_WIDTH + 3) / 4u, (WINDOW_HEIGHT + 3) / 4u, 1));

    res.push_back(
        avk::sync::image_memory_barrier(s_gc.buffers[inFlightIndex].avkdColorBuffer->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::shader_read_only_optimal, avk::layout::color_attachment_optimal }));

    res.push_back(
        avk::sync::image_memory_barrier(outputImage->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::general, avk::layout::shader_read_only_optimal }));                        


    return res;
}

template<size_t N>
uint32_t IconUniform<N>::IndexFromSeed(uint32_t seed)
{
    if(seed == 0)
    {
        return 0;
    }

    assert(s_gc.surfaceData.size());

    auto it = s_gc.seedToIndex.find(seed);
    if (it == s_gc.seedToIndex.end())
    {
        return 0;
    }

    return it->second;
}

#include <filesystem>
#include <vector>
#include <string>

std::vector<std::filesystem::path> GetAllSaveGameFiles() {
    std::vector<std::filesystem::path> sfsFiles;
    for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path())) {
        if (entry.path().extension() == ".sfs") {
            sfsFiles.push_back(entry.path());
        }
    }
    return sfsFiles;
}

std::vector<uint8_t> ExtractPngFromSaveFile(const std::filesystem::path& saveFilePath) {
    std::ifstream saveFile(saveFilePath, std::ios::binary);
    if (!saveFile.is_open()) {
        throw std::runtime_error("Failed to open save file.");
    }

    ArchiveHeader archiveHeader;
    saveFile.read(reinterpret_cast<char*>(&archiveHeader), sizeof(ArchiveHeader));
    if (saveFile.gcount() != sizeof(ArchiveHeader)) {
        throw std::runtime_error("Failed to read archive header.");
    }

    // Check if the fourCC matches expected value for a valid save file
    if (strncmp(archiveHeader.fourCC, "SF1 ", 4) != 0) {
        throw std::runtime_error("Invalid save file format.");
    }

    // Seek to the screenshot section
    saveFile.seekg(archiveHeader.screenshotHeader.offset, std::ios::beg);
    if (!saveFile.good()) {
        throw std::runtime_error("Failed to seek to screenshot section.");
    }

    // Read the compressed screenshot data
    std::vector<uint8_t> compressedScreenshot(archiveHeader.screenshotHeader.compressedSize);
    saveFile.read(reinterpret_cast<char*>(compressedScreenshot.data()), archiveHeader.screenshotHeader.compressedSize);
    if (saveFile.gcount() != static_cast<std::streamsize>(archiveHeader.screenshotHeader.compressedSize)) {
        throw std::runtime_error("Failed to read compressed screenshot data.");
    }
    return compressedScreenshot;
}

void DrawUI()
{
    struct SaveGame {
        std::string hash;
        std::string timestamp;
        std::filesystem::path file;
        struct nk_image screenshot;
    };

    static int32_t screenshotIndex = 0;

    static std::vector<SaveGame> saveGames;
    static bool savegamesSearched = false;

    static bool loadWindowOpen = false; 
    static int loadWindowIndex = -1;

    if(!savegamesSearched) {
        auto saveGameFiles = GetAllSaveGameFiles();
        uint32_t i = 0;
        for (const auto& file : saveGameFiles) {
            SaveGame saveGame;
            saveGame.file = file;

            auto data = ExtractPngFromSaveFile(file);
            saveGame.screenshot = nk_sdlsurface_imgfrompng((const char*)data.data(), data.size());

            std::filesystem::file_time_type ftime = std::filesystem::last_write_time(file);
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
            std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&cftime), "%Y-%m-%d %H:%M:%S");
            saveGame.timestamp = ss.str();
            saveGames.push_back(saveGame);

            ++i;
        }

        std::sort(saveGames.begin(), saveGames.end(), [](const SaveGame& a, const SaveGame& b) {
            return a.timestamp > b.timestamp;
        });

        savegamesSearched = true;
    }

    struct nk_color clear = { 0,0,0,0 };

    auto& ctx = nk_context->ctx;

    float panelWidth = WINDOW_WIDTH * 0.33f;
    float panelHeight = WINDOW_HEIGHT; // Full height
    float panelX = 0; // Starting from the left edge
    float panelY = 0; // Starting from the top

    struct nk_style *s = &ctx.style;
    nk_style_push_color(&ctx, &s->window.background, nk_rgba(64,64,64,230));
    nk_style_push_style_item(&ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(64,64,64,230)));

    if(saveGames.empty() && !emulationThreadRunning && !s_helpShown) {
        s_showHelp = true;
        s_helpShown = true;
    }

    if(s_showHelp)
    {
        float windowWidth = 600.0f;
        float windowHeight = 400.0f; // Adjusted window height to accommodate all elements including the button at the bottom.
        float windowX = (WINDOW_WIDTH - windowWidth) / 2.0f;
        float windowY = (WINDOW_HEIGHT - windowHeight) / 2.0f;

        if (nk_begin(&ctx, "Welcome", nk_rect(windowX, windowY, windowWidth, windowHeight),
            NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE)) {

            nk_layout_row_dynamic(&ctx, 20, 1);
            nk_label(&ctx, "Welcome to the Unauthorized Remake of Starflight", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(&ctx, 15, 1);
            nk_label(&ctx, "Key Bindings:", NK_TEXT_LEFT);
            nk_label(&ctx, "F1 - Display the overlay", NK_TEXT_LEFT);
            nk_label(&ctx, "F2 - Switch in and out of rotoscoped (remade) mode.", NK_TEXT_LEFT);
            nk_label(&ctx, "F3 - Switch between EGA (16 color) and CGA (4 color) modes.", NK_TEXT_LEFT);

            nk_layout_row_dynamic(&ctx, 30, 1);
            nk_label_wrap(&ctx, "Save games are saved as they were in the original game, via pressing ESC while and saving the game. This will end that individual session and the game can then be restarted. This remake allows for an unlimited number of managed saves. They are now managed through the remake software accessible via this overlay.");

            nk_layout_row_dynamic(&ctx, 30, 1); // This row is for spacing purposes, ensuring the button is at the very bottom.

            nk_layout_row_dynamic(&ctx, 30, 1); // Adjusted to ensure the Start Game button is at the very bottom.
            if (nk_button_label(&ctx, "Start Game")) {
                StartEmulationThread("");
                s_showHelp = false;
                s_shouldToggleMenu = true;
            }
        }
        else
        {
            s_showHelp = false;
        }
        nk_end(&ctx);
    }
    else
    {
        if (nk_begin(&ctx, "Starflight - Reimaged Panel", nk_rect(panelX, panelY, panelWidth, panelHeight),
            NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND)) {
            
            // Adjust layout spacing to fit the new panel size
            nk_layout_row_dynamic(&ctx, 40, 1);
            nk_label(&ctx, "Starflight - Reimaged", NK_TEXT_CENTERED);

            nk_layout_row_dynamic(&ctx, 30, 1);
            if (nk_button_label(&ctx, "About")) {
                s_showHelp = true;
            }

            // Rotoscope toggle
            nk_bool useRotoscope = s_useRotoscope;
            nk_layout_row_dynamic(&ctx, 30, 2);
            if (nk_checkbox_label(&ctx, "Rotoscope Graphics", (nk_bool*)&useRotoscope)) {
                s_useRotoscope = useRotoscope != 0;
            }

            // Graphics mode selection
            if (!useRotoscope) { // Only show EGA/CGA options if Rotoscope is disabled
                nk_layout_row_dynamic(&ctx, 30, 2);
                if (nk_option_label(&ctx, "EGA Graphics", s_useEGA)) {
                    s_useEGA = 1;
                }
                if (nk_option_label(&ctx, "CGA Graphics", !s_useEGA)) {
                    s_useEGA = 0;
                }
            } else { // Place a spacer if s_useRotoscope is true
                nk_layout_row_dynamic(&ctx, 30, 1); // Adjust for a single spacer row
                //nk_spacing(&ctx, 1); // Add a spacer
            }

            nk_layout_row_static(&ctx, 30, (int)(panelWidth - 20), 1);
            if (!emulationThreadRunning)
            {
                if (nk_button_label(&ctx, "Start Game")) {
                    StartEmulationThread("");
                    loadWindowOpen = false;
                }
            }
            else
            {
                if (nk_button_label(&ctx, pauseEmulationThread ? "Resume Game" : "Pause Game")) {
                    pauseEmulationThread = !pauseEmulationThread;
                }
            }

            nk_layout_row_dynamic(&ctx, 320, 1); // Height for the save game display
            if (nk_group_begin(&ctx, "Save Games", NK_WINDOW_BORDER)) {
                // Display only one save game at a time
                const auto& saveGame = saveGames[screenshotIndex]; // Use the current index to get the save game

                nk_layout_row_begin(&ctx, NK_DYNAMIC, 230, 3); // Three elements in the row: < button, screenshot, > button
                {
                    // Previous save game button
                    nk_layout_row_push(&ctx, 0.10f);
                    if (nk_button_label(&ctx, "<")) {
                        screenshotIndex = (screenshotIndex + saveGames.size() - 1) % saveGames.size(); // Decrement index with wrap-around
                    }
                    // Display save game screenshot
                    nk_layout_row_push(&ctx, 0.80f);
                    nk_image(&ctx, saveGame.screenshot);
                    // Next save game button
                    nk_layout_row_push(&ctx, 0.10f);
                    if (nk_button_label(&ctx, ">")) {
                        screenshotIndex = (screenshotIndex + 1) % saveGames.size(); // Increment index with wrap-around
                    }
                }
                nk_layout_row_end(&ctx);

                // Display save game timestamp below the screenshot
                nk_layout_row_dynamic(&ctx, 20, 1); // Adjust height as needed for the timestamp
                nk_label(&ctx, saveGame.timestamp.c_str(), NK_TEXT_CENTERED);

                // Load button for the current save game
                nk_layout_row_dynamic(&ctx, 30, 1); // Adjust height as needed for the load button
                if (nk_button_label(&ctx, "Load")) {
                    loadWindowOpen = true;
                    loadWindowIndex = screenshotIndex;
                }

                nk_group_end(&ctx);
            }
        }
        nk_end(&ctx);
    }

    float loadWindowWidth = 800.0f;
    float loadWindowHeight = 650.0f;
    float loadWindowX = (WINDOW_WIDTH - loadWindowWidth) / 2.0f;
    float loadWindowY = (WINDOW_HEIGHT - loadWindowHeight) / 2.0f;

    // Load confirmation window
    if (loadWindowOpen && loadWindowIndex >= 0) {
        if (nk_begin(&ctx, "Confirm Load Game", nk_rect(loadWindowX, loadWindowY, loadWindowWidth, loadWindowHeight), NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE)) {
            // Display a larger screenshot for confirmation
            const auto& saveGame = saveGames[loadWindowIndex]; 
            nk_layout_row_dynamic(&ctx, 540, 1); 
            nk_image(&ctx, saveGame.screenshot); 

            nk_layout_row_dynamic(&ctx, 20, 1);
            nk_label(&ctx, saveGame.timestamp.c_str(), NK_TEXT_CENTERED);

            nk_layout_row_dynamic(&ctx, 30, 2); // Two buttons in the same row
            if (nk_button_label(&ctx, "OK")) {
                StopEmulationThread();
                StartEmulationThread(saveGame.file);
                loadWindowOpen = false; // Close the window after loading
                s_shouldToggleMenu = true;
            }
            if (nk_button_label(&ctx, "Cancel")) {
                loadWindowOpen = false; // Close the window without loading
            }
        }
        else
        {
            loadWindowOpen = false;
        }
        nk_end(&ctx);
    }

    nk_style_pop_color(&ctx);
    nk_style_pop_style_item(&ctx);

    nk_sdlsurface_render(nk_context, clear, 1);
}

static HeadingAndThrust calculateHeadingAndThrust(const vec2<int16_t>& deadReckoning, float currentHeading, float currentThrust) {
    HeadingAndThrust result;
    float targetHeading = 0.0f;

    if (deadReckoning.x == 1 && deadReckoning.y == 0) {
        targetHeading = 0.0f; // East
    } else if (deadReckoning.x == 1 && deadReckoning.y == 1) {
        targetHeading = 45.0f; // Northeast
    } else if (deadReckoning.x == 0 && deadReckoning.y == 1) {
        targetHeading = 90.0f; // North
    } else if (deadReckoning.x == -1 && deadReckoning.y == 1) {
        targetHeading = 135.0f; // Northwest
    } else if (deadReckoning.x == -1 && deadReckoning.y == 0) {
        targetHeading = 180.0f; // West
    } else if (deadReckoning.x == -1 && deadReckoning.y == -1) {
        targetHeading = 225.0f; // Southwest
    } else if (deadReckoning.x == 0 && deadReckoning.y == -1) {
        targetHeading = 270.0f; // South
    } else if (deadReckoning.x == 1 && deadReckoning.y == -1) {
        targetHeading = 315.0f; // Southeast
    }

    if (deadReckoning.x != 0 || deadReckoning.y != 0) {
        float maxTurnRate = 5.0f; // Adjust this value as needed

        float deltaHeading = targetHeading - currentHeading;

        if (deltaHeading > 180.0f) {
            deltaHeading -= 360.0f;
        } else if (deltaHeading < -180.0f) {
            deltaHeading += 360.0f;
        }

        if (deltaHeading > 0.0f) {
            result.heading = currentHeading + std::min(deltaHeading, maxTurnRate);
        } else {
            result.heading = currentHeading + std::max(deltaHeading, -maxTurnRate);
        }

        float thrustage = 0.3f;

        if (std::abs(deltaHeading) > 0.0) {
            thrustage = 1.0f;
        }

        result.thrust = (currentThrust * 0.9f) + (thrustage * 0.1f);
    } else {
        result.thrust = currentThrust - 0.01f;
        if (result.thrust < 0.0f) {
            result.thrust = 0.0f;
        }
        result.heading = currentHeading;
    }

    // Ensure heading wraps correctly at 360 degrees
    if (result.heading >= 360.0f) {
        result.heading -= 360.0f;
    } else if (result.heading < 0.0f) {
        result.heading += 360.0f;
    }

    return result;
}

static HeadingAndThrust calculateHeadingAndSpeedToDeadReckoning(int heading, float speed, float currentHeading, float currentThrust) {
    // Convert numerical heading to dead reckoning
    float targetHeading = (heading - 35) * 45.0;

    // Adjust speed to binary state as per instructions
    speed = (speed != 0.0f) ? 1.0f : 0.0f;

    // Convert speed to dead reckoning, apply rounding, and then cast to int16_t
    int16_t deadReckoningX = static_cast<int16_t>(std::round(cos(targetHeading * M_PI / 180.0f) * speed));
    int16_t deadReckoningY = static_cast<int16_t>(std::round(sin(targetHeading * M_PI / 180.0f) * speed));

    // Call the original calculateHeadingAndThrust with the converted and casted values
    return calculateHeadingAndThrust({deadReckoningX, deadReckoningY}, currentHeading, currentThrust);
}

static std::once_flag s_animIndexFlag;
static int s_restingPoseIndex = -1;
static int s_armatureActionIndex = -1;

void InitializeAnimationIndices()
{
    for (int i = 0; i < s_gc.station.model->GetAnimations().size(); ++i) {
        if (s_gc.station.model->GetAnimations()[i].Name == "RestingPose") {
            s_restingPoseIndex = i;
        } else if (s_gc.station.model->GetAnimations()[i].Name == "ArmatureAction") {
            s_armatureActionIndex = i;
        }
    }
}

void InitializeCommonResources()
{
    SF_GLTF_PBR_Renderer::CreateInfo RendererCI = {};
    InitializeGLTFPBRCreateInfo(RendererCI);

    CreateUniformBuffer(s_gc.m_pDevice, sizeof(HLSL::CameraAttribs), "cbCameraAttribs", &s_gc.cameraAttribsCB);
    //CreateUniformBuffer(s_gc.m_pDevice, sizeof(HLSL::GLTFNodeShaderTransforms), "cbPrimitiveAttribs", &s_gc.PBRPrimitiveAttribsCB);
    CreateUniformBuffer(s_gc.m_pDevice, SF_GLTF_PBR_Renderer::GetPBRPrimitiveAttribsSizeStatic(RendererCI, SF_PBR_Renderer::PSO_FLAG_ALL, 0), "cbPrimitiveAttribs", &s_gc.PBRPrimitiveAttribsCB);
    const size_t JointsBufferSize = sizeof(float4x4) * /* m_Settings.MaxJointCount */ 64 * 2;
    CreateUniformBuffer(s_gc.m_pDevice, JointsBufferSize, "cbJointTransforms", &s_gc.jointsBuffer);

    BufferDesc VertBuffDesc;
    VertBuffDesc.Name = "Dummy vertex buffer";
    VertBuffDesc.Usage = USAGE_IMMUTABLE;
    VertBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
    VertBuffDesc.Size = sizeof(float) * 3;

    BufferData VBData;
    float DummyVertex[] = {0.0f, 0.0f, 0.0f};
    VBData.pData = DummyVertex;
    VBData.DataSize = sizeof(DummyVertex);

    s_gc.m_pDevice->CreateBuffer(VertBuffDesc, &VBData, &s_gc.dummyVertexBuffer);

    CreateUniformBuffer(s_gc.m_pDevice, SF_GLTF_PBR_Renderer::GetPRBFrameAttribsSizeStatic(RendererCI.MaxLightCount, RendererCI.MaxShadowCastingLightCount), "PBR frame attribs buffer", &s_gc.frameAttribsCB);

    CreateUniformBuffer(s_gc.m_pDevice, SF_PBR_Renderer::GetHeightmapAttribsSizeStatic(), "Heightmap attribs buffer", &s_gc.heightmapAttribsCB);

    CreateUniformBuffer(s_gc.m_pDevice, SF_PBR_Renderer::GetTerrainAttribsSizeStatic(), "Terrain attribs buffer", &s_gc.terrainAttribsCB);

    CreateUniformBuffer(s_gc.m_pDevice, sizeof(HLSL::CameraAttribs), "Camera Attribs CB", &s_gc.pcbCameraAttribs);
    CreateUniformBuffer(s_gc.m_pDevice, sizeof(HLSL::LightAttribs), "Light Attribs CB", &s_gc.pcbLightAttribs);

    BufferDesc SBDesc;
    SBDesc.Name           = "PBR instance attribs SB";
    SBDesc.Size           = sizeof(HLSL::PBRInstanceAttribs) * SF_PBR_Renderer::MaxInstanceCount;
    SBDesc.Usage          = USAGE_DYNAMIC;
    SBDesc.BindFlags      = BIND_UNORDERED_ACCESS | BIND_SHADER_RESOURCE;
    SBDesc.Mode           = BUFFER_MODE_STRUCTURED;
    SBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
    SBDesc.ElementByteStride = sizeof(HLSL::PBRInstanceAttribs);

    s_gc.m_pDevice->CreateBuffer(SBDesc, nullptr, &s_gc.instanceAttribsSB);

    s_gc.instanceAttribsSBView = s_gc.instanceAttribsSB->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE);    
}

void InitStation()
{
    InitModel("C:/Users/Dean/Downloads/station29.glb", s_gc.station);

    for (int i = 0; i < s_gc.station.model->GetAnimations().size(); ++i) {
        auto& anim = s_gc.station.model->GetAnimations()[i];
        if (anim.Name == "ArmatureAction") {
            s_gc.spaceManState.walkingAnimationLength = anim.End - anim.Start;
            s_gc.spaceManState.walkingAnimationStart = anim.Start;
            continue;
        }
        if (anim.Name == "RestingPose") {
            s_gc.spaceManState.restingPoseLength = anim.End - anim.Start;
            s_gc.spaceManState.restingPoseStart = anim.Start;
            continue;
        }
    }
}

void UpdateStation(VulkanContext::frame_id_t inFlightIndex)
{
    VulkanContext::frame_id_t frameCount = s_gc.vc.current_frame();

    double currentTimeInSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - s_gc.epoch).count();

    InterpolatorPoint spacePoint{};
    std::pair<GraphicsContext::SpaceManState::AnimationState, double> spaceManState{};

    {
        std::lock_guard<std::mutex> lg(s_gc.spaceManMutex);
        spacePoint = s_gc.spaceMan.interpolate(currentTimeInSeconds);
        auto isWalking = !s_gc.spaceMan.isExtrapolating(currentTimeInSeconds);

        auto points = s_gc.spaceMan.ActivePoints();
        if (isWalking && points.size() > 0)
        {
            if (points.back().x == spacePoint.position.x &&
                points.back().y == spacePoint.position.y &&
                points.back().z == spacePoint.position.z)
            {
                isWalking = false;
            }
        }

        s_gc.spaceMan.expire(currentTimeInSeconds);

        spaceManState = s_gc.spaceManState.getAnimationState(isWalking);
    }

    TranslateMan(spacePoint);

    float rotationAngle = currentTimeInSeconds * 0.005f;
    //float rotationAngle = (float)frameCount * 0.01f;

    float axisOne = rotationAngle;
    float axisTwo = rotationAngle / 3.0f;

    float4x4 RotationMatrixCam = float4x4::RotationY(axisOne) * float4x4::RotationZ(-axisTwo);
    float4x4 RotationMatrixModel = float4x4::RotationY(axisTwo) * float4x4::RotationZ(axisOne);

    s_gc.station.model->ComputeTransforms(s_gc.renderParams.SceneIndex, s_gc.station.transforms[0]);
    s_gc.station.aabb = s_gc.station.model->ComputeBoundingBox(s_gc.renderParams.SceneIndex, s_gc.station.transforms[0], nullptr);
    s_gc.planet.worldspaceAABB = s_gc.station.aabb;

    // Center and scale model
    float  MaxDim = 0;
    float3 ModelDim{ s_gc.station.aabb.Max - s_gc.station.aabb.Min };
    MaxDim = std::max(MaxDim, ModelDim.x);
    MaxDim = std::max(MaxDim, ModelDim.y);
    MaxDim = std::max(MaxDim, ModelDim.z);

    float4x4 InvYAxis = float4x4::Identity();
    InvYAxis._22 = -1;
#if 1
    s_gc.station.scale = (1.0f / std::max(MaxDim, 0.01f)) * 0.5f;
    auto     Translate = -s_gc.station.aabb.Min - 0.5f * ModelDim;
    InvYAxis._22 = -1;
#else
    s_gc.station.scale = 1.0f;
    float3 Translate = { 0.f, 0.f, 0.f };
#endif

    s_gc.station.modelTransform = float4x4::Translation(Translate) * float4x4::Scale(s_gc.station.scale) * InvYAxis;
    s_gc.station.scaleAndTransform = float4x4::Translation(Translate) * float4x4::Scale(s_gc.station.scale);
#if 0
    if (s_gc.station.model->Animations.size())
    {
        const GLTF::Animation* anim = nullptr;
        int animIndex = -1;
        for (int i = 0; i < s_gc.station.model->Animations.size(); ++i) {
            if (s_gc.station.model->Animations[i].Name == "ArmatureAction") {
                anim = &s_gc.station.model->Animations[i];
                animIndex = i;
                break;
            }
        }

        double animationLength = anim->End - anim->Start;

        double timeElapsed = std::fmod(currentTimeInSeconds, animationLength) + anim->Start;

        s_gc.station.model->ComputeTransforms(s_gc.renderParams.SceneIndex, s_gc.station.transforms[0], s_gc.station.modelTransform, animIndex, timeElapsed);
    }
    else
    {
        s_gc.station.model->ComputeTransforms(s_gc.renderParams.SceneIndex, s_gc.station.transforms[0], s_gc.station.modelTransform);
    }
#endif

    std::call_once(s_animIndexFlag, InitializeAnimationIndices);

    int animIndex = -1;
    double timeElapsed = spaceManState.second;
    switch (spaceManState.first)
    {
        case GraphicsContext::SpaceManState::AnimationState::Standing:
            animIndex = s_restingPoseIndex;
            break;
        case GraphicsContext::SpaceManState::AnimationState::StartingWalking:
            animIndex = s_restingPoseIndex;
            break;
        case GraphicsContext::SpaceManState::AnimationState::StoppingWalking:
            timeElapsed = s_gc.spaceManState.restingPoseLength - timeElapsed;
            animIndex = s_restingPoseIndex;
            break;
        case GraphicsContext::SpaceManState::AnimationState::Walking:
            animIndex = s_armatureActionIndex;
            timeElapsed += s_gc.spaceManState.walkingAnimationStart;
            break;
    }

    s_gc.station.model->ComputeTransforms(s_gc.renderParams.SceneIndex, s_gc.station.transforms[0], s_gc.station.modelTransform, animIndex, timeElapsed);
        
    s_gc.station.aabb = s_gc.station.model->ComputeBoundingBox(s_gc.renderParams.SceneIndex, s_gc.station.transforms[0], nullptr);
    s_gc.station.transforms[1] = s_gc.station.transforms[0];

    float YFov = PI_F / 4.0f;
    float ZNear = 0.1f;
    float ZFar = 100.f;

    float4x4 CameraView;

    const auto* pCameraNode = s_gc.station.camera;
    const auto* pCamera = pCameraNode->pCamera;
    const auto& CameraGlobalTransform = s_gc.station.transforms[inFlightIndex & 0x01].NodeGlobalMatrices[pCameraNode->Index];

    // GLTF camera is defined such that the local +X axis is to the right,
    // the lens looks towards the local -Z axis, and the top of the camera
    // is aligned with the local +Y axis.
    // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#cameras
    // We need to inverse the Z axis as our camera looks towards +Z.
    float4x4 InvZAxis = float4x4::Identity();
    InvZAxis._33 = -1;

#if 0
    CameraView = CameraGlobalTransform.Inverse() * InvZAxis;
    s_gc.renderParams.ModelTransform = float4x4::Identity();
#else
    auto trans = float4x4::Translation(0.0f, -0.057f, 0.264f);
    auto cam = QuaternionF(0.0f, 0.2079117f, 0.9781476f, 0.0f);

    auto modelTransform = QuaternionF::RotationFromAxisAngle(float3{ -1.f, 0.0f, 0.0f }, -PI_F / 2.f).ToMatrix();

    //auto invModelTransform = modelTransform.Inverse();
    
    CameraView = RotationMatrixCam * cam.ToMatrix() * trans;
    s_gc.renderParams.ModelTransform = RotationMatrixModel * modelTransform; // QuaternionF::RotationFromAxisAngle(float3{ -1.f, 0.0f, 0.0f }, -PI_F / 2.f).ToMatrix();
    //s_gc.renderParams.ModelTransform = float4x4::Identity();
#endif

    YFov = pCamera->Perspective.YFov;
    ZNear = pCamera->Perspective.ZNear;
    ZFar = pCamera->Perspective.ZFar;

    // Apply pretransform matrix that rotates the scene according the surface orientation
    CameraView *= GetSurfacePretransformMatrix(float3{ 0, 0, 1 });

    // Rotate the camera up by 15 degrees around the y-axis
    //float angle = 45.0f * (M_PI / 180.0f); // Convert degrees to radians
    //float4x4 RotateX = float4x4::RotationX(angle);
    //CameraView = RotateX * CameraView;    

    float4x4 CameraWorld = CameraView.Inverse();

    // Get projection matrix adjusted to the current screen orientation
    const auto CameraProj = GetAdjustedProjectionMatrix(YFov, ZNear, ZFar);
    const auto CameraViewProj = CameraView * CameraProj;

    float3 CameraWorldPos = float3::MakeVector(CameraWorld[3]);

    auto& CurrCamAttribs = s_gc.cameraAttribs[inFlightIndex & 0x01];

    CurrCamAttribs.f4ViewportSize = float4{ static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT), 1.f / (float)WINDOW_WIDTH, 1.f / (float)WINDOW_HEIGHT };
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

    s_gc.cameraAttribs[(inFlightIndex + 1) & 0x01] = CurrCamAttribs;
}

struct ShaderParams
{
    float OcclusionStrength = 1;
    float EmissionScale = 1;
    float IBLScale = 2.0f;
    float AverageLogLum = 0.3f;
    float MiddleGray = 0.18f;
    float WhitePoint = 3.f;

    float4 HighlightColor = float4{ 0, 0, 0, 0 };
    float4 WireframeColor = float4{ 0.8f, 0.7f, 0.5f, 1.0f };

    float SSRScale = 1;
    float SSAOScale = 1;
    int   PostFXDebugMode = 0;
};

static ShaderParams m_ShaderAttribs;


namespace Diligent
{

VkImageCreateInfo TextureDescToVkImageCreateInfo(const TextureDesc& Desc, const RefCntAutoPtr<IRenderDeviceVk>& pRenderDeviceVk) noexcept
{
    const auto  IsMemoryless = (Desc.MiscFlags & MISC_TEXTURE_FLAG_MEMORYLESS) != 0;
    const auto& FmtAttribs = GetTextureFormatAttribs(Desc.Format);
    const auto  ImageView2DSupported = !Desc.Is3D() || pRenderDeviceVk->GetAdapterInfo().Texture.TextureView2DOn3DSupported;

    VkImageCreateInfo ImageCI = {};

    ImageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageCI.pNext = nullptr;
    ImageCI.flags = 0;
    if (Desc.Type == RESOURCE_DIM_TEX_CUBE || Desc.Type == RESOURCE_DIM_TEX_CUBE_ARRAY)
        ImageCI.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    if (FmtAttribs.IsTypeless)
        ImageCI.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT; // Specifies that the image can be used to create a
    // VkImageView with a different format from the image.

    if (Desc.Is1D())
        ImageCI.imageType = VK_IMAGE_TYPE_1D;
    else if (Desc.Is2D())
        ImageCI.imageType = VK_IMAGE_TYPE_2D;
    else if (Desc.Is3D())
    {
        ImageCI.imageType = VK_IMAGE_TYPE_3D;
        if (ImageView2DSupported)
            ImageCI.flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
    }
    else
    {
        LOG_ERROR_AND_THROW("Unknown texture type");
    }

    TEXTURE_FORMAT InternalTexFmt = Desc.Format;
    if (FmtAttribs.IsTypeless)
    {
        TEXTURE_VIEW_TYPE PrimaryViewType;
        if (Desc.BindFlags & BIND_DEPTH_STENCIL)
            PrimaryViewType = TEXTURE_VIEW_DEPTH_STENCIL;
        else if (Desc.BindFlags & BIND_UNORDERED_ACCESS)
            PrimaryViewType = TEXTURE_VIEW_UNORDERED_ACCESS;
        else if (Desc.BindFlags & BIND_RENDER_TARGET)
            PrimaryViewType = TEXTURE_VIEW_RENDER_TARGET;
        else
            PrimaryViewType = TEXTURE_VIEW_SHADER_RESOURCE;
        InternalTexFmt = GetDefaultTextureViewFormat(Desc, PrimaryViewType);
    }

    ImageCI.format = TexFormatToVkFormat(InternalTexFmt);

    ImageCI.extent.width = Desc.GetWidth();
    ImageCI.extent.height = Desc.GetHeight();
    ImageCI.extent.depth = Desc.GetDepth();
    ImageCI.mipLevels = Desc.MipLevels;
    ImageCI.arrayLayers = Desc.GetArraySize();

    ImageCI.samples = static_cast<VkSampleCountFlagBits>(Desc.SampleCount);
    ImageCI.tiling = VK_IMAGE_TILING_OPTIMAL;

    ImageCI.usage = BindFlagsToVkImageUsage(Desc.BindFlags, IsMemoryless, true);
    // TRANSFER_SRC_BIT and TRANSFER_DST_BIT are required by CopyTexture
    ImageCI.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    if (Desc.BindFlags & (BIND_DEPTH_STENCIL | BIND_RENDER_TARGET))
        DEV_CHECK_ERR(ImageView2DSupported, "imageView2DOn3DImage in VkPhysicalDevicePortabilitySubsetFeaturesKHR is not enabled, can not create depth-stencil target with 2D image view");

    if (Desc.MiscFlags & MISC_TEXTURE_FLAG_GENERATE_MIPS)
    {
        VERIFY_EXPR(!IsMemoryless);
    }
    if (Desc.MiscFlags & MISC_TEXTURE_FLAG_SUBSAMPLED)
    {
        ImageCI.usage &= ~(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
        ImageCI.flags |= VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT;
    }

    ImageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ImageCI.queueFamilyIndexCount = 0;
    ImageCI.pQueueFamilyIndices = nullptr;

    if (Desc.Usage == USAGE_SPARSE)
    {
        ImageCI.flags &= ~VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT; // not compatible
        ImageCI.flags |=
            VK_IMAGE_CREATE_SPARSE_BINDING_BIT |
            VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT |
            (Desc.MiscFlags & MISC_TEXTURE_FLAG_SPARSE_ALIASING ? VK_IMAGE_CREATE_SPARSE_ALIASED_BIT : 0);
    }

    return ImageCI;
}
}

static FfxResource ffxGetResource(ITextureView* textureView)
{
    VkImage image = (VkImage)textureView->GetTexture()->GetNativeHandle();

    VkImageCreateInfo vkici = TextureDescToVkImageCreateInfo(textureView->GetTexture()->GetDesc(), s_gc.m_pDeviceVk);

    return ffxGetResourceVK((void*)image, ffxGetImageResourceDescriptionVK(image, vkici, FFX_RESOURCE_USAGE_RENDERTARGET), nullptr, FFX_RESOURCE_STATE_PIXEL_COMPUTE_READ);
}

void RenderStation(VulkanContext::frame_id_t inFlightIndex)
{
    if(s_gc.currentScene != SCENE_STATION)
    {
        s_gc.shadowMap->InitializeResourceBindings(s_gc.station.model);
        s_gc.currentScene = SCENE_STATION;
    }

    auto sunBehavior = [](const float4x4& lightGlobalTransform, double currentTimeInSeconds) {
        float3 lightDir = float3{ 0.0f, -0.07f, -0.07f };
        float3 Direction = -normalize(lightDir);

        float rotationAngle = currentTimeInSeconds * 0.025f;
        float4x4 rotationMatrix = float4x4::RotationY(rotationAngle);
        float4 rotatedDirection = rotationMatrix * float4(Direction, 0.0f);
        Direction = float3(rotatedDirection);
        return Direction;
    };

    RenderSFModel(inFlightIndex, s_gc.station, sunBehavior);
}

void RenderSFModel(VulkanContext::frame_id_t inFlightIndex, GraphicsContext::SFModel& model, const SunBehaviorFn& sunBehavior)
{
    ITextureView*        pRTVOffscreen   = s_gc.buffers[inFlightIndex].offscreenColorBuffer->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);

    ITextureView*        pRTV   = s_gc.buffers[inFlightIndex].diligentColorBuffer;
    ITextureView*        pDSV   = s_gc.buffers[inFlightIndex].diligentDepthBuffer;
    ITextureView*        pPrevDSV = s_gc.buffers[(inFlightIndex + 1) & 0x01].diligentDepthBuffer;
    ITextureView*        pShadowDSV = s_gc.buffers[inFlightIndex].diligentShadowDepthBuffer;

    if(!s_gc.applyPostFX)
    {
        s_gc.applyPostFX.Initialize(s_gc.m_pDevice, VkFormatToTexFormat(VK_FORMAT_R8G8B8A8_SRGB), s_gc.frameAttribsCB);
    }

    PostFXContext::FrameDesc FrameDesc;
    FrameDesc.Index  = s_gc.vc.current_frame();
    FrameDesc.Width  = WINDOW_WIDTH;
    FrameDesc.Height = WINDOW_HEIGHT;
    s_gc.postFXContext->PrepareResources(s_gc.m_pDevice, FrameDesc, PostFXContext::FEATURE_FLAG_NONE);

    s_gc.bloom->PrepareResources(s_gc.m_pDevice, s_gc.m_pImmediateContext, s_gc.postFXContext.get(), Bloom::FEATURE_FLAG_NONE);

    if (s_gc.ssao)
    {
        s_gc.ssao->PrepareResources(s_gc.m_pDevice, s_gc.m_pImmediateContext, s_gc.postFXContext.get(), ScreenSpaceAmbientOcclusion::FEATURE_FLAG_NONE);
    }
    
    #if defined(DE_SSR)
    s_gc.ssr->PrepareResources(s_gc.m_pDevice, s_gc.m_pImmediateContext, s_gc.postFXContext.get(), ScreenSpaceReflection::FEATURE_FLAG_NONE);
    #endif

    HLSL::LightAttribs  LightAttrs = {};
    HLSL::CameraAttribs CamAttribs = {};

    s_gc.gBuffer->Resize(s_gc.m_pDevice, WINDOW_WIDTH, WINDOW_HEIGHT);

    const auto& CurrCamAttribs = s_gc.cameraAttribs[inFlightIndex & 0x01];
    const auto& PrevCamAttribs = s_gc.cameraAttribs[(inFlightIndex + 1) & 0x01];
    const auto& CurrTransforms = model.transforms[inFlightIndex & 0x01];
    const auto& PrevTransforms = model.transforms[(inFlightIndex + 1) & 0x01];

    const auto& DynamicCurrTransforms = model.dynamicMeshTransforms[inFlightIndex & 0x01];
    const auto& DynamicPrevTransforms = model.dynamicMeshTransforms[(inFlightIndex + 1) & 0x01];


    MapHelper<HLSL::PBRFrameAttribs> FrameAttribs{ s_gc.m_pImmediateContext, s_gc.frameAttribsCB, MAP_WRITE, MAP_FLAG_DISCARD };

    if (model.dynamicMesh)
    {
        model.dynamicMesh->PrepareResources();
    }

    FrameAttribs->Camera = CurrCamAttribs;
    FrameAttribs->PrevCamera = PrevCamAttribs;

    FrameAttribs->PrevCamera.f4ExtraData[0] = float4{
        m_ShaderAttribs.SSRScale,
        static_cast<float>(m_ShaderAttribs.PostFXDebugMode),
        0,
        0,
    };

    {
        StateTransitionDesc Barriers[] = {
        {
            s_gc.shadowMap->GetShadowMap()->GetTexture(), RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_DEPTH_WRITE, STATE_TRANSITION_FLAG_UPDATE_STATE},
        };
        s_gc.m_pImmediateContext->TransitionResourceStates(_countof(Barriers), Barriers);
    }

    SF_GLTF::Light m_DefaultLight{};
    m_DefaultLight.Type = SF_GLTF::Light::TYPE::DIRECTIONAL;
    m_DefaultLight.Intensity = 4.0f;
    m_DefaultLight.Color = float3{ 1, 1, 1 };

    float3      m_LightDirection{};
    m_LightDirection = normalize(float3(-0.7f, 0.0f, 1.0f));

    const size_t MaxLightCount = s_gc.pbrRenderer->GetSettings().MaxLightCount;

    int LightCount = 0;
    auto* Lights = reinterpret_cast<HLSL::PBRLightAttribs*>(FrameAttribs + 1);
    auto* ShadowMaps = reinterpret_cast<HLSL::PBRShadowMapInfo*>(Lights + MaxLightCount);
    if (!model.lights.empty())
    {
        auto baseModelTransform = QuaternionF::RotationFromAxisAngle(float3{ -1.f, 0.0f, 0.0f }, -PI_F / 2.f).ToMatrix();

        LightCount = std::min(static_cast<Uint32>(model.lights.size()), s_gc.pbrRenderer->GetSettings().MaxLightCount);
        for (int i = 0; i < LightCount; ++i)
        {
            const auto& LightNode = *model.lights[i];
            auto LightGlobalTransform = model.transforms[inFlightIndex & 0x01].NodeGlobalMatrices[LightNode.Index];

            SF_GLTF::Light l = *LightNode.pLight;
            l.Intensity /= 512.0f;

            float3 lightDir = {};
            float3 Direction = {};

            if (LightNode.Name == "Sun")
            {
                l.Type = SF_GLTF::Light::TYPE::DIRECTIONAL;
                l.Intensity = 1.0f;
                LightGlobalTransform *= s_gc.renderParams.ModelTransform;
                double currentTimeInSeconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - s_gc.epoch).count();

                Direction = sunBehavior(LightGlobalTransform, currentTimeInSeconds);
            }
            else
            {
                LightGlobalTransform *= s_gc.renderParams.ModelTransform;
                // The light direction is along the negative Z axis of the light's local space.
                // https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_lights_punctual#adding-light-instances-to-nodes
                lightDir = float3{ LightGlobalTransform._31, LightGlobalTransform._32, LightGlobalTransform._33 };
                Direction = -normalize(lightDir);
            }
            
            float3 Position = float3{ LightGlobalTransform._41, LightGlobalTransform._42, LightGlobalTransform._43 };
            SF_GLTF_PBR_Renderer::PBRLightShaderAttribsData AttribsData = { &l, &Position, &Direction, model.scale };

            if (LightNode.Name == "Sun")
            {
                s_gc.shadowMap->RenderShadowMap(CurrCamAttribs, Direction, inFlightIndex, s_gc.renderParams, &ShadowMaps[0], model);
                AttribsData.ShadowMapIndex = 0;
                AttribsData.NumCascades = 2;

                if(s_gc.currentScene == SCENE_TERRAIN)
                {
                    CamAttribs = CurrCamAttribs;

                    LightAttrs = s_gc.shadowMap->GetLightAttribs();
                    LightAttrs.f4Direction = float4(Direction, 1.0f);
                }                
            }
            else
            {
                AttribsData.ShadowMapIndex = -1;
                AttribsData.NumCascades = 0;
            }
            
            SF_GLTF_PBR_Renderer::WritePBRLightShaderAttribs(AttribsData, Lights + i);
        }
    }
    else
    {
        SF_GLTF_PBR_Renderer::WritePBRLightShaderAttribs({ &m_DefaultLight, nullptr, &m_LightDirection, model.scale }, Lights);
        LightCount = 1;
    }

    {
        StateTransitionDesc Barriers[] = {
        {
            s_gc.shadowMap->GetShadowMap()->GetTexture(), RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_DEPTH_READ, STATE_TRANSITION_FLAG_UPDATE_STATE},
        };
        s_gc.m_pImmediateContext->TransitionResourceStates(_countof(Barriers), Barriers);
    }

    Uint32 BuffersMask = GBUFFER_RT_FLAG_ALL_COLOR_TARGETS | ((inFlightIndex & 0x01) ? GBUFFER_RT_FLAG_DEPTH1 : GBUFFER_RT_FLAG_DEPTH0);
    s_gc.gBuffer->Bind(s_gc.m_pImmediateContext, BuffersMask, nullptr, BuffersMask);

    auto& Renderer = FrameAttribs->Renderer;
    s_gc.pbrRenderer->SetInternalShaderParameters(Renderer);

    Renderer.OcclusionStrength = m_ShaderAttribs.OcclusionStrength;
    Renderer.EmissionScale = m_ShaderAttribs.EmissionScale;
    Renderer.AverageLogLum = m_ShaderAttribs.AverageLogLum;
    Renderer.MiddleGray = m_ShaderAttribs.MiddleGray;
    Renderer.WhitePoint = m_ShaderAttribs.WhitePoint;
    Renderer.IBLScale = float4{ m_ShaderAttribs.IBLScale };
    Renderer.HighlightColor = m_ShaderAttribs.HighlightColor;
    Renderer.UnshadedColor = m_ShaderAttribs.WireframeColor;
    Renderer.PointSize = 1;
    Renderer.MipBias = 0;
    Renderer.LightCount = LightCount;
    Renderer.Time = std::chrono::duration<float>(std::chrono::steady_clock::now() - s_gc.epoch).count();

    auto RenderModel = [&](SF_GLTF_PBR_Renderer::RenderInfo::ALPHA_MODE_FLAGS AlphaModes) {
        const auto OrigAlphaModes = s_gc.renderParams.AlphaModes;

        s_gc.renderParams.AlphaModes &= AlphaModes;
        if (s_gc.renderParams.AlphaModes != SF_GLTF_PBR_Renderer::RenderInfo::ALPHA_MODE_FLAG_NONE)
        {
            if (model.dynamicMesh)
            {
                SF_GLTF_PBR_Renderer::RenderInfo ri = s_gc.renderParams;
                ri.Flags |= SF_GLTF_PBR_Renderer::PSO_FLAG_USE_HEIGHTMAP;
                ri.Flags |= SF_GLTF_PBR_Renderer::PSO_FLAG_USE_INSTANCING;
                ri.Flags |= SF_GLTF_PBR_Renderer::PSO_FLAG_USE_TERRAINING;
                ri.Flags |= SF_GLTF_PBR_Renderer::PSO_FLAG_USE_TEXCOORD1;

                ri.TerrainTextureOffset = s_gc.terrainTextureOffset;

                const std::string showPlanet = "Earth-like";

                // Pick the earth-like planet and convert it to the TerrainInfo on RenderInfo
                auto it = std::find_if(model.planetTypes.begin(), model.planetTypes.end(), [showPlanet](const auto& planetType) {
                    return planetType.name == showPlanet;
                });

                if (it != model.planetTypes.end())
                {
                    using TerrainInfo = SF_GLTF_PBR_Renderer::RenderInfo::TerrainInfo;

                    std::deque<TerrainInfo> terrainInfos;
                    float endBiomHeight = 16.0f;
                    for (auto it_biom = it->boundaries.rbegin(); it_biom != it->boundaries.rend(); ++it_biom)
                    {
                        auto materialIndex = model.biomMaterialIndex[it_biom->name];
                        terrainInfos.push_front({ materialIndex, it_biom->startHeight, endBiomHeight });
                        endBiomHeight = it_biom->startHeight;
                    }

                    ri.TerrainInfos = std::vector<TerrainInfo>(terrainInfos.begin(), terrainInfos.end());
                }

                ri.pWaterHeightMap = s_gc.buffers[inFlightIndex].waterHeightMap->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
                
                s_gc.pbrRenderer->Render(s_gc.m_pImmediateContext, *model.dynamicMesh, DynamicCurrTransforms, &DynamicPrevTransforms, ri, &model.bindings);
            }
            else
            {
                s_gc.pbrRenderer->Render(s_gc.m_pImmediateContext, *model.model, CurrTransforms, &PrevTransforms, s_gc.renderParams, &model.bindings);
            }
        }

        s_gc.renderParams.AlphaModes = OrigAlphaModes;
    };

    s_gc.renderParams.AlphaModes = SF_GLTF_PBR_Renderer::RenderInfo::ALPHA_MODE_FLAG_ALL;

    s_gc.pbrRenderer->Begin(s_gc.m_pImmediateContext);

    RenderModel(SF_GLTF_PBR_Renderer::RenderInfo::ALPHA_MODE_FLAG_OPAQUE | SF_GLTF_PBR_Renderer::RenderInfo::ALPHA_MODE_FLAG_MASK);

    {
        ITextureView* pEnvMapSRV = model.env;

        HLSL::ToneMappingAttribs TMAttribs;
        TMAttribs.iToneMappingMode     = (s_gc.renderParams.Flags & SF_GLTF_PBR_Renderer::PSO_FLAG_ENABLE_TONE_MAPPING) ? TONE_MAPPING_MODE_UNCHARTED2 : TONE_MAPPING_MODE_NONE;
        TMAttribs.bAutoExposure        = 0;
        TMAttribs.fMiddleGray          = m_ShaderAttribs.MiddleGray;
        TMAttribs.bLightAdaptation     = 0;
        TMAttribs.fWhitePoint          = m_ShaderAttribs.WhitePoint;
        TMAttribs.fLuminanceSaturation = 1.0;

        EnvMapRenderer::RenderAttribs EnvMapAttribs;
        EnvMapAttribs.pEnvMap       = pEnvMapSRV;
        EnvMapAttribs.AverageLogLum = m_ShaderAttribs.AverageLogLum;
        EnvMapAttribs.MipLevel      = 0.0f;  //m_EnvMapMipLevel;
        // It is essential to write zero alpha because we use alpha channel
        // to attenuate SSR for transparent surfaces.
        EnvMapAttribs.Alpha                = 0.0;
        EnvMapAttribs.ConvertOutputToSRGB  = (s_gc.renderParams.Flags & SF_GLTF_PBR_Renderer::PSO_FLAG_CONVERT_OUTPUT_TO_SRGB) != 0;
        EnvMapAttribs.ComputeMotionVectors = true;

        s_gc.envMapRenderer->Prepare(s_gc.m_pImmediateContext, EnvMapAttribs, TMAttribs);
        s_gc.envMapRenderer->Render(s_gc.m_pImmediateContext);
    }    

    RenderModel(SF_GLTF_PBR_Renderer::RenderInfo::ALPHA_MODE_FLAG_BLEND);

    {
        s_gc.m_pImmediateContext->SetRenderTargets(0, nullptr, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        StateTransitionDesc Barriers[GBUFFER_RT_COUNT];
        for (Uint32 i = 0; i < GBUFFER_RT_COUNT; ++i)
            Barriers[i] = StateTransitionDesc{ s_gc.gBuffer->GetBuffer(i), RESOURCE_STATE_UNKNOWN, RESOURCE_STATE_SHADER_RESOURCE, STATE_TRANSITION_FLAG_UPDATE_STATE };
        s_gc.m_pImmediateContext->TransitionResourceStates(GBUFFER_RT_COUNT, Barriers);
    }

    ITextureView* pCurrDepthSRV = s_gc.gBuffer->GetBuffer((inFlightIndex & 0x01) ? GBUFFER_RT_DEPTH1 : GBUFFER_RT_DEPTH0)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    ITextureView* pPrevDepthSRV = s_gc.gBuffer->GetBuffer((inFlightIndex & 0x01) ? GBUFFER_RT_DEPTH0 : GBUFFER_RT_DEPTH1)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    {
        PostFXContext::RenderAttributes PostFXAttibs;
        PostFXAttibs.pDevice = s_gc.m_pDevice;
        PostFXAttibs.pDeviceContext = s_gc.m_pImmediateContext;
        PostFXAttibs.pCameraAttribsCB = s_gc.frameAttribsCB;
        PostFXAttibs.pCurrDepthBufferSRV = pCurrDepthSRV;
        PostFXAttibs.pPrevDepthBufferSRV = pPrevDepthSRV;
        PostFXAttibs.pMotionVectorsSRV = s_gc.gBuffer->GetBuffer(GBUFFER_RT_MOTION_VECTORS)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        s_gc.postFXContext->Execute(PostFXAttibs);
    }

    #if defined(DE_SSR)
    {
        HLSL::ScreenSpaceReflectionAttribs SSRAttribs{};
        SSRAttribs.RoughnessChannel = 0;
        SSRAttribs.IsRoughnessPerceptual = true;

        ScreenSpaceReflection::RenderAttributes SSRRenderAttribs{};
        SSRRenderAttribs.pDevice = s_gc.m_pDevice;
        SSRRenderAttribs.pDeviceContext = s_gc.m_pImmediateContext;
        SSRRenderAttribs.pPostFXContext = s_gc.postFXContext.get();
        SSRRenderAttribs.pColorBufferSRV = s_gc.gBuffer->GetBuffer(GBUFFER_RT_RADIANCE)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        SSRRenderAttribs.pDepthBufferSRV = pCurrDepthSRV;
        SSRRenderAttribs.pNormalBufferSRV = s_gc.gBuffer->GetBuffer(GBUFFER_RT_NORMAL)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        SSRRenderAttribs.pMaterialBufferSRV = s_gc.gBuffer->GetBuffer(GBUFFER_RT_MATERIAL_DATA)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        SSRRenderAttribs.pMotionVectorsSRV = s_gc.gBuffer->GetBuffer(GBUFFER_RT_MOTION_VECTORS)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        SSRRenderAttribs.pSSRAttribs = &SSRAttribs;
        s_gc.ssr->Execute(SSRRenderAttribs);
    }
    #endif
    #if defined(FX_SSR)
    {
        struct SSSRSettings
        {
            float TemporalStabilityFactor = 0.7f;
            float DepthBufferThickness = 0.025f;
            float RoughnessThreshold = 0.5f;
            float VarianceThreshold = 0.0f;
            uint32_t MaxTraversalIntersections = 128;
            uint32_t MinTraversalOccupancy = 4;
            uint32_t MostDetailedMip = 0;
            uint32_t SamplesPerQuad = 1;
            bool TemporalVarianceGuidedTracingEnabled = true;
        };

        SSSRSettings outSettings;

        FfxSssrDispatchDescription desc = {};
        desc.commandList = ffxGetCommandListVK(s_gc.m_pImmediateContextVk->GetVkCommandBuffer());

        // Assuming the resources are correctly set up and accessible
        desc.color = ffxGetResource(s_gc.gBuffer->GetBuffer(GBUFFER_RT_RADIANCE)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
        desc.depth = ffxGetResource(pCurrDepthSRV);
        desc.motionVectors = ffxGetResource(s_gc.gBuffer->GetBuffer(GBUFFER_RT_MOTION_VECTORS)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
        desc.normal = ffxGetResource(s_gc.gBuffer->GetBuffer(GBUFFER_RT_NORMAL)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
        desc.materialParameters = ffxGetResource(s_gc.gBuffer->GetBuffer(GBUFFER_RT_MATERIAL_DATA)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
        desc.environmentMap = ffxGetResource(model.env);
        desc.brdfTexture = ffxGetResource(s_gc.pbrRenderer->GetPreintegratedGGX_SRV());
        desc.output = ffxGetResource(s_gc.buffers[inFlightIndex].diligentSsrBuffer);

        auto& camAttribs = s_gc.cameraAttribs[0];

        *reinterpret_cast<float4x4*>(desc.invViewProjection) = camAttribs.mViewProjInvT.Transpose();
        *reinterpret_cast<float4x4*>(desc.projection) = camAttribs.mProjT.Transpose();
        *reinterpret_cast<float4x4*>(desc.invProjection) = camAttribs.mProjInvT.Transpose();
        *reinterpret_cast<float4x4*>(desc.view) = camAttribs.mViewT.Transpose();
        *reinterpret_cast<float4x4*>(desc.invView) = camAttribs.mViewInvT.Transpose();
        *reinterpret_cast<float4x4*>(desc.prevViewProjection) = camAttribs.mViewProjT.Transpose();

        desc.renderSize = { WINDOW_WIDTH, WINDOW_HEIGHT };
        desc.motionVectorScale = { -0.5f, -0.5f };
        desc.iblFactor = 1.0f; //
        desc.normalUnPackMul = 1.0f;
        desc.normalUnPackAdd = 0.0f;
        desc.roughnessChannel = 0;
        desc.isRoughnessPerceptual = true;
        desc.temporalStabilityFactor = outSettings.TemporalStabilityFactor;
        desc.depthBufferThickness = outSettings.DepthBufferThickness;
        desc.roughnessThreshold = outSettings.RoughnessThreshold;
        desc.varianceThreshold = outSettings.VarianceThreshold;
        desc.maxTraversalIntersections = outSettings.MaxTraversalIntersections;
        desc.minTraversalOccupancy = outSettings.MinTraversalOccupancy;
        desc.mostDetailedMip = outSettings.MostDetailedMip;
        desc.samplesPerQuad = outSettings.SamplesPerQuad;
        desc.temporalVarianceGuidedTracingEnabled = outSettings.TemporalVarianceGuidedTracingEnabled;

        auto ffxResult = ffxSssrContextDispatch(&s_gc.sssrContext, &desc);
        assert(ffxResult == FFX_OK);
    }
    #endif

    Bloom::RenderAttributes BloomRenderAttribs{};
    BloomRenderAttribs.pDevice = s_gc.m_pDevice;
    BloomRenderAttribs.pDeviceContext = s_gc.m_pImmediateContext;
    BloomRenderAttribs.pPostFXContext = s_gc.postFXContext.get();
    BloomRenderAttribs.pColorBufferSRV = s_gc.gBuffer->GetBuffer(GBUFFER_RT_RADIANCE)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    BloomRenderAttribs.pBloomAttribs = &s_gc.bloomSettings;
    s_gc.bloom->Execute(BloomRenderAttribs);

    if (s_gc.ssao)
    {
        HLSL::ScreenSpaceAmbientOcclusionAttribs SSAOSettings{};

        ScreenSpaceAmbientOcclusion::RenderAttributes SSAORenderAttribs{};
        SSAORenderAttribs.pDevice = s_gc.m_pDevice;
        SSAORenderAttribs.pDeviceContext = s_gc.m_pImmediateContext;
        SSAORenderAttribs.pPostFXContext = s_gc.postFXContext.get();
        SSAORenderAttribs.pDepthBufferSRV = pCurrDepthSRV;
        SSAORenderAttribs.pNormalBufferSRV = s_gc.gBuffer->GetBuffer(GBUFFER_RT_NORMAL)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        SSAORenderAttribs.pSSAOAttribs = &SSAOSettings;
        s_gc.ssao->Execute(SSAORenderAttribs);
    }

    s_gc.m_pImmediateContext->SetRenderTargets(1, &pRTVOffscreen, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    // Clear the back buffer
    const float ClearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};
    s_gc.m_pImmediateContext->ClearRenderTarget(pRTVOffscreen, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    s_gc.m_pImmediateContext->SetPipelineState(s_gc.applyPostFX.pPSO);
    s_gc.applyPostFX.ptex2DRadianceVar->Set(s_gc.bloom->GetBloomTextureSRV());
    s_gc.applyPostFX.ptex2DNormalVar->Set(s_gc.gBuffer->GetBuffer(GBUFFER_RT_NORMAL)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
    #if defined(DE_SSR)
    s_gc.applyPostFX.ptex2DSSR->Set(s_gc.ssr->GetSSRRadianceSRV());
    #endif
    #if defined(FX_SSR)
    s_gc.applyPostFX.ptex2DSSR->Set(s_gc.buffers[inFlightIndex].diligentSsrBufferSrv);
    #endif
    s_gc.applyPostFX.ptex2DPecularIBL->Set(s_gc.gBuffer->GetBuffer(GBUFFER_RT_SPECULAR_IBL)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
    s_gc.applyPostFX.ptex2DBaseColorVar->Set(s_gc.gBuffer->GetBuffer(GBUFFER_RT_BASE_COLOR)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
    s_gc.applyPostFX.ptex2DMaterialDataVar->Set(s_gc.gBuffer->GetBuffer(GBUFFER_RT_MATERIAL_DATA)->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
    s_gc.applyPostFX.ptex2DPreintegratedGGXVar->Set(s_gc.pbrRenderer->GetPreintegratedGGX_SRV());
    s_gc.applyPostFX.ptex2DSSAOVar->Set(s_gc.ssao->GetAmbientOcclusionSRV());
    s_gc.m_pImmediateContext->CommitShaderResources(s_gc.applyPostFX.pSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    s_gc.m_pImmediateContext->Draw({3, DRAW_FLAG_VERIFY_ALL});

    if(s_gc.currentScene == Scene::SCENE_TERRAIN)
    {
        //s_gc.m_pImmediateContext->BeginDebugGroup("EpipolarLightScattering");

        HLSL::EpipolarLightScatteringAttribs m_PPAttribs{};

        EpipolarLightScattering::FrameAttribs FrameAttribs;

        //float scale = (69933.0f - 47984.07031f) / (400.0f - 1.0f);
        //float offset = 47984.07031f;
        
        //float scale = (70000.0f - 48000.0f) / (0.96f - 0.982f);
        //float offset = 48000.0f - (0.982 * scale);

        /*
        mProj {1.93137, 0.00, 0.00, 0.00}
                {0.00, 2.41421, 0.00, 0.00}
                {0.00, 0.00, 1.00252, -1515.24634}
                {0.00, 0.00, 1.00, 0.00} 144 float4x4 (column_major)
        */

       /*
           CameraAttribs CamAttribs;
            WriteShaderMatrix(&CamAttribs.mView, m_mCameraView, !m_PackMatrixRowMajor);
            WriteShaderMatrix(&CamAttribs.mProj, m_mCameraProj, !m_PackMatrixRowMajor);
            WriteShaderMatrix(&CamAttribs.mViewProj, mViewProj, !m_PackMatrixRowMajor);
            WriteShaderMatrix(&CamAttribs.mViewProjInv, mViewProj.Inverse(), !m_PackMatrixRowMajor);
            float fNearPlane = 0.f, fFarPlane = 0.f;
            m_mCameraProj.GetNearFarClipPlanes(fNearPlane, fFarPlane, m_pDevice->GetDeviceInfo().NDC.MinZ == -1);
            CamAttribs.fNearPlaneZ      = fNearPlane;
            CamAttribs.fFarPlaneZ       = fFarPlane * 0.999999f;
            CamAttribs.f4Position       = m_f3CameraPos;
            CamAttribs.f4ViewportSize.x = static_cast<float>(m_pSwapChain->GetDesc().Width);
            CamAttribs.f4ViewportSize.y = static_cast<float>(m_pSwapChain->GetDesc().Height);
            CamAttribs.f4ViewportSize.z = 1.f / CamAttribs.f4ViewportSize.x;
            CamAttribs.f4ViewportSize.w = 1.f / CamAttribs.f4ViewportSize.y;
       
       */

        const float WORLD_TO_EARTH_SCALE = 1511.4625f;

        auto adjustedCamAttribs = CamAttribs;
        auto adjustedLightAttribs = LightAttrs;

        //adjustedLightAttribs.f4Direction = float4(-0.554699242f, -0.0599640049f, -0.829887390f, 0.0f);
        //adjustedLightAttribs.f4Direction = float4(0.05742f, 0.99037f, -0.12608f, 0.00f);

        adjustedLightAttribs.f4Direction.w = 0.0f;
        //adjustedLightAttribs.f4Intensity = float4(10.0f, 10.0f, 10.0f, 10.0f);

        //adjustedCamAttribs.f4Position = (adjustedCamAttribs.f4Position * WORLD_TO_EARTH_SCALE);
        adjustedCamAttribs.f4Position = (adjustedCamAttribs.f4Position * 133.0f);
        adjustedCamAttribs.f4Position.w = 1.0f;
        
        //adjustedCamAttribs.f4Position = float4(0.0f, 8000.0f, 0.0f, 1.0f);

        float originalNear = 0.f, originalFar = 0.f;
        adjustedCamAttribs.mProj.GetNearFarClipPlanes(originalNear, originalFar, s_gc.m_pDevice->GetDeviceInfo().NDC.MinZ == -1);

        adjustedCamAttribs.mProj = float4x4::Projection(
            CamAttribs.f4ExtraData[0].x, // FOV
            CamAttribs.f4ViewportSize.x / CamAttribs.f4ViewportSize.y,
            originalNear * -WORLD_TO_EARTH_SCALE,
            originalFar * -WORLD_TO_EARTH_SCALE,
            s_gc.m_pDevice->GetDeviceInfo().NDC.MinZ == -1
        );

        //adjustedCamAttribs.mProj._11 *= WORLD_TO_EARTH_SCALE; // Scale x axis
        //adjustedCamAttribs.mProj._22 *= WORLD_TO_EARTH_SCALE; // Scale y axis
        //adjustedCamAttribs.mProj._33 *= WORLD_TO_EARTH_SCALE; // Scale z axis

        adjustedCamAttribs.mView = adjustedCamAttribs.mView.Transpose();

#if 0
        adjustedCamAttribs.mView = float4x4{
            0.973666370f, 0.0408147201f, 0.224294260f, 0.00000000f,
            0.00000000f, 0.983843684f, -0.179029569f, 0.00000000f,
            -0.227977529f, 0.174315080f, 0.957935572f, 0.00000000f,
            0.00000000f, -7870.74951f, 1432.23657f, 1.00000000f
        };
#endif

#if 0
        adjustedCamAttribs.mProj = float4x4{
            1.93137074f, 0.00000000f, 0.00000000f, 0.00000000f,
            0.00000000f, 2.41421342f, 0.00000000f, 0.00000000f,
            0.00000000f, 0.00000000f, 1.00252461f, 1.00000000f,
            0.00000000f, 0.00000000f, -1515.24634f, 0.00000000f
        };
#endif

        float3 negCamPos = float3(-adjustedCamAttribs.f4Position.x, 
                                -adjustedCamAttribs.f4Position.y,
                                -adjustedCamAttribs.f4Position.z);

        adjustedCamAttribs.mView._41 = negCamPos.x * adjustedCamAttribs.mView._11 + 
                                    negCamPos.y * adjustedCamAttribs.mView._21 + 
                                    negCamPos.z * adjustedCamAttribs.mView._31;
                                    
        adjustedCamAttribs.mView._42 = negCamPos.x * adjustedCamAttribs.mView._12 + 
                                    negCamPos.y * adjustedCamAttribs.mView._22 + 
                                    negCamPos.z * adjustedCamAttribs.mView._32;
                                    
        adjustedCamAttribs.mView._43 = negCamPos.x * adjustedCamAttribs.mView._13 + 
                                    negCamPos.y * adjustedCamAttribs.mView._23 + 
                                    negCamPos.z * adjustedCamAttribs.mView._33;

        //adjustedCamAttribs.mView._41 = 0.0f;
        //adjustedCamAttribs.mView._42 = 7870.74951f;
        //adjustedCamAttribs.mView._43 = 1432.23657f;

        ////adjustedCamAttribs.mProj.m[2][2] = scale;
        //adjustedCamAttribs.mProj.m[3][2] = -1515.24634f;
        //adjustedCamAttribs.mProj.m[2][3] = 1.0f;
        
        // FIXME FIXME FIXME
        //adjustedCamAttribs.f4Position.y = 8000.0f;

        float fNearPlane = 0.f, fFarPlane = 0.f;
        adjustedCamAttribs.mProj.GetNearFarClipPlanes(fNearPlane, fFarPlane, s_gc.m_pDevice->GetDeviceInfo().NDC.MinZ == -1);
        adjustedCamAttribs.fNearPlaneZ      = fNearPlane;
        adjustedCamAttribs.fFarPlaneZ       = fFarPlane * 0.999999f;

        adjustedCamAttribs.mViewProj = (adjustedCamAttribs.mView * adjustedCamAttribs.mProj);
        adjustedCamAttribs.mViewProjInv = (adjustedCamAttribs.mView * adjustedCamAttribs.mProj).Inverse();

        adjustedCamAttribs.mViewInv = adjustedCamAttribs.mView.Inverse();

        adjustedCamAttribs.mProjInv = adjustedCamAttribs.mProj.Inverse();

        FrameAttribs.pDevice        = s_gc.m_pDevice;
        FrameAttribs.pDeviceContext = s_gc.m_pImmediateContext;
        FrameAttribs.dElapsedTime   = std::chrono::duration<double>(std::chrono::steady_clock::now() - s_gc.epoch).count();;
        FrameAttribs.pLightAttribs  = &adjustedLightAttribs;
        FrameAttribs.pCameraAttribs = &adjustedCamAttribs;

        m_PPAttribs.iNumCascades = 2;
        m_PPAttribs.fNumCascades = (float)2.0f;
        m_PPAttribs.iFirstCascadeToRayMarch = 1;

        FrameAttribs.pcbLightAttribs  = nullptr; //s_gc.pcbLightAttribs;
        FrameAttribs.pcbCameraAttribs = nullptr; //s_gc.pcbCameraAttribs;

        m_PPAttribs.fMaxShadowMapStep = static_cast<float>(s_gc.shadowMap->m_ShadowSettings.Resolution / 4);

        m_PPAttribs.f2ShadowMapTexelSize = float2(1.f / static_cast<float>(s_gc.shadowMap->m_ShadowSettings.Resolution), 1.f / static_cast<float>(s_gc.shadowMap->m_ShadowSettings.Resolution));
        m_PPAttribs.uiMaxSamplesOnTheRay = s_gc.shadowMap->m_ShadowSettings.Resolution;

        m_PPAttribs.uiNumSamplesOnTheRayAtDepthBreak = 32u;

        // During the ray marching, on each step we move by the texel size in either horz
        // or vert direction. So resolution of min/max mipmap should be the same as the
        // resolution of the original shadow map
        m_PPAttribs.uiMinMaxShadowMapResolution    = s_gc.shadowMap->m_ShadowSettings.Resolution;
        m_PPAttribs.uiInitialSampleStepInSlice     = std::min(m_PPAttribs.uiInitialSampleStepInSlice, m_PPAttribs.uiMaxSamplesInSlice);
        m_PPAttribs.uiEpipoleSamplingDensityFactor = std::min(m_PPAttribs.uiEpipoleSamplingDensityFactor, m_PPAttribs.uiInitialSampleStepInSlice);

        FrameAttribs.ptex2DSrcColorBufferSRV = s_gc.buffers[inFlightIndex].offscreenColorBuffer->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        FrameAttribs.ptex2DSrcDepthBufferSRV = pCurrDepthSRV;
        FrameAttribs.ptex2DDstColorBufferRTV = pRTV;
        FrameAttribs.ptex2DDstDepthBufferDSV = pDSV;
        FrameAttribs.ptex2DShadowMapSRV      = s_gc.shadowMap->GetShadowMap();

        s_gc.m_pImmediateContext->SetRenderTargets(1, &pRTV, pDSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        s_gc.m_pImmediateContext->ClearRenderTarget(pRTV, ClearColor, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        s_gc.m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.0f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Begin new frame
        //s_gc.epipolarLightScattering->PrepareForNewFrame(FrameAttribs, m_PPAttribs);
        PrepareForNewFrame(s_gc.epipolarLightScattering.get(), FrameAttribs, &m_PPAttribs);

        // Render the sun
        s_gc.epipolarLightScattering->RenderSun(pRTV->GetDesc().Format, pDSV->GetDesc().Format, 1);

        // Perform the post processing
        s_gc.epipolarLightScattering->PerformPostProcessing();

        //s_gc.m_pImmediateContext->EndDebugGroup();
    }

    s_gc.m_pImmediateContext->Flush();
}

void GraphicsUpdate()
{
    if (graphicsMode != toSetGraphicsMode)
    {
        graphicsMode = toSetGraphicsMode;

        std::fill(graphicsPixels.begin(), graphicsPixels.end(), 0);
        std::fill(textPixels.begin(), textPixels.end(), 0);
        std::fill(rotoscopePixels.begin(), rotoscopePixels.end(), ClearPixel);

        modeChangeComplete.release();
    }

    if(s_shouldToggleMenu)
    {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<float>(now - frameSync.uiTriggerTimestamp).count();
        constexpr float menuActivationInSeconds = 0.25f;
        if (duration >= menuActivationInSeconds)
        {
            Magnum::Float timePoint = duration;
            Magnum::Float uiTriggerValue = frameSync.uiTrigger.at(timePoint);

            frameSync.uiTriggerTimestamp = now;

            if (uiTriggerValue >= 1.0f) {
                SDL_ShowCursor(SDL_ENABLE);
                frameSync.uiTrigger = Magnum::Animation::Track<Magnum::Float, Magnum::Float, Magnum::Float>{
                    {{0.0f, 1.0f}, {menuActivationInSeconds, 0.0f}}, // Keyframe data
                    Magnum::Math::lerp,           // Interpolator function
                    Magnum::Animation::Extrapolation::Constant, // Extrapolation before
                    Magnum::Animation::Extrapolation::Constant  // Extrapolation after
                };
            } else {
                SDL_ShowCursor(SDL_DISABLE);
                frameSync.uiTrigger = Magnum::Animation::Track<Magnum::Float, Magnum::Float, Magnum::Float>{
                    {{0.0f, 0.0f}, {menuActivationInSeconds, 1.0f}}, // Keyframe data
                    Magnum::Math::lerp,           // Interpolator function
                    Magnum::Animation::Extrapolation::Constant, // Extrapolation before
                    Magnum::Animation::Extrapolation::Constant  // Extrapolation after
                };
            }
            s_shouldToggleMenu = false;
        }
    }

    int semCount = INT32_MAX;

    uint32_t gameContext = frameSync.gameContext;
    FrameToRender ftr{};

    if (frameSync.maneuvering && !frameSync.inGameOps)
    {
       std::unique_lock<std::mutex> lock(frameSync.mutex);

       while(frameSync.framesToRender.size() == 0 && frameSync.maneuvering)
       {
            frameSync.frameCompleted.wait(lock);
       }

       if (frameSync.maneuvering)
       {
           ftr = frameSync.framesToRender.front();

           ++frameSync.framesToRender.front().renderCount;

           if (frameSync.framesToRender.front().renderCount >= GetFramesPerGameFrame())
           {
               frameSync.stoppedFrame = frameSync.framesToRender.front();
               frameSync.framesToRender.pop_front();
               frameSync.gameTickTimer = 0;
           }
       }
       else
       {
           ftr = frameSync.stoppedFrame;
       }
    }
    else
    {
        std::unique_lock<std::mutex> lock(frameSync.mutex);

        ftr = frameSync.stoppedFrame;
    }
    

    SDL_Texture* currentTexture = NULL;
    uint32_t stride = 0;
    const void* data = nullptr;
    size_t dataSize = 0;

    static std::vector<uint32_t> fullRes{};
    static std::vector<Rotoscope> backbuffer{};

    static std::vector<RotoscopeShader> shaderBackBuffer{};
    static UniformBlock uniform{};

    if (s_gc.shouldInitPlanets)
    {
        // Ship initialization

        const uint32_t shipTextureSize = 512 * 512 * 4; // Assuming 512x512 RGBA images
        const uint32_t totalDataSize = shipTextureSize * shipTextures.size();

        auto shipBuffer = s_gc.vc.create_buffer(
            AVK_STAGING_BUFFER_MEMORY_USAGE,
            vk::BufferUsageFlagBits::eTransferSrc,
            avk::generic_buffer_meta::create_from_size(totalDataSize)
        );

        std::vector<avk::recorded_commands_t> commands{};

        commands.push_back(
            avk::sync::buffer_memory_barrier(shipBuffer.as_reference(),
                avk::stage::auto_stage >> avk::stage::auto_stage,
                avk::access::auto_access >> avk::access::auto_access
            ));

        commands.push_back(avk::sync::image_memory_barrier(s_gc.shipImage->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::undefined, avk::layout::transfer_dst}));

        int i = 0;
        for(auto& textureInfo : shipTextures)
        {
            std::vector<uint8_t> image;
            unsigned width, height;
            unsigned error = lodepng::decode(image, width, height, textureInfo.first, LCT_RGBA, 8);
            if (error) {
                printf("decoder error %d, %s\n", error, lodepng_error_text(error));
                exit(-1);
            }

            commands.push_back(shipBuffer->fill(image.data(), 0, i * shipTextureSize, shipTextureSize));
            commands.push_back(avk::copy_buffer_to_image_layer_mip_level(shipBuffer, s_gc.shipImage->get_image(), i, 0, avk::layout::transfer_dst, vk::ImageAspectFlagBits::eColor, i * shipTextureSize));

            ++i;
        }

        commands.push_back(avk::sync::image_memory_barrier(s_gc.shipImage->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::transfer_dst, avk::layout::shader_read_only_optimal }));

        s_gc.vc.record_and_submit_with_fence(commands, *s_gc.mQueue)->wait_until_signalled();

        // Planet initialization
        commands.clear();

        const uint32_t mapSize = 48 * 24 * 4;
        const uint32_t dataSize = mapSize * s_gc.surfaceData.size();

        auto sb = s_gc.vc.create_buffer(
            AVK_STAGING_BUFFER_MEMORY_USAGE,
            vk::BufferUsageFlagBits::eTransferSrc,
            avk::generic_buffer_meta::create_from_size(dataSize)
        );

        commands.push_back(
            avk::sync::buffer_memory_barrier(sb.as_reference(),
                avk::stage::auto_stage >> avk::stage::auto_stage,
                avk::access::auto_access >> avk::access::auto_access
            ));

        commands.push_back(
            avk::sync::image_memory_barrier(s_gc.planetAlbedoImages->get_image(),
                avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::shader_read_only_optimal, avk::layout::transfer_dst }));

        i = 0;
        for(auto& ps : s_gc.surfaceData)
        {
            commands.push_back(sb->fill(ps.second.albedo.data(), 0, i * mapSize, mapSize));
            commands.push_back(avk::copy_buffer_to_image_layer_mip_level(sb, s_gc.planetAlbedoImages->get_image(), i, 0, avk::layout::transfer_dst, vk::ImageAspectFlagBits::eColor, i * mapSize));

            s_gc.seedToIndex.emplace(ps.first, i);

            ++i;
        }

        commands.push_back(avk::sync::image_memory_barrier(s_gc.planetAlbedoImages->get_image(),
            avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::transfer_dst, avk::layout::shader_read_only_optimal }));

        s_gc.vc.record_and_submit_with_fence(commands, *s_gc.mQueue)->wait_until_signalled();

        s_gc.shouldInitPlanets = false;
        s_gc.planetsDone.release();
    }

    if (fullRes.size() == 0)
    {
        fullRes.resize(WINDOW_WIDTH * WINDOW_HEIGHT);
        backbuffer.resize(GRAPHICS_MODE_WIDTH * GRAPHICS_MODE_HEIGHT);
        shaderBackBuffer.resize(GRAPHICS_MODE_WIDTH * GRAPHICS_MODE_HEIGHT);

        uniform.graphics_mode_width = GRAPHICS_MODE_WIDTH;
        uniform.graphics_mode_height = GRAPHICS_MODE_HEIGHT;
        uniform.window_width = WINDOW_WIDTH;
        uniform.window_height = WINDOW_HEIGHT;

        s_gc.epipolarLightScattering->OnWindowResize(s_gc.m_pDevice, WINDOW_WIDTH, WINDOW_HEIGHT);
    }

    uniform.useEGA = s_useEGA ? 1 : 0;
    uniform.useRotoscope = s_useRotoscope ? 1 : 0;
    uniform.iTime = std::chrono::duration<float>(std::chrono::steady_clock::now() - s_gc.epoch).count();
    uniform.game_context = gameContext;
    uniform.alienVar1 = s_alienVar1;
    uniform.adjust = s_adjust;
    uniform.planetSize = frameSync.currentPlanetSphereSize;
    uniform.nebulaBase = 0.0f;
    uniform.nebulaMultiplier = 50.0f;
    uniform.orbitMask = ftr.orbitMask;
    uniform.zoomLevel = 1.0f;

    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration<float>(now - frameSync.uiTriggerTimestamp).count();
    uniform.menuVisibility = frameSync.uiTrigger.at(static_cast<Magnum::Float>(duration));
    uniform.blurAmount = 20.0f; // emulationThreadRunning ? 20.0f : 0.0f;
    
    // If we're in a system, nebulas behave a little differently
    if (uniform.game_context == 1 || uniform.game_context == 2)
    {
        uniform.nebulaBase = frameSync.inNebula ? 2.0f : 0.0f;
        uniform.nebulaMultiplier = 5.0f;
    }

    bool hasNavigation = false;
    bool hasAuxSysPixel = false;
    bool hasStarMap = false;
    bool hasPortPixel = false;
    static uint32_t activeAlien = 0;

    std::vector<avk::recorded_commands_t> commands{};

    auto setActiveAlien = [&](uint32_t newAlien){
        const static std::unordered_map<uint32_t, std::string> aliens = {
            { 9 , "mechan"}
        };

        if(activeAlien != newAlien)
        {
            const uint32_t dataSize = 512 * 512 * 4;

            std::array<avk::image_sampler, 3> alienImgs = {
                s_gc.alienColorImage,
                s_gc.alienDepthImage,
                s_gc.alienBackgroundImage
            };

            std::array<std::string, 3> files = {
                "npc.png",
                "depth.png",
                "background.png"
            };

            for(int i = 0; i < 3; i++) {
                auto sb = s_gc.vc.create_buffer(
                    AVK_STAGING_BUFFER_MEMORY_USAGE,
                    vk::BufferUsageFlagBits::eTransferSrc,
                    avk::generic_buffer_meta::create_from_size(dataSize)
                );

                std::vector<uint8_t> image;
                unsigned width, height;

                std::string filename = "mechan/" + files[i];

                unsigned error = lodepng::decode(image, width, height, filename, LCT_RGBA, 8);
                if (error)
                {
                    printf("decoder error %d, %s loading %s\n", error, lodepng_error_text(error), filename.c_str());
                    exit(-1);
                }

                commands.push_back(
                    avk::sync::image_memory_barrier(alienImgs[i]->get_image(),
                        avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::shader_read_only_optimal, avk::layout::transfer_dst }));

                sb->fill(image.data(), 0, 0, dataSize),

                commands.push_back(
                    avk::sync::buffer_memory_barrier(sb.as_reference(),
                        avk::stage::auto_stage >> avk::stage::auto_stage,
                        avk::access::auto_access >> avk::access::auto_access
                    ));

                commands.push_back(avk::copy_buffer_to_image(sb, alienImgs[i]->get_image(), avk::layout::transfer_dst));

                commands.push_back(
                    avk::sync::image_memory_barrier(alienImgs[i]->get_image(),
                        avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::transfer_dst, avk::layout::shader_read_only_optimal }));

            }
            
            activeAlien = newAlien;
        }
    };

    // Choose the correct texture based on the current mode
    if (graphicsMode == SFGraphicsMode::Graphics)
    {
#if 0
        currentTexture = graphicsTexture;
        stride = GRAPHICS_MODE_WIDTH;
        data = graphicsPixels.data() + graphicsDisplayOffset;
#else
        {
            std::lock_guard<std::mutex> lg(rotoscopePixelMutex);

            vec2<int32_t> auxSysTL = { INT32_MAX, INT32_MAX };
            vec2<int32_t> auxSysBR = { INT32_MIN, INT32_MIN };

            vec2<int32_t> starMapTL = { INT32_MAX, INT32_MAX };
            vec2<int32_t> starMapBR = { INT32_MIN, INT32_MIN };

            for (int i = 0; i < GRAPHICS_MODE_WIDTH * GRAPHICS_MODE_HEIGHT; ++i)
            {
#if defined(USE_CPU_RASTERIZATION)
                backbuffer[i] = rotoscopePixels[i];
#else
                shaderBackBuffer[i] = rotoscopePixels[i];

                if (shaderBackBuffer[i].content == NavigationalPixel)
                {
                    hasNavigation = true;
                    shaderBackBuffer[i].navMask = 0xff;
                }

                if (shaderBackBuffer[i].content == AuxSysPixel)
                {
                    hasAuxSysPixel = true;
                    int currentX = i % GRAPHICS_MODE_WIDTH;
                    int currentY = i / GRAPHICS_MODE_WIDTH;
                    auxSysTL.x = (std::min)(auxSysTL.x, currentX);
                    auxSysTL.y = (std::min)(auxSysTL.y, currentY);
                    auxSysBR.x = (std::max)(auxSysBR.x, currentX);
                    auxSysBR.y = (std::max)(auxSysBR.y, currentY);
                }

                if (shaderBackBuffer[i].content == StarMapPixel)
                {
                    hasStarMap = true;
                    int currentX = i % GRAPHICS_MODE_WIDTH;
                    int currentY = i / GRAPHICS_MODE_WIDTH;
                    starMapTL.x = (std::min)(starMapTL.x, currentX);
                    starMapTL.y = (std::min)(starMapTL.y, currentY);
                    starMapBR.x = (std::max)(starMapBR.x, currentX);
                    starMapBR.y = (std::max)(starMapBR.y, currentY);
                }

                if(shaderBackBuffer[i].content == RunBitPixel)
                {
                    switch(shaderBackBuffer[i].runBitData.tag)
                    {
                        case 9: // MECHAN 9
                            setActiveAlien(shaderBackBuffer[i].runBitData.tag);
                            break;
                        case 44: // Spaceport
                            hasPortPixel = true;
                            break;
                        default:
                        break;
                    }
                }
#endif
            }

            auto processPixelType = [&](PixelContents pixelType, vec2<int32_t>& tl, vec2<int32_t>& br) {
                int width = br.x - tl.x + 1;
                int height = br.y - tl.y + 1;

                for (int i = 0; i < GRAPHICS_MODE_WIDTH * GRAPHICS_MODE_HEIGHT; ++i) {
                    int currentX = i % GRAPHICS_MODE_WIDTH;
                    int currentY = i / GRAPHICS_MODE_WIDTH;

                    if (currentX >= tl.x && currentX <= br.x && currentY >= tl.y && currentY <= br.y) {
                        shaderBackBuffer[i].content = pixelType;
                        shaderBackBuffer[i].blt_x = currentX - tl.x;
                        shaderBackBuffer[i].blt_y = currentY - tl.y;
                        shaderBackBuffer[i].blt_w = width;
                        shaderBackBuffer[i].blt_h = height;
                    }
                }
            };

            if (hasAuxSysPixel) {
                processPixelType(AuxSysPixel, auxSysTL, auxSysBR);
            }

            if (hasStarMap) {
                processPixelType(StarMapPixel, starMapTL, starMapBR);
            }
        }

#if 0
        static int frameCount = 0;
        std::string filename = "frames/frame_" + std::to_string(frameCount++) + ".png";
        std::vector<unsigned char> png;
        unsigned width = GRAPHICS_MODE_WIDTH, height = GRAPHICS_MODE_HEIGHT;
        std::vector<unsigned char> image;
        image.resize(width * height * 4);
        for (unsigned y = 0; y < height; y++)
        {
            for (unsigned x = 0; x < width; x++)
            {
                uint32_t pixel = graphicsPixels[y * width + x];
                image[4 * width * y + 4 * x + 0] = (pixel >> 16) & 0xFF; // R
                image[4 * width * y + 4 * x + 1] = (pixel >> 8) & 0xFF; // G
                image[4 * width * y + 4 * x + 2] = pixel & 0xFF; // B
                image[4 * width * y + 4 * x + 3] = 255; // A
            }
        }
        unsigned error = lodepng::encode(filename, image, width, height);
        if (error)
        {
            printf("encoder error %u: %s\n", error, lodepng_error_text(error));
        }
#endif    

#if defined(USE_CPU_RASTERIZATION)
        DoRotoscope(fullRes, backbuffer, icons);
#endif
        currentTexture = windowTexture;
        stride = WINDOW_WIDTH;
        data = fullRes.data();
        dataSize = fullRes.size() * sizeof(uint32_t);
#endif
    }
    else if (graphicsMode == SFGraphicsMode::Text)
    {
        currentTexture = textTexture;
        stride = TEXT_MODE_WIDTH;
        data = textPixels.data();
        dataSize = textPixels.size() * sizeof(uint32_t);
    }

    if (uniform.menuVisibility > 0.0f)
    {
        DrawUI();
    }

    s_gc.vc.sync_before_render();

    auto imageAvailableSemaphore = s_gc.vc.consume_current_image_available_semaphore();
    const auto inFlightIndex = s_gc.vc.in_flight_index_for_frame();

    s_gc.buffers[inFlightIndex].command->reset();

    const int framesPerGameFrame = GetFramesPerGameFrame();
    
    float interpolationFactor = (float)ftr.renderCount / (float)framesPerGameFrame;

    if (hasPortPixel)
    {
        UpdateStation(inFlightIndex);
        RenderStation(inFlightIndex);
    }

    if (hasNavigation)
    {
        float secondsElapsedIn = (std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - frameSync.maneuveringStartTime
        ).count()) / 1000.0f;

        float secondsElapsedOut = (std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - frameSync.maneuveringEndTime
        ).count()) / 1000.0f;

        if (frameSync.maneuvering || secondsElapsedOut < 1.0f)
        {
            uint8_t navMaskValue = 0xff;
            if (secondsElapsedIn <= 1.0) {
                navMaskValue = static_cast<uint8_t>(secondsElapsedIn * 255);
            } else if (secondsElapsedOut <= 1.0) {
                // Ensure that secondsElapsedOut is used only after secondsElapsedIn exceeds 1.0
                navMaskValue = static_cast<uint8_t>((1.0f - secondsElapsedOut) * 255);
            }

            for (int i = 0; i < GRAPHICS_MODE_WIDTH * GRAPHICS_MODE_HEIGHT; ++i)
            {
                if (shaderBackBuffer[i].content == NavigationalPixel)
                {
                    shaderBackBuffer[i].navMask = 0xff;
                }
                else
                {
                    shaderBackBuffer[i].navMask = navMaskValue;
                }
            }

            struct SolidRect
            {
                int x0;
                int y0;
                int x1;
                int y1;

                uint32_t chromaKey;
            };

            const std::vector<SolidRect> rects = {
                { 12, 0, 68, 7, 0xaaaaaa },
                { 83, 2, 157, 74, 0x0 },
                { 3, 148,157, 198, 0x0 },
            };

            for(int i = 0; i < rects.size(); ++i)
            {
                auto& rect = rects[i];
                for (int y = rect.y0; y < rect.y1; ++y) {
                    for (int x = rect.x0; x < rect.x1; ++x) {
                        int index = y * GRAPHICS_MODE_WIDTH + x;
                        shaderBackBuffer[index].navMask = i == 1 ? 0x8000 : 0xff00;
                        shaderBackBuffer[index].navMask += navMaskValue;
                        shaderBackBuffer[index].chromaKey = rect.chromaKey;
                    }
                }
            }
        }

        auto cat = calculateHeadingAndThrust(ftr.deadReckoning, frameSync.shipHeading, frameSync.thrust);
        frameSync.shipHeading = cat.heading;
        frameSync.thrust = cat.thrust;

        uniform.heading = cat.heading;
        uniform.thrust = cat.thrust;

        uniform.deadX = (float)ftr.deadReckoning.x * interpolationFactor;
        uniform.deadY = (float)ftr.deadReckoning.y * interpolationFactor;

        uniform.worldX = (float)ftr.worldCoord.x + uniform.deadX;
        uniform.worldY = (float)ftr.worldCoord.y + uniform.deadY;

        uniform.screenX = ftr.screenCoord.x;
        uniform.screenY = ftr.screenCoord.y;

        printf("Frame: x %d y %d, deadx %f deady %f xd %f yd %f\n", ftr.worldCoord.x, ftr.worldCoord.y,
            uniform.deadX, uniform.deadY,
            uniform.worldX,
            uniform.worldY);

    }

    if (!emulationThreadRunning)
    {
        if (frameSync.demoMode == 0)
        {
            auto titleCommands = TitlePass(inFlightIndex, uniform);
            commands.insert(commands.end(), titleCommands.begin(), titleCommands.end());
        }
        else
        {
            auto planetCommands = DemoPass(inFlightIndex, uniform);
            commands.insert(commands.end(), planetCommands.begin(), planetCommands.end());
        }
    }
    else if (graphicsMode == SFGraphicsMode::Text)
    {
        auto textCommands = TextPass(inFlightIndex, uniform, data, dataSize);
        commands.insert(commands.end(), textCommands.begin(), textCommands.end());
    }
    else
    {
#if defined(USE_CPU_RASTERIZATION)
        auto cpuCommands = CPUCopyPass(inFlightIndex, data, dataSize);
        commands.insert(commands.end(), cpuCommands.begin(), cpuCommands.end());
#else
        avk::compute_pipeline navPipeline{};
        avk::compute_pipeline orreryPipeline{};
        avk::compute_pipeline starmapPipeline{};

        IconUniform<64> ic{};
        IconUniform<64> orrery{};
        IconUniform<700> starmap{};

        // ( 0 = planet surface, 1=orbit, 2=system)         
        // ( 3 = hyperspace, 4 = encounter, 5 = starport)

        switch (frameSync.gameContext)
        {
            case 0: // planet surface
            case 2: // system
            case 3: // hyperspace
            case 5: // starport
                {
                    ic = IconUniform<64>(ftr.iconList);
                    for (int i = 0; i < 32; ++i)
                    {
                        if (ic.icons[i].isActive)
                        {
                            ShaderIcon& si = ic.icons[i];

                            if (si.id >= 27 && si.id <= 34)
                                continue;

                            si.screenX -= uniform.deadX * 4.0f;
                            si.screenY += uniform.deadY * 6.0f;

                            si.bltX -= uniform.deadX * 4.0f;
                            si.bltY += uniform.deadY * 6.0f;
                        }
                    }

                    if (hasAuxSysPixel)
                    {
                        auto sol = ftr.solarSystem;

                        if (ftr.iconList.size() > 0)
                        {
                            // Put the ship in the icon list
                            auto ship = ftr.iconList.back();
                            ship.y = -ship.y;
                            sol.push_back(ship);
                        }

                        // Solar system orrery
                        orreryPipeline = s_gc.orreryPipeline;
                        orrery = IconUniform<64>(sol);
                    }

                    navPipeline = s_gc.navigationPipeline;
                }
                break;
            case 1: // orbit
                {
                    for (auto& i : ftr.iconList)
                    {
                        if (i.seed == frameSync.currentPlanet)
                        {
                            ic = IconUniform<64>(i);
                            ic.icons[0].screenX = 36;
                            ic.icons[0].screenY = 61;
                            ic.icons[0].bltX = 36;
                            ic.icons[0].bltY = 66;

                            break;
                        }
                    }

                    navPipeline = s_gc.orbitPipeline;
                    auto status = frameSync.GetOrbitStatus();

                    uniform.planetSize = status.apparentSphereSize;
                    uniform.orbitCamX = status.camPos.x;
                    uniform.orbitCamY = status.camPos.y;
                    uniform.orbitCamZ = status.camPos.z;

                    if (frameSync.orbitState == OrbitState::Insertion || frameSync.orbitState == OrbitState::Orbiting || frameSync.orbitState == OrbitState::Landing)
                    {
                        uniform.worldX -= (status.camPos.x - frameSync.staringPos.x) * 2.0f;
                        uniform.worldY -= (status.camPos.y - frameSync.staringPos.y) * 10.0f;
                    }

                    if (frameSync.orbitState == OrbitState::Landing)
                    {
                        // Freeze time
                        uniform.iTime = std::chrono::duration<float>(frameSync.orbitTimestamp - s_gc.epoch).count();
                    }
                }
                break;
            case 4: // encounter
                {
                    vec2<float> arena{0.0f, 0.0f};

                    auto combatLocale = ftr.iconList;

                    for (Icon& icon : combatLocale)
                    {
                        float distX = std::abs(icon.x);
                        float distY = std::abs(icon.y);

                        if (distX > arena.x) arena.x = distX;
                        if (distY > arena.y) arena.y = distY;
                    }

                    // Orig area is 9 x 15

                    const vec2<float> smallestArena = { 16.0f, 20.0f };

                    float zoomLevel = 12.0f; // Default zoom level when everything fits in the smallest area

                    //printf("Current Arena Size: Width = %f, Height = %f zoomlevel %f\n", currentArenaWidth, currentArenaHeight, zoomLevel);

                    const float scale = 1.0f;

                    vec2<float> shipAt = {};

                    std::vector<Icon> dummies;

                    for (Icon& icon : combatLocale)
                    {
                        if(icon.inst_type == SF_INSTANCE_SHIP_COMBAT)
                        {
                            icon.x += uniform.deadX;
                            icon.y += uniform.deadY;

                            icon.x *= scale;
                            icon.y *= -scale;

                            shipAt = {icon.x, icon.y};
                        }
                        else if(icon.inst_type == SF_INSTANCE_VESSEL)
                        {
                            // Debris doesn't move.
                            if (icon.species != 24)
                            {
                                auto vesselIt = frameSync.combatTheatre.find(icon.iaddr);
                                if (vesselIt != frameSync.combatTheatre.end())
                                {
#if 1
                                    {
                                        Icon dummy = icon;
                                        dummy.x = scale * dummy.x;
                                        dummy.y = scale * -dummy.y;
                                        dummy.screenX = dummy.x;
                                        dummy.screenY = dummy.y;

                                        dummy.bltX = dummy.x;
                                        dummy.bltY = dummy.y;
                                        dummies.push_back(dummy);
                                    }
#endif

                                    auto& interp = vesselIt->second.interp;

                                    auto shipPos = interp->interpolate((float)frameSync.completedFrames);
                                    vesselIt->second.lastKnownPoint.emplace(shipPos);

                                    icon.x = scale * shipPos.position.x;
                                    icon.y = scale * -shipPos.position.y;
                                    icon.vesselHeading = shipPos.heading;
                                }
                            }
                            else
                            {
                                icon.x *= scale;
                                icon.y *= -scale;
                            }
                        }

                        icon.screenX = icon.x;
                        icon.screenY = icon.y;

                        icon.bltX = icon.x;
                        icon.bltY = icon.y;
                    }

                    uniform.zoomLevel = zoomLevel;

                    uniform.worldX = shipAt.x * zoomLevel;
                    uniform.worldY = shipAt.y * zoomLevel;

                    for (const auto& missile : ftr.missiles) {

                        auto vesselIt = frameSync.combatTheatre.find(missile.nonce);
                        if (vesselIt != frameSync.combatTheatre.end())
                        {
                            auto& interp = vesselIt->second.interp;

                            auto misPos = interp->interpolate((float)frameSync.completedFrames);

                            Icon missileIcon;
                            missileIcon.x = scale * misPos.position.x;
                            missileIcon.y = scale * -misPos.position.y;
                            missileIcon.screenX = missileIcon.x;
                            missileIcon.screenY = missileIcon.y;
                            missileIcon.bltX = missileIcon.x;
                            missileIcon.bltY = missileIcon.y;
                            missileIcon.clr = missile.mr.morig == 1 ? 0x1 : 0x4;
                            missileIcon.id = 252;
                            missileIcon.vesselHeading = misPos.heading;

                            combatLocale.push_back(missileIcon);
                        }
                    }

                    for(const auto& laser : frameSync.lasers) {
                        Icon laserIcon;

                        laserIcon.x = scale * laser.x0;
                        laserIcon.y = scale * -laser.y0;
                        laserIcon.screenX = laserIcon.x;
                        laserIcon.screenY = laserIcon.y;
                        laserIcon.bltX = laserIcon.x;
                        laserIcon.bltY = laserIcon.y;

                        auto targetIconIt = std::find_if(ftr.iconList.begin(), ftr.iconList.end(), [&](const auto& icon) {
                            return icon.x == laser.x1 && icon.y == laser.y1;
                        });

                        bool shipFound = false;

                        if (targetIconIt != ftr.iconList.end()) {
                            auto targetVesselIt = frameSync.combatTheatre.find(targetIconIt->iaddr);
                            if (targetVesselIt != frameSync.combatTheatre.end() && targetVesselIt->second.lastKnownPoint.has_value()) {
                                // Adjust the target of the laser
                                laserIcon.x1 = scale *  targetVesselIt->second.lastKnownPoint.value().position.x;
                                laserIcon.y1 = scale * -targetVesselIt->second.lastKnownPoint.value().position.y;
                                shipFound = true;
                            }
                        }
                        
                        if(!shipFound) {
                            // No icon found, improvise
                            laserIcon.x1 = scale * laser.x1;
                            laserIcon.y1 = scale * -laser.y1;
                        }

                        laserIcon.clr = laser.color;
                        laserIcon.id = 251;

                        combatLocale.push_back(laserIcon);
                    }

                    for(const auto& explo : frameSync.explosions) {
                        Icon expIcon;

                        if(explo.targetsPlayer)
                        {
                            // Will need to align somewhere along the spacecraft somewhere.
                            expIcon.x = shipAt.x;
                            expIcon.y = shipAt.y;
                        }
                        else
                        {
                            expIcon.x = scale * explo.worldLocation.x;
                            expIcon.y = scale * -explo.worldLocation.y;
                        }

                        expIcon.screenX = expIcon.x;
                        expIcon.screenY = expIcon.y;

                        expIcon.bltX = expIcon.x;
                        expIcon.bltY = expIcon.y;

                        expIcon.clr = 4;
                        expIcon.id = 250;

                        float secondsSinceExplosion = std::chrono::duration<float>(std::chrono::steady_clock::now() - explo.timestamp).count();
                        expIcon.vesselHeading = secondsSinceExplosion;

                        combatLocale.push_back(expIcon);
                    }

#if 0
                    for (const auto& dummy : dummies) {
                        combatLocale.push_back(dummy);
                    }
#endif

                    ic = IconUniform<64>(combatLocale);
                    navPipeline = s_gc.encounterPipeline;
                }
                break;
            default:
                assert(false);
                break;
        }

        if (hasStarMap)
        {
            starmapPipeline = s_gc.starmapPipeline;

            starmap = IconUniform<700>(ftr.starMap.starmap);
            uniform.worldX = ftr.starMap.offset.x;
            uniform.worldY = ftr.starMap.offset.y;

            uniform.screenX = ftr.starMap.window.x;
            uniform.screenY = ftr.starMap.window.y;

            uniform.deadX = (int16_t)Read16(0xe542);
            uniform.deadY = (int16_t)Read16(0xe546);

            uniform.orbitCamX = (int16_t)Read16(0xE54A);
            uniform.orbitCamY = (int16_t)Read16(0xE54E);
        }

        auto gpuCommands = GPURotoscope(inFlightIndex,
            s_gc.buffers[inFlightIndex].gameOutput,
            uniform,
            ic,
            orrery,
            starmap,
            shaderBackBuffer,
            navPipeline,
            orreryPipeline,
            starmapPipeline);

        commands.insert(commands.end(), gpuCommands.begin(), gpuCommands.end());

        if (frameSync.takeScreenshot)
        {
            std::vector<avk::recorded_commands_t> res{};

            auto tempCombinedImageSampler = s_gc.vc.create_image_sampler(
                s_gc.vc.create_image_view(
                    s_gc.vc.create_image(WINDOW_WIDTH, WINDOW_HEIGHT, vk::Format::eR8G8B8A8Unorm, 1, avk::memory_usage::device, avk::image_usage::general_image | avk::image_usage::shader_storage)
                ),
                s_gc.vc.create_sampler(avk::filter_mode::bilinear, avk::border_handling_mode::clamp_to_edge));

            auto readbackBuffer = s_gc.vc.create_buffer(
                AVK_STAGING_BUFFER_MEMORY_USAGE,
                vk::BufferUsageFlagBits::eTransferDst,
                avk::generic_buffer_meta::create_from_size(WINDOW_WIDTH * WINDOW_HEIGHT * 4)
            );

            auto screenshot = GPURotoscope(inFlightIndex,
                tempCombinedImageSampler,
                uniform,
                ic,
                orrery,
                starmap,
                shaderBackBuffer,
                navPipeline,
                orreryPipeline,
                starmapPipeline);

            screenshot.push_back(
                avk::sync::image_memory_barrier(tempCombinedImageSampler->get_image(),
                    avk::stage::auto_stage >> avk::stage::auto_stage).with_layout_transition({ avk::layout::read_only_optimal, avk::layout::transfer_src }));

            screenshot.push_back(
                avk::copy_image_to_buffer(tempCombinedImageSampler->get_image(), avk::layout::transfer_src, vk::ImageAspectFlagBits::eColor, readbackBuffer)
            );

            s_gc.vc.record_and_submit_with_fence(screenshot, *s_gc.mQueue)->wait_until_signalled();

            auto address = readbackBuffer->map_memory(avk::mapping_access::read);
            serializedSnapshot.resize(0);
            lodepng::encode(serializedSnapshot, static_cast<unsigned char*>(address.get()), WINDOW_WIDTH, WINDOW_HEIGHT, LCT_RGBA, 8);
            
            frameSync.takeScreenshot = false;
            frameSync.screenshotSemaphore.release();
        }
#endif
    }

    auto compositorCommands = GPUCompositor(inFlightIndex, uniform, nk_surface->pixels, WINDOW_WIDTH * WINDOW_HEIGHT * 4);

    commands.insert(commands.end(), compositorCommands.begin(), compositorCommands.end());

    s_gc.vc.record(std::move(commands))
    .into_command_buffer(s_gc.buffers[inFlightIndex].command)
    .then_submit_to(*s_gc.mQueue)
    // Do not start to render before the image has become available:
    .waiting_for(imageAvailableSemaphore >> avk::stage::color_attachment_output)
    .submit();

    s_gc.m_pImmediateContext->FinishFrame();

    s_gc.vc.render_frame();

    int numkeys = 0;
    auto keys = SDL_GetKeyboardState(&numkeys);
    assert(numkeys < sizeof(s_keyboardState));
    memcpy(s_keyboardState, keys, numkeys);

    SDL_PumpEvents();
    keyboard->update();

    //if (!keyboard->areArrowKeysDown())
    //{
    //    std::lock_guard<std::mutex> lg(s_deadReckoningMutex);
    //    s_deadReckoning = { 0, 0 };
    //}

    if (graphicsIsShutdown)
        return;


#if defined(ENABLE_OFFSCREEN_VIDEO_RENDERER)
    if(graphicsMode == SFGraphicsMode::Graphics)
    {
        data = (uint8_t*)graphicsPixels.data() + 0x20000;

        stride = GRAPHICS_MODE_WIDTH;

        SDL_UpdateTexture(offscreenTexture, NULL, data, stride * sizeof(uint32_t));
        SDL_RenderClear(offscreenRenderer);
        SDL_RenderCopy(offscreenRenderer, offscreenTexture, NULL, NULL);
        SDL_RenderPresent(offscreenRenderer);
    }
#endif

    {
        std::lock_guard<std::mutex> lock(frameSync.mutex);
        frameSync.completedFrames++;
        frameSync.completedFramesPerGameFrame++;
    }
}

void GraphicsWait()
{
    SDL_Event event;

    while(1)
    {
        SDL_WaitEvent(&event);
        switch (event.type)
        {
            case SDL_QUIT:
                return;
        }
    }
}

void GraphicsQuit()
{
    if (graphicsIsShutdown)
        return;

    StopEmulationThread();

    StopSpeech();

    if(std::this_thread::get_id() != graphicsThread.get_id())
    {
        if(graphicsThread.joinable())
        {
            stopSemaphore.release();

            s_gc.vc.device().waitIdle();

            graphicsThread.join();
        }
    }

#if defined(ENABLE_OFFSCREEN_VIDEO_RENDERER)
    SDL_DestroyRenderer(offscreenRenderer);
    SDL_DestroyWindow(offscreenWindow);
    SDL_DestroyTexture(offscreenTexture);
#endif
    
    SDL_DestroyTexture(graphicsTexture);
    SDL_DestroyTexture(textTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    graphicsIsShutdown = true;

    s_graphicsShutdown.release();
}

void GraphicsSetCursor(int x, int y)
{
    cursorx = x;
    cursory = y;
}

void GraphicsCarriageReturn()
{
    cursorx = 0;
    cursory++;
}

void GraphicsChar(unsigned char s)
{
    if (graphicsMode != Text)
    {   
        //assert(false);
        return;
    }
    
    for(int jj=0; jj<8; jj++)
    {
        int offset = ((int)s)*8 + jj;
        for(int ii=0; ii<8; ii++)
        {
            int color = 0;
            if ((vgafont8[offset]) & (1<<(7-ii)))
            {
                color = 0xFFFFFFFF;
            }

            textPixels[(cursory*8+jj) * TEXT_MODE_WIDTH + (cursorx*8+ii)] = color;
        }
    }

    cursorx++;
    if (cursorx >=80)
    {
        GraphicsCarriageReturn();
    }
}

void GraphicsText(char *s, int n)
{
    for(int i=0; i<n; i++)
    {
        GraphicsChar(s[i]);
    }
}

// 0 = text, 1 = ega graphics
void GraphicsMode(int mode)
{
    if (toSetGraphicsMode == mode)
        return;

    toSetGraphicsMode = (SFGraphicsMode)mode;

    modeChangeComplete.acquire();
}

void WaitForVBlank()
{
    graphicsRetrace.lock();
    graphicsRetrace.unlock();
}

void GraphicsClear(int color, uint32_t offset, int byteCount)
{
    std::lock_guard<std::mutex> lg(rotoscopePixelMutex);
    uint32_t dest = (uint32_t)offset;

    dest <<= 4; // Convert to linear addres
    dest -= 0xa0000; // Subtract from EGA page
    dest *= 4; // Convert to our SDL memory linear address

    uint32_t destOffset = 0;

    auto c = colortable[color&0xF];

    byteCount = 0x2000;

    for(uint32_t i = 0; i < byteCount * 4; ++i)
    {
        graphicsPixels[dest + destOffset + i] = c;
        rotoscopePixels[dest + destOffset + i] = ClearPixel;
    }
}

void GraphicsCopyLine(uint16_t sourceSeg, uint16_t destSeg, uint16_t si, uint16_t di, uint16_t count)
{
    std::lock_guard<std::mutex> lg(rotoscopePixelMutex);

    uint32_t src = (uint32_t)sourceSeg;
    uint32_t dest = (uint32_t)destSeg;

    src <<= 4; // Convert to linear addres
    src -= 0xa0000; // Subtract from EGA page
    src *= 4; // Convert to our SDL memory linear address

    dest <<= 4; // Convert to linear addres
    dest -= 0xa0000; // Subtract from EGA page
    dest *= 4; // Convert to our SDL memory linear address

    uint32_t srcOffset = (uint32_t)si * 4;
    uint32_t destOffset = (uint32_t)di * 4;

    for(uint32_t i = 0; i < count * 4; ++i)
    {
        graphicsPixels[dest + destOffset + i] = graphicsPixels[src + srcOffset + i];
        rotoscopePixels[dest + destOffset + i] = rotoscopePixels[src + srcOffset + i];
    }
}

uint8_t GraphicsPeek(int x, int y, uint32_t offset, Rotoscope* pc)
{
    auto pixel = GraphicsPeekDirect(x, y, offset, pc);

    auto it = std::find(std::begin(colortable), std::end(colortable), pixel);
    assert(it != std::end(colortable));
    return std::distance(std::begin(colortable), it);
}

uint32_t GraphicsPeekDirect(int x, int y, uint32_t offset, Rotoscope* pc)
{
    if(offset == 0)
    {
        assert(false);
        offset = 0xa000;
    }

    offset <<= 4; // Convert to linear addres
    offset -= 0xa0000; // Subtract from EGA page
    offset *= 4; // Convert to our SDL memory linear address

    y = 199 - y;

    if(x < 0 || x >= GRAPHICS_MODE_WIDTH || y < 0 || y >= GRAPHICS_MODE_HEIGHT)
    {
        return colortable[0];
    }

    if(pc)
    {
        *pc = rotoscopePixels[y * GRAPHICS_MODE_WIDTH + x + offset];
    }

    return graphicsPixels[y * GRAPHICS_MODE_WIDTH + x + offset];
}

void GraphicsPixelDirect(int x, int y, uint32_t color, uint32_t offset, Rotoscope pc)
{
    std::lock_guard<std::mutex> lg(rotoscopePixelMutex);

    if(offset == 0)
    {
        assert(false);
        offset = 0xa000;
    }

    offset <<= 4; // Convert to linear addres
    offset -= 0xa0000; // Subtract from EGA page
    offset *= 4; // Conver to our SDL memory linear address

    y = 199 - y;

    if(x < 0 || x >= GRAPHICS_MODE_WIDTH || y < 0 || y >= GRAPHICS_MODE_HEIGHT)
    {
        return;
    }

    if (pc.content == ClearPixel)
    {
        //assert(color == 0);
    }

    pc.argb = color;
    rotoscopePixels[y * GRAPHICS_MODE_WIDTH + x + offset] = pc;

    graphicsPixels[y * GRAPHICS_MODE_WIDTH + x + offset] = color;
}

void GraphicsLine(int x1, int y1, int x2, int y2, int color, int xormode, uint32_t offset)
{
    float x = x1;
    float y = y1;
    float dx = (x2 - x1);
    float dy = (y2 - y1);
    int n = fabs(dx);
    if (fabs(dy) > n) n = fabs(dy);
    if (n == 0) return;
    dx /= n;
    dy /= n;

    Rotoscope rs{};
    rs.content = LinePixel;
    rs.lineData.x0 = x1;
    rs.lineData.x1 = x2;
    rs.lineData.y0 = 199 - y1;
    rs.lineData.y1 = 199 - y2;
    rs.lineData.total = n;
    rs.fgColor = color;

    for(int i=0; i<=n; i++)
    {
        rs.lineData.n = i;

        rs.bgColor = GraphicsPeek(x, y, offset);
        GraphicsPixel(x, y, color, offset, rs);
        x += dx;
        y += dy;
    }
}

void GraphicsPixel(int x, int y, int color, uint32_t offset, Rotoscope pc)
{
    pc.EGAcolor = color & 0xf;
    GraphicsPixelDirect(x, y, colortable[color&0xF], offset, pc);
}

std::unordered_map<char, int> font1_table = {
    {' ', 0x0000}, {'!', 0x4904}, {'"', 0xB400}, {'#', 0xFFFF},
    {'$', 0xF45E}, {'%', 0xA54A}, {'&', 0x0000}, {'\'', 0x4800},
    {'[', 0x2922}, {']', 0x8928}, {'*', 0x1550}, {'+', 0x0BA0},
    {',', 0x0128}, {'-', 0x0380}, {'.', 0x0004}, {'/', 0x2548},
    {'0', 0xF6DE}, {'1', 0x4924}, {'2', 0xE7CE}, {'3', 0xE59E},
    {'4', 0xB792}, {'5', 0xF39E}, {'6', 0xD3DE}, {'7', 0xE524},
    {'8', 0xF7DE}, {'9', 0xF792}, {':', 0x0820}, {';', 0x0828},
    {'<', 0x2A22}, {'=', 0x1C70}, {'>', 0x88A8}, {'?', 0xE584},
    {'@', 0xFFCE}, {'A', 0x57DA}, {'B', 0xD75C}, {'C', 0x7246},
    {'D', 0xD6DC}, {'E', 0xF34E}, {'F', 0xF348}, {'G', 0x7256},
    {'H', 0xB7DA}, {'I', 0xE92E}, {'J', 0x24DE}, {'K', 0xB75A},
    {'L', 0x924E}, {'M', 0xBFDA}, {'N', 0xBFFA}, {'O', 0x56D4},
    {'P', 0xF7C8}, {'Q', 0xF7A6}, {'R', 0xF7EA}, {'S', 0x739C},
    {'T', 0xE924}, {'U', 0xB6DE}, {'V', 0xB6D4}, {'W', 0xB7FA},
    {'X', 0xB55A}, {'Y', 0xB7A4}, {'Z', 0xE54E}
};

std::unordered_map<char, std::array<uint16_t, 3>> font2_table = {
    {' ', {0x0000, 0x0000, 0x0000}}, {'!', {0x0000, 0x0000, 0x0000}},
    {'"', {0x0000, 0x0000, 0x0000}}, {'#', {0x0000, 0x0000, 0x0000}},
    {'$', {0x0000, 0x0000, 0x0000}}, {'%', {0x0000, 0x0000, 0x0000}},
    {'&', {0x0000, 0x0000, 0x0000}}, {'\'', {0x6F00, 0x0000, 0x0000}},
    {'[', {0x0000, 0x0000, 0x0000}}, {']', {0x0000, 0x0000, 0x0000}},
    {'*', {0x0000, 0x0000, 0x0000}}, {'+', {0x0000, 0x0000, 0x0000}},
    {',', {0x0006, 0xF000, 0x0000}}, {'-', {0x0070, 0x0000, 0x0000}},
    {'.', {0x0200, 0x0000, 0x0000}}, {'/', {0x0000, 0x0000, 0x0000}},
    {'0', {0x76F7, 0xBDED, 0xC000}}, {'1', {0x6718, 0xC633, 0xC000}},
    {'2', {0x76C6, 0x6663, 0xE000}}, {'3', {0x76C6, 0x61ED, 0xC000}},
    {'4', {0x35AD, 0x6F98, 0xC000}}, {'5', {0xFE31, 0xE1ED, 0xC000}},
    {'6', {0x76F1, 0xEDED, 0xC000}}, {'7', {0xFEC6, 0x6631, 0x8000}},
    {'8', {0x76F6, 0xEDED, 0xC000}}, {'9', {0x76F6, 0xF1ED, 0xC000}},
    {':', {0x1400, 0x0000, 0x0000}}, {';', {0x0000, 0x0000, 0x0000}},
    {'<', {0x0000, 0x0000, 0x0000}}, {'=', {0x0000, 0x0000, 0x0000}},
    {'>', {0x0000, 0x0000, 0x0000}}, {'?', {0x0000, 0x0000, 0x0000}},
    {'@', {0x0000, 0x0000, 0x0000}}, {'A', {0x3673, 0x9FE7, 0x2000}},
    {'B', {0xEDDE, 0xDDE7, 0x0000}}, {'C', {0x34CC, 0xC430, 0x0000}},
    {'D', {0xEDDD, 0xDDE0, 0x0000}}, {'E', {0xFCCE, 0xCCF0, 0x0000}},
    {'F', {0xFCCE, 0xCCC0, 0x0000}}, {'G', {0x3231, 0x8DA4, 0xC000}},
    {'H', {0xCE73, 0xFCE7, 0x2000}}, {'I', {0xF666, 0x66F0, 0x0000}},
    {'J', {0x3333, 0x3BF0, 0x0000}}, {'K', {0xDDDD, 0xEDD0, 0x0000}},
    {'L', {0xCCCC, 0xCCF0, 0x0000}}, {'M', {0xC71E, 0xF5C7, 0x1C40}},
    {'N', {0xCE7B, 0xBCE7, 0x3900}}, {'O', {0x3273, 0x9CE5, 0xC000}},
    {'P', {0xFDDD, 0xFCC0, 0x0000}}, {'Q', {0x312C, 0xB2D9, 0x2340}},
    {'R', {0xFDDD, 0xEDD0, 0x0000}}, {'S', {0x7CC6, 0x33E0, 0x0000}},
    {'T', {0xF666, 0x6660, 0x0000}}, {'U', {0xCE73, 0x9CE7, 0xE000}},
    {'V', {0xCE73, 0x9CE4, 0xC000}}, {'W', {0xC71C, 0x71D7, 0xF280}},
    {'X', {0xDDD2, 0xDDD0, 0x0000}}, {'Y', {0xDDDD, 0xF660, 0x0000}},
    {'Z', {0xF324, 0xCCF0, 0x0000}}
};

#include <unordered_map>

std::unordered_map<char, std::array<int, 4>> font3_table = {
    {'A', {0x3673, 0x9CFF, 0x39C8, 0x0000}},
    {'B', {0xEDDD, 0xEDDD, 0xE000, 0x0000}},
    {'C', {0x34CC, 0xCCC4, 0x3000, 0x0000}},
    {'D', {0xEDDD, 0xDDDD, 0xE000, 0x0000}},
    {'E', {0xFCCC, 0xECCC, 0xF000, 0x0000}},
    {'F', {0xFCCC, 0xECCC, 0xC000, 0x0000}},
    {'G', {0x3231, 0x8C6F, 0x2930, 0x0000}},
    {'H', {0xCE73, 0x9FE7, 0x39C8, 0x0000}},
    {'I', {0xF666, 0x6666, 0xF000, 0x0000}},
    {'J', {0x3333, 0x333B, 0xF000, 0x0000}},
    {'K', {0xDDDD, 0xDEDD, 0xD000, 0x0000}},
    {'L', {0xCCCC, 0xCCCC, 0xF000, 0x0000}},
    {'M', {0xC71E, 0xF5C7, 0x1C71, 0xC400}},
    {'N', {0xCE7B, 0xBCE7, 0x39C8, 0x0000}},
    {'O', {0x3273, 0x9CE7, 0x2930, 0x0000}},
    {'P', {0xFDDD, 0xDFCC, 0xC000, 0x0000}},
    {'Q', {0x312C, 0xB2CB, 0x2D92, 0x3400}},
    {'R', {0xFDDD, 0xDFED, 0xD000, 0x0000}},
    {'S', {0x7CCC, 0x6333, 0xE000, 0x0000}},
    {'T', {0xF666, 0x6666, 0x6000, 0x0000}},
    {'U', {0xCE73, 0x9CE7, 0x39F8, 0x0000}},
    {'V', {0xCE73, 0x9CE7, 0x3930, 0x0000}},
    {'W', {0xC71C, 0x71C7, 0x1D7A, 0x2800}},
    {'X', {0xDDDD, 0x2DDD, 0xD000, 0x0000}},
    {'Y', {0xDDDD, 0xDF66, 0x6000, 0x0000}},
    {'Z', {0xF332, 0x4CCC, 0xF000, 0x0000}}
};

std::unordered_map<char, int> char_width_table = {
    {' ', 4}, {'!', 4}, {'"', 4}, {'#', 4},
    {'$', 4}, {'%', 4}, {'&', 5}, {'\'', 3},
    {'[', 4}, {']', 4}, {'*', 4}, {'+', 4},
    {',', 3}, {'-', 3}, {'.', 1}, {'/', 4},
    {'0', 5}, {'1', 5}, {'2', 5}, {'3', 5},
    {'4', 5}, {'5', 5}, {'6', 5}, {'7', 5},
    {'8', 5}, {'9', 5}, {':', 1}, {';', 1},
    {'<', 4}, {'=', 4}, {'>', 4}, {'?', 4},
    {'@', 4}, {'A', 5}, {'B', 4}, {'C', 4},
    {'D', 4}, {'E', 4}, {'F', 4}, {'G', 5},
    {'H', 5}, {'I', 4}, {'J', 4}, {'K', 4},
    {'L', 4}, {'M', 6}, {'N', 5}, {'O', 5},
    {'P', 4}, {'Q', 6}, {'R', 4}, {'S', 4},
    {'T', 4}, {'U', 5}, {'V', 5}, {'W', 6},
    {'X', 4}, {'Y', 4}, {'Z', 4}
};

int16_t GraphicsFONT(uint16_t num, uint32_t character, int x1, int y1, int color, int xormode, uint32_t offset)
{
    char c = (char)character;

    Rotoscope rs{};

    rs.content = TextPixel;
    rs.textData.character = c;
    rs.textData.fontNum = num;
    rs.fgColor = color;
    rs.textData.xormode = xormode;

    switch(num)
    {
        case 1:
        {
            auto width = 3;
            auto height = 5;
            auto image = font1_table[c];

            GraphicsBLT(x1, y1, height, width, (const char*)&image, color, xormode, offset, rs);

            return width;
        }
        case 2:
        {
            auto width = char_width_table[c];
            auto height = 7;
            auto image = font2_table[c].data();

            GraphicsBLT(x1, y1, height, width, (const char*)image, color, xormode, offset, rs);

            return width;
        }
        case 3:
        {
            auto width = char_width_table[c];
            auto height = 9;
            auto image = font3_table[c].data();

            GraphicsBLT(x1, y1, height, width, (const char*)image, color, xormode, offset, rs);

            return width;            
        }
        default:
            assert(false);
            break;
    }

    assert(false);
    return 1;
}

void GraphicsBLT(int16_t x1, int16_t y1, int16_t h, int16_t w, const char* image, int color, int xormode, uint32_t offset, Rotoscope pc)
{
    auto img = (const short int*)image;
    int n = 0;

    uint16_t xoffset = 0;
    uint16_t yoffset = 0;

    pc.blt_w = w;
    pc.blt_h = h;

    for(int y=y1; y>y1-h; y--)
    {
        xoffset = 0;

        for(int x=x1; x<x1+w; x++)
        {
            int x0 = x;
            int y0 = y;

            Rotoscope srcPc{};
            bool hasPixel = false;
            auto src = GraphicsPeek(x0, y0, offset, &srcPc);

            pc.blt_x = xoffset;
            pc.blt_y = yoffset;

            if(pc.content == TextPixel)
            {
                pc.bgColor = src;
            }

            if ((*img) & (1<<(15-n)))
            {
                if(xormode) {
                    auto xored = src ^ (color&0xF);

                    if(srcPc.content == TextPixel)
                    {
                        srcPc.bgColor = srcPc.bgColor ^ (color & 0xf);
                        srcPc.fgColor = srcPc.fgColor ^ (color & 0xf);
                        GraphicsPixel(x0, y0, xored, offset, srcPc);
                    }
                    else
                    {
                        GraphicsPixel(x0, y0, xored, offset, pc);
                    }
                }
                else
                {
                    GraphicsPixel(x0, y0, color, offset, pc);
                }
            }
            else
            {
                GraphicsPixel(x0, y0, src, offset, pc);
            }
            
            n++;
            if (n == 16)
            {
                n = 0;
                img++;
            }

            ++xoffset;
        }

        ++yoffset;
    }
}

bool GraphicsHasKey()
{
    return keyboard->checkForKeyStroke();
}

uint16_t GraphicsGetKey()
{
    return keyboard->getKeyStroke();
}

void GraphicsPushKey(uint16_t key)
{
    keyboard->pushKeyStroke(key);
}

void GraphicsSave(char *filename)
{
  FILE *file = fopen(filename, "w");
  if (file == NULL) {
      fprintf(stderr, "Error: Cannot write file\n");
      exit(1);
  }
  printf("Store image %s\n", filename);

  fprintf(file, "P3\n");
  fprintf(file, "%i %i\n", GRAPHICS_MODE_WIDTH, GRAPHICS_MODE_HEIGHT);
  fprintf(file, "255\n");

  for(int j=0; j<GRAPHICS_MODE_HEIGHT; j++)
  {
      for(int i=0; i<GRAPHICS_MODE_WIDTH; i++)
      {
          int c = graphicsPixels[j * GRAPHICS_MODE_WIDTH + i];
          fprintf(file, "%i %i %i ", (c>>16)&0xFF, (c>>8)&0xFF, (c>>0)&0xFF);
      }
      fprintf(file, "\n");
  }
  fclose(file);
}

void GraphicsSaveScreen()
{
    std::stringstream ss;
    {
        cereal::BinaryOutputArchive oarchive(ss);
        oarchive(rotoscopePixels);
    } // Archive goes out of scope, ensuring all contents are flushed into the stringstream

    std::string str = ss.str();
    serializedRotoscope.assign(str.begin(), str.end());

    frameSync.takeScreenshot = true;

    frameSync.screenshotSemaphore.acquire();
}

bool IsGraphicsShutdown()
{
    return graphicsIsShutdown;
}