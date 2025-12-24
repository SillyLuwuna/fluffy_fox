cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_SCA_FOR_MODULES ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(FluffyFoxGraphicsEngine)


file(GLOB SOURCES "src/*.cpp")
# file(GLOB HEADERS "include/*.hpp")

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include)
# set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)

set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)


# list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

# set up Vulkan C++ module
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan glfw)


# set up shader compilation

function (add_slang_shader_target TARGET)
	cmake_parse_arguments("SHADER" "" "" "SOURCES" ${ARGN})
	# set(SLANGC_EXECUTABLE ../programs/slang/bin/slangc)
	set(SLANGC_EXECUTABLE slangc)
	set(SHADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/shaders)
	set(ENTRY_POINTS -entry vertMain -entry fragMain)
	file(GLOB SHADER_SOURCES "${SHADERS_DIR}/*.slang")
	add_custom_command(
		  OUTPUT ${SHADERS_DIR}
		  COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
	)
	add_custom_command(
		  OUTPUT  ${SHADERS_DIR}/slang.spv
		  COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o slang.spv
		  WORKING_DIRECTORY ${SHADERS_DIR}
		  DEPENDS ${SHADERS_DIR} ${SHADER_SOURCES}
		  COMMENT "Compiling Slang Shaders"
		  VERBATIM
	)
	add_custom_target(${TARGET} DEPENDS ${SHADERS_DIR}/slang.spv)
endfunction()

add_slang_shader_target(shaders SOURCES ${SHADER_SLANG_SOURCES})
add_dependencies(${PROJECT_NAME} shaders)


# add_library(VulkanCppModule)
# add_library(Vulkan::cppm ALIAS VulkanCppModule)
#
# target_compile_definitions(VulkanCppModule
#         PUBLIC
# 		VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
# 		VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
# )
# target_include_directories(VulkanCppModule
#         PRIVATE
#         "${Vulkan_INCLUDE_DIR}"
# )
# target_link_libraries(VulkanCppModule
#         PUBLIC
#         Vulkan::Vulkan
# )
# target_link_libraries(VulkanCppModule
#         PUBLIC
#         Vulkan::Vulkan
# )
#
# set_target_properties(VulkanCppModule PROPERTIES CXX_STANDARD 20)
#
# target_sources(VulkanCppModule
#         PUBLIC
#         FILE_SET cxx_modules TYPE CXX_MODULES
#         BASE_DIRS
#         "${Vulkan_INCLUDE_DIR}"
#         FILES
#         "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm"
# )
#
# target_link_libraries(${PROJECT_NAME} Vulkan::cppm)
# target_link_libraries(${PROJECT_NAME} Vulkan::cppm glfw)
#
# # Add the vulkan.cppm file directly as a source file
# target_sources(VulkanCppModule
#         PRIVATE
#         "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm"
# )
#
# # find_package(stb REQUIRED)
# # set(STB_INCLUDEDIR ${stb_INCLUDE_DIRS})
#
# # add_executable (glslang::validator IMPORTED)
# # find_program (GLSLANG_VALIDATOR "glslangValidator" HINTS $ENV{VULKAN_SDK}/bin REQUIRED)
# # set_property (TARGET glslang::validator PROPERTY IMPORTED_LOCATION "${GLSLANG_VALIDATOR}")
# # find_program(SLANGC_EXECUTABLE slangc HINTS $ENV{VULKAN_SDK}/bin REQUIRED)

# if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
# 	target_compile_options(${PROJECT_NAME} PRIVATE -fmodules -O2)
# elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
# 	target_compile_options(${PROJECT_NAME} PRIVATE -fmodules)
# elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
#     # MSVC enables modules by default with C++20
# endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_definitions(${PROJECT_NAME} PRIVATE
		VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=0
		VULKAN_HPP_ALLOW_DEFAULT_STRUCT_CONSTRUCTORS=1
	)

	target_compile_options(${PROJECT_NAME} PRIVATE
		-O0
	)
endif()
