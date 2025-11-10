// Header:
// File Name: key_driver.h
// Author: DJS
// Date: 2025年7月5日

#ifndef _MY_KEY_H_
#define _MY_KEY_H_

#define USE_CUBEMX_GPIO 1 // 默认使用cubemx初始化gpio

#ifdef __cplusplus
extern "C" {
#endif

#include "gpio.h"
#include "stdio.h"

// key状态
typedef enum {
    KEY_State_NONE = 0, // 无事件
    KEY_State_UP,
    KEY_State_DOWN,
} KEY_State_t;

// KEY句柄信息
typedef struct {
    GPIO_TypeDef *GPIOx;    // gpio端口，依赖hal库
    uint16_t Pin;           // gpio引脚
    uint8_t pressDownLevel; // 有效电平，0按下低电平，1按下高电平
    KEY_State_t State;      // 当前状态
    KEY_State_t LastState;  // 上次状态
    uint32_t lastChangeTime; // 上次状态变化时间
} KEY_Device_t;

// 初始化
void KEY_InitDev(KEY_Device_t *dev, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
                 uint8_t pressDownLevel);
// 状态查询
KEY_State_t KEY_GetState(KEY_Device_t *dev);
void KEY_DeInitDev(KEY_Device_t *dev);
#ifdef __cplusplus
}
#endif

#endif
