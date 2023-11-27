#include "vulkan_helper.h"

void VulkanContext::construct_swap_chain_creation_info(swapchain_creation_mode aCreationMode) {

    auto setOwnership = [this]() {
        // Handle queue family ownership:

        const std::vector<uint32_t> queueFamilyIndices = { 0 };

        switch (queueFamilyIndices.size()) {
        case 0:
            {
                printf("You must assign at least set one queue(family) to window! You can use window::set_queue_family_ownership.");
                exit(0);
            }
            break;
        case 1:
            mImageCreateInfoSwapChain
                .setSharingMode(vk::SharingMode::eExclusive)
                .setQueueFamilyIndexCount(1u)
                .setPQueueFamilyIndices(&queueFamilyIndices[0]); // could also leave at nullptr!
            break;
        default:
            // Have to use separate queue families!
            // If the queue families differ, then we'll be using the concurrent mode [2]
            mImageCreateInfoSwapChain
                .setSharingMode(vk::SharingMode::eConcurrent)
                .setQueueFamilyIndexCount(static_cast<uint32_t>(queueFamilyIndices.size()))
                .setPQueueFamilyIndices(queueFamilyIndices.data());
            break;
        }
    };

    auto srfCaps = physical_device().getSurfaceCapabilitiesKHR(surface());
    auto extent = get_resolution_for_window();
    auto surfaceFormat = get_config_surface_format(surface());
    if (aCreationMode == swapchain_creation_mode::update_existing_swapchain) {
        mImageCreateInfoSwapChain.setExtent(vk::Extent3D(extent.x, extent.y, 1u));
        if (mResourceRecreationDeterminator.is_recreation_required_for(recreation_determinator::reason::image_format_or_properties_changed)) {
            auto imageUsageProperties = get_config_image_usage_properties();
            auto [imageUsage, imageTiling, createFlags] = avk::to_vk_image_properties(imageUsageProperties);
            mImageCreateInfoSwapChain
                .setFormat(surfaceFormat.format)
                .setUsage(imageUsage)
                .setTiling(imageTiling)
                .setFlags(createFlags);
            setOwnership();
        }
    }
    else {
        auto imageUsageProperties = get_config_image_usage_properties();
        auto [imageUsage, imageTiling, createFlags] = avk::to_vk_image_properties(imageUsageProperties);

        mImageCreateInfoSwapChain = vk::ImageCreateInfo{}
            .setImageType(vk::ImageType::e2D)
            .setFormat(surfaceFormat.format)
            .setExtent(vk::Extent3D(extent.x, extent.y, 1u))
            .setMipLevels(1)
            .setArrayLayers(1)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setTiling(imageTiling)
            .setUsage(imageUsage)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFlags(createFlags);

        setOwnership();
    }

    // With all settings gathered, construct/update swap chain creation info.
    if (aCreationMode == swapchain_creation_mode::update_existing_swapchain) {
        mSwapChainCreateInfo
            .setImageExtent(vk::Extent2D{ mImageCreateInfoSwapChain.extent.width, mImageCreateInfoSwapChain.extent.height })
            .setOldSwapchain(mSwapChain.get());
        if (mResourceRecreationDeterminator.is_recreation_required_for(recreation_determinator::reason::presentation_mode_changed)) {
            mSwapChainCreateInfo.setPresentMode(get_config_presentation_mode(surface()));
        }
        if (mResourceRecreationDeterminator.is_recreation_required_for(recreation_determinator::reason::presentable_images_count_changed)) {
            mSwapChainCreateInfo.setMinImageCount(get_config_number_of_presentable_images());
        }
        if (mResourceRecreationDeterminator.is_recreation_required_for(recreation_determinator::reason::image_format_or_properties_changed)) {
            mSwapChainCreateInfo
                .setImageColorSpace(surfaceFormat.colorSpace)
                .setImageFormat(mImageCreateInfoSwapChain.format)
                .setImageUsage(mImageCreateInfoSwapChain.usage)
                .setImageSharingMode(mImageCreateInfoSwapChain.sharingMode)
                .setQueueFamilyIndexCount(mImageCreateInfoSwapChain.queueFamilyIndexCount)
                .setPQueueFamilyIndices(mImageCreateInfoSwapChain.pQueueFamilyIndices);

            mSwapChainImageFormat = surfaceFormat.format;
        }
    }
    else {
        mSwapChainCreateInfo = vk::SwapchainCreateInfoKHR{}
            .setSurface(surface())
            .setMinImageCount(get_config_number_of_presentable_images())
            .setImageFormat(mImageCreateInfoSwapChain.format)
            .setImageColorSpace(surfaceFormat.colorSpace)
            .setImageExtent(vk::Extent2D{ mImageCreateInfoSwapChain.extent.width, mImageCreateInfoSwapChain.extent.height })
            .setImageArrayLayers(mImageCreateInfoSwapChain.arrayLayers) // The imageArrayLayers specifies the amount of layers each image consists of. This is always 1 unless you are developing a stereoscopic 3D application. [2]
            .setImageUsage(mImageCreateInfoSwapChain.usage)
            .setPreTransform(srfCaps.currentTransform) // To specify that you do not want any transformation, simply specify the current transformation. [2]
            .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque) // => no blending with other windows
            .setPresentMode(get_config_presentation_mode(surface()))
            .setClipped(VK_TRUE) // we don't care about the color of pixels that are obscured, for example because another window is in front of them.  [2]
            .setOldSwapchain({}) // TODO: This won't be enought, I'm afraid/pretty sure. => advanced chapter
            .setImageSharingMode(mImageCreateInfoSwapChain.sharingMode)
            .setQueueFamilyIndexCount(mImageCreateInfoSwapChain.queueFamilyIndexCount)
            .setPQueueFamilyIndices(mImageCreateInfoSwapChain.pQueueFamilyIndices);

        mSwapChainImageFormat = surfaceFormat.format;
    }

    mSwapChainExtent = mSwapChainCreateInfo.imageExtent;
}