#include "vulkan_helper.h"

#include <mutex>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

static std::mutex sSubmitMutex;
static std::mutex sConcurrentAccessMutex;

#ifdef _WIN32

#include <windows.h>

VkResult createWindowSurfaceWin32(VkInstance instance,
    const VkAllocationCallbacks* allocator,
    VkSurfaceKHR* surface,
    void* window)
{
    VkResult err;
    VkWin32SurfaceCreateInfoKHR sci;
    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;

    vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)
        vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
    if (!vkCreateWin32SurfaceKHR)
    {
        printf("Win32: Vulkan instance missing VK_KHR_win32_surface extension\n");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo((SDL_Window*)window, &wmInfo);

    memset(&sci, 0, sizeof(sci));
    sci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    sci.hinstance = GetModuleHandle(NULL);
    sci.hwnd = wmInfo.info.win.window;

    err = vkCreateWin32SurfaceKHR(instance, &sci, allocator, surface);
    if (err)
    {
        printf("Win32: Failed to create Vulkan surface: %d", err);
    }

    return err;
}

#endif

void VulkanContext::create_swap_chain(swapchain_creation_mode aCreationMode, void* windowObject, uint32_t width, uint32_t height)
{
    if (aCreationMode == swapchain_creation_mode::create_new_swapchain) {
        mCurrentFrame = 0; // Start af frame 0
        mSwapChainExtent = vk::Extent2D{ width, height };
    }

    VkSurfaceKHR surface;
#ifdef _WIN32
    createWindowSurfaceWin32(mInstance, nullptr, &surface, windowObject);
#endif
    vk::ObjectDestroy<vk::Instance, DISPATCH_LOADER_CORE_TYPE> deleter(vulkan_instance(), nullptr, dispatch_loader_core());
    mSurface = vk::UniqueHandle<vk::SurfaceKHR, DISPATCH_LOADER_CORE_TYPE>(surface, deleter);

    construct_swap_chain_creation_info(aCreationMode);

    auto lifetimeHandler = [this](vk::UniqueHandle<vk::SwapchainKHR, DISPATCH_LOADER_CORE_TYPE>&& aOldResource) { this->handle_lifetime(std::move(aOldResource)); };
    // assign the new swap chain instead of the old one, if one exists
    avk::assign_and_lifetime_handle_previous(mSwapChain, device().createSwapchainKHRUnique(mSwapChainCreateInfo, nullptr, dispatch_loader_core()), lifetimeHandler);

    construct_backbuffers(aCreationMode);

    // if we are creating a new swap chain from the ground up, or if the number of concurrent frames change
    // set up fences and other basic initialization
    if (aCreationMode == swapchain_creation_mode::create_new_swapchain ||
        mResourceRecreationDeterminator.is_recreation_required_for(recreation_determinator::reason::presentable_images_count_changed) ||
        mResourceRecreationDeterminator.is_recreation_required_for(recreation_determinator::reason::concurrent_frames_count_changed)) {
        update_concurrent_frame_synchronization(aCreationMode);
    }

    mResourceRecreationDeterminator.reset();
}

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

