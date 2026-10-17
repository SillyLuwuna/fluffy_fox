cmake_minimum_required(VERSION 4.3)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# project variables
set(PROJECT "fluffy_fox")
set(LIB "fluffy_fox_lib")
set(BIN "fluffy_fox")
set(MAIN "src/main.cpp")
set(SRC_DIR "src")
set(TEST_DIR "test")
set(INCLUDE_DIR "include")
set(THIRD_PARTY_DIR "third_party")
set(SHADER_DIR "shaders")
set(SLANGC_EXECUTABLE "/home/luna/Documents/binaries/vulkan_lunarg_sdk/x86_64/bin/slangc")

# project settings and files
option(BUILD_TESTS "Build tests" OFF)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR})
set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_DIR})

project(${PROJECT})

find_package (Vulkan REQUIRED)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${SRC_DIR}/*.cpp)
list(FILTER SRC_FILES EXCLUDE REGEX ".*${MAIN}$")
file(GLOB_RECURSE INCLUDE_FILES CONFIGURE_DEPENDS ${INCLUDE_DIR}/*.hpp)
add_library(${LIB} STATIC ${SRC_FILES} ${INCLUDE_FILES})
set_target_properties(${LIB} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${LIB} PUBLIC ${INCLUDE_DIR})

target_compile_definitions(${LIB} PUBLIC $<$<CONFIG:Debug>:DEBUG>)

set(BUILD_SHARED_LIBS OFF)

# library options
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
# set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)

set(BGFX_BUILD_TOOLS ON CACHE BOOL "" FORCE)
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BGFX_INSTALL OFF CACHE BOOL "" FORCE)

set(EVENT__LIBRARY_TYPE "STATIC" CACHE STRING "" FORCE)

target_compile_definitions(${LIB} INTERFACE
	VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
	VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
	GLFW_INCLUDE_VULKAN=1
)

# libraries
add_subdirectory(${THIRD_PARTY_DIR}/libevent)
# add_subdirectory(${THIRD_PARTY_DIR}/bgfx)
add_subdirectory(${THIRD_PARTY_DIR}/glfw)
add_subdirectory(${THIRD_PARTY_DIR}/spdlog)
# add_subdirectory(${THIRD_PARTY_DIR}/hwinfo)
add_subdirectory(${THIRD_PARTY_DIR}/tinygltf)
add_subdirectory(${THIRD_PARTY_DIR}/glm)

# libevent bugfix on newer gcc versions
get_property(libevent_targets DIRECTORY ${THIRD_PARTY_DIR}/libevent PROPERTY BUILDSYSTEM_TARGETS)
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

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${THIRD_PARTY_DIR}/stb)

target_link_libraries(${LIB} PUBLIC
	event
	event_core
	event_extra
	event_pthreads
	pthread
#	bgfx
	Vulkan::Vulkan
	glfw
	spdlog
#	hwinfo
	stb
)

# shaders

function (add_slang_shader_target TARGET)
	cmake_parse_arguments ("SHADER" "" "" "SOURCES" ${ARGN})
	set(SHADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/shaders)
	set(ENTRY_POINTS -entry vertMain -entry fragMain)
	add_custom_command (
		OUTPUT ${SHADERS_DIR}
		COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
	)
	add_custom_command (
		OUTPUT ${SHADERS_DIR}/slang.spv
		COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o slang.spv
		WORKING_DIRECTORY ${SHADERS_DIR}
		DEPENDS ${SHADERS_DIR} ${SHADER_SOURCES}
		COMMENT "Compiling Slang Shaders"
		VERBATIM
	)
	add_custom_target(${TARGET} DEPENDS ${SHADERS_DIR}/slang.spv)
endfunction()

file(GLOB_RECURSE SHADER_SLANG_SOURCES CONFIGURE_DEPENDS ${SHADER_DIR}/*.slang)
add_slang_shader_target(SHADERS SOURCES ${SHADER_SLANG_SOURCES})
add_dependencies(${LIB} SHADERS)

# file(
# 	COPY ${THIRD_PARTY_DIR}/bgfx/bgfx/src/bgfx_shader.sh
# 	DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_DIR}
# )
# file(GLOB_RECURSE SHADERS CONFIGURE_DEPENDS ${SHADER_DIR}/*.sc)
# include(${THIRD_PARTY_DIR}/bgfx/cmake/bgfxToolUtils.cmake)
# bgfx_compile_shaders(
# 	TYPE VERTEX
# 	SHADERS ${SHADER_DIR}/vs_basic.sc
# 	VARYING_DEF ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_DIR}/varying.def.sc
# 	OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_DIR}
# )

# executable
add_executable(${BIN} ${MAIN} ${SHADERS})

target_link_libraries(${BIN} PUBLIC ${LIB})

target_compile_options(${BIN} PRIVATE
	-Wall
	-Wextra
	-Wpedantic
	-O0
)

# tests
if (BUILD_TESTS)
	add_executable(${BIN}_test ${TEST_MAIN})
	target_link_libraries(${BIN}_test PUBLIC ${LIB})

	target_compile_definitions(${BIN}_test PRIVATE DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN)

	add_subdirectory(lib/doctest)
	target_link_libraries(${BIN}_test PUBLIC
		doctest
	)

	file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS ${TEST_DIR}/*.cpp)
	target_sources(${BIN}_test PRIVATE ${TEST_FILES})
endif (BUILD_TESTS)

