#ifndef VKW_VK_GLOBAL_FUNCTION_1_0
#define VKW_VK_GLOBAL_FUNCTION_1_0(X)
#endif

#ifndef VKW_VK_GLOBAL_FUNCTION_1_1
#define VKW_VK_GLOBAL_FUNCTION_1_1(X)
#endif

// Global scope vulkan API function list
// 'Global scope API' consist of all functionality that does not
// require vulkan instance object or any of its children to be created.

#ifdef VK_VERSION_1_0
VKW_VK_GLOBAL_FUNCTION_1_0(CreateInstance)
VKW_VK_GLOBAL_FUNCTION_1_0(EnumerateInstanceExtensionProperties)
VKW_VK_GLOBAL_FUNCTION_1_0(EnumerateInstanceLayerProperties)
VKW_VK_GLOBAL_FUNCTION_1_0(GetInstanceProcAddr)
#endif

#ifdef VK_VERSION_1_1
VKW_VK_GLOBAL_FUNCTION_1_1(EnumerateInstanceVersion)
#endif

#undef VKW_VK_GLOBAL_FUNCTION_1_0
#undef VKW_VK_GLOBAL_FUNCTION_1_1


// Instance-level core vulkan API functions. Those functions are device-independent
// therefore should be retrieved by vkGetInstanceProcAddr()

#ifndef VKW_VK_INSTANCE_FUNCTION_1_0
#define VKW_VK_INSTANCE_FUNCTION_1_0(X)
#endif

#ifndef VKW_VK_INSTANCE_FUNCTION_1_1
#define VKW_VK_INSTANCE_FUNCTION_1_1(X)
#endif

#ifdef VK_VERSION_1_0
VKW_VK_INSTANCE_FUNCTION_1_0( CreateDevice )
VKW_VK_INSTANCE_FUNCTION_1_0( DestroyInstance )
VKW_VK_INSTANCE_FUNCTION_1_0( EnumeratePhysicalDevices )
VKW_VK_INSTANCE_FUNCTION_1_0( EnumerateDeviceExtensionProperties )
VKW_VK_INSTANCE_FUNCTION_1_0( EnumerateDeviceLayerProperties )
VKW_VK_INSTANCE_FUNCTION_1_0(GetDeviceProcAddr)
VKW_VK_INSTANCE_FUNCTION_1_0( GetPhysicalDeviceFeatures )
VKW_VK_INSTANCE_FUNCTION_1_0( GetPhysicalDeviceFormatProperties )
VKW_VK_INSTANCE_FUNCTION_1_0( GetPhysicalDeviceImageFormatProperties )
VKW_VK_INSTANCE_FUNCTION_1_0( GetPhysicalDeviceMemoryProperties )
VKW_VK_INSTANCE_FUNCTION_1_0( GetPhysicalDeviceProperties )
VKW_VK_INSTANCE_FUNCTION_1_0( GetPhysicalDeviceQueueFamilyProperties )
VKW_VK_INSTANCE_FUNCTION_1_0( GetPhysicalDeviceSparseImageFormatProperties )
#endif

#ifdef VK_API_VERSION_1_1
VKW_VK_INSTANCE_FUNCTION_1_1( EnumeratePhysicalDeviceGroups )
VKW_VK_INSTANCE_FUNCTION_1_1( GetPhysicalDeviceFeatures2 )
VKW_VK_INSTANCE_FUNCTION_1_1( GetPhysicalDeviceProperties2 )
VKW_VK_INSTANCE_FUNCTION_1_1( GetPhysicalDeviceFormatProperties2 )
VKW_VK_INSTANCE_FUNCTION_1_1( GetPhysicalDeviceImageFormatProperties2 )
VKW_VK_INSTANCE_FUNCTION_1_1( GetPhysicalDeviceQueueFamilyProperties2 )
VKW_VK_INSTANCE_FUNCTION_1_1( GetPhysicalDeviceMemoryProperties2 )
VKW_VK_INSTANCE_FUNCTION_1_1( GetPhysicalDeviceSparseImageFormatProperties2 )
VKW_VK_INSTANCE_FUNCTION_1_1( GetPhysicalDeviceExternalBufferProperties )
VKW_VK_INSTANCE_FUNCTION_1_1( GetPhysicalDeviceExternalFenceProperties )
VKW_VK_INSTANCE_FUNCTION_1_1( GetPhysicalDeviceExternalSemaphoreProperties )
#endif

