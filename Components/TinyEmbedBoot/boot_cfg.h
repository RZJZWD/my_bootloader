#ifndef _BOOT_CFG_H_
#define _BOOT_CFG_H_
#include "stm32h7xx.h"

// 基础配置，可自定义
// app加载地址
#define BOOT_APP_ADDRESS (0x08010000)
// 固件分包大小
#define BOOT_FIRMWARE_PACKET_SIZE (512)
// 命令帧中数据的大小
#define BOOT_FRAME_DATA_SIZE 2048 // 命令帧数据长度
// flash结束地址
#define BOOT_FLASH_END_ADDRESS FLASH_END // 这里使用的hal库定义
// flash大小
#define BOOT_FLASH_SIZE FLASH_SIZE // 这里使用的hal库定义
// 设备名称
#define BOOT_DEVICE_NAME "STM32H750"
// 上电等待时间选择启动模式，毫秒
#define BOOT_START_WAIT_TIME_MS 2000 // 选择启动模式等待
// boot版本
#define BOOT_VERSION "v0.0.1"

#endif