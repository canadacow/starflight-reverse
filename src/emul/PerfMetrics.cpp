#include "PerfMetrics.h"

void PerformanceMetrics::Initialize(IRenderDevice* pDevice)
{
    // Automatically initialize all query types
    for (int i = static_cast<int>(QueryType::SceneRender); 
         i <= static_cast<int>(QueryType::Total); 
         i++)
    {
        auto queryType = static_cast<QueryType>(i);
        queries[queryType].query = std::make_unique<DurationQueryHelper>(pDevice, 5);
    }
    
    // Initialize frame counter
    frameCounter = 0;
    lastFrameTime = std::chrono::steady_clock::now();
    frameHistory.clear();
    averageFPS = 0.0;
}

void PerformanceMetrics::Mark(const RefCntAutoPtr<IDeviceContext>& pContext, QueryType queryType)
{
    auto& query = queries[queryType];
    query.query->Begin(pContext);
    query.startTime = std::chrono::steady_clock::now();

    if(queryType == QueryType::Total)
    {
        BeginFrame();
    }
}

void PerformanceMetrics::End(const RefCntAutoPtr<IDeviceContext>& pContext, QueryType queryType)
{
    auto& query = queries[queryType];
    query.query->End(pContext, query.GPUtime);
    query.CPUtime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - query.startTime).count();

    if(queryType == QueryType::Total)
    {
        EndFrame();
    }
}

void PerformanceMetrics::BeginFrame()
{
    lastFrameTime = std::chrono::steady_clock::now();
}

void PerformanceMetrics::EndFrame()
{
    frameHistory.push_back({std::chrono::steady_clock::now()});
    UpdateAverageFPS();
    
    frameCounter++;
}

double PerformanceMetrics::GetAverageFPS()
{
    UpdateAverageFPS();
    return averageFPS;
}

void PerformanceMetrics::UpdateAverageFPS()
{
    auto currentTime = std::chrono::steady_clock::now();

    // Remove frames older than 2 seconds
    auto cutoffTime = currentTime - std::chrono::duration<double>(frameHistoryWindowSeconds);
    while (!frameHistory.empty() && frameHistory.front().timestamp < cutoffTime)
    {
        frameHistory.pop_front();
    }
    
    averageFPS = static_cast<double>(frameHistory.size()) / frameHistoryWindowSeconds;
}
