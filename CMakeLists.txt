cmake_minimum_required(VERSION 2.8)
project(stbdjvul)
add_compile_options(-O3)
include_directories(src)
add_executable(stbdjvul src/stbdjvul.c src/dependencies.c)
target_link_libraries(stbdjvul m)
