#include "EpipolarLightScattering.hpp"

#include "Shaders/PostProcess/ScreenSpaceAmbientOcclusion/public/ScreenSpaceAmbientOcclusionStructures.fxh"


namespace Diligent {

void PrepareForNewFrame(void* pEpipolarLightScattering, EpipolarLightScattering::FrameAttribs& FrameAttribs, void* m_PPAttribs)
{
    static_cast<EpipolarLightScattering*>(pEpipolarLightScattering)->PrepareForNewFrame(FrameAttribs, *static_cast<EpipolarLightScatteringAttribs*>(m_PPAttribs));
}

} // namespace Diligent
