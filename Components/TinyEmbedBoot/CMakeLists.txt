set(COMPONENT_NAME TinyEmbedBoot)

set(SOURCES
    boot.c
    boot_cmd.c
)
set(HEADERS
    boot.h
    boot_cmd.h
    boot_cfg.h
)

# 检查是否有源文件
if(NOT SOURCES)
    message(WARNING "No source files found in ${COMPONENT_NAME}")
endif()

# 创建TinyEmbedBoot组件静态库
add_library(${COMPONENT_NAME} STATIC ${SOURCES} ${HEADERS})

# 设置目标属性
set_target_properties(${COMPONENT_NAME} PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS ON
)
# 设置包含目录
target_include_directories(${COMPONENT_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)
# 添加链接库依赖
target_link_libraries(${COMPONENT_NAME}
    BSP_drivers  # 依赖 BSP驱动的key驱动
)