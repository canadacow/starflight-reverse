#pragma once

#include <assert.h>

#define VK_ENABLE_BETA_EXTENSIONS
#define DISPATCH_LOADER_CORE_TYPE vk::DispatchLoaderDynamic
#define DISPATCH_LOADER_EXT_TYPE vk::DispatchLoaderDynamic
#include <avk/avk.hpp>

#ifdef _WIN32
	#include <windows.h>
    #include <vulkan/vulkan_win32.h>
#elif __linux__
    #include <xcb/xcb.h>
	#include <vulkan/vulkan_xcb.h>
#endif

template<typename T>
struct vec2 {
    union {
        struct {
            T x;
            T y;
        };
        struct {
            T u;
            T v;
        };
    };
};

class VulkanContext : public avk::root
{
public:
	vk::Instance vulkan_instance()
	{
		if (!mInstance) {
			if constexpr (std::is_same_v<std::remove_cv_t<decltype(mDispatchLoaderCore)>, vk::DispatchLoaderDynamic>) {
				reinterpret_cast<vk::DispatchLoaderDynamic*>(&mDispatchLoaderCore)->init(vkGetInstanceProcAddr);
			}

			auto appInfo = vk::ApplicationInfo("Starflight", VK_MAKE_VERSION(1, 0, 0),
									"Starflight", VK_MAKE_VERSION(1, 0, 0), // TODO: Real version of Auto-Vk-Toolkit
									VK_API_VERSION_1_2);

			std::vector<const char*> supportedValidationLayers;
			supportedValidationLayers.push_back("VK_LAYER_KHRONOS_validation");

			std::vector<const char*> requiredExtensions;
#ifdef _WIN32
			requiredExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#else
			requiredExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
			requiredExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

			auto instCreateInfo = vk::InstanceCreateInfo()
				.setPApplicationInfo(&appInfo)
				.setEnabledExtensionCount(static_cast<uint32_t>(requiredExtensions.size()))
				.setPpEnabledExtensionNames(requiredExtensions.data())
				.setEnabledLayerCount(static_cast<uint32_t>(supportedValidationLayers.size()))
				.setPpEnabledLayerNames(supportedValidationLayers.data());

			mInstance = vk::createInstance(instCreateInfo, nullptr, mDispatchLoaderCore);

			if constexpr (std::is_same_v<std::remove_cv_t<decltype(mDispatchLoaderCore)>, vk::DispatchLoaderDynamic>) {
				reinterpret_cast<vk::DispatchLoaderDynamic*>(&mDispatchLoaderCore)->init(mInstance);
			}
		}

		return mInstance;
	}
	
	vk::PhysicalDevice& physical_device() override
	{
		if (!mPhysicalDevice) {
			mPhysicalDevice = vulkan_instance().enumeratePhysicalDevices().front();
		}
		return mPhysicalDevice;
	}
	const vk::PhysicalDevice& physical_device() const override
	{
		assert(mPhysicalDevice);
		return mPhysicalDevice;
	}
	
