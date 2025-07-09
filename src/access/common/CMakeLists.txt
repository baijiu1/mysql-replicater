aux_source_directory(. COMM)
include_directories(.)
find_package(PostgreSQL REQUIRED)

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
add_library(acesscomm STATIC ${COMM})

target_link_libraries(acesscomm PRIVATE PostgreSQL::PostgreSQL)