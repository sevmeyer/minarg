cmake_minimum_required(VERSION 3.5)

project(minarg)

# Library target
add_library(minarg INTERFACE)
target_include_directories(minarg INTERFACE "include")
target_compile_features(minarg INTERFACE cxx_std_11)

# Include tests in top-level build
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	add_subdirectory("extern/catch2")
	add_subdirectory("test")
endif ()
