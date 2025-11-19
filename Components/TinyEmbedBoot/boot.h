#ifndef _BOOT_H_
#define _BOOT_H_
#include "boot_cfg.h"
// #include "boot_cmd.h"
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
// 设备名称
#define DEVICE_INFO_MODEL BOOT_DEVICE_NAME
// 设备flash大小
#define DEVICE_INFO_FLASH_SIZE (BOOT_FLASH_SIZE) // byte大小
// 设备app加载地址
#define DEVICE_INFO_APP_ADDRESS BOOT_APP_ADDRESS
// 设备固件分包大小
#define DEVICE_INFO_FIRMWARE_PACKET_SIZE BOOT_FIRMWARE_PACKET_SIZE
// 设备boot版本
#define DEVICE_INFO_BOOT_VERSION BOOT_VERSION
// 设备名称长度
#define DEVICE_INFO_MODEL_LENGTH 32
// 设备boot版本信息长度
#define DEVICE_INFO_BOOT_VERSION_LENGTH 16
// 固件信息单个长度 包好 包数 CRC32都是4字节
#define FIRMWARE_PACKET_INFO_SIZE 4

// 上电BOOT等待时间，超时进入app(如有)
#define BOOT_WAIT_TIME_MS BOOT_START_WAIT_TIME_MS
#define APPLICATION_START_ADDRESS BOOT_APP_ADDRESS
#define FLASH_END_ADDRESS BOOT_FLASH_END_ADDRESS

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
    ERROR_CODE_NO_ERROR = 0x00,               // 无错误
    ERROR_CODE_PARSE_FAILED = 0x01,           // 解析：上位机命令帧解析错误
    ERROR_CODE_PARSE_UNKNOWN_CMD = 0x02,      // 解析：未知命令字（解析时使用）
    ERROR_CODE_PARSE_ERROR_LENGTH = 0x03,     // 解析：数据长度错误
    ERROR_CODE_PARSE_ERROR_CHECKSUM = 0x04,   // 解析：校验和错误
    ERROR_CODE_FIRMWARE_INVALID_DATA = 0x05,  // 固件：非法固件数据
    ERROR_CODE_FIRMWARE_FLASH_ERROR = 0x06,   // 固件：Flash错误
    ERROR_CODE_FIRMWARE_VERIFY_FAILED = 0x07, // 固件：校验错误
    ERROR_CODE_NUMS
} BootErrorCode_t;

// 设备信息结构体（1字节对齐）
typedef struct {
    char model[DEVICE_INFO_MODEL_LENGTH];
    uint32_t flashSize;
    uint32_t appAddr;
    uint32_t firmware_packet;
    char bootVersion[DEVICE_INFO_BOOT_VERSION_LENGTH];
} ALIGNED(1) deviceInfo_t;

// 固件结构体
typedef struct {
    uint32_t packetNum;
    uint32_t packetTotalNum;
    uint32_t packetCRC32;
    uint8_t firmware[DEVICE_INFO_FIRMWARE_PACKET_SIZE];
} ALIGNED(1) firmwareInfo_t;

// 设备信息联合体，用于打包
typedef union {
    uint8_t rawData[sizeof(deviceInfo_t)];
    deviceInfo_t deviceInfo;
} BOOT_DeviceInfo_t;

// 固件联合体，用于读取
typedef union {
    uint8_t rawDate[sizeof(firmwareInfo_t)];
    firmwareInfo_t firmwareInfo;
} BOOT_FirmwareInfo_t;
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
