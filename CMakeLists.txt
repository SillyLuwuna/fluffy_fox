cmake_minimum_required(VERSION 4.3)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PROJECT "fluffy_fox")
set(LIB "fluffy_fox_lib")
set(BIN "fluffy_fox")
set(MAIN "src/main.cpp")
set(SRC_DIR "src")
set(TEST_DIR "test")
set(INCLUDE_DIR "include")
set(THIRD_PARTY_DIR "third_party")

option(BUILD_TESTS "Build tests" OFF)

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR})
set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_DIR})

project(${PROJECT})

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${SRC_DIR}/*.cpp)
list(FILTER SRC_FILES EXCLUDE REGEX ".*${MAIN}$")
file(GLOB_RECURSE INCLUDE_FILES CONFIGURE_DEPENDS ${INCLUDE_DIR}/*.hpp)
add_library(${LIB} STATIC ${SRC_FILES} ${INCLUDE_FILES})
set_target_properties(${LIB} PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(${LIB} PUBLIC ${INCLUDE_DIR})

target_compile_definitions(${LIB} PRIVATE $<$<CONFIG:Debug>:DEBUG>)

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

add_subdirectory(${THIRD_PARTY_DIR}/libevent)
add_subdirectory(${THIRD_PARTY_DIR}/bgfx)
add_subdirectory(${THIRD_PARTY_DIR}/glfw)
add_subdirectory(${THIRD_PARTY_DIR}/spdlog)
add_subdirectory(${THIRD_PARTY_DIR}/hwinfo)

# # libevent bugfix on newer gcc versions
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

target_link_libraries(${LIB} PUBLIC
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

add_executable(${BIN} ${MAIN})

target_link_libraries(${BIN} PUBLIC ${LIB})

target_compile_options(${BIN} PRIVATE
	-Wall
	-Wextra
	-Wpedantic
	-O3
)

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

