// Header:
// File Name: TIMPWM_driver.h
// Author: DJS
// Date: 

#ifndef _MY_TIMPWM_H_
#define _MY_TIMPWM_H_

#define USE_CUBEMX_TIM 1    //默认使用cubemx初始化TIM

#ifdef __cplusplus
extern "C" {
#endif

//板级驱动和初始化，如果有的话
#include "tim.h"    

// 前置声明 防止函数指针参数类型未定义
typedef struct TIMPWM_Device_t TIMPWM_Device_t;
// 以下就是驱动的结构体和函数之类的
typedef void (*TIMPWM_StartFunc)(TIMPWM_Device_t*);
typedef void (*TIMPWM_SetComparaFunc)(TIMPWM_Device_t*, uint32_t);
typedef uint32_t (*TIMPWM_GetComparaFunc)(TIMPWM_Device_t*);

struct TIMPWM_Device_t{
	TIM_HandleTypeDef *htim;
	uint32_t channel;
	TIMPWM_StartFunc Start;
	TIMPWM_SetComparaFunc SetCompara;
	TIMPWM_GetComparaFunc GetCompara;
};

void TIMPWM_InitDev(TIMPWM_Device_t *dev, TIM_HandleTypeDef *htim, uint32_t channel);

void TIMPWM_Start(TIMPWM_Device_t *dev);
void TIMPWM_SetCompara(TIMPWM_Device_t *dev, uint32_t compara);
uint32_t TIMPWM_GetCompara(TIMPWM_Device_t *dev);
#ifdef __cplusplus
}
#endif


#endif