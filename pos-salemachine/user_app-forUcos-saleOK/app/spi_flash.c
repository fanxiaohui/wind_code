/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : spi_flash.c
* Author             : MCD Application Team
* Version            : V2.0.1
* Date               : 06/13/2008
* Description        : This file provides a set of functions needed to manage the
*                      communication between SPI peripheral and SPI M25P64 FLASH.
********************************************************************************
* THE RESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "spi_flash.h"
/* Private typedef -----------------------------------------------------------*/
#define SPI_FLASH_PageSize    0X1FF

/* Private define ------------------------------------------------------------*/
#define WRITE      0x02  /* Write to Memory instruction */
#define WRSR       0x01  /* Write Status Register instruction */
#define WREN       0x06  /* Write enable instruction */

#define READ       0x03  /* Read from Memory instruction */
#define RDSR       0x05  /* Read Status Register instruction  */
#define RDID       0x9F  /* Read identification */
#define SE         0xD8  /* Sector Erase instruction */
#define BE         0xC7  /* Bulk Erase instruction */

#define WIP_Flag   0x01  /* Write In Progress (WIP) flag */

#define Dummy_Byte 0xFF
#define MAXCAP     4095

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : SPI_FLASH_Init
* Description    : Initializes the peripherals used by the SPI FLASH driver.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_Init(void)
{
    //  SPI_InitTypeDef  SPI_InitStructure;
    //  GPIO_InitTypeDef GPIO_InitStructure;
    //
    //  /* Enable SPI1 and GPIO clocks */
    //  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIO_CS, ENABLE);
    //
    //  	GPIO_PinAFConfig(GPIO_CS,GPIO_Pin_SCK,GPIO_AF_SPI1);
    //	GPIO_PinAFConfig(GPIO_CS,GPIO_Pin_SI,GPIO_AF_SPI1);
    //	GPIO_PinAFConfig(GPIO_CS,GPIO_Pin_SO,GPIO_AF_SPI1);
    //
    //  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    //  /* Configure SPI1 pins: SCK, MISO and MOSI */
    //  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_SCK | GPIO_Pin_SI | GPIO_Pin_SO;
    //	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    //	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    //	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    //	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    //
    //  GPIO_Init(GPIO_CS, &GPIO_InitStructure);
    //
    //  /* Configure I/O for Flash Chip select */
    //  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_CS;
    //
    //
    //	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    //	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    //	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    //	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    //  GPIO_Init(GPIO_CS, &GPIO_InitStructure);
    //
    //  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_RES;
    //	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    //	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    //	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    //	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    //  GPIO_Init(GPIO_RES, &GPIO_InitStructure);
    //
    //  /* Deselect the FLASH: Chip Select high */
    //  spi_cs_High;
    //
    //  /* SPI1 configuration */
    //  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    //  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    //  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    //  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    //  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    //  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    //  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
    //  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    //  SPI_InitStructure.SPI_CRCPolynomial = 7;
    //  SPI_Init(SPI1, &SPI_InitStructure);
    //
    //  /* Enable SPI1  */
    //  SPI_Cmd(SPI1, ENABLE);
    spi_RES_High;
}
unsigned char  AT45DBXX_STATUS(void)
{
    unsigned char temp;
    spi_cs_Low;
    SPI_FLASH_SendByte(0Xd7);
    temp =  SPI_FLASH_SendByte(0Xff);
    spi_cs_High;
    return temp; //bit7 0=busy  bit6 1=Compare
    // bit1 1=PROTECT bit0 0=528byte 1=512byte
}

