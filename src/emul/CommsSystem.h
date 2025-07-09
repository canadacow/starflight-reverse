#pragma once

#include "SFModel.h"
#include "vulkan_helper.h"
#include "../pbr/SF_GLTF_PBR_Renderer.hpp"
#include <memory>

namespace Diligent {

    void InitComms(SFModel& comms);

    void UpdateComms(VulkanContext::frame_id_t inFlightIndex, 
        VulkanContext::frame_id_t frameCount, 
        double currentTimeInSeconds,
        SF_GLTF_PBR_Renderer::RenderInfo& renderParams,
        std::unique_ptr<HLSL::CameraAttribs[]>& cameraAttribs,
        SFModel& comms,
        int2 screenSize);
}