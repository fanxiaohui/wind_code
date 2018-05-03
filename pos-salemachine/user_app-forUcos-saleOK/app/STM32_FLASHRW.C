#include "STM32_FLASHRW.H"
#define PageSize  2048
#define BaseAddr 0x080E0000	  /*第11页地址，在程序中，擦除页也为第11，如果修改此处，需要修改程序中擦除的页码*/
/*******************************************************************************
flash的写函数：
输入：
		u32 StartAddr	flash起始地址
		u32 *p_data	待写入数据指针
		u32 size	写入数据的数量
输出：
		0：正确执行
		非0：出错
注意：输入数据一定是u32 的指针，即数据一定是按照4字节对齐写入的。
所以：size也是u32的个数（字节数的4分之一）
*******************************************************************************/

u8 FlashWrite(u32 StartAddr, u32 *p_data, u32 size)
{
    volatile FLASH_Status FLASHStatus;

    u32 EndAddr;
    vu32 NbrOfPage = 0;
    u32 EraseCounter = 0x0, Address = 0x0;
    int i;
    int MemoryProgramStatus = 1;

    StartAddr = StartAddr + BaseAddr;
    EndAddr = StartAddr + size * 4;

    //为一是通过
    FLASH_Unlock();          //解锁函数
    NbrOfPage = ((EndAddr - StartAddr) / PageSize) + 1;	//有多少个页被擦除	//清除所有已有标志
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGAERR | FLASH_FLAG_WRPERR);
    //擦页
    FLASHStatus = FLASH_COMPLETE;
    for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
    {
        FLASHStatus = FLASH_EraseSector(FLASH_Sector_11, VoltageRange_3); /*擦除第11页*/
    }
    //开始写数据
    Address = StartAddr;
    i = 0;
    while((Address < EndAddr) && (FLASHStatus == FLASH_COMPLETE))
    {
        FLASHStatus = FLASH_ProgramWord(Address, p_data[i++]);
        Address = Address + 4;
    }
    //检验数据是否出错
    Address = StartAddr;
    i = 0;
    while((Address < EndAddr) && (MemoryProgramStatus != 0))
    {
        if((*(vu32 *) Address) != p_data[i++])
        {
            MemoryProgramStatus = 0;
            return 1;
        }
        Address += 4;
    }
    return 0;
}

u32 FlashRead(u32 StartAddr, u32 *p_data, u32 size)
{
    u32 EndAddr = StartAddr + BaseAddr + size * 4;
    int MemoryProgramStatus = 1;
    u32 Address = 0x0;
    int i = 0;
    Address = StartAddr + BaseAddr;
    while((Address < EndAddr) && (MemoryProgramStatus != 0))
    {
        p_data[i++] = (*(vu32 *) Address);
        Address += 4;
    }
    return 0;
}
