cmake_minimum_required(VERSION 3.10)# compiler settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SRC_FILES src/*.cpp)
file(GLOB_RECURSE TEST_FILES test/*.cpp)

project(fluffy_fox)

add_executable(fluffy_fox ${SRC_FILES})

target_compile_options(fluffy_fox PRIVATE
	-Wall
	-Wextra
	-Wpedantic
	-O3
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
# set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)

set(BGFX_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BGFX_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(lib/libevent)
add_subdirectory(lib/bgfx)
add_subdirectory(lib/glfw)
add_subdirectory(lib/spdlog)
add_subdirectory(lib/hwinfo)

target_link_libraries(fluffy_fox PRIVATE
	event
	event_core
	event_extra
	event_pthreads
	pthread
	bgfx
	glfw
	spdlog
	hwinfo
)

target_include_directories(fluffy_fox PUBLIC include)
