cmake_minimum_required(VERSION 3.5)
project(teller)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES teller.cpp)
add_executable(teller ${SOURCE_FILES})
