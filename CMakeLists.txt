cmake_minimum_required(VERSION 3.10)# compiler settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SRC_FILES src/*.cpp)
file(GLOB_RECURSE TEST_FILES test/*.cpp)

project(networking)

add_executable(networking ${SRC_FILES})

target_compile_options(networking PRIVATE
	-Wall
	-Wextra
	-Wpedantic
	-O2
)

target_link_options(networking PRIVATE
)

add_subdirectory(lib/libevent)
target_link_libraries(networking
	PRIVATE
	event
	event_core
	event_extra
	event_pthreads
	pthread
)

target_include_directories(networking PUBLIC include lib/doctest)
target_include_directories(networking PUBLIC include)




# testing
# project(cmake_test LANGUAGES CXX)
# list(FILTER SRC_FILES EXCLUDE REGEX ".*src/main.cpp$")
# add_executable(test ${SRC_FILES} ${TEST_FILES} ${HEADER_FILES} ${IMPLEMENTATION_FILES})
# target_include_directories(test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include) # includes
