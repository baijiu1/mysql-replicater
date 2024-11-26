
aux_source_directory(. NetList)
include_directories(.)
include_directories(../config)
link_directories(.)
include_directories(/usr/local/include/openssl)
find_package(OpenSSL REQUIRED)
include_directories(../parser)
link_directories(../parser)
link_libraries(Parser)

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
add_library(Net STATIC ${NetList})

target_link_libraries(Net PRIVATE OpenSSL::SSL)
target_link_libraries(Net PRIVATE OpenSSL::Crypto)