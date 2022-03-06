
file(GLOB ${PROJECT_NAME}_SOURCE *.cpp *.h *.hpp ../include/*.h ../include/*.hpp)

add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCE})

add_subdirectory(loader)

target_link_libraries(${PROJECT_NAME} LOADER_LIB)