# BSP 驱动组件
set(DRIVER_NAME BSP_drivers)

# 显式指定源文件（只包含 key 和 led）
set(SOURCES
    "src/key_driver.c"
    "src/led_driver.c"
)

# 显式指定头文件（只包含 key 和 led）
set(HEADERS
    "inc/key_driver.h"
    "inc/led_driver.h"
)

# 检查是否有源文件
if(NOT SOURCES)
    message(WARNING "No source files found in BSP driver component")
endif()

# 创建组件静态库
add_library(${DRIVER_NAME} STATIC ${SOURCES} ${HEADERS})

# 设置目标属性
set_target_properties(${DRIVER_NAME} PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS ON
)

# 设置包含目录
target_include_directories(${DRIVER_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    # 添加其他公共包含目录
)

# 设置私有包含目录（仅组件内部使用）
target_include_directories(${DRIVER_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 添加编译定义
target_compile_definitions(${DRIVER_NAME} PRIVATE
    # BSP 特定的定义
)

# 添加链接库依赖
target_link_libraries(${DRIVER_NAME}
    stm32cubemx  # 依赖 CubeMX 生成的 HAL 库
)

# 安装规则（可选）
install(TARGETS ${DRIVER_NAME} 
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY inc/ 
    DESTINATION include/${DRIVER_NAME}
    FILES_MATCHING PATTERN "*.h"
)
