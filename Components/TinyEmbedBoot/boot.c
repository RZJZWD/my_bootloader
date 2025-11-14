#include "boot.h"
#include "boot_cmd.h"
#include "key_driver.h"
#include "led_driver.h"
#include <string.h>

extern KEY_Device_t K1;
extern LED_Device_t LED;

static BootState_t current_boot_state = BOOT_STATE_WAIT;
static parse_result_t command_parse_result;
static command_frame_t current_frame;
static volatile bool is_recived = false;
static volatile BootErrorCode_t bootErrorCode = ERROR_CODE_NO_ERROR;
static volatile bool is_run_app = false;
// 发送函数指针
static Boot_SendData_Func boot_send_func = NULL;

// Boot初始化状态
static bool boot_initialized = false;

// 错误信息
const char *ErrorMessage[ERROR_CODE_NUMS] = {
    "No error",              // 0x00
    "Command parse failed",  // 0x01
    "Unknown command",       // 0x02
    "Invalid data format",   // 0x03
    "Flash operation error", // 0x04
    "Verification failed"    // 0x05
};

// 静态函数声明
static void Boot_ProcessReceivedCommand(void);
static bool Boot_SendFrame(command_type_t cmd, uint8_t *data,
                           uint16_t data_len);
static void Boot_SendAckResponse(void);
static void Boot_SendEnterBootResponse(void);
static void Boot_SendErrorResponse(BootErrorCode_t errorCode);

const char *GetErrorMessage(BootErrorCode_t errorCode) {
    if (errorCode >= ERROR_CODE_NUMS) {
        return "Unknown error code";
    }
    return ErrorMessage[errorCode];
}

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
    bootErrorCode = ERROR_CODE_NO_ERROR;
    is_run_app = false;
    boot_initialized = true;
}

