#include "bootloader.h"
#include "debug.h"
#include "stdint.h"
#include "stm32g0xx_hal_flash.h"
#include "stm32g0xx_hal.h"
#include "string.h"

uint32_t GetMark(void){
    return *(volatile uint32_t *)(MARK_ADDRESS);
}

HAL_StatusTypeDef ResetMark(void){
    HAL_StatusTypeDef status = HAL_OK;
    
    status = HAL_FLASH_Unlock();
    if (status != HAL_OK) return status;
    
    FLASH_EraseInitTypeDef erase_struct = {
        .TypeErase = TYPEERASE_PAGES,
        .Page = MARK_PAGE,
        .NbPages = 1
    };
    uint32_t erase_error = 0;
    status = HAL_FLASHEx_Erase(&erase_struct, &erase_error);
    if (status != HAL_OK) return status;
    
    status = HAL_FLASH_Lock();
    return status;
}

HAL_StatusTypeDef SetMark(uint32_t MarkValue){
    HAL_StatusTypeDef status = HAL_OK;

    if (*(volatile uint32_t *)MARK_ADDRESS != FLASH_ERASED){
        status = ResetMark();
        if (status != HAL_OK) return status;
    }

    status = HAL_FLASH_Unlock();
    if (status != HAL_OK) return status;
    
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, MARK_ADDRESS, MARK_VALUE);
    if (status != HAL_OK) return status;

    status = HAL_FLASH_Lock();
    return status;
}

HAL_StatusTypeDef EraseTheApp(void){
    HAL_StatusTypeDef status;
    status = HAL_FLASH_Unlock();
    if (status != HAL_OK) return status;

    FLASH_EraseInitTypeDef erase_struct = {
        .TypeErase = TYPEERASE_PAGES,
        .Page = MAIN_APP_PAGE,
        .NbPages = PAGES_AMOUNT - MAIN_APP_PAGE
    };
    uint32_t erase_error = 0;
    status = HAL_FLASHEx_Erase(&erase_struct, &erase_error);
    if (status != HAL_OK) return status;

    status = HAL_FLASH_Lock();
    return status;
}

HAL_StatusTypeDef ReceiveHeader(bootloader_header_t * data){
    return HAL_UART_Receive(&huart1, (uint8_t *)data, 13, HAL_MAX_DELAY);
}

HAL_StatusTypeDef SendHeader(bootloader_header_t * data){
    HAL_UART_Transmit(&huart1, (uint8_t *)data, 13, HAL_MAX_DELAY);
}

HAL_StatusTypeDef ProgramApplication(uint8_t * data, uint32_t * address, uint32_t size){
    // HAL_StatusTypeDef status = HAL_UART_Receive(&huart1, data, size, 300);
    // if (status != HAL_OK) return status;
    
    HAL_StatusTypeDef status = HAL_FLASH_Unlock();
    if (status != HAL_OK) return status;

    for (uint32_t iter = 0; iter < size; iter += 8){
        status = HAL_FLASH_Program(TYPEPROGRAM_DOUBLEWORD, *address, *((uint64_t *)(data + iter)));
        if (status != HAL_OK) return status;
        (*address) += 8;
    }

    status = HAL_FLASH_Lock();
    return status;
}

HAL_StatusTypeDef JumpToApp(void){
    uint32_t p_jump_func_addr;
    void(* p_jump_func)(void);
    
    p_jump_func_addr = *(volatile uint32_t *)(MAIN_APP_ADDRESS + 4);
    p_jump_func = (void (*)(void))p_jump_func_addr;
    
    __disable_irq();

    SCB->VTOR = 0x08006800;
    
    __set_MSP(*(volatile uint32_t *)MAIN_APP_ADDRESS);

    p_jump_func();
}
