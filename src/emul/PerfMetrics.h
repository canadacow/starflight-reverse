#pragma once

#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/Query.h"
#include "Graphics/GraphicsTools/interface/DurationQueryHelper.hpp"

#include <unordered_map>
#include <chrono>
#include <memory>
#include <deque>

using namespace Diligent;

class PerformanceMetrics
{
public:

    enum class QueryType
    {
        SceneRender,
        EpipolarLightScattering,
        VolumetricClouds,
        ScreenSpaceReflection,
        ScreenSpaceAmbientOcclusion,
        Bloom,
        UI,
        PostFX,
        Compositing,
        Total
    };

    struct QueryData
    {
        std::unique_ptr<DurationQueryHelper> query;
        double GPUtime;
        double CPUtime;

        std::chrono::steady_clock::time_point startTime;
    };

    struct FrameData
    {
        std::chrono::steady_clock::time_point timestamp;
    };

    std::unordered_map<QueryType, QueryData> queries;

    // FPS tracking
    size_t frameCounter = 0;
    std::chrono::steady_clock::time_point lastFrameTime;
    std::deque<FrameData> frameHistory;
    static constexpr double frameHistoryWindowSeconds = 2.0; // Keep 2 seconds worth of frames
    double averageFPS = 0.0;

    void Initialize(IRenderDevice* pDevice);

    void Mark(const RefCntAutoPtr<IDeviceContext>& pContext, QueryType queryType);
    void End(const RefCntAutoPtr<IDeviceContext>& pContext, QueryType queryType);
    
    // Frame counting and FPS calculation
    void BeginFrame();
    void EndFrame();

    double GetGPUValue(QueryType queryType) const
    {
        return queries.at(queryType).GPUtime;
    }

    double GetCPUValue(QueryType queryType) const
    {
        return queries.at(queryType).CPUtime;
    }
    
    size_t GetFrameCount() const { return frameCounter; }
    double GetAverageFPS();
private:
    void UpdateAverageFPS();
};
