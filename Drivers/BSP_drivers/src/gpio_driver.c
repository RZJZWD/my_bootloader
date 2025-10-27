#include "gpio_driver.h"
void GPIO_Set(GPIO_Device_t *dev) {
    HAL_GPIO_WritePin(dev->GPIOx, dev->Pin, GPIO_PIN_SET);
}
void GPIO_Reset(GPIO_Device_t *dev) {
    HAL_GPIO_WritePin(dev->GPIOx, dev->Pin, GPIO_PIN_RESET);
}
void GPIO_InitDev(GPIO_Device_t *dev, GPIO_TypeDef *GPIOx, uint16_t Pin) {
    // 设备句柄是否有效，如果是无效指针就退出
    if (dev == NULL)
        return;
    dev->GPIOx = GPIOx;
    dev->Pin = Pin;
    dev->Set = GPIO_Set;
    dev->Reset = GPIO_Reset;
#if (USE_CUBEMX_GPIO == 0)
    // 如果不使用cubemx初始化就要自己初始化
    // 初始化代码
#endif
}