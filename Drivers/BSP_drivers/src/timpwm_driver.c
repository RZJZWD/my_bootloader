#include "timpwm_driver.h"
void TIMPWM_Start(TIMPWM_Device_t *dev) {
    HAL_TIMEx_PWMN_Start(dev->htim, dev->channel);
}
void TIMPWM_SetCompara(TIMPWM_Device_t *dev, uint32_t compara) {
    __HAL_TIM_SET_COMPARE(dev->htim, dev->channel, compara);
}
uint32_t TIMPWM_GetCompara(TIMPWM_Device_t *dev) {
    uint32_t ret;
    ret = __HAL_TIM_GET_COMPARE(dev->htim, dev->channel);
    return ret;
}
void TIMPWM_InitDev(TIMPWM_Device_t *dev, TIM_HandleTypeDef *htim,
                    uint32_t channel) {
    // 设备句柄是否有效，如果是无效指针就退出
    if (dev == NULL)
        return;
    dev->htim = htim;
    dev->channel = channel;
    dev->Start = TIMPWM_Start;
    dev->SetCompara = TIMPWM_SetCompara;
    dev->GetCompara = TIMPWM_GetCompara;

#if (USE_CUBEMX_TIM == 0)
    // 不使用cubemx初始化
    // 初始化代码
#endif
}
