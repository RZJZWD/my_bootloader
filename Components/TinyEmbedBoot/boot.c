#include "boot.h"
#include "boot_cmd.h"
#include "key_driver.h"
#include "led_driver.h"

extern KEY_Device_t key_device;
extern LED_Device_t led_device;

static BootState_t current_boot_state = BOOT_STATE_WAIT;
static parse_result_t command_parse_result;
static command_frame_t current_frame;
static volatile bool is_recived = false;
static volatile bool is_process_error = false;

// 发送函数指针
static Boot_SendData_Func boot_send_func = NULL;

// Boot初始化状态
static bool boot_initialized = false;

BootState_t Boot_GetCurrentState(void) { return current_boot_state; }
void Boot_Init(Boot_SendData_Func send_func) {
    if (boot_initialized) {
        return; // 避免重复初始化
    }

    // 初始化命令解析器
    command_parser_init();

    // 设置发送函数
    boot_send_func = send_func;

    // 初始化状态机
    current_boot_state = BOOT_STATE_WAIT;
    is_recived = false;
    is_process_error = false;

    boot_initialized = true;
}

void Boot_ProcessStateMachine(void) {
    switch (current_boot_state) {
    case BOOT_STATE_WAIT:
        if (HAL_GetTick() > BOOT_WAIT_TIME_MS) {
            current_boot_state = BOOT_STATE_APPLICATION_JUMP;
        } else if (KEY_GetState(&key_device) == KEY_State_DOWN) {
            current_boot_state = BOOT_STATE_BOOTLOADER;
        }
        break;

    case BOOT_STATE_BOOTLOADER:
        BootState_t bootloader_result = Boot_EnterBootloaderMode();
        if (bootloader_result == BOOT_STATE_APPLICATION_JUMP) {
            current_boot_state = BOOT_STATE_APPLICATION_JUMP;
        }
        break;

    case BOOT_STATE_APPLICATION_JUMP:
        if (Boot_IsApplicationValid()) {
            Boot_JumpToApplication();
        } else {
            current_boot_state = BOOT_STATE_BOOTLOADER;
        }
        break;

    default:
        current_boot_state = BOOT_STATE_BOOTLOADER;
        break;
    }
}

uint8_t Boot_IsApplicationValid(void) {
    const VectorTableType *app_vector_table =
        (VectorTableType *)APPLICATION_START_ADDRESS;

    // 检查栈指针是否在有效地址范围内
    if ((app_vector_table->stack_pointer < 0x20000000) ||
        (app_vector_table->stack_pointer > 0x20020000)) {
        return 0;
    }

    // 检查复位处理函数指针是否在有效地址范围内
    uint32_t reset_handler_address = (uint32_t)app_vector_table->reset_handler;
    if (reset_handler_address < APPLICATION_START_ADDRESS ||
        reset_handler_address > FLASH_END_ADDRESS) {
        return 0;
    }

    // 检查中断向量表是否包含有效数据
    uint32_t *app_start_address = (uint32_t *)APPLICATION_START_ADDRESS;
    for (int i = 0; i < 16; i++) {
        if (app_start_address[i] != 0xFFFFFFFF &&
            app_start_address[i] != 0x00000000) {
            return 1;
        }
    }
    return 0;
}

void Boot_JumpToApplication(void) {
    const VectorTableType *app_vector_table =
        (VectorTableType *)APPLICATION_START_ADDRESS;

    // 禁用中断
    __disable_irq();

    // 设置主栈指针
    __set_MSP(app_vector_table->stack_pointer);

    // 设置中断向量表偏移
    SCB->VTOR = APPLICATION_START_ADDRESS;

    // 跳转到应用程序复位处理函数
    app_vector_table->reset_handler();

    // 如果跳转失败，程序不会运行到这里
    while (1)
        ;
}

BootState_t Boot_EnterBootloaderMode(void) {
    // Bootloader模式实现
    if (is_recived) {
        // 命令处理状态机
    } else if (is_process_error) {
        // 给上位机发送错误消息，利用command_build_frame打包，
        // 命令字为CMD_ERROR_RESPONSE 数据为错误信息表
        // 写一个错误信息表用来根据错误号给上位机反馈错误信息
    }
}

void Boot_ReceiveCommand(uint8_t received_byte) {
    // 命令处理实现
    command_parse_result = command_process_byte(received_byte);

    switch (command_parse_result) {
    case PARSE_SUCCESS:
        if (command_get_frame(&current_frame)) {
            is_recived = true;
            is_process_error = false;
        }

        break;
    case PARSE_ERROR_HEADER:
    case PARSE_ERROR_INVALID_CMD:
    case PARSE_ERROR_LENGTH:
    case PARSE_ERROR_CHECKSUM:
        is_process_error = true;
        is_recived = false;
        break;
    case PARSE_INCOMPLETE:
        // 正常状态，不做处理
        break;
    default:
        is_process_error = true;
        is_recived = false;
        break;
    }
}