#include "PerfMetrics.h"

void PerformanceMetrics::Initialize(IRenderDevice* pDevice)
{
    for (auto& query : queries)
    {
        query.second.query = std::make_unique<DurationQueryHelper>(pDevice, 5);
    }
}

void PerformanceMetrics::Mark(const RefCntAutoPtr<IDeviceContext>& pContext, QueryType queryType)
{
    auto& query = queries[queryType];
    query.query->Begin(pContext);
}

void PerformanceMetrics::End(const RefCntAutoPtr<IDeviceContext>& pContext, QueryType queryType)
{
    auto& query = queries[queryType];
    query.query->End(pContext, query.time);
}
