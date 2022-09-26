find_program(PYTHON_EXE NAMES python python3 py REQUIRED)


set(VULKAN_REGISTRY_LOCATION ${VULKAN_SDK_PATH}/share/vulkan/registry)
if(WIN32)
    set(VULKAN_LAYER_DESC_LOCATION ${VULKAN_SDK_PATH}/Bin)
elseif(UNIX)
    set(VULKAN_LAYER_DESC_LOCATION ${VULKAN_SDK_PATH}/etc/vulkan/explicit_layer.d)
endif()
add_custom_command(OUTPUT SymbolTable.inc
        COMMAND ${PYTHON_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/generate_headers.py -path=${VULKAN_REGISTRY_LOCATION} >${CMAKE_CURRENT_BINARY_DIR}/SymbolTable.inc)

add_custom_command(OUTPUT DeviceFeatures.inc
        COMMAND ${PYTHON_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/generate_device_feature_map.py -path=${VULKAN_REGISTRY_LOCATION} >${CMAKE_CURRENT_BINARY_DIR}/DeviceFeatures.inc)

add_custom_command(OUTPUT LayerMap.inc
        COMMAND ${PYTHON_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/generate_layer_map.py -path=${VULKAN_LAYER_DESC_LOCATION} >${CMAKE_CURRENT_BINARY_DIR}/LayerMap.inc)


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SymbolTable.inc ${CMAKE_CURRENT_BINARY_DIR}/DeviceFeatures.inc ${CMAKE_CURRENT_BINARY_DIR}/LayerMap.inc DESTINATION ./include/vkw)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB ${PROJECT_NAME}_SOURCE *.cpp *.h *.hpp ../include/vkw/*.h ../include/vkw/*.hpp)

add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCE} SymbolTable.inc DeviceFeatures.inc LayerMap.inc)

add_subdirectory(loader)

target_link_libraries(${PROJECT_NAME} LOADER_LIB)
