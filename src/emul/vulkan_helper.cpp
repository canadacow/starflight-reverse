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

std::vector<avk::semaphore> VulkanContext::remove_all_present_semaphore_dependencies_for_frame(frame_id_t aPresentFrameId)
{
    // No need to protect against concurrent access since that would be misuse of this function.
    // This shall never be called from the invokee callbacks as being invoked through a parallel invoker.

    // Find all to remove
    auto to_remove = std::remove_if(
        std::begin(mPresentSemaphoreDependencies), std::end(mPresentSemaphoreDependencies),
        [maxTTL = aPresentFrameId - number_of_frames_in_flight()](const auto& tpl) {
            return std::get<frame_id_t>(tpl) <= maxTTL;
        });
    // return ownership of all the semaphores to remove to the caller
    std::vector<avk::semaphore> moved_semaphores;
    for (decltype(to_remove) it = to_remove; it != std::end(mPresentSemaphoreDependencies); ++it) {
        moved_semaphores.push_back(std::move(std::get<avk::semaphore>(*it)));
    }
    // Erase and return
    mPresentSemaphoreDependencies.erase(to_remove, std::end(mPresentSemaphoreDependencies));
    return moved_semaphores;
}

std::vector<avk::command_buffer> VulkanContext::clean_up_command_buffers_for_frame(frame_id_t aPresentFrameId)
{
    std::vector<avk::command_buffer> removedBuffers;
    if (mLifetimeHandledCommandBuffers.empty()) {
        return removedBuffers;
    }
    // No need to protect against concurrent access since that would be misuse of this function.
    // This shall never be called from the invokee callbacks as being invoked through a parallel invoker.

    // Up to the frame with id 'maxTTL', all command buffers can be safely removed
    const auto maxTTL = aPresentFrameId - number_of_frames_in_flight();

    // 1. SINGLE USE COMMAND BUFFERS
    // Can not use the erase-remove idiom here because that would invalidate iterators and references
    // HOWEVER: "[...]unless the erased elements are at the end or the beginning of the container,
    // in which case only the iterators and references to the erased elements are invalidated." => Let's do that!
    auto eraseBegin = std::begin(mLifetimeHandledCommandBuffers);
    if (std::end(mLifetimeHandledCommandBuffers) == eraseBegin || std::get<frame_id_t>(*eraseBegin) > maxTTL) {
        return removedBuffers;
    }
    // There are elements that we can remove => find position until where:
    auto eraseEnd = eraseBegin;
    while (eraseEnd != std::end(mLifetimeHandledCommandBuffers) && std::get<frame_id_t>(*eraseEnd) <= maxTTL) {
        // return ownership of all the command_buffers to remove to the caller
        removedBuffers.push_back(std::move(std::get<avk::command_buffer>(*eraseEnd)));
        ++eraseEnd;
    }
    mLifetimeHandledCommandBuffers.erase(eraseBegin, eraseEnd);
    return removedBuffers;
}

void VulkanContext::clean_up_outdated_swapchain_resources_for_frame(frame_id_t aPresentFrameId)
{
    if (mOutdatedSwapChainResources.empty()) {
        return;
    }

    // Up to the frame with id 'maxTTL', all swap chain resources can be safely removed
    const auto maxTTL = aPresentFrameId - number_of_frames_in_flight();

    auto eraseBegin = std::begin(mOutdatedSwapChainResources);
    if (std::end(mOutdatedSwapChainResources) == eraseBegin || std::get<frame_id_t>(*eraseBegin) > maxTTL) {
        return;
    }
    auto eraseEnd = eraseBegin;
    while (eraseEnd != std::end(mOutdatedSwapChainResources) && std::get<frame_id_t>(*eraseEnd) <= maxTTL) {
        ++eraseEnd;
    }
    mOutdatedSwapChainResources.erase(eraseBegin, eraseEnd);
}

void VulkanContext::fill_in_present_semaphore_dependencies_for_frame(std::vector<vk::Semaphore>& aSemaphores, frame_id_t aFrameId) const
{
    for (const auto& [frameId, sem] : mPresentSemaphoreDependencies) {
        if (frameId == aFrameId) {
            auto& info = aSemaphores.emplace_back(sem->handle());
        }
    }
}

