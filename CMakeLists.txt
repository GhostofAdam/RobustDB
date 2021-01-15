cmake_minimum_required (VERSION 3.1)

project(RobustDB)
add_subdirectory(src/parser)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BINARY_DIR build)
set(EXECUTABLE_OUTPUT_PATH build)
set(SOURCE
	${SOURCE}
	src/backend/Database.cpp
    src/backend/DBMS.cpp
    src/backend/Index.cpp
    src/backend/Table.cpp
)

set(HEADERS
	${HEADERS}
)

add_library(${CMAKE_PROJECT_NAME}_lib ${SOURCE} ${HEADERS})
add_executable(${CMAKE_PROJECT_NAME} src/main.cpp)
target_link_libraries(${CMAKE_PROJECT_NAME} sql_parser ${CMAKE_PROJECT_NAME}_lib)