void VulkanContext::construct_backbuffers(swapchain_creation_mode aCreationMode) {
    const auto swapChainImages = device().getSwapchainImagesKHR(swap_chain());
    const auto imagesInFlight = swapChainImages.size();

    assert(imagesInFlight == get_config_number_of_presentable_images()); // TODO: Can it happen that these two ever differ? If so => handle!

    auto extent = get_resolution_for_window();
    auto imageResize = [&extent](avk::image_t& aPreparedImage) {
        if (aPreparedImage.depth() == 1u) {
            aPreparedImage.create_info().extent.width = extent.x;
            aPreparedImage.create_info().extent.height = extent.y;
        }
        else {
            printf("WARNING: No idea how to update a 3D image with dimensions %dx%dx%d\n", aPreparedImage.width(), aPreparedImage.height(), aPreparedImage.depth());
        }
    };
    auto lifetimeHandlerLambda = [this](outdated_swapchain_resource_t&& rhs) { this->handle_lifetime(std::move(rhs)); };

    // Create the new image views:
    std::vector<avk::image_view> newImageViews;
    newImageViews.reserve(imagesInFlight);
    for (size_t i = 0; i < imagesInFlight; ++i) {
        auto& ref = newImageViews.emplace_back(create_image_view(wrap_image(swapChainImages[i], mImageCreateInfoSwapChain, get_config_image_usage_properties(), vk::ImageAspectFlagBits::eColor)));
        ref.enable_shared_ownership();
    }

    avk::assign_and_lifetime_handle_previous(mSwapChainImageViews, std::move(newImageViews), lifetimeHandlerLambda);

    bool additionalAttachmentsChanged = mResourceRecreationDeterminator.is_recreation_required_for(recreation_determinator::reason::backbuffer_attachments_changed);
    bool imageFormatChanged = mResourceRecreationDeterminator.is_recreation_required_for(recreation_determinator::reason::image_format_or_properties_changed);

    auto additionalAttachments = get_additional_back_buffer_attachments();
    // Create a renderpass for the back buffers
    std::vector<avk::attachment> renderpassAttachments = {
        avk::attachment::declare_for(*mSwapChainImageViews[0], avk::on_load::clear.from_previous_layout(avk::layout::undefined), avk::usage::color(0), avk::on_store::store.in_layout(avk::layout::color_attachment_optimal))
    };
    renderpassAttachments.insert(std::end(renderpassAttachments), std::begin(additionalAttachments), std::end(additionalAttachments));

    //recreate render pass only if really necessary, otherwise keep using the old one
    if (aCreationMode == swapchain_creation_mode::create_new_swapchain || imageFormatChanged || additionalAttachmentsChanged)
    {
        auto newRenderPass = create_renderpass(renderpassAttachments, {
            // We only create one subpass here => create default dependencies as per specification chapter 8.1) Render Pass Creation:
            avk::subpass_dependency{avk::subpass::external >> avk::subpass::index(0),
                avk::stage::none  >> avk::stage::all_graphics,
                avk::access::none >> avk::access::input_attachment_read | avk::access::color_attachment_read | avk::access::color_attachment_write | avk::access::depth_stencil_attachment_read | avk::access::depth_stencil_attachment_write
            },
            avk::subpass_dependency{avk::subpass::index(0) >> avk::subpass::external,
                avk::stage::all_graphics                                                          >> avk::stage::none,
                avk::access::color_attachment_write | avk::access::depth_stencil_attachment_write >> avk::access::none
            }
        });
        if (mBackBufferRenderpass.has_value() && mBackBufferRenderpass.is_shared_ownership_enabled()) {
            newRenderPass.enable_shared_ownership();
        }
        avk::assign_and_lifetime_handle_previous(mBackBufferRenderpass, std::move(newRenderPass), lifetimeHandlerLambda);
    }
    
    std::vector<avk::framebuffer> newBuffers;
    newBuffers.reserve(imagesInFlight);
    for (size_t i = 0; i < imagesInFlight; ++i) {
        auto& imView = mSwapChainImageViews[i];
        auto imExtent = imView->get_image().create_info().extent;
        // Create one image view per attachment
        std::vector<avk::image_view> imageViews;
        imageViews.reserve(renderpassAttachments.size());
        imageViews.push_back(imView); // The color attachment is added in any case
        // reuse image views if updating, however not if there are new additional attachments
        // if the number of presentation images is now higher than the previous creation, we need new image views on top of previous ones
        if (aCreationMode == swapchain_creation_mode::update_existing_swapchain && !additionalAttachmentsChanged && i < mBackBuffers.size()) {
            const auto& backBufferImageViews = mBackBuffers[i]->image_views();
            for (int j = 1; j < backBufferImageViews.size(); j++) {
                imageViews.emplace_back(create_image_view_from_template(*backBufferImageViews[j], imageResize));
            }
        }
        else {
            for (auto& aa : additionalAttachments) {
                if (aa.is_used_as_depth_stencil_attachment()) {
                    imageViews.emplace_back(create_depth_image_view(
                        create_image(imExtent.width, imExtent.height, aa.format(), 1, avk::memory_usage::device, avk::image_usage::read_only_depth_stencil_attachment))); // TODO: read_only_* or better general_*?
                }
                else {
                    imageViews.emplace_back(create_image_view(
                        create_image(imExtent.width, imExtent.height, aa.format(), 1, avk::memory_usage::device, avk::image_usage::general_color_attachment)));
                }
            }
        }
        auto& ref = newBuffers.emplace_back(create_framebuffer(mBackBufferRenderpass, std::move(imageViews), extent.x, extent.y));
        ref.enable_shared_ownership();
    }

    avk::assign_and_lifetime_handle_previous(mBackBuffers, std::move(newBuffers), lifetimeHandlerLambda);
}

