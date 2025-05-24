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
    frameTimes.clear();
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
    auto currentTime = std::chrono::steady_clock::now();
    double frameTime = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - lastFrameTime).count();
    
    // Add current frame time to the queue
    frameTimes.push_back(frameTime);
    
    // Maintain the maximum number of samples
    if (frameTimes.size() > maxFrameTimesSamples)
    {
        frameTimes.pop_front();
    }
    
    // Calculate average FPS from the collected frame times
    double totalTime = 0.0;
    for (const auto& time : frameTimes)
    {
        totalTime += time;
    }
    
    if (totalTime > 0.0 && !frameTimes.empty())
    {
        averageFPS = static_cast<double>(frameTimes.size()) / totalTime;
    }
    
    // Increment frame counter
    frameCounter++;
}
