cmake_minimum_required(VERSION 3.22)

project(test_boot_cmd)

# 设置C标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 启用测试
enable_testing()

# 包含头文件目录
include_directories(
    ../Components
    ../Components/TinyEmbedBoot
)

# 创建测试可执行文件 - 直接包含所有需要的源文件
add_executable(test_boot_cmd 
    test_boot_cmd.c
    ../Components/TinyEmbedBoot/boot_cmd.c
)
# 添加测试
add_test(NAME test_boot_cmd COMMAND test_boot_cmd)
