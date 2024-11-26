add_subdirectory(common)
add_subdirectory(config)
add_subdirectory(net)
add_subdirectory(parser)
add_subdirectory(process)

link_libraries(common)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/bin)
#
#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)
#

add_executable(mysql_replicater main.cpp)


#target_link_libraries(mysql_replicater ${LIBCOMMON})
#target_link_libraries(mysql_replicater ${LIBCONF})