// boot状态机及不同状态的处理函数
void Boot_ProcessStateMachine(void) {
    if (!boot_initialized) {
        return;
    }
    switch (current_boot_state) {
    case BOOT_STATE_WAIT:
        if (HAL_GetTick() > BOOT_WAIT_TIME_MS) {
            current_boot_state = BOOT_STATE_APPLICATION_JUMP;
        } else if (KEY_GetState(&K1) == KEY_State_DOWN) {
            current_boot_state = BOOT_STATE_BOOTLOADER;
            const char enter_boot_str[] = "Enter BootLoader Mode\n";
            boot_send_func((uint8_t *)enter_boot_str, strlen(enter_boot_str));
        }
        break;

    case BOOT_STATE_BOOTLOADER:
        BootState_t bootloader_result = Boot_EnterBootloaderMode();
        if (bootloader_result == BOOT_STATE_APPLICATION_JUMP) {
            const char jump_to_app_str[] = "Jump To APP\n";
            boot_send_func((uint8_t *)jump_to_app_str, strlen(jump_to_app_str));
            current_boot_state = BOOT_STATE_APPLICATION_JUMP;
        }
        break;

    case BOOT_STATE_APPLICATION_JUMP:
        if (Boot_IsApplicationValid()) {
            Boot_JumpToApplication();
        } else {
            current_boot_state = BOOT_STATE_BOOTLOADER;
            const char enter_boot_str[] = "Enter BootLoader Mode\n";
            boot_send_func((uint8_t *)enter_boot_str, strlen(enter_boot_str));
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

    // 禁用irq中断，仅关闭IRQ（普通中断），但不关闭FIQ（快速中断）
    __disable_irq();
    // 禁用全部中断
    __set_PRIMASK(1);

    // 复位所有时钟到默认
    HAL_RCC_DeInit();
    KEY_DeInitDev(&K1);
    LED_DeInitDev(&LED);
    // 关闭systick，复位到默认值
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    // 关闭所有中断，清除所有中断挂起标志
    for (uint32_t i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    // 设置中断向量表偏移
    SCB->VTOR = APPLICATION_START_ADDRESS;
    // 设置主栈指针
    __set_MSP(app_vector_table->stack_pointer);
    // 使用RTOS时，这句很重要，设置为特权级模式，使用MSP指针
    __set_CONTROL(0);

    // 在跳转前开启全局中断，让APP可以响应中断
    __set_PRIMASK(0);
    __enable_irq();

    // 跳转到应用程序复位处理函数
    app_vector_table->reset_handler();

    // 如果跳转成功，程序不会运行到这里，用户可以在这里添加处理代码
    while (1)
        ;
}

BootState_t Boot_EnterBootloaderMode(void) {
    // Bootloader模式实现
    LED_Blink(&LED, 1000);
    if (is_recived && bootErrorCode == ERROR_CODE_NO_ERROR) {
        // 命令处理状态机
        is_recived = false; // 立即重置
        Boot_ProcessReceivedCommand();
        if (is_run_app) {
            is_run_app = false;
            return BOOT_STATE_APPLICATION_JUMP;
        }

    } else if (bootErrorCode != ERROR_CODE_NO_ERROR) {
        // 给上位机发送错误消息，利用command_build_frame打包，
        // 命令字为CMD_ERROR_RESPONSE 数据为错误信息表
        // 写一个错误信息表用来根据错误号给上位机反馈错误信息
        Boot_SendErrorResponse(bootErrorCode);
        bootErrorCode = ERROR_CODE_NO_ERROR; // 重置错误码
    }

    if (KEY_GetState(&K1) == KEY_State_DOWN) {
        return BOOT_STATE_APPLICATION_JUMP;
    }
    return BOOT_STATE_BOOTLOADER;
}

/**
 * @brief 处理接收到的命令
 */
static void Boot_ProcessReceivedCommand(void) {
    switch (current_frame.command) {
    case CMD_ENTER_BOOT:
        // 已经进入Bootloader，发送确认
        Boot_SendEnterBootResponse();
        // Boot_SendAckResponse();
        break;

    case CMD_UPLOAD:
        // 处理固件上传
        Boot_ProcessUploadCommand(&current_frame);
        Boot_SendAckResponse(); // 暂时只回复ACK
        break;

    case CMD_VERIFY:
        // 处理验证命令
        // Boot_ProcessVerifyCommand(&current_frame);
        Boot_SendAckResponse(); // 暂时只回复ACK
        break;

    case CMD_RUN_APP:
        // 收到跳转APP命令
        Boot_SendAckResponse();
        // 设置标志，让状态机在下个循环跳转
        is_run_app = true;
        break;
    case CMD_ACK:
        // 收到ACK测试命令
        Boot_SendAckResponse();
        break;

    default:
        Boot_SendErrorResponse(ERROR_CODE_UNKNOWN_CMD);
        break;
    }
}
/**
 * @brief 处理固件升级指令
 * @param frame 命令帧
 * @return 1成功，0失败
 */
uint8_t Boot_ProcessUploadCommand(command_frame_t *frame) {
    static uint8_t total_packet;
    static uint8_t num_packet;
    uint8_t *firmware;
}
/**
 * @brief 发送命令帧
 */
static bool Boot_SendFrame(command_type_t cmd, uint8_t *data,
                           uint16_t data_len) {
    uint8_t tx_buffer[FRAME_SIZE];
    uint16_t frame_len = command_build_frame(cmd, data, data_len, tx_buffer);
    return boot_send_func(tx_buffer, frame_len);
}
/**
 * @brief 发送ACK响应
 */
static void Boot_SendAckResponse(void) { Boot_SendFrame(CMD_ACK, NULL, 0); }

/**
 * @brief 发送EnterBoot响应，用于发送设备信息
 */
static void Boot_SendEnterBootResponse(void) {
    BOOT_DeviceInfo_t device;

    memset(&device, 0, sizeof(BOOT_DeviceInfo_t));
    // 设置设备模型名
    strncpy(device.deviceInfo.model, DEVICE_INFO_MODEL,
            DEVICE_INFO_MODEL_LENGTH - 1);
    device.deviceInfo.model[DEVICE_INFO_MODEL_LENGTH - 1] = '\0'; // 确保终止

    // 设置flash尺寸
    device.deviceInfo.flashSize = DEVICE_INFO_FLASH_SIZE;

    // 设置app加载地址
    device.deviceInfo.appAddr = DEVICE_INFO_APP_ADDRESS;

    // 设置boot版本
    strncpy(device.deviceInfo.bootVersion, DEVICE_INFO_BOOT_VERSION,
            DEVICE_INFO_BOOT_VERSION_LENGTH - 1);
    device.deviceInfo.bootVersion[DEVICE_INFO_BOOT_VERSION_LENGTH - 1] =
        '\0'; // 确保终止

    Boot_SendFrame(CMD_ENTER_BOOT, device.rawData, sizeof(BOOT_DeviceInfo_t));
}
/**
 * @brief 发送错误响应
 */
static void Boot_SendErrorResponse(BootErrorCode_t errorCode) {
    const char *error_msg = GetErrorMessage(errorCode);
    uint16_t error_msg_len = strlen(error_msg);
    Boot_SendFrame(CMD_ERROR_RESPONSE, (uint8_t *)error_msg, error_msg_len);
}

void Boot_ReceiveCommand(uint8_t received_byte) {
    // 命令接收处理实现
    command_parse_result = command_process_byte(received_byte);

    switch (command_parse_result) {
    case PARSE_SUCCESS:
        if (command_get_frame(&current_frame)) {
            is_recived = true;
            bootErrorCode = ERROR_CODE_NO_ERROR;
        }
        break;
    case PARSE_ERROR_HEADER:
        bootErrorCode = ERROR_CODE_PARSE_FAILED;
        is_recived = false;
        break;
    case PARSE_ERROR_INVALID_CMD:
        bootErrorCode = ERROR_CODE_UNKNOWN_CMD;
        is_recived = false;
        break;
    case PARSE_ERROR_LENGTH:
        bootErrorCode = ERROR_CODE_PARSE_FAILED;
        is_recived = false;
        break;
    case PARSE_ERROR_CHECKSUM:
        bootErrorCode = ERROR_CODE_PARSE_FAILED;
        is_recived = false;
        break;
    case PARSE_INCOMPLETE:
        // 正常状态，不做处理
        break;
    default:
        bootErrorCode = ERROR_CODE_NO_ERROR;
        is_recived = false;
        break;
    }
}