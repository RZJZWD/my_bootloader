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
    ERROR_CODE_PARSE_FAILED = 0x01,
    ERROR_CODE_UNKNOWN_CMD = 0x02,
    ERROR_CODE_INVALID_DATA = 0x03,
    ERROR_CODE_FLASH_ERROR = 0x04,
    ERROR_CODE_VERIFY_FAILED = 0x05,
} BootErrorCode_t;

// 数据发送函数指针类型
typedef bool (*Boot_SendData_Func)(uint8_t *data, uint16_t length);

/**
 * @brief Boot模块初始化
 * @param send_func 数据发送函数指针
 */
void Boot_Init(Boot_SendData_Func send_func);
void Boot_ProcessStateMachine(void);
uint8_t Boot_IsApplicationValid(void);
void Boot_JumpToApplication(void);
BootState_t Boot_EnterBootloaderMode(void);
BootState_t Boot_GetCurrentState(void);

// 通信模块
void Boot_ReceiveCommand(uint8_t received_byte);

#ifdef __cplusplus
}
#endif

#endif