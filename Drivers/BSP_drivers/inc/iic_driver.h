// Header:
// File Name: iic_driver.h
// Author: DJS
// Date: 

#ifndef _MY_IIC_H_
#define _MY_IIC_H_

#define USE_CUBEMX_IIC 1    //默认使用cubemx初始化iic

#ifdef __cplusplus
extern "C" {
#endif
//板级驱动和初始化，如果有的话
#include "i2c.h"    
// 前置声明 防止函数指针参数类型未定义
typedef struct IIC_Device_t IIC_Device_t;

// 函数指针类型定义
typedef int32_t (*IIC_TransmitFunc)(IIC_Device_t*, uint16_t, uint8_t*, uint32_t);
typedef int32_t (*IIC_ReceiveFunc)(IIC_Device_t*, uint16_t, uint8_t*, uint32_t);
struct IIC_Device_t{
	//外设句柄
	I2C_HandleTypeDef *hiic;
	uint16_t address;
	//传输超时参数
	uint32_t timeout;
	//传输函数
	IIC_TransmitFunc transmit;
	IIC_ReceiveFunc  receive;
};
void IIC_InitDev(IIC_Device_t *dev, I2C_HandleTypeDef *hiic, uint16_t address);
// 传输
int32_t IIC_Transmit(IIC_Device_t *dev, uint16_t addr, uint8_t *data, uint32_t size);
int32_t IIC_Receive(IIC_Device_t *dev, uint16_t addr, uint8_t *data, uint32_t size);

#ifdef __cplusplus
}
#endif


#endif