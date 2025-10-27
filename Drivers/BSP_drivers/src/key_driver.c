#include "key_driver.h"
void KEY_InitDev(KEY_Device_t *dev, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
                 uint8_t pressDownLevel) {
    // 设备句柄是否有效，如果是无效指针就退出
    if (dev == NULL)
        return;
    dev->GPIOx = GPIOx;
    dev->Pin = GPIO_Pin;
    dev->pressDownLevel = pressDownLevel;
    dev->State = KEY_State_UP;
    dev->LastState = KEY_State_UP;

#if (USE_CUBEMX_GPIO == 0)
    // 如果不使用cubemx初始化就要自己初始化
    // 初始化代码
#endif
}

// 控制函数

// 状态查询
KEY_State_t KEY_GetState(KEY_Device_t *dev) {
    // 获取当前时间
    uint32_t current_time = HAL_GetTick();
    // 读取当前物理状态
    GPIO_PinState pin_state = HAL_GPIO_ReadPin(dev->GPIOx, dev->Pin);
    // 临时存储按键状态
    KEY_State_t current_state;
    if ((pin_state == GPIO_PIN_SET && dev->pressDownLevel) ||
        (pin_state == GPIO_PIN_RESET && !dev->pressDownLevel)) {
        current_state = KEY_State_DOWN;
    } else {
        current_state = KEY_State_UP;
    }

    if (dev->State != current_state) {
        // 消抖处理（20ms）
        if (current_time - dev->lastChangeTime > 20) {
            dev->LastState = dev->State; // 保存上次状态
            dev->State = current_state;  // 更新当前状态
            dev->lastChangeTime = current_time;
            return dev->State;
        }
    } else {
        // 状态没有变化时，重置消抖计时
        dev->lastChangeTime = current_time;
    }

    return KEY_State_NONE;
}