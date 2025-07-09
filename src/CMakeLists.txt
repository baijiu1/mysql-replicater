add_subdirectory(common)
add_subdirectory(config)
add_subdirectory(net)
add_subdirectory(parser)
add_subdirectory(process)
add_subdirectory(access)

#link_libraries(common)
#include_directories(./access)

link_libraries(access)
#target_link_directories(access PUBLIC ./access/access)
#target_link_libraries(access)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/src/bin)
#
#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)
#
find_package(CLI11 REQUIRED)

add_executable(mysql_replicater main.cpp)
target_link_libraries(mysql_replicater PRIVATE CLI11)
target_link_libraries(mysql_replicater PUBLIC access)

#target_link_libraries(mysql_replicater PUBLIC ${access})
#target_link_libraries(mysql_replicater ${LIBCONF})

