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
}

void PerformanceMetrics::Mark(const RefCntAutoPtr<IDeviceContext>& pContext, QueryType queryType)
{
    auto& query = queries[queryType];
    query.query->Begin(pContext);
    query.startTime = std::chrono::steady_clock::now();
}

void PerformanceMetrics::End(const RefCntAutoPtr<IDeviceContext>& pContext, QueryType queryType)
{
    auto& query = queries[queryType];
    query.query->End(pContext, query.GPUtime);
    query.CPUtime = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - query.startTime).count();
}
