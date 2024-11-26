aux_source_directory(. MYCOMMON)

add_subdirectory(libconfig)
include_directories(../config)
include_directories(libconfig/lib)
include_directories(../config/*)
include_directories(.)
link_directories(../config)
include_directories(../net)
link_libraries(Net)
include_directories(../parser)

include_directories(../process)
link_directories(../process)
link_libraries(Process)

link_libraries(Parser)

link_libraries(ConFig)
link_libraries(config)
link_libraries(config++)
find_package(PostgreSQL REQUIRED)


set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
add_library(common STATIC ${MYCOMMON})
target_link_libraries(common PRIVATE PostgreSQL::PostgreSQL)
#target_link_libraries(common libconfig)
#target_link_libraries(common ConFig)