#undef VKW_VK_INSTANCE_FUNCTION_1_0
#undef VKW_VK_INSTANCE_FUNCTION_1_1

// Instance-level extensions API functions. Those functions are device-independent
// therefore should be retrieved by vkGetInstanceProcAddr()

#ifndef VKW_VK_INSTANCE_VK_KHR_surface_FUNCTION
#define VKW_VK_INSTANCE_VK_KHR_surface_FUNCTION(X)
#endif

#ifndef VKW_VK_INSTANCE_VK_KHR_win32_surface_FUNCTION
#define VKW_VK_INSTANCE_VK_KHR_win32_surface_FUNCTION(X)
#endif

#ifdef VK_KHR_surface
VKW_VK_INSTANCE_VK_KHR_surface_FUNCTION( DestroySurfaceKHR )
VKW_VK_INSTANCE_VK_KHR_surface_FUNCTION( GetPhysicalDeviceSurfaceCapabilitiesKHR )
VKW_VK_INSTANCE_VK_KHR_surface_FUNCTION( GetPhysicalDeviceSurfaceFormatsKHR )
VKW_VK_INSTANCE_VK_KHR_surface_FUNCTION( GetPhysicalDeviceSurfaceSupportKHR )
VKW_VK_INSTANCE_VK_KHR_surface_FUNCTION( GetPhysicalDeviceSurfacePresentModesKHR )


#ifdef _WIN32 // VK_KHR_win32_surface
VKW_VK_INSTANCE_VK_KHR_win32_surface_FUNCTION(CreateWin32SurfaceKHR)
VKW_VK_INSTANCE_VK_KHR_win32_surface_FUNCTION(GetPhysicalDeviceWin32PresentationSupportKHR)
#endif

#endif

#undef VKW_VK_INSTANCE_VK_KHR_win32_surface_FUNCTION
#undef VKW_VK_INSTANCE_VK_KHR_surface_FUNCTION

#ifndef VKW_VK_INSTANCE_VK_EXT_debug_utils_FUNCTION
#define VKW_VK_INSTANCE_VK_EXT_debug_utils_FUNCTION(X)
#endif

#ifdef VK_EXT_debug_utils
VKW_VK_INSTANCE_VK_EXT_debug_utils_FUNCTION( CreateDebugUtilsMessengerEXT )
VKW_VK_INSTANCE_VK_EXT_debug_utils_FUNCTION( DestroyDebugUtilsMessengerEXT )
VKW_VK_INSTANCE_VK_EXT_debug_utils_FUNCTION( SubmitDebugUtilsMessageEXT )
#endif

#undef VKW_VK_INSTANCE_VK_EXT_debug_utils_FUNCTION

// Device-level core vulkan API functions. Those functions are device-dependant
// and therefore should be retrieved by vkGetDeviceProcAddr() to avoid
// implementation defined runtime overhead of internal dispatching

#ifndef VKW_VK_DEVICE_FUNCTION_1_0
#define VKW_VK_DEVICE_FUNCTION_1_0(X)
#endif

#ifndef VKW_VK_DEVICE_FUNCTION_1_1
#define VKW_VK_DEVICE_FUNCTION_1_1(X)
#endif

#ifndef VKW_VK_DEVICE_FUNCTION_1_2
#define VKW_VK_DEVICE_FUNCTION_1_2(X)
#endif