	vk::Device& device() override
	{
		if (!mDevice) {
			// Select one queue that can handle everything:
			auto queueFamilyIndex = avk::queue::find_best_queue_family_for(physical_device(), {}, avk::queue_selection_preference::versatile_queue, {});
			auto queues = avk::make_vector(avk::queue::prepare(this, 0, 0));
			auto config = avk::queue::get_queue_config_for_DeviceCreateInfo(std::begin(queues), std::end(queues));
			for (auto i = 0; i < std::get<0>(config).size(); ++i) {
				std::get<0>(config)[i].setPQueuePriorities(std::get<1>(config)[i].data());
			}

			// Create the device using the queue information from above:
			// Add the swapchain extension based on the platform:
			std::vector<const char*> deviceExtensions;
			deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
			deviceExtensions.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);

			vk::DeviceCreateInfo deviceCreateInfo{};
			deviceCreateInfo.setQueueCreateInfoCount(1u)
				.setPQueueCreateInfos(std::get<0>(config).data())
				.setEnabledExtensionCount(static_cast<uint32_t>(deviceExtensions.size()))
				.setPpEnabledExtensionNames(deviceExtensions.data());

			mDevice = physical_device().createDevice(deviceCreateInfo,
				nullptr,
				mDispatchLoaderCore
			);

			queues[0].assign_handle();

			// Store the queue:
			mQueue = std::move(queues[0]);

			if constexpr (std::is_same_v<std::remove_cv_t<decltype(mDispatchLoaderCore)>, vk::DispatchLoaderDynamic>) {
				reinterpret_cast<vk::DispatchLoaderDynamic*>(&mDispatchLoaderCore)->init(mDevice);
			}
			
#if defined(AVK_USE_VMA)
			// With everything in place, create the memory allocator:
			VmaAllocatorCreateInfo allocatorInfo = {};
			allocatorInfo.physicalDevice = physical_device();
			allocatorInfo.device = mDevice.get();
			allocatorInfo.instance = vulkan_instance();
			vmaCreateAllocator(&allocatorInfo, &mMemoryAllocator);
#else
			mMemoryAllocator = std::make_tuple(physical_device(), mDevice);
#endif
		}
		return mDevice;
	}
	const vk::Device& device() const override
	{
		assert(mDevice);
		return mDevice;
	}

	vk::Queue queue()
	{
		if (!mDevice) {
			device();
		}
		return mQueue.handle();
	}

	DISPATCH_LOADER_CORE_TYPE& dispatch_loader_core() override
	{
		return mDispatchLoaderCore;
	}
	const DISPATCH_LOADER_CORE_TYPE& dispatch_loader_core() const override
	{
		return mDispatchLoaderCore;
	}

	DISPATCH_LOADER_EXT_TYPE& dispatch_loader_ext() override
	{
		return mDispatchLoaderCore;
	}
	const DISPATCH_LOADER_EXT_TYPE& dispatch_loader_ext() const override
	{
		return mDispatchLoaderCore;
	}

	AVK_MEM_ALLOCATOR_TYPE& memory_allocator() override
	{
		if (!mDevice) {
			device();
		}
		return mMemoryAllocator;
	}

	const AVK_MEM_ALLOCATOR_TYPE& memory_allocator()const  override
	{
		if (!mDevice) {
			device();
		}
		return mMemoryAllocator;
	}

	enum struct swapchain_creation_mode
	{
		update_existing_swapchain,
		create_new_swapchain
	};

	using frame_id_t = int64_t;
	using outdated_swapchain_t = std::tuple<vk::UniqueHandle<vk::SwapchainKHR, DISPATCH_LOADER_CORE_TYPE>, std::vector<avk::image_view>, avk::renderpass, std::vector<avk::framebuffer>>;
	using outdated_swapchain_resource_t = std::variant<vk::UniqueHandle<vk::SwapchainKHR, DISPATCH_LOADER_CORE_TYPE>, std::vector<avk::image_view>, avk::renderpass, std::vector<avk::framebuffer>, outdated_swapchain_t>;

	const auto& surface() const {
		return mSurface.get();
	}
	/** Gets this window's swap chain */
	const auto& swap_chain() const {
		return mSwapChain.get();
	}
	/** Gets this window's swap chain's image format */
	const auto& swap_chain_image_format() const {
		return mSwapChainImageFormat;
	}
	/** Gets this window's swap chain's color space */
	const auto& swap_chain_color_space() const {
		return mSwapChainColorSpace;
	}
	/** Gets this window's swap chain's dimensions */
	auto swap_chain_extent() const {
		return mSwapChainExtent;
	}

	void create_swap_chain(swapchain_creation_mode aCreationMode, void* windowObject = nullptr, uint32_t width = 0, uint32_t height = 0);
	void construct_backbuffers(swapchain_creation_mode aCreationMode);
	
private:
	vk::Instance mInstance;
	vk::PhysicalDevice mPhysicalDevice;
	vk::Device mDevice;
	avk::queue mQueue;
	DISPATCH_LOADER_CORE_TYPE mDispatchLoaderCore;
#if defined(AVK_USE_VMA)
	VmaAllocator mMemoryAllocator;
#else
	std::tuple<vk::PhysicalDevice, vk::Device> mMemoryAllocator;
