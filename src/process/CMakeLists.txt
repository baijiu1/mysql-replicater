aux_source_directory(. processList)


include_directories(../parser)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

add_library(Process STATIC ${processList})