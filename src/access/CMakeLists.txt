add_subdirectory(table)
add_subdirectory(common)
add_subdirectory(index)
add_subdirectory(hash)

aux_source_directory(. ACC)
#include_directories(table)
link_libraries(tab1)
target_link_directories(tab1 PUBLIC ./table/tab1)
#
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
add_library(access STATIC ${ACC})

target_link_libraries(access PUBLIC tab1)

