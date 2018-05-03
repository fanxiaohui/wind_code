#ifndef STM32_FLASH_H
#define STM32_FLASH_H

#include "config.h"

u8 FlashWrite(u32 StartAddr, u32 *p_data, u32 size);
u32 FlashRead(u32 StartAddr, u32 *p_data, u32 size);

#endif
