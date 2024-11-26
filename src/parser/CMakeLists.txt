
set_source_files_properties("binlogTypes.cpp" PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties("binlogEvent.cpp" PROPERTIES HEADER_FILE_ONLY TRUE)

aux_source_directory(. ParserList)
include_directories(.)
include_directories(../config)
include_directories(../process)
link_libraries(Process)



set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
add_library(Parser STATIC ${ParserList})