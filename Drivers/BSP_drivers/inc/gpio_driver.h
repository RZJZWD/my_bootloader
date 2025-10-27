// Header:
// File Name: GPIO_driver.h
// Author: DJS
// Date: 

#ifndef _MY_GPIO_H_
#define _MY_GPIO_H_

#define USE_CUBEMX_GPIO 1    //默认使用cubemx初始化GPIO

#ifdef __cplusplus
extern "C" {
#endif

//板级驱动和初始化，如果有的话
#include "gpio.h"    

// 前置声明 防止函数指针参数类型未定义
typedef struct GPIO_Device_t GPIO_Device_t;

typedef void (*GPIO_SetFunc)(GPIO_Device_t*);
typedef void (*GPIO_ResetFunc)(GPIO_Device_t*);
// 以下就是驱动的结构体和函数之类的
struct GPIO_Device_t{
	GPIO_TypeDef *GPIOx;
	uint16_t Pin;
	GPIO_SetFunc Set;
	GPIO_ResetFunc Reset;
};
void GPIO_InitDev(GPIO_Device_t *dev, GPIO_TypeDef *GPIOx, uint16_t Pin);
void GPIO_Set(GPIO_Device_t *dev);
void GPIO_Reset(GPIO_Device_t *dev);

#ifdef __cplusplus
}
#endif


#endif