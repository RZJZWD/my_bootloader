#include "boot_cmd.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// 测试用例函数声明
void test_build_frame(void);
void test_parse_frame(void);
void test_parse_invalid_header(void);
void test_parse_invalid_command(void);
void test_parse_invalid_length(void);
void test_parse_invalid_checksum(void);
void test_parse_incomplete_frame(void);
void test_build_and_parse_roundtrip(void);

// 辅助函数：打印帧内容
void print_frame(const command_frame_t *frame, const char *label) {
    printf("%s:\n", label);
    printf("  Command: 0x%02X\n", frame->command);
    printf("  Data Length: %d\n", frame->data_length);
    printf("  Checksum: 0x%02X\n", frame->checksum);

    if (frame->data_length > 0) {
        printf("  Data: ");
        for (uint16_t i = 0; i < frame->data_length; i++) {
            printf("%02X ", frame->data[i]);
        }
        printf("\n");
    }
    printf("\n");
}

// 辅助函数：比较两个帧是否相等
bool compare_frames(const command_frame_t *frame1,
                    const command_frame_t *frame2) {
    if (frame1->command != frame2->command)
        return false;
    if (frame1->data_length != frame2->data_length)
        return false;
    if (frame1->checksum != frame2->checksum)
        return false;
    if (memcmp(frame1->data, frame2->data, frame1->data_length) != 0)
        return false;
    return true;
}

int main() {
    printf("Starting boot command parser tests...\n\n");

    // 初始化解析器
    command_parser_init();

    // 运行测试用例
    test_build_frame();
    test_parse_frame();
    test_parse_invalid_header();
    test_parse_invalid_command();
    test_parse_invalid_length();
    test_parse_invalid_checksum();
    test_parse_incomplete_frame();
    test_build_and_parse_roundtrip();

    printf("All tests passed!\n");
    return 0;
}

// 测试构造命令帧
void test_build_frame(void) {
    printf("=== Test: Build Frame ===\n");

    uint8_t output_buffer[FRAME_SIZE];
    uint8_t test_data[] = {0x11, 0x22, 0x33, 0x44, 0x55};
    uint16_t data_len = sizeof(test_data);

    // 构建带数据的帧
    uint16_t frame_len =
        command_build_frame(CMD_UPLOAD, test_data, data_len, output_buffer);

    printf("Built frame length: %d\n", frame_len);

    // 验证帧结构
    assert(output_buffer[0] == FRAME_HEADER1);
    assert(output_buffer[1] == FRAME_HEADER2);
    assert(output_buffer[2] == CMD_UPLOAD);
    assert(output_buffer[3] == (data_len & 0xFF));        // 长度低字节
    assert(output_buffer[4] == ((data_len >> 8) & 0xFF)); // 长度高字节

    // 验证数据
    assert(memcmp(&output_buffer[5], test_data, data_len) == 0);

    printf("Frame build test passed!\n\n");
}

// 测试解析正确的命令帧
void test_parse_frame(void) {
    printf("=== Test: Parse Frame ===\n");

    command_parser_init();

    // 构建一个测试帧
    uint8_t test_data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t output_buffer[FRAME_SIZE];
    uint16_t frame_len = command_build_frame(CMD_VERIFY, test_data,
                                             sizeof(test_data), output_buffer);

    printf("Built frame length: %d\n", frame_len);
    printf("Frame bytes: ");
    for (int i = 0; i < frame_len; i++) {
        printf("%02X ", output_buffer[i]);
    }
    printf("\n");

    // 逐个字节解析
    parse_result_t result;
    for (uint16_t i = 0; i < frame_len; i++) {
        result = command_process_byte(output_buffer[i]);
        printf("Byte %d: 0x%02X, Parse result: %d\n", i, output_buffer[i],
               result);

        if (i == frame_len - 1) {
            // 在断言前添加更多信息
            if (result != PARSE_SUCCESS) {
                printf("ERROR: Expected PARSE_SUCCESS(%d), got %d\n",
                       PARSE_SUCCESS, result);
                printf("Current state: %d\n", get_rx_state());
            }
            assert(result == PARSE_SUCCESS);
        } else {
            assert(result == PARSE_INCOMPLETE);
        }
    }

    // 获取解析的帧
    command_frame_t parsed_frame;
    bool got_frame = command_get_frame(&parsed_frame);
    assert(got_frame == true);
    assert(parsed_frame.command == CMD_VERIFY);
    assert(parsed_frame.data_length == sizeof(test_data));
    assert(memcmp(parsed_frame.data, test_data, sizeof(test_data)) == 0);

    printf("Frame parse test passed!\n\n");
}

// 测试解析帧头错误的情况
void test_parse_invalid_header(void) {
    printf("=== Test: Parse Invalid Header ===\n");

    command_parser_init();

    // 发送错误的帧头
    parse_result_t result = command_process_byte(0xBB); // 错误的第一个帧头
    assert(result == PARSE_INCOMPLETE);

    result = command_process_byte(FRAME_HEADER1);
    assert(result == PARSE_INCOMPLETE);

    result = command_process_byte(0x66); // 错误的第二个帧头
    assert(result == PARSE_ERROR_HEADER);

    printf("Invalid header test passed!\n\n");
}

