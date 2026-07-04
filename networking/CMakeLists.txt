cmake_minimum_required(VERSION 3.10)# compiler settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SRC_FILES src/*.cpp) # recursively finds .cpp files in src and saves them to variable SRC_FILES
file(GLOB_RECURSE HEADER_FILES include/*.hpp)
file(GLOB_RECURSE IMPLEMENTATION_FILES include/*.ipp)
file(GLOB_RECURSE TEST_FILES test/*.cpp)

# binary
project(networking)
add_executable(networking ${SRC_FILES} ${HEADER_FILES} ${IMPLEMENTATION_FILES}) # adds all the .cpp files to be compiled
target_include_directories(networking PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include) # includes


# testing
# project(cmake_test LANGUAGES CXX)
# list(FILTER SRC_FILES EXCLUDE REGEX ".*src/main.cpp$")
# add_executable(test ${SRC_FILES} ${TEST_FILES} ${HEADER_FILES} ${IMPLEMENTATION_FILES})
# target_include_directories(test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include) # includes
