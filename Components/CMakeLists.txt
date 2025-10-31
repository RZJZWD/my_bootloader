cmake_minimum_required(VERSION 3.22)

project(Components VERSION 1.0.0 LANGUAGES C)

# 添加TinyEmbedBoot组件
add_subdirectory(TinyEmbedBoot)


add_library(Components INTERFACE)

target_link_libraries(Components INTERFACE
    TinyEmbedBoot   #boot代码
)