// 测试解析无效命令的情况
void test_parse_invalid_command(void) {
    printf("=== Test: Parse Invalid Command ===\n");

    command_parser_init();

    // 发送正确的帧头，但命令无效
    parse_result_t result = command_process_byte(FRAME_HEADER1);
    assert(result == PARSE_INCOMPLETE);

    result = command_process_byte(FRAME_HEADER2);
    assert(result == PARSE_INCOMPLETE);

    result = command_process_byte(0xFF); // 无效命令
    assert(result == PARSE_ERROR_INVALID_CMD);

    printf("Invalid command test passed!\n\n");
}

// 测试解析长度过长的情况
void test_parse_invalid_length(void) {
    printf("=== Test: Parse Invalid Length ===\n");

    command_parser_init();

    // 发送帧头和有效命令
    parse_result_t result = command_process_byte(FRAME_HEADER1);
    result = command_process_byte(FRAME_HEADER2);
    result = command_process_byte(CMD_UPLOAD);

    // 设置过大的数据长度（超过FRAME_SIZE）
    result = command_process_byte(0xFF); // 长度低字节
    assert(result == PARSE_INCOMPLETE);

    result = command_process_byte(0xFF); // 长度高字节，长度 = 0xFFFF
    assert(result == PARSE_ERROR_LENGTH);

    printf("Invalid length test passed!\n\n");
}

// 测试解析校验和错误的情况
void test_parse_invalid_checksum(void) {
    printf("=== Test: Parse Invalid Checksum ===\n");

    command_parser_init();

    // 构建一个帧
    uint8_t test_data[] = {0x01, 0x02, 0x03};
    uint8_t output_buffer[FRAME_SIZE];
    uint16_t frame_len = command_build_frame(CMD_ENTER_BOOT, test_data,
                                             sizeof(test_data), output_buffer);

    // 修改最后一个字节（校验和），使其错误
    output_buffer[frame_len - 1] = 0xFF;

    // 解析除最后一个字节外的所有字节
    for (uint16_t i = 0; i < frame_len - 1; i++) {
        parse_result_t result = command_process_byte(output_buffer[i]);
        assert(result == PARSE_INCOMPLETE);
    }

    // 最后一个字节（错误的校验和）应该导致校验和错误
    parse_result_t result = command_process_byte(output_buffer[frame_len - 1]);
    assert(result == PARSE_ERROR_CHECKSUM);

    printf("Invalid checksum test passed!\n\n");
}

// 测试解析不完整帧的情况
void test_parse_incomplete_frame(void) {
    printf("=== Test: Parse Incomplete Frame ===\n");

    command_parser_init();

    // 只发送部分帧
    uint8_t partial_frame[] = {FRAME_HEADER1, FRAME_HEADER2, CMD_RUN_APP};

    for (uint16_t i = 0; i < sizeof(partial_frame); i++) {
        parse_result_t result = command_process_byte(partial_frame[i]);
        assert(result == PARSE_INCOMPLETE);
    }

    // 验证没有帧就绪
    command_frame_t frame;
    bool got_frame = command_get_frame(&frame);
    assert(got_frame == false);

    printf("Incomplete frame test passed!\n\n");
}

// 测试构建和解析的往返测试
void test_build_and_parse_roundtrip(void) {
    printf("=== Test: Build and Parse Roundtrip ===\n");

    // 测试多个命令类型
    command_type_t test_commands[] = {CMD_ENTER_BOOT,    CMD_UPLOAD, CMD_VERIFY,
                                      CMD_RUN_APP,       CMD_ACK,    CMD_NACK,
                                      CMD_ERROR_RESPONSE};

    // 测试不同长度的数据
    uint8_t test_data_sets[][10] = {
        {},                                  // 无数据
        {0x01},                              // 1字节数据
        {0x01, 0x02, 0x03},                  // 3字节数据
        {0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA} // 6字节数据
    };

    for (size_t cmd_idx = 0;
         cmd_idx < sizeof(test_commands) / sizeof(test_commands[0]);
         cmd_idx++) {
        for (size_t data_idx = 0;
             data_idx < sizeof(test_data_sets) / sizeof(test_data_sets[0]);
             data_idx++) {
            uint16_t data_len = 0;
            if (data_idx > 0) {
                data_len = sizeof(test_data_sets[data_idx]);
            }

            printf("Testing command 0x%02X with %d bytes data...\n",
                   test_commands[cmd_idx], data_len);

            // 构建帧
            uint8_t output_buffer[FRAME_SIZE];
            uint16_t frame_len = command_build_frame(test_commands[cmd_idx],
                                                     test_data_sets[data_idx],
                                                     data_len, output_buffer);

            // 解析帧
            command_parser_init();
            parse_result_t result;
            for (uint16_t i = 0; i < frame_len; i++) {
                result = command_process_byte(output_buffer[i]);
                if (i == frame_len - 1) {
                    assert(result == PARSE_SUCCESS);
                } else {
                    assert(result == PARSE_INCOMPLETE);
                }
            }

            // 验证解析的帧
            command_frame_t parsed_frame;
            bool got_frame = command_get_frame(&parsed_frame);
            assert(got_frame == true);
            assert(parsed_frame.command == test_commands[cmd_idx]);
            assert(parsed_frame.data_length == data_len);
            if (data_len > 0) {
                assert(memcmp(parsed_frame.data, test_data_sets[data_idx],
                              data_len) == 0);
            }
        }
    }

    printf("Roundtrip test passed!\n\n");
}