#ifdef VK_VERSION_1_0
VKW_VK_DEVICE_FUNCTION_1_0( AllocateCommandBuffers )
VKW_VK_DEVICE_FUNCTION_1_0( AllocateDescriptorSets )
VKW_VK_DEVICE_FUNCTION_1_0( AllocateMemory )
VKW_VK_DEVICE_FUNCTION_1_0( BeginCommandBuffer )
VKW_VK_DEVICE_FUNCTION_1_0( BindBufferMemory )
VKW_VK_DEVICE_FUNCTION_1_0( BindImageMemory )
VKW_VK_DEVICE_FUNCTION_1_0( CmdBeginQuery )
VKW_VK_DEVICE_FUNCTION_1_0( CmdBeginRenderPass )
VKW_VK_DEVICE_FUNCTION_1_0( CmdBindDescriptorSets )
VKW_VK_DEVICE_FUNCTION_1_0( CmdBindIndexBuffer )
VKW_VK_DEVICE_FUNCTION_1_0( CmdBindPipeline )
VKW_VK_DEVICE_FUNCTION_1_0( CmdBindVertexBuffers )
VKW_VK_DEVICE_FUNCTION_1_0( CmdBlitImage )
VKW_VK_DEVICE_FUNCTION_1_0( CmdClearAttachments )
VKW_VK_DEVICE_FUNCTION_1_0( CmdClearColorImage )
VKW_VK_DEVICE_FUNCTION_1_0( CmdClearDepthStencilImage )
VKW_VK_DEVICE_FUNCTION_1_0( CmdCopyBuffer )
VKW_VK_DEVICE_FUNCTION_1_0( CmdCopyBufferToImage )
VKW_VK_DEVICE_FUNCTION_1_0( CmdCopyImage )
VKW_VK_DEVICE_FUNCTION_1_0( CmdDispatch )
VKW_VK_DEVICE_FUNCTION_1_0( CmdDispatchIndirect )
VKW_VK_DEVICE_FUNCTION_1_0( CmdDraw )
VKW_VK_DEVICE_FUNCTION_1_0( CmdDrawIndexed )
VKW_VK_DEVICE_FUNCTION_1_0( CmdDrawIndirect )
VKW_VK_DEVICE_FUNCTION_1_0( CmdDrawIndexedIndirect )
VKW_VK_DEVICE_FUNCTION_1_0( CmdEndQuery )
VKW_VK_DEVICE_FUNCTION_1_0( CmdEndRenderPass )
VKW_VK_DEVICE_FUNCTION_1_0( CmdExecuteCommands )
VKW_VK_DEVICE_FUNCTION_1_0( CmdCopyImageToBuffer )
VKW_VK_DEVICE_FUNCTION_1_0( CmdNextSubpass )
VKW_VK_DEVICE_FUNCTION_1_0( CmdPipelineBarrier )
VKW_VK_DEVICE_FUNCTION_1_0( CmdPushConstants )
VKW_VK_DEVICE_FUNCTION_1_0( CmdResetEvent )
VKW_VK_DEVICE_FUNCTION_1_0( CmdResetQueryPool )
VKW_VK_DEVICE_FUNCTION_1_0( CmdSetDepthBias )
VKW_VK_DEVICE_FUNCTION_1_0( CmdSetEvent )
VKW_VK_DEVICE_FUNCTION_1_0( CmdSetLineWidth )
VKW_VK_DEVICE_FUNCTION_1_0( CmdSetScissor )
VKW_VK_DEVICE_FUNCTION_1_0( CmdSetViewport )
VKW_VK_DEVICE_FUNCTION_1_0( CmdWaitEvents )
VKW_VK_DEVICE_FUNCTION_1_0( CmdWriteTimestamp )
VKW_VK_DEVICE_FUNCTION_1_0( CreateBuffer )
VKW_VK_DEVICE_FUNCTION_1_0( CreateBufferView )
VKW_VK_DEVICE_FUNCTION_1_0( CreateCommandPool )
VKW_VK_DEVICE_FUNCTION_1_0( CreateComputePipelines )
VKW_VK_DEVICE_FUNCTION_1_0( CreateDescriptorPool )
VKW_VK_DEVICE_FUNCTION_1_0( CreateDescriptorSetLayout )
VKW_VK_DEVICE_FUNCTION_1_0( CreateEvent )
VKW_VK_DEVICE_FUNCTION_1_0( CreateFence )
VKW_VK_DEVICE_FUNCTION_1_0( CreateFramebuffer )
VKW_VK_DEVICE_FUNCTION_1_0( CreateGraphicsPipelines )
VKW_VK_DEVICE_FUNCTION_1_0( CreateImage )
VKW_VK_DEVICE_FUNCTION_1_0( CreateImageView )
VKW_VK_DEVICE_FUNCTION_1_0( CreatePipelineLayout )
VKW_VK_DEVICE_FUNCTION_1_0( CreateRenderPass )
VKW_VK_DEVICE_FUNCTION_1_0( CreateQueryPool )
VKW_VK_DEVICE_FUNCTION_1_0( CreateSampler )
VKW_VK_DEVICE_FUNCTION_1_0( CreateSemaphore )
VKW_VK_DEVICE_FUNCTION_1_0( CreateShaderModule )
VKW_VK_DEVICE_FUNCTION_1_0( DestroyBuffer )
VKW_VK_DEVICE_FUNCTION_1_0( DestroyBufferView )
VKW_VK_DEVICE_FUNCTION_1_0( DestroyCommandPool )
VKW_VK_DEVICE_FUNCTION_1_0( DestroyDescriptorPool )
VKW_VK_DEVICE_FUNCTION_1_0( DestroyDescriptorSetLayout )
VKW_VK_DEVICE_FUNCTION_1_0( DestroyDevice )
VKW_VK_DEVICE_FUNCTION_1_0( DestroyEvent )
VKW_VK_DEVICE_FUNCTION_1_0( DestroyFence )
VKW_VK_DEVICE_FUNCTION_1_0( DestroyFramebuffer )
VKW_VK_DEVICE_FUNCTION_1_0( DestroyImage )
VKW_VK_DEVICE_FUNCTION_1_0( DestroyImageView )
VKW_VK_DEVICE_FUNCTION_1_0( DestroyPipeline )
VKW_VK_DEVICE_FUNCTION_1_0( DestroyPipelineLayout )
VKW_VK_DEVICE_FUNCTION_1_0( DestroyQueryPool )
VKW_VK_DEVICE_FUNCTION_1_0( DestroyRenderPass )
VKW_VK_DEVICE_FUNCTION_1_0( DestroySampler )
VKW_VK_DEVICE_FUNCTION_1_0( DestroySemaphore )
VKW_VK_DEVICE_FUNCTION_1_0( DestroyShaderModule )
VKW_VK_DEVICE_FUNCTION_1_0( DeviceWaitIdle )
VKW_VK_DEVICE_FUNCTION_1_0( EndCommandBuffer )
VKW_VK_DEVICE_FUNCTION_1_0( FlushMappedMemoryRanges )
VKW_VK_DEVICE_FUNCTION_1_0( FreeCommandBuffers )
VKW_VK_DEVICE_FUNCTION_1_0( FreeDescriptorSets )
VKW_VK_DEVICE_FUNCTION_1_0( FreeMemory )
VKW_VK_DEVICE_FUNCTION_1_0( GetBufferMemoryRequirements )
VKW_VK_DEVICE_FUNCTION_1_0( GetDeviceQueue )
VKW_VK_DEVICE_FUNCTION_1_0( GetEventStatus )
VKW_VK_DEVICE_FUNCTION_1_0( GetImageMemoryRequirements )
VKW_VK_DEVICE_FUNCTION_1_0( GetImageSparseMemoryRequirements )
VKW_VK_DEVICE_FUNCTION_1_0( GetImageSubresourceLayout )
VKW_VK_DEVICE_FUNCTION_1_0( GetQueryPoolResults )
VKW_VK_DEVICE_FUNCTION_1_0( InvalidateMappedMemoryRanges )
VKW_VK_DEVICE_FUNCTION_1_0( GetDeviceMemoryCommitment )
VKW_VK_DEVICE_FUNCTION_1_0( MapMemory )
VKW_VK_DEVICE_FUNCTION_1_0( QueueSubmit )
VKW_VK_DEVICE_FUNCTION_1_0( QueueWaitIdle )
VKW_VK_DEVICE_FUNCTION_1_0( ResetCommandBuffer )
VKW_VK_DEVICE_FUNCTION_1_0( ResetEvent )
VKW_VK_DEVICE_FUNCTION_1_0( ResetFences )
VKW_VK_DEVICE_FUNCTION_1_0( SetEvent )
VKW_VK_DEVICE_FUNCTION_1_0( UnmapMemory )
VKW_VK_DEVICE_FUNCTION_1_0( UpdateDescriptorSets )
VKW_VK_DEVICE_FUNCTION_1_0( WaitForFences )
VKW_VK_DEVICE_FUNCTION_1_0( CreatePipelineCache )
VKW_VK_DEVICE_FUNCTION_1_0( DestroyPipelineCache )
VKW_VK_DEVICE_FUNCTION_1_0( GetPipelineCacheData )
VKW_VK_DEVICE_FUNCTION_1_0( MergePipelineCaches )
VKW_VK_DEVICE_FUNCTION_1_0( ResetDescriptorPool )
VKW_VK_DEVICE_FUNCTION_1_0( GetRenderAreaGranularity )
VKW_VK_DEVICE_FUNCTION_1_0( CmdSetBlendConstants )
VKW_VK_DEVICE_FUNCTION_1_0( CmdSetDepthBounds )
VKW_VK_DEVICE_FUNCTION_1_0( CmdSetStencilCompareMask )
VKW_VK_DEVICE_FUNCTION_1_0( CmdSetStencilWriteMask )
VKW_VK_DEVICE_FUNCTION_1_0( CmdSetStencilReference )
VKW_VK_DEVICE_FUNCTION_1_0( CmdUpdateBuffer )
VKW_VK_DEVICE_FUNCTION_1_0( CmdFillBuffer )
VKW_VK_DEVICE_FUNCTION_1_0( CmdResolveImage )
VKW_VK_DEVICE_FUNCTION_1_0( QueueBindSparse )
VKW_VK_DEVICE_FUNCTION_1_0( GetFenceStatus )
VKW_VK_DEVICE_FUNCTION_1_0( ResetCommandPool )
VKW_VK_DEVICE_FUNCTION_1_0( CmdCopyQueryPoolResults )
#endif

