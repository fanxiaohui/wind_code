#include "STM32_FLASHRW.H"
#define PageSize  2048
#define BaseAddr 0x080E0000	  /*��11ҳ��ַ���ڳ����У�����ҳҲΪ��11������޸Ĵ˴�����Ҫ�޸ĳ����в�����ҳ��*/
/*******************************************************************************
flash��д������
���룺
		u32 StartAddr	flash��ʼ��ַ
		u32 *p_data	��д������ָ��
		u32 size	д�����ݵ�����
�����
		0����ȷִ��
		��0������
ע�⣺��������һ����u32 ��ָ�룬������һ���ǰ���4�ֽڶ���д��ġ�
���ԣ�sizeҲ��u32�ĸ������ֽ�����4��֮һ��
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

    //Ϊһ��ͨ��
    FLASH_Unlock();          //��������
    NbrOfPage = ((EndAddr - StartAddr) / PageSize) + 1;	//�ж��ٸ�ҳ������	//����������б�־
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGAERR | FLASH_FLAG_WRPERR);
    //��ҳ
    FLASHStatus = FLASH_COMPLETE;
    for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
    {
        FLASHStatus = FLASH_EraseSector(FLASH_Sector_11, VoltageRange_3); /*������11ҳ*/
    }
    //��ʼд����
    Address = StartAddr;
    i = 0;
    while((Address < EndAddr) && (FLASHStatus == FLASH_COMPLETE))
    {
        FLASHStatus = FLASH_ProgramWord(Address, p_data[i++]);
        Address = Address + 4;
    }
    //���������Ƿ����
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