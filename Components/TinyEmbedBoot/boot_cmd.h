#ifndef _BOOT_CMD_H_
#define _BOOT_CMD_H_
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// 命令类型定义
typedef enum {
    CMD_VALID_START,
    CMD_ENTER_BOOT = 0x01,
    CMD_UPLOAD = 0x02,
    CMD_VERIFY = 0x03,
    CMD_RUN_APP = 0x04,
    CMD_ACK = 0x05,
    CMD_NACK = 0x06,
    CMD_ERROR_RESPONSE = 0x07,
    CMD_VALID_END
} command_type_t;

struct test {
    int a : 1;
};

// 帧头定义
#define FRAME_HEADER1 0xAA
#define FRAME_HEADER2 0x55

// 命令帧数据长度大小
#define FRAME_DATA_SIZE 2048
#define FRAME_SIZE (FRAME_DATA_SIZE + 128)

// 命令帧结构
typedef struct {
    command_type_t command;
    // 上位机传来的长度为小端序组成的两字节数据，所以先收到低字节，再收高字节
    uint16_t data_length;
    uint8_t data[FRAME_DATA_SIZE];
    uint8_t checksum;
} command_frame_t;
// 解析结果
typedef enum {
    PARSE_SUCCESS,
    PARSE_ERROR_HEADER,
    PARSE_ERROR_INVALID_CMD,
    PARSE_ERROR_LENGTH,
    PARSE_ERROR_CHECKSUM,
    PARSE_INCOMPLETE
} parse_result_t;

// 接收状态机
typedef enum {
    RX_STATE_IDLE,
    RX_STATE_HEADER1,
    RX_STATE_HEADER2,
    RX_STATE_CMD,
    RX_STATE_LEN_LOW,
    RX_STATE_LEN_HIGH,
    RX_STATE_DATA,
    RX_STATE_CHECKSUM
} rx_state_t;

/**
 * @brief 初始化解析器
 */
void command_parser_init(void);

/**
 * @brief 获取当前接收状态的接口
 * @return 返回当前接收状态
 */
rx_state_t get_rx_state();
/**
 * @brief 按字节接收并解析命令帧
 * @param byte 字节数据
 * @return parse_result_t 解析结果
 */
parse_result_t command_process_byte(uint8_t byte);
/**
 * @brief 获取当前命令帧
 * @param frame 空命令帧类型
 * @return true 获取成功，命令帧还没有准备好
 * @return false 获取失败
 */
bool command_get_frame(command_frame_t *frame);
/**
 * @brief 构建命令帧
 * @param cmd 命令字
 * @param data 数据
 * @param data_len 数据长度
 * @param output_buffer 输出缓冲
 * @return uint16_t 命令帧长度，帧头+命令字+数据位+数据+校验
 */
uint16_t command_build_frame(command_type_t cmd, uint8_t *data,
                             uint16_t data_len, uint8_t *output_buffer);

#ifdef __cplusplus
}
#endif

#endif
