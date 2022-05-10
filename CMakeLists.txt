cmake_minimum_required(VERSION 3.20)
include(add_target/add_target.cmake)

project(progress)

include(CTest)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")

# cmake-format: off
add_target(progress
	HEADER_ONLY include/task.hpp
	INCLUDES INTERFACE include
	FEATURES INTERFACE cxx_std_20
	BOOST_TEST test/test_task.cpp
)
# cmake-format: on
