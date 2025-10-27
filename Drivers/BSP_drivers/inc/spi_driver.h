// Header:
// File Name: spi_driver.h
// Author: DJS
// Date:

#ifndef _MY_SPI_H_
#define _MY_SPI_H_

#define USE_CUBEMX_SPI 1 // 默认使用cubemx初始化spi

#ifdef __cplusplus
extern "C" {
#endif

#include "spi.h" //板级驱动和初始化，如果有的话

// 前置声明 防止函数指针内参数类型未定义
typedef struct SPI_Device_t SPI_Device_t;

// 函数指针类型定义
typedef int32_t (*SPI_TransmitFunc)(SPI_Device_t *, uint8_t *, uint32_t);
typedef int32_t (*SPI_ReceiveFunc)(SPI_Device_t *, uint8_t *, uint32_t);
struct SPI_Device_t {
    // 外设句柄
    SPI_HandleTypeDef *hspi;
    // 传输超时参数
    uint32_t transmitTimout;
    uint32_t receiveTimout;
    // 传输函数
    SPI_TransmitFunc transmit;
    SPI_ReceiveFunc receive;
};
void SPI_InitDev(SPI_Device_t *dev, SPI_HandleTypeDef *hspi);
// 传输
int32_t SPI_Transmit(SPI_Device_t *dev, uint8_t *data, uint32_t size);
int32_t SPI_Receive(SPI_Device_t *dev, uint8_t *data, uint32_t size);
int32_t SPI_Transmit_DMA(SPI_Device_t *dev, uint8_t *data, uint32_t size);
#ifdef __cplusplus
}
#endif

#endif