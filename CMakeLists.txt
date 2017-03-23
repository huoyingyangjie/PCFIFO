cmake_minimum_required(VERSION 3.5)
project(PCFIFO)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11  -O0")

set(SOURCE_FILES main.cpp  src/kfifo.cpp src/PCFIFO.cpp)

include_directories(./include)
add_executable(runner ${SOURCE_FILES} )
target_link_libraries(runner pthread)

