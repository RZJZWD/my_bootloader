#include "boot_cmd.h"
#include "string.h"

static rx_state_t rx_state = RX_STATE_HEADER1;
static command_frame_t current_frame;
static uint16_t data_recived_size = 0;
static bool frame_ready = false;

static void clear_command_frame(command_frame_t *frame) {
    memset(frame, 0, sizeof(command_frame_t));
}
/**
 * @brief 计算校验和
 * @param data 从命令字到数据全部的数据
 * @param length 长度为命令(1byte)+数据长度信息(2byte)+实际数据长度
 * @return
 */
static uint8_t calculate_checksum(uint8_t *data, uint16_t length) {
    uint8_t sum = 0;

    for (uint16_t i = 0; i < length; i++) {
        sum += data[i];
    }
    return (uint8_t)~sum;
}
/**
 * @brief 验证命令是否合法
 * @param cmd 命令字
 * @return
 */
static bool is_valid_command(command_type_t cmd) {
    return ((cmd > CMD_VALID_START) && (cmd < CMD_VALID_END));
}

void command_parser_init(void) {
    rx_state = RX_STATE_HEADER1;
    clear_command_frame(&current_frame);
    data_recived_size = 0;
    frame_ready = false;
}

rx_state_t get_rx_state(void) { return rx_state; }

parse_result_t command_process_byte(uint8_t byte) {
    parse_result_t ret;
    switch (rx_state) {
    case RX_STATE_HEADER1:
        if (byte == FRAME_HEADER1) {
            rx_state = RX_STATE_HEADER2;
        } else {
            rx_state = RX_STATE_HEADER1;
        }
        ret = PARSE_INCOMPLETE;
        break;

    case RX_STATE_HEADER2:
        if (byte == FRAME_HEADER2) {
            rx_state = RX_STATE_CMD;
        } else {
            rx_state = RX_STATE_HEADER1;
            return PARSE_ERROR_HEADER;
        }
        ret = PARSE_INCOMPLETE;
        break;

    case RX_STATE_CMD:
        if (is_valid_command((command_type_t)byte)) {
            current_frame.command = (command_type_t)byte;
            rx_state = RX_STATE_LEN_LOW;
            ret = PARSE_INCOMPLETE;
        } else {
            rx_state = RX_STATE_HEADER1;
            ret = PARSE_ERROR_INVALID_CMD;
        }
        break;

    case RX_STATE_LEN_LOW:
        current_frame.data_length = byte;
        rx_state = RX_STATE_LEN_HIGH;
        ret = PARSE_INCOMPLETE;
        break;

    case RX_STATE_LEN_HIGH:
        data_recived_size = 0;
        current_frame.data_length |= (byte << 8);
        if (current_frame.data_length == 0) {
            rx_state = RX_STATE_CHECKSUM;
            ret = PARSE_INCOMPLETE;
        } else if (current_frame.data_length < FRAME_DATA_SIZE) {
            rx_state = RX_STATE_DATA;
            ret = PARSE_INCOMPLETE;
        } else {
            rx_state = RX_STATE_HEADER1;
            ret = PARSE_ERROR_LENGTH;
        }
        break;

    case RX_STATE_DATA:
        current_frame.data[data_recived_size++] = byte;
        if (data_recived_size == current_frame.data_length) {
            rx_state = RX_STATE_CHECKSUM;
        }
        ret = PARSE_INCOMPLETE;
        break;

    case RX_STATE_CHECKSUM:
        current_frame.checksum = byte;
        uint8_t checksum;

        // // 调试：打印计算校验和的数据
        // uint16_t calc_len = 1 + 2 + current_frame.data_length;
        // printf("Calculating checksum for %d bytes: ", calc_len);
        // for (int i = 0; i < calc_len; i++) {
        //     printf("%02X ", *((uint8_t *)&current_frame.command + i));
        // }
        // printf("\n");

        /*
         *这里要解释一下命令字加数据长度(header_size)为什么是4字节
         *因为command_frame_t结构体的command枚举量在上,数据位(2byte)在下，
         *所以command填充为2byte
         */
        uint16_t header_size = sizeof(command_type_t) + sizeof(uint16_t);
        checksum = calculate_checksum((uint8_t *)&current_frame,
                                      header_size + current_frame.data_length);

        if (checksum == current_frame.checksum) {
            frame_ready = true;
            ret = PARSE_SUCCESS;
        } else {
            ret = PARSE_ERROR_CHECKSUM;
        }
        rx_state = RX_STATE_HEADER1;
        break;
    default:
        rx_state = RX_STATE_HEADER1;
        ret = PARSE_INCOMPLETE;
        break;
    }
    return ret;
}

bool command_get_frame(command_frame_t *frame) {
    if (frame_ready) {
        memcpy(frame, &current_frame, sizeof(command_frame_t));
        frame_ready = false;
        clear_command_frame(&current_frame);
        return true;
    }
    return false;
}

uint16_t command_build_frame(command_type_t cmd, uint8_t *data,
                             uint16_t data_len, uint8_t *output_buffer) {
    uint16_t index = 0;

    // 帧头
    output_buffer[index++] = FRAME_HEADER1;
    output_buffer[index++] = FRAME_HEADER2;

    // 命令字
    output_buffer[index++] = (uint8_t)cmd;

    // 数据长度（小端序）
    output_buffer[index++] = (uint8_t)(data_len & 0xff);        // 低八位
    output_buffer[index++] = (uint8_t)((data_len >> 8) & 0xff); // 高八位

    // 数据
    if (data != NULL && data_len > 0) {
        memcpy(&output_buffer[index], data, data_len);
        index += data_len;
    }

    // 校验和
    uint8_t checksum = calculate_checksum(&output_buffer[2], index - 2);
    output_buffer[index++] = checksum;

    return index;
}
