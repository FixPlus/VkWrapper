if (DEFINED ENV{VULKAN_SDK})
    message(STATUS "Found Vulkan SDK: $ENV{VULKAN_SDK}")
else ()
    message(FATAL_ERROR "VULKAN_SDK is not set. Please install Vulkan SDK from here: https://vulkan.lunarg.com/")
endif ()