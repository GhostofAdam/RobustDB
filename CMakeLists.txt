# CMAKE_MINIMUM_REQUIRED(VERSION 3.1)


PROJECT(robust_db)

set(CMAKE_CXX_FLAGS "--std=c++11 -O3 -Wall -Wextra ${CMAKE_CXX_FLAGS}")

# include
include_directories(
include
)
# source
FILE(GLOB_RECURSE src_files "src/*.cpp")
set (main_cpp "src/main.cpp")

# target
add_library(RobustDB SHARED ${src_files})
ADD_EXECUTABLE(main ${main_cpp})
TARGET_LINK_LIBRARIES(main RobustDB)

