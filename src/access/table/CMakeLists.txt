
aux_source_directory(. TAB_LIST)
include_directories(../common)
find_package(PostgreSQL REQUIRED)
#target_include_directories(table PUBLIC ../common/acesscomm)

link_libraries(acesscomm)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
message(${TAB_LIST})
add_library(tab1 STATIC ${TAB_LIST})

target_link_libraries(tab1 PRIVATE PostgreSQL::PostgreSQL)
#target_link_libraries(table PUBLIC acesscomm)
