find_program(PYTHON_EXE NAMES python python3 py REQUIRED)


set(VULKAN_REGISTRY_LOCATION $ENV{VULKAN_SDK}/share/vulkan/registry)
if(WIN32)
    set(VULKAN_LAYER_DESC_LOCATION $ENV{VULKAN_SDK}/Bin)
elseif(UNIX)
    set(VULKAN_LAYER_DESC_LOCATION $ENV{VULKAN_SDK}/etc/vulkan/explicit_layer.d)
endif()
add_custom_command(OUTPUT SymbolTable.inc
        COMMAND ${PYTHON_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/generate_headers.py -path=${VULKAN_REGISTRY_LOCATION} >${CMAKE_CURRENT_BINARY_DIR}/SymbolTable.inc
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/generate_headers.py)

add_custom_command(OUTPUT DeviceFeatures.inc
        COMMAND ${PYTHON_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/generate_device_feature_map.py -path=${VULKAN_REGISTRY_LOCATION} >${CMAKE_CURRENT_BINARY_DIR}/DeviceFeatures.inc
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/generate_device_feature_map.py)

add_custom_command(OUTPUT LayerMap.inc
        COMMAND ${PYTHON_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/generate_layer_map.py -path=${VULKAN_LAYER_DESC_LOCATION} >${CMAKE_CURRENT_BINARY_DIR}/LayerMap.inc
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/generate_layer_map.py)

add_custom_command(OUTPUT VulkanTypeTraits.inc
        COMMAND ${PYTHON_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/generate_type_constructors.py -path=${VULKAN_REGISTRY_LOCATION} >${CMAKE_CURRENT_BINARY_DIR}/VulkanTypeTraits.inc
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/generate_type_constructors.py)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SymbolTable.inc ${CMAKE_CURRENT_BINARY_DIR}/DeviceFeatures.inc ${CMAKE_CURRENT_BINARY_DIR}/LayerMap.inc ${CMAKE_CURRENT_BINARY_DIR}/VulkanTypeTraits.inc DESTINATION include/vkw)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

if (WIN32)
    set(VULKAN_SDK_LIB_DIR $ENV{VULKAN_SDK}/Lib)
elseif (UNIX)
    set(VULKAN_SDK_LIB_DIR $ENV{VULKAN_SDK}/lib)
endif ()

find_library(SPIRV_TOOLS NAMES SPIRV-Tools PATHS ${VULKAN_SDK_LIB_DIR} NO_DEFAULT_PATH REQUIRED)
find_library(SPIRV_LINK NAMES SPIRV-Tools-link PATHS ${VULKAN_SDK_LIB_DIR} NO_DEFAULT_PATH REQUIRED)
find_library(SPIRV_OPT NAMES SPIRV-Tools-opt PATHS ${VULKAN_SDK_LIB_DIR} NO_DEFAULT_PATH REQUIRED)
message(STATUS "Found SPIRV-Tools: " ${SPIRV_TOOLS})
message(STATUS "Found SPIRV-Link: " ${SPIRV_LINK})
message(STATUS "Found SPIRV-Opt: " ${SPIRV_OPT})

file(GLOB ${PROJECT_NAME}_SOURCE *.cpp *.h *.hpp ../include/vkw/*.h ../include/vkw/*.hpp)

add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCE} ../external/SPIRV-Reflect/spirv_reflect.cpp DeviceFeatures.inc LayerMap.inc VulkanTypeTraits.inc)

add_subdirectory(loader)

target_link_libraries(${PROJECT_NAME} PRIVATE LOADER_LIB ${SPIRV_OPT} ${SPIRV_TOOLS} ${SPIRV_LINK} ${SPIRV_OPT})
target_include_directories(${PROJECT_NAME} PRIVATE $ENV{Vulkan_INCLUDE_DIR} ../external/SPIRV-Reflect)