void VulkanContext::acquire_next_swap_chain_image_and_prepare_semaphores()
{
    if (mResourceRecreationDeterminator.is_any_recreation_necessary()) {
        if (mResourceRecreationDeterminator.has_concurrent_frames_count_changed_only()) { //only update framesInFlight fences/semaphores
            mActivePresentationQueue->handle().waitIdle(); // ensure the semaphores which we are going to potentially delete are not being used
            update_concurrent_frame_synchronization(swapchain_creation_mode::update_existing_swapchain);
            mResourceRecreationDeterminator.reset();
        }
        else { //recreate the whole swap chain then
            update_resolution_and_recreate_swap_chain();
        }
    }
    // Update the presentation queue only after potential swap chain updates:
    update_active_presentation_queue();

    // Get the next image from the swap chain, GPU -> GPU sync from previous present to the following acquire
    auto imgAvailableSem = image_available_semaphore_for_frame();

    // Update previous image index before getting a new image index for the current frame:
    mPreviousFrameImageIndex = mCurrentFrameImageIndex;

    try
    {
        auto result = device().acquireNextImageKHR(
            swap_chain(), // the swap chain from which we wish to acquire an image
            // At this point, I have to rant about the `timeout` parameter:
            // The spec says: "timeout specifies how long the function waits, in nanoseconds, if no image is available."
            // HOWEVER, don't think that a numeric_limit<int64_t>::max() will wait for nine quintillion nanoseconds!
            //    No, instead it will return instantly, yielding an invalid swap chain image index. OMG, WTF?!
            // Long story short: make sure to pass the UNSINGEDint64_t's maximum value, since only that will disable the timeout.
            std::numeric_limits<uint64_t>::max(), // a timeout in nanoseconds for an image to become available. Using the maximum value of a 64 bit unsigned integer disables the timeout. [1]
            imgAvailableSem->handle(), // The next two parameters specify synchronization objects that are to be signaled when the presentation engine is finished using the image [1]
            nullptr,
            &mCurrentFrameImageIndex); // a variable to output the index of the swap chain image that has become available. The index refers to the VkImage in our swapChainImages array. We're going to use that index to pick the right command buffer. [1]
        if (vk::Result::eSuboptimalKHR == result) {
            LOG_INFO("Swap chain is suboptimal in acquire_next_swap_chain_image_and_prepare_semaphores. Going to recreate it...");
            mResourceRecreationDeterminator.set_recreation_required_for(recreation_determinator::reason::suboptimal_swap_chain);

            // Workaround for binary semaphores:
            // Since the semaphore is in a wait state right now, we'll have to wait for it until we can use it again.
            auto fen = create_fence();
            vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eAllCommands;
            mActivePresentationQueue->handle().submit({ // TODO: This works, but is arguably not the greatest of all solutions... => Can it be done better with Timeline Semaphores? (Test on AMD!)
                vk::SubmitInfo{}
                    .setCommandBufferCount(0u)
                    .setWaitSemaphoreCount(1u)
                    .setPWaitSemaphores(imgAvailableSem->handle_addr())
                    .setPWaitDstStageMask(&waitStage)
                }, fen->handle());
            fen->wait_until_signalled();

            acquire_next_swap_chain_image_and_prepare_semaphores();
            return;
        }
    }
    catch (vk::OutOfDateKHRError omg) {
        LOG_INFO(fmt::format("Swap chain out of date in acquire_next_swap_chain_image_and_prepare_semaphores. Reason[{}] in frame#{}. Going to recreate it...", omg.what(), current_frame()));
        mResourceRecreationDeterminator.set_recreation_required_for(recreation_determinator::reason::invalid_swap_chain);
        acquire_next_swap_chain_image_and_prepare_semaphores();
        return;
    }

    // It could be that the image index that has been returned is currently in flight.
    // There's no guarantee that we'll always get a nice cycling through the indices.
    // => Must handle this case!
    assert(current_image_index() == mCurrentFrameImageIndex);
    if (mImagesInFlightFenceIndices[current_image_index()] >= 0) {
        LOG_DEBUG_VERBOSE(fmt::format("Frame #{}: Have to issue an extra fence-wait because swap chain returned image[{}] but fence[{}] is currently in use.", current_frame(), mCurrentFrameImageIndex, mImagesInFlightFenceIndices[current_image_index()]));
        auto& xf = mFramesInFlightFences[mImagesInFlightFenceIndices[current_image_index()]];
        xf->wait_until_signalled();
        // But do not reset! Otherwise we will wait forever at the next wait_until_signalled that will happen for sure.
    }

    // Set the image available semaphore to be consumed:
    mCurrentFrameImageAvailableSemaphore = imgAvailableSem;

    // Set the fence to be used:
    mCurrentFrameFinishedFence = current_fence();
}