void VulkanContext::handle_lifetime(avk::command_buffer aCommandBuffer, std::optional<frame_id_t> aFrameId)
{
    std::scoped_lock<std::mutex> guard(sSubmitMutex); // Protect against concurrent access from invokees
    if (!aFrameId.has_value()) {
        aFrameId = current_frame();
    }

    aCommandBuffer->invoke_post_execution_handler(); // Yes, do it now!

    auto& refTpl = mLifetimeHandledCommandBuffers.emplace_back(aFrameId.value(), std::move(aCommandBuffer));
    // ^ Prefer code duplication over recursive_mutex
}

void VulkanContext::handle_lifetime(outdated_swapchain_resource_t&& aOutdatedSwapchain, std::optional<frame_id_t> aFrameId)
{
    std::scoped_lock<std::mutex> guard(sSubmitMutex); // Protect against concurrent access from invokees
    if (!aFrameId.has_value()) {
        aFrameId = current_frame();
    }
    mOutdatedSwapChainResources.emplace_back(aFrameId.value(), std::move(aOutdatedSwapchain));
}

void VulkanContext::update_concurrent_frame_synchronization(swapchain_creation_mode aCreationMode)
{
    // ============= SYNCHRONIZATION OBJECTS ===========
        // per CONCURRENT FRAME:

    auto framesInFlight = get_config_number_of_concurrent_frames();

    mFramesInFlightFences.clear();
    mImageAvailableSemaphores.clear();
    mFramesInFlightFences.reserve(framesInFlight);
    mImageAvailableSemaphores.reserve(framesInFlight);
    for (uint32_t i = 0; i < framesInFlight; ++i) {
        mFramesInFlightFences.push_back(create_fence(true)); // true => Create the fences in signalled state, so that `avk::context().logical_device().waitForFences` at the beginning of `window::render_frame` is not blocking forever, but can continue immediately.
        mImageAvailableSemaphores.push_back(create_semaphore());
    }

    auto imagesInFlight = get_config_number_of_presentable_images();
    mImagesInFlightFenceIndices.clear();
    mImagesInFlightFenceIndices.reserve(imagesInFlight);
    for (uint32_t i = 0; i < imagesInFlight; ++i) {
        mImagesInFlightFenceIndices.push_back(-1);
    }
    assert(mImagesInFlightFenceIndices.size() == imagesInFlight);

    // when updating, the current fence must be unsignaled.
    if (aCreationMode == swapchain_creation_mode::update_existing_swapchain) {
        current_fence()->reset();
    }

    assert(static_cast<frame_id_t>(mFramesInFlightFences.size()) == get_config_number_of_concurrent_frames());
    assert(static_cast<frame_id_t>(mImageAvailableSemaphores.size()) == get_config_number_of_concurrent_frames());
}

