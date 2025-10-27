#include "spi_driver.h"
int32_t SPI_Transmit(SPI_Device_t *dev, uint8_t *data, uint32_t size) {
    HAL_StatusTypeDef status =
        HAL_SPI_Transmit(dev->hspi, data, size, dev->transmitTimout);
    return (status == HAL_OK) ? 0 : -1;
}
int32_t SPI_Receive(SPI_Device_t *dev, uint8_t *data, uint32_t size) {
    HAL_StatusTypeDef status =
        HAL_SPI_Receive(dev->hspi, data, size, dev->receiveTimout);
    return (status == HAL_OK) ? 0 : -1;
}
int32_t SPI_Transmit_DMA(SPI_Device_t *dev, uint8_t *data, uint32_t size) {
    HAL_StatusTypeDef status = HAL_SPI_Transmit_DMA(dev->hspi, data, size);
    return (status == HAL_OK) ? 0 : -1;
}
void SPI_InitDev(SPI_Device_t *dev, SPI_HandleTypeDef *hspi) {
    // 设备句柄是否有效，如果是无效指针就退出
    if (dev == NULL)
        return;
    dev->hspi = hspi;
    dev->transmitTimout = 100;
    dev->receiveTimout = 500;
    dev->transmit = SPI_Transmit;
    dev->receive = SPI_Receive;
#if (USE_CUBEMX_SPI == 0)
    // 不使用cubemx初始化
    // 初始化代码
#endif
}