void VulkanContext::sync_before_render()
{
    // Wait for the fence before proceeding, GPU -> CPU synchronization via fence
    const auto ci = current_in_flight_index();
    auto cf = current_fence();
    assert(cf->handle() == mFramesInFlightFences[current_in_flight_index()]->handle());
    cf->wait_until_signalled();
    cf->reset();

    // Keep house with the in-flight images:
    //   However, we don't know which index this fence had been mapped to => we have to search
    for (auto& mapping : mImagesInFlightFenceIndices) {
        if (ci == mapping) {
            mapping = -1;
            break;
        }
    }

    // At this point we are certain that the frame which has used the current fence before is done.
    //  => Clean up the resources of that previous frame!
    auto semaphoresToBeFreed = remove_all_present_semaphore_dependencies_for_frame(current_frame());
    auto commandBuffersToBeFreed = clean_up_command_buffers_for_frame(current_frame());
    clean_up_outdated_swapchain_resources_for_frame(current_frame());

    acquire_next_swap_chain_image_and_prepare_semaphores();
}

void VulkanContext::render_frame()
{
    const auto fenceIndex = static_cast<int>(current_in_flight_index());

    // EXTERN -> WAIT
    std::vector<vk::Semaphore> waitSemHandles;
    fill_in_present_semaphore_dependencies_for_frame(waitSemHandles, current_frame());
    std::vector<vk::SemaphoreSubmitInfoKHR> waitSemInfos(waitSemHandles.size());
    std::ranges::transform(std::begin(waitSemHandles), std::end(waitSemHandles), std::begin(waitSemInfos), [](const auto& semHandle) {
        return vk::SemaphoreSubmitInfoKHR{ semHandle, 0, vk::PipelineStageFlagBits2KHR::eAllCommands }; // TODO: Really ALL_COMMANDS or could we also use NONE?
        });

    if (!has_consumed_current_image_available_semaphore()) {
        // Being in this branch indicates that image available semaphore has not been consumed yet
        // meaning that no render calls were (correctly) executed  (hint: check if all invokess are possibly disabled).
        auto imgAvailable = consume_current_image_available_semaphore();
        waitSemInfos.emplace_back(imgAvailable->handle())
            .setStageMask(vk::PipelineStageFlagBits2KHR::eAllCommands);
    }

    if (!has_used_current_frame_finished_fence()) {
        // Need an additional submission to signal the fence.
        auto fence = use_current_frame_finished_fence();
        assert(fence->handle() == mFramesInFlightFences[fenceIndex]->handle());

        // Using a temporary semaphore for the signal operation:
        auto sigSem = create_semaphore();
        vk::SemaphoreSubmitInfoKHR sigSemInfo{ sigSem->handle() };

        // Waiting on the same semaphores here and during vkPresentKHR should be fine: (TODO: is it?)
        auto submitInfo = vk::SubmitInfo2KHR{}
            .setWaitSemaphoreInfoCount(static_cast<uint32_t>(waitSemInfos.size()))
            .setPWaitSemaphoreInfos(waitSemInfos.data())
            .setCommandBufferInfoCount(0u)    // Submit ZERO command buffers :O
            .setSignalSemaphoreInfoCount(1u)
            .setPSignalSemaphoreInfos(&sigSemInfo);
#ifdef AVK_USE_SYNCHRONIZATION2_INSTEAD_OF_CORE
        auto errorCode = mActivePresentationQueue->handle().submit2KHR(1u, &submitInfo, fence->handle(), dispatch_loader_ext());
        if (vk::Result::eSuccess != errorCode) {
            AVK_LOG_WARNING("submit2KHR returned " + vk::to_string(errorCode));
        }
#else
        auto errorCode = mActivePresentationQueue->handle().submit2(1u, &submitInfo, fence->handle(), dispatch_loader_core());
        if (vk::Result::eSuccess != errorCode) {
            AVK_LOG_WARNING("submit2 returned " + vk::to_string(errorCode));
        }
#endif

        // Consequently, the present call must wait on the temporary semaphore only:
        waitSemHandles.clear();
        waitSemHandles.emplace_back(sigSem->handle());
        // Add it as dependency to the current frame, so that it gets properly lifetime-handled:
        add_present_dependency_for_current_frame(std::move(sigSem));
    }

    try
    {
        // SIGNAL -> PRESENT
        auto presentInfo = vk::PresentInfoKHR{}
            .setWaitSemaphoreCount(waitSemHandles.size())
            .setPWaitSemaphores(waitSemHandles.data())
            .setSwapchainCount(1u)
            .setPSwapchains(&swap_chain())
            .setPImageIndices(&mCurrentFrameImageIndex)
            .setPResults(nullptr);
        auto result = mActivePresentationQueue->handle().presentKHR(presentInfo);

        // Submitted => store the image index for extra reuse-safety:
        mImagesInFlightFenceIndices[mCurrentFrameImageIndex] = fenceIndex;

        if (vk::Result::eSuboptimalKHR == result) {
            LOG_INFO("Swap chain is suboptimal in render_frame. Going to recreate it...");
            mResourceRecreationDeterminator.set_recreation_required_for(recreation_determinator::reason::suboptimal_swap_chain);
            // swap chain will be recreated in the next frame
        }
    }
    catch (vk::OutOfDateKHRError omg) {
        LOG_INFO(fmt::format("Swap chain out of date in render_frame. Reason[{}] in frame#{}. Going to recreate it...", omg.what(), current_frame()));
        mResourceRecreationDeterminator.set_recreation_required_for(recreation_determinator::reason::invalid_swap_chain);
        // Just do nothing. Ignore the failure. This frame is lost.
        // swap chain will be recreated in the next frame
    }

    // increment frame counter
    ++mCurrentFrame;
}

