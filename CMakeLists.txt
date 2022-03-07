cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJECT_NAME VkWrapper)

project(${PROJECT_NAME} LANGUAGES CXX)

include(cmake/find_vulkan.cmake)

include_directories(include)
include_directories(external)
add_subdirectory(source)

install(TARGETS ${PROJECT_NAME})
install(DIRECTORY include DESTINATION .)
install(DIRECTORY external/vma DESTINATION ./include)
