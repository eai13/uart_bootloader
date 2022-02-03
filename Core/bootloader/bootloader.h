#ifndef _BOOTLOADER_H_
#define _BOOTLOADER_H_

#include "stdint.h"
#include "stm32g0xx_hal.h"
#include "stm32g0xx_hal_flash.h"

#define FLASH_ERASED    0xFFFFFFFF

#define PAGES_AMOUNT    64
#define PAGE_SIZE       2048

#define MARK_ADDRESS    (uint32_t)0x08006000 // 24576 for bootloader
#define MARK_PAGE       12
#define MARK_VALUE      0x42424242

#define MAIN_APP_ADDRESS    (uint32_t)0x08006800
#define MAIN_APP_PAGE       13

#define MYNAME          "ST32"
#define HOSTNAME        "HOST"

#define IN_BOOT_MODE    HAL_GPIO_ReadPin(BTN_GPIO_Port, BTN_Pin) == 0

typedef enum{
    WRITE           = 0x01,
    READ            = 0x02,
    ERASE           = 0x03,
    PING            = 0x04,
    BOOT_OK         = 0x05,
    BOOT_ERROR      = 0x06,
    // START_WRITE     = 0x07,
    // START_READ      = 0x08,
    JUMP            = 0x07
}command_t;

#pragma pack(push, 1)
typedef struct {
    uint8_t device_src[4];
    uint8_t device_dest[4];
    uint8_t cmd;
    uint32_t payload_size;
}bootloader_header_t;
#pragma pack(pop)

uint32_t            GetMark(void);
HAL_StatusTypeDef   ResetMark(void);
HAL_StatusTypeDef   SetMark(uint32_t MarkValue);

HAL_StatusTypeDef   EraseTheApp(void);

HAL_StatusTypeDef   ReceiveHeader(bootloader_header_t * data);
HAL_StatusTypeDef   SendHeader(bootloader_header_t * data);

HAL_StatusTypeDef   ProgramApplication(uint8_t * data, uint32_t * address, uint32_t size);

HAL_StatusTypeDef   JumpToApp(void);

#endif