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

set(BUILD_SHARED_LIBS OFF)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
# set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)

set(BGFX_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BGFX_INSTALL OFF CACHE BOOL "" FORCE)

set(EVENT__LIBRARY_TYPE "STATIC" CACHE STRING "" FORCE)

add_subdirectory(third_party/libevent)
add_subdirectory(third_party/bgfx)
add_subdirectory(third_party/glfw)
add_subdirectory(third_party/spdlog)
add_subdirectory(third_party/hwinfo)

# libevent bugfix on newer gcc versions
get_property(libevent_targets DIRECTORY third_party/libevent PROPERTY BUILDSYSTEM_TARGETS)
foreach(tgt ${libevent_targets})
    get_target_property(tgt_type ${tgt} TYPE)
    if(tgt_type STREQUAL "STATIC_LIBRARY" OR
       tgt_type STREQUAL "SHARED_LIBRARY" OR
       tgt_type STREQUAL "MODULE_LIBRARY" OR
       tgt_type STREQUAL "EXECUTABLE" OR
       tgt_type STREQUAL "OBJECT_LIBRARY")
	   target_compile_definitions(${tgt} PRIVATE _GNU_SOURCE)
       target_compile_options(${tgt} PRIVATE -include limits.h)
    endif()
endforeach()

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