#endif

	void construct_swap_chain_creation_info(swapchain_creation_mode aCreationMode);

	avk::image_usage get_config_image_usage_properties()
	{
		return avk::image_usage::color_attachment | avk::image_usage::transfer_destination | avk::image_usage::transfer_source | avk::image_usage::presentable | avk::image_usage::tiling_optimal;
	}

	vk::SurfaceFormatKHR get_config_surface_format(const vk::SurfaceKHR & aSurface)
	{
		return vk::SurfaceFormatKHR{ vk::Format::eA8B8G8R8UnormPack32, vk::ColorSpaceKHR::eSrgbNonlinear };
	}

	vk::PresentModeKHR get_config_presentation_mode(const vk::SurfaceKHR & aSurface)
	{
		return vk::PresentModeKHR::eMailbox;
	}

	uint32_t get_config_number_of_presentable_images()
	{
		auto srfCaps = physical_device().getSurfaceCapabilitiesKHR(surface());
		auto imageCount = srfCaps.minImageCount + 1u;
		if (srfCaps.maxImageCount > 0) { // A value of 0 for maxImageCount means that there is no limit
			imageCount = (std::min)(imageCount, srfCaps.maxImageCount);
		}
		return imageCount;
	}

	frame_id_t get_config_number_of_concurrent_frames()
	{
		return get_config_number_of_presentable_images();
	}

	vec2<uint32_t> get_resolution_for_window()
	{
		assert(mSwapChainExtent.width != 0);
		assert(mSwapChainExtent.height != 0);
		return {mSwapChainExtent.width, mSwapChainExtent.height};
	}

	std::vector<avk::attachment> get_additional_back_buffer_attachments()
	{
		return {};
	}

	[[nodiscard]] frame_id_t current_frame() const {
		return mCurrentFrame;
	}

	[[nodiscard]] frame_id_t number_of_frames_in_flight() const {
		return static_cast<frame_id_t>(mFramesInFlightFences.size());
	}

	/** Returns the "in flight index" for the requested frame. */
	[[nodiscard]] frame_id_t current_in_flight_index() const {
		return current_frame() % number_of_frames_in_flight();
	}

	/** Returns the fence for the current frame. */
	[[nodiscard]] avk::fence current_fence() {
		return mFramesInFlightFences[current_in_flight_index()];
	}

	void handle_lifetime(avk::command_buffer aCommandBuffer, std::optional<frame_id_t> aFrameId = {});
	void handle_lifetime(outdated_swapchain_resource_t&& aOutdatedSwapchain, std::optional<frame_id_t> aFrameId = {});

	void update_concurrent_frame_synchronization(swapchain_creation_mode aCreationMode);

#pragma region configuration properties
	// A function which returns whether or not the window should be resizable
	bool mShallBeResizable = false;

	// A function which returns the surface format for this window's surface
	avk::unique_function<vk::SurfaceFormatKHR(const vk::SurfaceKHR&)> mSurfaceFormatSelector;

	// A function which returns the desired presentation mode for this window's surface
	avk::unique_function<vk::PresentModeKHR(const vk::SurfaceKHR&)> mPresentationModeSelector;

	// A function which returns the desired number of presentable images in the swap chain
	avk::unique_function<uint32_t()> mNumberOfPresentableImagesGetter;

	// A function which returns the number of images which can be rendered into concurrently
	// According to this number, the number of semaphores and fences will be determined.
	avk::unique_function<frame_id_t()> mNumberOfConcurrentFramesGetter;

	// A function which returns attachments which shall be attached to the back buffer
	// in addition to the obligatory color attachment.
	avk::unique_function<std::vector<avk::attachment>()> mAdditionalBackBufferAttachmentsGetter;
#pragma endregion

#pragma region swap chain data for this window surface
	// The frame counter/frame id/frame index/current frame number
	frame_id_t mCurrentFrame;
	
	// The window's surface
	vk::UniqueHandle<vk::SurfaceKHR, DISPATCH_LOADER_CORE_TYPE> mSurface;
	// The swap chain create info
	vk::SwapchainCreateInfoKHR mSwapChainCreateInfo;
	// The swap chain for this surface
	vk::UniqueHandle<vk::SwapchainKHR, DISPATCH_LOADER_CORE_TYPE> mSwapChain;
	// The swap chain's image format
	vk::Format mSwapChainImageFormat;
	// The swap chain's color space
	vk::ColorSpaceKHR mSwapChainColorSpace;
	// The swap chain's extent
	vk::Extent2D mSwapChainExtent;
	// Queue family indices which have shared ownership of the swap chain images
	avk::unique_function<std::vector<uint32_t>()> mQueueFamilyIndicesGetter;
	// Image data of the swap chain images
	vk::ImageCreateInfo mImageCreateInfoSwapChain;
	// All the image views of the swap chain
	std::vector<avk::image_view> mSwapChainImageViews; // ...but the image views do!

	// A function which returns the image usage properties for the back buffer images
	avk::unique_function<avk::image_usage()> mImageUsageGetter;

#pragma endregion

