find_program(PYTHON_EXE NAMES python python3 py REQUIRED)

if(WIN32)
    set(VKW_USE_PLATFORM win32)
elseif(LINUX)
    set(VKW_USE_PLATFORM wayland) # TODO: add option to choose window platform for linux
else(APPLE)
    set(VKW_USE_PLATFORM macos)
endif()

set(VULKAN_REGISTRY_LOCATION ${VULKAN_SDK_PATH}/share/vulkan/registry)

add_custom_command(OUTPUT SymbolTable.inc
        COMMAND ${PYTHON_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/generate_headers.py -platform=${VKW_USE_PLATFORM} -path=${VULKAN_REGISTRY_LOCATION} >${CMAKE_CURRENT_BINARY_DIR}/SymbolTable.inc)

add_custom_command(OUTPUT DeviceFeatures.inc
        COMMAND ${PYTHON_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/generate_device_feature_classes.py -path=${VULKAN_REGISTRY_LOCATION} >${CMAKE_CURRENT_BINARY_DIR}/DeviceFeatures.inc)


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SymbolTable.inc ${CMAKE_CURRENT_BINARY_DIR}/DeviceFeatures.inc DESTINATION ./include)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB ${PROJECT_NAME}_SOURCE *.cpp *.h *.hpp ../include/*.h ../include/*.hpp)

add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCE} SymbolTable.inc DeviceFeatures.inc)

add_subdirectory(loader)

target_link_libraries(${PROJECT_NAME} LOADER_LIB)