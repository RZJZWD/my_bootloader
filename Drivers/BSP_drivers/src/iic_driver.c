#include "iic_driver.h"
void IIC_InitDev(IIC_Device_t *dev, I2C_HandleTypeDef *hiic, uint16_t address) {
    // 设备句柄是否有效，如果是无效指针就退出
    if (dev == NULL)
        return;
    dev->hiic = hiic;
    dev->address = address;
    dev->timeout = 50;
    dev->transmit = IIC_Transmit;
    dev->receive = IIC_Receive;
#if (USE_CUBEMX_IIC == 1)
    // 不使用cubemx初始化
    // 初始化代码
#endif
}
// 传输
int32_t IIC_Transmit(IIC_Device_t *dev, uint16_t addr, uint8_t *data,
                     uint32_t size) {
    HAL_StatusTypeDef status =
        HAL_I2C_Master_Transmit(dev->hiic, addr, data, size, dev->timeout);
    return (status == HAL_OK) ? 0 : 1;
}
int32_t IIC_Receive(IIC_Device_t *dev, uint16_t addr, uint8_t *data,
                    uint32_t size) {
    HAL_StatusTypeDef status =
        HAL_I2C_Master_Receive(dev->hiic, addr, data, size, dev->timeout);
    return (status == HAL_OK) ? 0 : 1;
}