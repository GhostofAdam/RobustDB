# CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -Wall -pthread")
PROJECT(robust_db)

# include
include_directories(include)

# source
FILE(GLOB_RECURSE src_files "src/*.cpp")
set (main_cpp "tools/main.cpp")

# target
add_library(RobustDB SHARED ${src_files})
ADD_EXECUTABLE(main ${main_cpp})
TARGET_LINK_LIBRARIES(main RobustDB)

