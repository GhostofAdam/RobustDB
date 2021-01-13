find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

BISON_TARGET(sql_yy
	${CMAKE_CURRENT_SOURCE_DIR}/sql.y
	${CMAKE_CURRENT_BINARY_DIR}/sql.tab.cpp
)

FLEX_TARGET(sql_lex
	${CMAKE_CURRENT_SOURCE_DIR}/sql.l
	${CMAKE_CURRENT_BINARY_DIR}/sql.yy.cpp
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

ADD_FLEX_BISON_DEPENDENCY(sql_lex sql_yy)

add_library(sql_parser
	${BISON_sql_yy_OUTPUT_SOURCE}
	${FLEX_sql_lex_OUTPUTS}
	${BISON_sql_yy_OUTPUT_HEADER}
)

set(SOURCE
	${SOURCE}
	${CMAKE_CURRENT_SOURCE_DIR}/Execute.cpp
	PARENT_SCOPE
)

set(HEADERS
	${HEADERS}
	${CMAKE_CURRENT_SOURCE_DIR}/type_def.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/Execute.hpp
	PARENT_SCOPE
)