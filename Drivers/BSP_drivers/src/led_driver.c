#include "led_driver.h"
void LED_InitDev(LED_Device_t *dev, GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin,
                 uint8_t ActiveLevel) {
    // 设备句柄是否有效，如果是无效指针就退出
    if (dev == NULL)
        return;
    dev->GPIOx = GPIOx;
    dev->Pin = GPIO_Pin;
    dev->ActiveLevel = ActiveLevel;

#if (USE_CUBEMX_GPIO == 0)
    // 如果不使用cubemx初始化就要自己初始化
    // 初始化代码
#endif

    LED_Off(dev);
}

// 控制函数
void LED_On(LED_Device_t *dev) {
    dev->State = LED_State_ON;
    HAL_GPIO_WritePin(
        dev->GPIOx, dev->Pin,
        (GPIO_PinState)(dev->ActiveLevel ? GPIO_PIN_SET : GPIO_PIN_RESET));
}
void LED_Off(LED_Device_t *dev) {
    dev->State = LED_State_OFF;
    HAL_GPIO_WritePin(
        dev->GPIOx, dev->Pin,
        (GPIO_PinState)(dev->ActiveLevel ? GPIO_PIN_RESET : GPIO_PIN_SET));
}
void LED_Toggle(LED_Device_t *dev) {
    dev->State = LED_State_TOGGLE;
    HAL_GPIO_TogglePin(dev->GPIOx, dev->Pin);
}
void LED_Blink(LED_Device_t *dev, uint32_t delay_ms) {
    dev->State = LED_State_BLINK;
    static uint32_t startTime;
    uint32_t currentTime;
    currentTime = HAL_GetTick();
    if (currentTime - startTime > delay_ms) {
        LED_Toggle(dev);
        startTime = HAL_GetTick();
    }
}

// 状态查询
uint8_t LED_GetState(LED_Device_t *dev) { return dev->State; }

void LED_DeInitDev(LED_Device_t *dev) { HAL_GPIO_DeInit(dev->GPIOx, dev->Pin); }