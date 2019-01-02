cmake_minimum_required(VERSION 3.11)
include(FetchContent)

# Target:  catch2
# Include: catch2/catch.hpp
# Website: https://github.com/catchorg/Catch2
# License: Boost Software License 1.0

# Declaration
FetchContent_Declare(catch2
	URL "https://github.com/catchorg/Catch2/releases/download/v2.3.0/catch.hpp"
	URL_HASH SHA256=5eb8532fd5ec0d28433eba8a749102fd1f98078c5ebf35ad607fb2455a000004
	DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/catch2"
	DOWNLOAD_NO_EXTRACT TRUE)

# Download
FetchContent_GetProperties(catch2)
if(NOT catch2_POPULATED)
	message(STATUS "Populating catch2")
	FetchContent_Populate(catch2)
endif()

# Target
add_library(catch2 INTERFACE)
target_include_directories(catch2 SYSTEM INTERFACE "${CMAKE_CURRENT_BINARY_DIR}")
