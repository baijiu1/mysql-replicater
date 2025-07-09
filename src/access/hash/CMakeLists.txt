aux_source_directory(. HASH_LIST)
include_directories(.)
#include_directories(../common)
#find_package(PostgreSQL REQUIRED)
#target_include_directories(table PUBLIC ../common/acesscomm)

#set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
#
#add_library(hash1 STATIC ${HASH_LIST})

#target_link_libraries(hash1 PRIVATE PostgreSQL::PostgreSQL)
#target_link_libraries(table PUBLIC acesscomm)