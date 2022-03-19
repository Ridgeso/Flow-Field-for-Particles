cmake_minimum_required(VERSION 3.11.0)
project(Particle VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

message(STATUS "using ${CMAKE_GENERATOR}")

include_directories(${CMAKE_SOURCE_DIR}/include/)

file(GLOB_RECURSE   HEADER_FILES ${CMAKE_SOURCE_DIR}/include/*)
file(GLOB           SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/*)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

add_executable(${PROJECT_NAME}
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

if (${CMAKE_BUILD_TYPE} MATCHES Debug)
    target_compile_definitions(${PROJECT_NAME} PUBLIC DEBUG)
endif()
target_compile_definitions(${PROJECT_NAME} PRIVATE HEAP)

##### LIBRARIES
# OPENGL
find_package(OpenGL REQUIRED)

# Additional Required Libraries
if (WIN32)  # On Windows
    set(LIBRARIES)
else (WIN32)  # on linux
    set(LIBRARIES dl pthread)
endif (WIN32)

target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL ${LIBRARIES} m)



set(LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/lib)

# GLEW LIBRARY
set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "Build the shared glew library disabled" FORCE)  # only static is enable
set(ONLY_LIBS ON CACHE BOOL "Build only libs" FORCE)

add_subdirectory(${LIB_PATH}/glew)
include_directories(${LIB_PATH}/glew/include)
target_link_libraries(${PROJECT_NAME} PRIVATE libglew_static)



# GLFW LIBRARY
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation disabled" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW documentation disabled" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW documentation disabled" FORCE)

add_subdirectory(${LIB_PATH}/glfw)
include_directories(${LIB_PATH}/glfw/include)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)