unsigned char AT45XX_ID(void)
{
    unsigned char temp;

    spi_cs_Low;
    SPI_FLASH_SendByte(0X9F);
    temp = SPI_FLASH_SendByte(0xff);
    temp = SPI_FLASH_SendByte(0xff);
    spi_cs_High;
    return temp;
}
/*读FLASH*/
u32 SPIReadFlash(u32 Addr, u8 *Dat, u16 lenght) //读flash，通过地址写入
{
    u16 Page = 0; //第一页地址
    u16 AddrofPage = 0; //第一次读的地址
    u16 AddrofLastPage;//最后一页地址
    u16 NumPage = 0; //除了第一页还共需要读的页面数



    Page = Addr / 512;
    AddrofPage = Addr % 512;
    AddrofLastPage = ((lenght % 512) + AddrofPage) % 512;
    if(lenght <= (511 - AddrofPage))
    {
        NumPage = 0;
    }
    else
    {
        NumPage = (lenght - (511 - AddrofPage)) / 512; //去掉第一页还有点整页
    }
    if(lenght <= (511 - AddrofPage)) //在当前页内能够读完所有数据
    {
        MainToBuffer1AT45(Page);
        AT45Buffer1Read(AddrofPage, Dat, lenght);

    }
    else
    {
        /*读第一页*/
        MainToBuffer1AT45(Page);
        AT45Buffer1Read(AddrofPage, Dat, 512 - AddrofPage);
        /*读后续整页*/
        Page++;
        Dat += 512 - AddrofPage;
        if(Page >= MAXCAP)
        {
            Page = 0; //芯片中从第一页读，第0页空着
        }
        while(NumPage--)//读后续整页
        {
            MainToBuffer1AT45(Page);
            AT45Buffer1Read(0, Dat, 512);

            Page++;
            Dat += 512;
            if(Page >= MAXCAP)
            {
                Page = 1;
            }
        }
        /*读最后页*/
        MainToBuffer1AT45(Page);
        AT45Buffer1Read(0, Dat, AddrofLastPage);

    }

    if(Addr + lenght <= MAXCAP * 512) //返回最后地址
    {
        return Addr + lenght; //
    }
    else
    {
        return (Addr + lenght) - MAXCAP * 512; //
    }
}
/*写FLASH*/
u32 SPIWriteFlash(u32 Addr, u8 *Dat, u16 lenght) //写flash，通过地址写入
{
    u16 Page = 0; //第一页地址
    u16 AddrofPage = 0; //第一次写的地址
    u16 AddrofLastPage;//最后一页地址
    u16 NumPage = 0; //除了第一页还共需要写的页面数

    Page = Addr / 512;
    AddrofPage = Addr % 512;
    AddrofLastPage = ((lenght % 512) + AddrofPage) % 512;
    if(lenght <= (511 - AddrofPage))
    {
        NumPage = 0;
    }
    else
    {
        NumPage = (lenght - (511 - AddrofPage)) / 512; //去掉第一页还有点整页
    }
    if(lenght <= (511 - AddrofPage)) //在当前页内能够写完所有数据
    {
        MainToBuffer1AT45(Page);
        AT45Buffer1Write(AddrofPage, Dat, lenght);
        Buffer1ToMainWithErase(Page);
    }
    else
    {
        /*写第一页*/
        MainToBuffer1AT45(Page);
        AT45Buffer1Write(AddrofPage, Dat, 512 - AddrofPage);
        Buffer1ToMainWithErase(Page);
        /*写后续整页*/
        Page++;
        Dat += 512 - AddrofPage;
        if(Page >= MAXCAP)
        {
            Page = 0; //芯片中从第一页写，第0页空着
        }
        while(NumPage--)//写后续整页
        {
            AT45Buffer1Write(0, Dat, 512);
            Buffer1ToMainWithErase(Page);
            Page++;
            Dat += 512;
            if(Page >= MAXCAP)
            {
                Page = 1;
            }
        }
        /*写最后页*/
        MainToBuffer1AT45(Page);
        AT45Buffer1Write(0, Dat, AddrofLastPage);
        Buffer1ToMainWithErase(Page);

    }

    if(Addr + lenght <= MAXCAP * 512) //返回最后地址
    {
        return Addr + lenght; //
    }
    else
    {
        return (Addr + lenght) - MAXCAP * 512; //
    }

}
void ContinuousArrayLowRead(unsigned int page, unsigned int addrs,
                            unsigned char *dat, unsigned int lenght)
{


    while((AT45DBXX_STATUS() & 0x80) == 0);
    spi_cs_Low;
    SPI_FLASH_SendByte(0X03);
    page <<= 1;
    if(addrs & 0x100)page = page + 1;
    SPI_FLASH_SendByte((page >> 8));
    SPI_FLASH_SendByte(page);
    SPI_FLASH_SendByte(addrs);
    while(lenght)
    {
        *dat = SPI_FLASH_SendByte(0Xff);
        dat++;
        lenght--;
    }
    spi_cs_High;
}
/*擦除Flash*/
void Buffer1ToMainWithErase(unsigned int page)
{

    while((AT45DBXX_STATUS() & 0x80) == 0);
    spi_cs_Low;
    SPI_FLASH_SendByte(0X83);
    SPI_FLASH_SendByte((page >> 6));
    SPI_FLASH_SendByte(page << 2);
    SPI_FLASH_SendByte(0x00);
    spi_cs_High;
}
void AT45Buffer1Write(unsigned int addrs, unsigned char *dat, unsigned int lenght)
{


    while((AT45DBXX_STATUS() & 0x80) == 0);
    spi_cs_Low;
    SPI_FLASH_SendByte(0X84);
    SPI_FLASH_SendByte(0x00);
    SPI_FLASH_SendByte(addrs >> 8);
    SPI_FLASH_SendByte(addrs);
    while(lenght)
    {
        SPI_FLASH_SendByte(*dat);
        dat++;
        lenght--;
    }
    spi_cs_High;
}
void AT45Buffer1Read(unsigned int addrs, unsigned char *dat, unsigned int lenght)
{

    while((AT45DBXX_STATUS() & 0x80) == 0);
    spi_cs_Low;
    SPI_FLASH_SendByte(0Xd1);
    SPI_FLASH_SendByte(0x00);
    SPI_FLASH_SendByte(addrs >> 8);
    SPI_FLASH_SendByte(addrs);
    while(lenght)
    {
        *dat = SPI_FLASH_SendByte(0xff);
        dat++;
        lenght--;
    }
    spi_cs_High;
}
void MainToBuffer1AT45(unsigned int page)
{

    while((AT45DBXX_STATUS() & 0x80) == 0);
    spi_cs_Low;
    SPI_FLASH_SendByte(0X53);
    SPI_FLASH_SendByte((page >> 6));
    SPI_FLASH_SendByte(page << 2);
    SPI_FLASH_SendByte(0x00);
    spi_cs_High;
}
void Buffer2ToMainWithErase(unsigned int page)
{

    while((AT45DBXX_STATUS() & 0x80) == 0);
    spi_cs_Low;
    SPI_FLASH_SendByte(0X86);
    SPI_FLASH_SendByte((page >> 6));
    SPI_FLASH_SendByte(page << 2);
    SPI_FLASH_SendByte(0x00);
    spi_cs_High;
}
void AT45Buffer2Write(unsigned int addrs, unsigned char *dat, unsigned int lenght)
{

    while((AT45DBXX_STATUS() & 0x80) == 0);
    spi_cs_Low;
    SPI_FLASH_SendByte(0X87);
    SPI_FLASH_SendByte(0x00);
    SPI_FLASH_SendByte(addrs >> 8);
    SPI_FLASH_SendByte(addrs);
    while(lenght)
    {
        SPI_FLASH_SendByte(*dat);
        dat++;
        lenght--;
    }
    spi_cs_High;
}
void AT45Buffer2Read(unsigned int addrs, unsigned char *dat, unsigned int lenght)
{

    while((AT45DBXX_STATUS() & 0x80) == 0);
    spi_cs_Low;
    SPI_FLASH_SendByte(0Xd3);
    SPI_FLASH_SendByte(0x00);
    SPI_FLASH_SendByte(addrs >> 8);
    SPI_FLASH_SendByte(addrs);
    while(lenght)
    {
        *dat = SPI_FLASH_SendByte(0xff);
        dat++;
        lenght--;
    }
    spi_cs_High;
}
void MainToBuffer2AT45(unsigned int page)
{

    while((AT45DBXX_STATUS() & 0x80) == 0);
    spi_cs_Low;
    SPI_FLASH_SendByte(0X55);
    SPI_FLASH_SendByte((page >> 6));
    SPI_FLASH_SendByte(page << 2);
    SPI_FLASH_SendByte(0x00);
    spi_cs_High;
}
void ChipErase(void)
{

    while((AT45DBXX_STATUS() & 0x80) == 0);
    spi_cs_Low;
    SPI_FLASH_SendByte(0Xc7);
    SPI_FLASH_SendByte(0x94);
    SPI_FLASH_SendByte(0x80);
    SPI_FLASH_SendByte(0x9a);
    spi_cs_High;
}
void PowerOfPage256(void)
{

    while((AT45DBXX_STATUS() & 0x80) == 0);

    spi_cs_Low;
    SPI_FLASH_SendByte(0X3d);
    SPI_FLASH_SendByte(0x2a);
    SPI_FLASH_SendByte(0x80);
    SPI_FLASH_SendByte(0xa6);
    spi_cs_High;
}
u8 SPI_FLASH_SendByte(u8 byte)
{
    /* Loop while DR register in not emplty */
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

    /* Send byte through the SPI1 peripheral */
    SPI_I2S_SendData(SPI1, byte);

    /* Wait to receive a byte */
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

    /* Return the byte read from the SPI bus */
    return SPI_I2S_ReceiveData(SPI1);
}
//在页内写数据
void WritePage(u16 Page, u16 AddrofPage, u8 *Dat, u16 lenght)
{
    MainToBuffer1AT45(Page);
    AT45Buffer1Write(AddrofPage, Dat, lenght);
    Buffer1ToMainWithErase(Page);
}
void ReadPageData(u16 Page, u16 AddrofPage, u8 *Dat, u16 lenght)
{
    MainToBuffer1AT45(Page);
    AT45Buffer1Read(AddrofPage, Dat, lenght);

}
//读取整页数据
void ReadPage(u16 Page, u8 *Dat)
{
    MainToBuffer1AT45(Page);
    AT45Buffer1Read(0, Dat, 512);
}
/*获取CPU唯一ID
CpuID:ID存放指针
*/
void GetLockCode(u32 *CpuID)
{

    CpuID[0] = *(vu32 *)(0x1FFF7A10);
    CpuID[1] = *(vu32 *)(0x1FFF7A14);
    CpuID[2] = *(vu32 *)(0x1FFF7A18);
}