#pragma region indispensable sync elements
	// Fences to synchronize between frames
	std::vector<avk::fence> mFramesInFlightFences;
	// Semaphores to wait for an image to become available
	std::vector<avk::semaphore> mImageAvailableSemaphores;
	// Fences to make sure that no two images are written into concurrently
	std::vector<int> mImagesInFlightFenceIndices;

	// Semaphores to be waited on before presenting the image PER FRAME.
	// The first element in the tuple refers to the frame id which is affected.
	// The second element in the is the semaphore to wait on.
	std::vector<std::tuple<frame_id_t, avk::semaphore>> mPresentSemaphoreDependencies;
#pragma endregion

	// The renderpass used for the back buffers
	avk::renderpass mBackBufferRenderpass;

	// The backbuffers of this window
	std::vector<avk::framebuffer> mBackBuffers;

	// The render pass for this window's UI calls
	vk::RenderPass mUiRenderPass;

	// This container handles (single use) command buffers' lifetimes.
	// A command buffer's lifetime lasts until the specified int64_t frame-id + number_of_in_flight_frames()
	std::deque<std::tuple<frame_id_t, avk::command_buffer>> mLifetimeHandledCommandBuffers;

	// This container handles old swap chain resources which are to be deleted at a certain future frame
	std::deque<std::tuple<frame_id_t, outdated_swapchain_resource_t>> mOutdatedSwapChainResources;

	// The queue that is used for presenting. It MUST be set to a valid queue if window::render_frame() is ever going to be invoked.
	avk::unique_function<avk::queue*()> mPresentationQueueGetter;

	// The presentation queue that is active
	avk::queue* mActivePresentationQueue = nullptr;

	// Current frame's image index
	uint32_t mCurrentFrameImageIndex;
	// Previous frame's image index
	uint32_t mPreviousFrameImageIndex;

	// Must be consumed EXACTLY ONCE per frame:
	std::optional<avk::semaphore> mCurrentFrameImageAvailableSemaphore;

	// Must be used EXACTLY ONCE per frame:
	std::optional<avk::fence> mCurrentFrameFinishedFence;
	
#pragma region recreation management
	struct recreation_determinator
	{
		/** Formulate the reason why resource recreation has become necessary.
		 */
		enum struct reason
		{
			suboptimal_swap_chain,                      // Indicates that 'vk::Result::eSuboptimalKHR' has been returned as the result of acquireNextImageKHR or presentKHR.
			invalid_swap_chain,                         // Indicates that the swapchain is out of date or otherwise in an invalid state.
			backbuffer_attachments_changed,             // Indicates that the additional attachments of the backbuffer have changed.
			presentation_mode_changed,                  // Indicates that the presentation mode has changed.
			concurrent_frames_count_changed,            // Indicates that the number of concurrent frames has been modified.
			presentable_images_count_changed,           // Indicates that the number of presentable images has been modified.
			image_format_or_properties_changed          // Indicates that the image format of the framebuffer has been modified.
		};
		/** Reset the state of the determinator. This should be done per frame once the required changes have been applied. */
		void reset() { mInvalidatedProperties.reset(); }

		/** Signal that recreation is potentially required and state the reason.
		 *  @param aReason the reason for the recreation
		 */
		void set_recreation_required_for(reason aReason)      { mInvalidatedProperties[static_cast<size_t>(aReason)] = true; }

		/** Probe whether recreation is required due to a specific reason.
		 *  @param aReason the reason for the recreation
		 *  @return true if the bit specified by aReason has been set to 1.
		 */
		bool is_recreation_required_for(reason aReason) const { return mInvalidatedProperties.test(static_cast<size_t>(aReason)); }

		/** Check whether any recreation is necessary.
		 *  This is a useful before more specific conditions are probed.
		 *  @return true if any of the bits have been set to 1.
		 */
		bool is_any_recreation_necessary()              const { return mInvalidatedProperties.any(); }

		/** Check whether a change to number of concurrent frames is the only reason for requiring recreation.
		 *  @return true if 'concurrent_frames_count_changed' is the only bit set to 1.
		 */
		bool has_concurrent_frames_count_changed_only() const { return mInvalidatedProperties.test(static_cast<size_t>(reason::concurrent_frames_count_changed)) && mInvalidatedProperties.count() == 1; }

		/** Check whether swapchain requires recreation.
		 *  @return true if any of the bits other than 'concurrent_frames_count_changed' are set to 1.
		 */
		bool is_swapchain_recreation_necessary()        const { return is_any_recreation_necessary() && !has_concurrent_frames_count_changed_only(); }
	private:
		std::bitset<8> mInvalidatedProperties = 0; // increase the length of bitset if more reasons are added.
	} mResourceRecreationDeterminator;
#pragma endregion
};