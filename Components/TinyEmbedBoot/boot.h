#ifndef _BOOT_H_
#define _BOOT_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

// 上电BOOT等待时间，超时进入app(如有)
#define BOOT_WAIT_TIME_MS 2000
#define APPLICATION_START_ADDRESS 0x08010000
#define FLASH_END_ADDRESS 0x080FFFFF

// 函数指针，用于复位函数实例化
typedef void (*FunctionPointer)(void);
typedef struct {
    uint32_t stack_pointer;
    FunctionPointer reset_handler;
} VectorTableType;

// boot状态机
typedef enum {
    BOOT_STATE_WAIT,
    BOOT_STATE_BOOTLOADER,
    BOOT_STATE_APPLICATION_JUMP,
} BootState_t;

// 错误码定义
typedef enum {
    ERROR_CODE_NO_ERROR = 0x00,
    ERROR_CODE_PARSE_FAILED = 0x01,
    ERROR_CODE_UNKNOWN_CMD = 0x02,
    ERROR_CODE_INVALID_DATA = 0x03,
    ERROR_CODE_FLASH_ERROR = 0x04,
    ERROR_CODE_VERIFY_FAILED = 0x05,
    ERROR_CODE_NUMS
} BootErrorCode_t;

/**
 * @brief 获取错误信息
 * @param errorCode 错误码
 * @return 错误信息字符串
 */
const char *GetErrorMessage(BootErrorCode_t errorCode);

// 数据发送函数指针类型
typedef bool (*Boot_SendData_Func)(uint8_t *data, uint16_t length);

/**
 * @brief Boot模块初始化
 * @param send_func 数据发送函数指针
 */
void Boot_Init(Boot_SendData_Func send_func);

/**
 * @brief boot状态机
 */
void Boot_ProcessStateMachine(void);

/**
 * @brief 验证固件是否合法
 * @return 0不合法 1合法
 */
uint8_t Boot_IsApplicationValid(void);

/**
 * @brief 跳转到固件
 */
void Boot_JumpToApplication(void);
/**
 * @brief 进入BootLoader模式，解析来自上位机的指令，进行固件升级等操作
 * @return BootState_t 返回boot状态，用于跳转app
 */
BootState_t Boot_EnterBootloaderMode(void);

/**
 * @brief 处理接收数据。在数据接收回调函数中调用
 * @param received_byte 按字节处理
 */
void Boot_ReceiveCommand(uint8_t received_byte);

#ifdef __cplusplus
}
#endif

#endif
