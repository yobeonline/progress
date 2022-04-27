cmake_minimum_required(VERSION 3.20)
include(add_target/add_target.cmake)

project(progress)

add_target(progress
	HEADER_ONLY include/task.hpp
	INCLUDES INTERFACE include
	FEATURES INTERFACE cxx_std_20
	BOOST_TEST test/test_task.cpp
)