#ifdef VK_API_VERSION_1_1
VKW_VK_DEVICE_FUNCTION_1_1( BindBufferMemory2 )
VKW_VK_DEVICE_FUNCTION_1_1( BindImageMemory2 )
VKW_VK_DEVICE_FUNCTION_1_1( GetDeviceGroupPeerMemoryFeatures )
VKW_VK_DEVICE_FUNCTION_1_1( CmdSetDeviceMask )
VKW_VK_DEVICE_FUNCTION_1_1( CmdDispatchBase )
VKW_VK_DEVICE_FUNCTION_1_1( GetImageMemoryRequirements2 )
VKW_VK_DEVICE_FUNCTION_1_1( GetBufferMemoryRequirements2 )
VKW_VK_DEVICE_FUNCTION_1_1( GetImageSparseMemoryRequirements2 )
VKW_VK_DEVICE_FUNCTION_1_1( TrimCommandPool )
VKW_VK_DEVICE_FUNCTION_1_1( GetDeviceQueue2 )
VKW_VK_DEVICE_FUNCTION_1_1( CreateSamplerYcbcrConversion )
VKW_VK_DEVICE_FUNCTION_1_1( DestroySamplerYcbcrConversion )
VKW_VK_DEVICE_FUNCTION_1_1( CreateDescriptorUpdateTemplate )
VKW_VK_DEVICE_FUNCTION_1_1( DestroyDescriptorUpdateTemplate )
VKW_VK_DEVICE_FUNCTION_1_1( UpdateDescriptorSetWithTemplate )
VKW_VK_DEVICE_FUNCTION_1_1( GetDescriptorSetLayoutSupport )
#endif


