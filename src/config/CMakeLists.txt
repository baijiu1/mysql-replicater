
aux_source_directory(. MYCFG)
include_directories(.)
include_directories(../parser)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
add_library(ConFig STATIC ${MYCFG})