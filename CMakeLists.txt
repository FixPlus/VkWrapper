cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJECT_NAME VkWrapper)

project(${PROJECT_NAME} LANGUAGES CXX)
set(PROJECT_VERSION 2.0)

include(cmake/find_dependencies.cmake)
include(cmake/find_vulkan_sdk.cmake)

# toggle ReferenceGuard

option(VKW_ENABLE_REFERENCE_GUARD "Toggle ReferenceGuard checker" OFF)
if (VKW_ENABLE_REFERENCE_GUARD)
    add_definitions(-DVKW_ENABLE_REFERENCE_GUARD)
endif ()

if (MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)
endif ()

include_directories(include)

add_subdirectory(source)

include(cmake/generate_export_configuration.cmake)