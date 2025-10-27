#include "uart_driver.h"
#include "stdlib.h"
#include "string.h"

// 传输函数
void UART_Transmit(UART_Device_t *dev, uint8_t *data, uint16_t size) {
    // 板级串口发送函数
    HAL_UART_Transmit(dev->huart, data, size, HAL_MAX_DELAY);
}
void UART_Receive_IT(UART_Device_t *dev, uint8_t *data, uint16_t size) {
    // 板级串口中断接收函数
    HAL_UART_Receive_IT(dev->huart, data, size);
}

// 设置回调函数
void UART_Set_RxCallback(UART_Device_t *dev,
                         UART_RxCallbackPointer rxcallback) {
    dev->rx_callback = rxcallback;
}
void UART_Set_ErrorCallback(UART_Device_t *dev,
                            UART_ErrorCallbackPointer errorcallback) {
    dev->error_callback = errorcallback;
}

// 初始化函数
void UART_InitDev(UART_Device_t *dev, UART_HandleTypeDef *huart) {
    // 设备句柄是否有效，如果是无效指针就退出
    if (dev == NULL)
        return;

    dev->huart = huart; // hal句柄
    dev->rx_byte = 0;
    dev->rx_buffSize = RX_BUFFER_SIZE;

#if (USE_CUBEMX_UART == 0)
    // 如果不使用cubemx初始化就要自己初始化
    // 初始化代码
#endif
    // 启动接收中断
    HAL_UART_Receive_IT(dev->huart, &dev->rx_byte, 1);
}

// 以下函数请根据具体设备设置

// 自定义中断处理(未开启)
void UART_userIRQ_Handler(UART_Device_t *dev) {}
