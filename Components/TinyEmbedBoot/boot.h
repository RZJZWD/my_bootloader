#ifndef _BOOT_H_
#define _BOOT_H_
#include "boot_cmd.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

// 编译器按字节对齐属性
#if defined(__CC_ARM)
#define ALIGNED(n) __attribute__((aligned(n)))
#elif defined(__CLANG_ARM)
#define ALIGNED(n) __attribute__((aligned(n)))
#elif defined(__IAR_SYSTEMS_ICC__)
#define ALIGNED(n) __ALIGNED(n)
#elif defined(__GNUC__)
#define ALIGNED(n) __attribute__((aligned(n)))
#elif defined(_MSC_VER)
#define ALIGNED(n) __declspec(align(n))
#else
#error "Unsupported compiler"
#endif

// 设备信息
#define DEVICE_INFO_MODEL "STM32H750"
#define DEVICE_INFO_FLASH_SIZE (128 * 1024)
#define DEVICE_INFO_APP_ADDRESS (0x08010000)
#define DEVICE_INFO_BOOT_VERSION "v1.1.1"
#define DEVICE_INFO_MODEL_LENGTH 32
#define DEVICE_INFO_BOOT_VERSION_LENGTH 16
// 上电BOOT等待时间，超时进入app(如有)
#define BOOT_WAIT_TIME_MS 2000
#define APPLICATION_START_ADDRESS DEVICE_INFO_APP_ADDRESS
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

// 设备信息结构体（1字节对齐）
typedef struct {
    char model[DEVICE_INFO_MODEL_LENGTH];
    uint32_t flashSize;
    uint32_t appAddr;
    char bootVersion[DEVICE_INFO_BOOT_VERSION_LENGTH];
} ALIGNED(1) deviceInfo_t;

typedef union {
    uint8_t rawData[sizeof(deviceInfo_t)];
    deviceInfo_t deviceInfo;
} BOOT_DeviceInfo_t;

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

/*暴露接口用来测试*/
uint8_t Boot_ProcessUploadCommand(command_frame_t *frame);

#ifdef __cplusplus
}
#endif

#endif