#ifdef VK_API_VERSION_1_2
VKW_VK_DEVICE_FUNCTION_1_2( CmdDrawIndirectCount )
VKW_VK_DEVICE_FUNCTION_1_2( CmdDrawIndexedIndirectCount )
VKW_VK_DEVICE_FUNCTION_1_2( CreateRenderPass2 )
VKW_VK_DEVICE_FUNCTION_1_2( CmdBeginRenderPass2 )
VKW_VK_DEVICE_FUNCTION_1_2( CmdNextSubpass2 )
VKW_VK_DEVICE_FUNCTION_1_2( CmdEndRenderPass2 )
VKW_VK_DEVICE_FUNCTION_1_2( ResetQueryPool )
VKW_VK_DEVICE_FUNCTION_1_2( GetSemaphoreCounterValue )
VKW_VK_DEVICE_FUNCTION_1_2( WaitSemaphores )
VKW_VK_DEVICE_FUNCTION_1_2( SignalSemaphore )
VKW_VK_DEVICE_FUNCTION_1_2( GetBufferDeviceAddress )
VKW_VK_DEVICE_FUNCTION_1_2( GetBufferOpaqueCaptureAddress )
VKW_VK_DEVICE_FUNCTION_1_2( GetDeviceMemoryOpaqueCaptureAddress )
#endif
#undef VKW_VK_DEVICE_FUNCTION_1_0
#undef VKW_VK_DEVICE_FUNCTION_1_1
#undef VKW_VK_DEVICE_FUNCTION_1_2



