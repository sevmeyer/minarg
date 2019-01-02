cmake_minimum_required(VERSION 3.5)

# Test executable
add_executable(minarg-test "main.cpp" "error.cpp" "help.cpp" "parse.cpp" "value.cpp")
target_link_libraries(minarg-test PRIVATE minarg catch2)

# Language properties
set_property(TARGET minarg-test PROPERTY CXX_STANDARD_REQUIRED TRUE)
set_property(TARGET minarg-test PROPERTY CXX_EXTENSIONS FALSE)

# Verbose compiler warnings
if(MSVC)
	target_compile_options(minarg-test PRIVATE /W4 /WX)
else()
	target_compile_options(minarg-test PRIVATE -Wall -Wextra -Werror -pedantic)
endif()