avk::command_buffer VulkanContext::record_and_submit(std::vector<avk::recorded_commands_t> aRecordedCommandsAndSyncInstructions, const avk::queue& aQueue, vk::CommandBufferUsageFlags aUsageFlags)
{
    auto& cmdPool = get_command_pool_for_single_use_command_buffers(aQueue);
    auto cmdBfr = cmdPool->alloc_command_buffer(aUsageFlags);

    record(std::move(aRecordedCommandsAndSyncInstructions))
        .into_command_buffer(cmdBfr)
        .then_submit_to(aQueue)
        .submit();

    return cmdBfr;
}

avk::semaphore VulkanContext::record_and_submit_with_semaphore(std::vector<avk::recorded_commands_t> aRecordedCommandsAndSyncInstructions, const avk::queue& aQueue, avk::stage::pipeline_stage_flags aSrcSignalStage, vk::CommandBufferUsageFlags aUsageFlags)
{
    auto& cmdPool = get_command_pool_for_single_use_command_buffers(aQueue);
    auto cmdBfr = cmdPool->alloc_command_buffer(aUsageFlags);
    auto sem = create_semaphore();

    record(std::move(aRecordedCommandsAndSyncInstructions))
        .into_command_buffer(cmdBfr)
        .then_submit_to(aQueue)
        .signaling_upon_completion(aSrcSignalStage >> sem)
        .submit();

    sem->handle_lifetime_of(std::move(cmdBfr));
    return sem;
}

avk::fence VulkanContext::record_and_submit_with_fence(std::vector<avk::recorded_commands_t> aRecordedCommandsAndSyncInstructions, const avk::queue& aQueue, vk::CommandBufferUsageFlags aUsageFlags)
{
    auto& cmdPool = get_command_pool_for_single_use_command_buffers(aQueue);
    auto cmdBfr = cmdPool->alloc_command_buffer(aUsageFlags);
    auto fen = create_fence();

    record(std::move(aRecordedCommandsAndSyncInstructions))
        .into_command_buffer(cmdBfr)
        .then_submit_to(aQueue)
        .signaling_upon_completion(fen)
        .submit();

    fen->handle_lifetime_of(std::move(cmdBfr));
    return fen;
}

avk::command_pool& VulkanContext::get_command_pool_for(uint32_t aQueueFamilyIndex, vk::CommandPoolCreateFlags aFlags)
{
    std::scoped_lock<std::mutex> guard(sConcurrentAccessMutex);
    auto it = std::find_if(std::begin(mCommandPools), std::end(mCommandPools),
        [lThreadId = std::this_thread::get_id(), lFamilyIdx = aQueueFamilyIndex, lFlags = aFlags](const std::tuple<std::thread::id, avk::command_pool>& existing) {
            auto& tid = std::get<0>(existing);
            auto& q = std::get<1>(existing);
            return tid == lThreadId && q->queue_family_index() == lFamilyIdx && lFlags == q->create_info().flags;
        });
    if (it == std::end(mCommandPools)) {
        return std::get<1>(mCommandPools.emplace_back(std::this_thread::get_id(), create_command_pool(aQueueFamilyIndex, aFlags)));
    }
    return std::get<1>(*it);
}

avk::command_pool& VulkanContext::get_command_pool_for(const avk::queue& aQueue, vk::CommandPoolCreateFlags aFlags)
{
    return get_command_pool_for(aQueue.family_index(), aFlags);
}

avk::command_pool& VulkanContext::get_command_pool_for_single_use_command_buffers(const avk::queue& aQueue)
{
    return get_command_pool_for(aQueue, vk::CommandPoolCreateFlagBits::eTransient);
}

avk::command_pool& VulkanContext::get_command_pool_for_reusable_command_buffers(const avk::queue& aQueue)
{
    return get_command_pool_for(aQueue, {});
}

avk::command_pool& VulkanContext::get_command_pool_for_resettable_command_buffers(const avk::queue& aQueue)
{
    return get_command_pool_for(aQueue, vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
}