// Device-level extensions API functions. Those functions are device-dependant
// and therefore should be retrieved by vkGetDeviceProcAddr() to avoid
// implementation defined runtime overhead of internal dispatching.


#ifndef VKW_VK_DEVICE_VK_KHR_swapchain_FUNCTION
#define VKW_VK_DEVICE_VK_KHR_swapchain_FUNCTION(X)
#endif

#ifdef VK_KHR_swapchain
VKW_VK_DEVICE_VK_KHR_swapchain_FUNCTION(CreateSwapchainKHR)
VKW_VK_DEVICE_VK_KHR_swapchain_FUNCTION(DestroySwapchainKHR)
VKW_VK_DEVICE_VK_KHR_swapchain_FUNCTION(GetSwapchainImagesKHR)
VKW_VK_DEVICE_VK_KHR_swapchain_FUNCTION(AcquireNextImageKHR)
VKW_VK_DEVICE_VK_KHR_swapchain_FUNCTION(QueuePresentKHR)
#endif


#undef VKW_VK_DEVICE_VK_KHR_swapchain_FUNCTION

#ifndef VKW_VK_DEVICE_VK_EXT_debug_marker_FUNCTION
#define VKW_VK_DEVICE_VK_EXT_debug_marker_FUNCTION(X)
#endif

#ifdef VK_EXT_debug_marker
VKW_VK_DEVICE_VK_EXT_debug_marker_FUNCTION(DebugMarkerSetObjectTagEXT)
VKW_VK_DEVICE_VK_EXT_debug_marker_FUNCTION(DebugMarkerSetObjectNameEXT)
VKW_VK_DEVICE_VK_EXT_debug_marker_FUNCTION(CmdDebugMarkerBeginEXT)
VKW_VK_DEVICE_VK_EXT_debug_marker_FUNCTION(CmdDebugMarkerEndEXT)
VKW_VK_DEVICE_VK_EXT_debug_marker_FUNCTION(CmdDebugMarkerInsertEXT)
#endif

#undef VKW_VK_DEVICE_VK_EXT_debug_marker_FUNCTION
