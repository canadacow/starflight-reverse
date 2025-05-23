#pragma once

#include "Common/interface/RefCntAutoPtr.hpp"
#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/Query.h"
#include "Graphics/GraphicsTools/interface/DurationQueryHelper.hpp"

#include <unordered_map>
#include <chrono>
#include <memory>

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
        double time;
    };

    std::unordered_map<QueryType, QueryData> queries;

    void Initialize(IRenderDevice* pDevice);

    void Mark(const RefCntAutoPtr<IDeviceContext>& pContext, QueryType queryType);
    void End(const RefCntAutoPtr<IDeviceContext>& pContext, QueryType queryType);

    double GetValue(QueryType queryType) const
    {
        return queries.at(queryType).time;
    }
};