void VulkanContext::set_queue_family_ownership(std::vector<uint32_t> aQueueFamilies)
{
    mQueueFamilyIndicesGetter = [families = std::move(aQueueFamilies)]() { return families; };

    // If the window has already been created, the new setting can't
    // be applied unless the swapchain is being recreated.
    if (is_alive()) {
        mResourceRecreationDeterminator.set_recreation_required_for(recreation_determinator::reason::image_format_or_properties_changed);
    }
}

void VulkanContext::set_queue_family_ownership(uint32_t aQueueFamily)
{
    set_queue_family_ownership(std::vector<uint32_t>{ aQueueFamily });
}

void VulkanContext::set_present_queue(avk::queue& aPresentQueue)
{
    if (nullptr == mActivePresentationQueue) {
        mActivePresentationQueue = &aPresentQueue;
    }
    else {
        // Update it later, namely after potential swap chain updates have been executed:
        mPresentationQueueGetter = [newPresentQueue = &aPresentQueue]() { return newPresentQueue; };
    }
}

void VulkanContext::update_resolution()
{
    // FIXME
}

void VulkanContext::update_resolution_and_recreate_swap_chain()
{
    // Gotta wait until the resolution has been updated on the main thread:
    update_resolution();
    mActivePresentationQueue->handle().waitIdle();

    create_swap_chain(swapchain_creation_mode::update_existing_swapchain);
}