/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : spi_flash.h
* Author             : MCD Application Team
* Version            : V2.0.1
* Date               : 06/13/2008
* Description        : Header for spi_flash.c file.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/


/* Includes ------------------------------------------------------------------*/
#ifndef SPI_FLASH_H
#define SPI_FLASH_H
#include "config.h"

#define GPIO_CS                    GPIOA
#define GPIO_RES                   GPIOA
#define RCC_AHB1Periph_GPIO_CS     RCC_AHB1Periph_GPIOA
//   RCC_APB2Periph_SPI1
#define GPIO_Pin_CS                GPIO_Pin_4
#define GPIO_Pin_RES               GPIO_Pin_3

#define GPIO_Pin_SI				   GPIO_Pin_7
#define GPIO_Pin_SO				   GPIO_Pin_6
#define GPIO_Pin_SCK			   GPIO_Pin_5
/* Exported macro ------------------------------------------------------------*/
/* Select SPI FLASH: Chip Select pin low  */
#define spi_cs_Low        GPIO_ResetBits(GPIO_CS, GPIO_Pin_CS)
/* Deselect SPI FLASH: Chip Select pin high */
#define spi_cs_High       GPIO_SetBits(GPIO_CS, GPIO_Pin_CS)

#define spi_RES_High      GPIO_SetBits(GPIO_RES, GPIO_Pin_RES)

#define spi_RES_Low       GPIO_ResetBits(GPIO_RES, GPIO_Pin_RES)

/* Exported functions ------------------------------------------------------- */
/*----- High layer function -----*/
//void SPI_FLASH_Init(void);
void SPI_FLASH_Init(void);
unsigned char  AT45DBXX_STATUS(void);
unsigned char AT45XX_ID(void);
void ContinuousArrayLowRead(unsigned int page, unsigned int addrs,
                            unsigned char *dat, unsigned int lenght);
void Buffer1ToMainWithErase(unsigned int page);
void AT45Buffer1Write(unsigned int addrs, unsigned char *dat, unsigned int lenght);
void AT45Buffer1Read(unsigned int addrs, unsigned char *dat, unsigned int lenght);
void MainToBuffer1AT45(unsigned int page);
void Buffer2ToMainWithErase(unsigned int page);
void AT45Buffer2Write(unsigned int addrs, unsigned char *dat, unsigned int lenght);
void AT45Buffer2Read(unsigned int addrs, unsigned char *dat, unsigned int lenght);

void MainToBuffer2AT45(unsigned int page);
void ChipErase(void);
void PowerOfPage256(void);
u8 SPI_FLASH_SendByte(u8 byte);
u32 SPIReadFlash(u32 Addr, u8 *Dat, u16 lenght); //读flash，通过地址写入;
u32 SPIWriteFlash(u32 Addr, u8 *Dat, u16 lenght); //写flash，通过地址写入
void WritePage(u16 Page, u16 AddrofPage, u8 *Dat, u16 lenght);
void ReadPageData(u16 Page, u16 AddrofPage, u8 *Dat, u16 lenght); //读取页的数据
void ReadPage(u16 Page, u8 *Dat);
void GetLockCode(u32 *CpuID);
#endif
