project(mysql_replicater)
cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_ARCHITECTURES "arm64") # 设置为支持 Universal Binary
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)  # 必须为 Debug 模式
add_compile_options(-g)      # 生成调试信息
add_compile_options(-fsanitize=address)
add_link_options(-fsanitize=address)



# 可选：确保使用 C++17 编译
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 例如，设置全局的构建类型
set(CMAKE_BUILD_TYPE Release)

add_subdirectory(src)
