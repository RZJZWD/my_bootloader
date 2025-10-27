// Header:
// File Name: led_driver.h
// Author: DJS
// Date: 2025年7月5日

#ifndef _MY_LED_H_
#define _MY_LED_H_

#define USE_CUBEMX_GPIO 1 // 默认使用cubemx初始化gpio

#ifdef __cplusplus
extern "C" {
#endif

#include "gpio.h"
#include "stdio.h"
// led状态
typedef struct LED_Device_t LED_Device_t;
typedef enum LED_State_t LED_State_t;
enum LED_State_t {
    LED_State_OFF = 0,
    LED_State_ON,
    LED_State_TOGGLE,
    LED_State_BLINK
};

// led句柄信息
struct LED_Device_t {
    GPIO_TypeDef *GPIOx; // gpio端口，依赖hal库
    uint16_t Pin;        // gpio引脚
    uint8_t ActiveLevel; // 有效电平，低电平亮设置为0，高电平亮设置为1
    LED_State_t State;
};

/**
 * @brief led初始化
 * @param led 句柄,其他参数详见LED_Device_t
 * @return void
 * @details 实例化led句柄
 * @see null
 */
void LED_InitDev(LED_Device_t *dev, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
                 uint8_t ActiveLevel);

// 控制函数
void LED_On(LED_Device_t *dev);
void LED_Off(LED_Device_t *dev);
void LED_Toggle(LED_Device_t *dev);
void LED_Blink(LED_Device_t *dev, uint32_t delay);
// 状态查询
uint8_t LED_GetState(LED_Device_t *dev);

#ifdef __cplusplus
}
#endif

#endif
