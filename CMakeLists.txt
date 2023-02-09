cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJECT_NAME VkWrapper)

project(${PROJECT_NAME} LANGUAGES CXX)

include(cmake/find_vulkan.cmake)

find_package(Boost 1.62 REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

if (MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)
endif ()

include_directories(include)
include_directories(external)
add_subdirectory(source)


install(TARGETS ${PROJECT_NAME})
install(DIRECTORY include DESTINATION .)
