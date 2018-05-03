#include "CortexM_StackCheck.h"
#include "config.h"

extern void Stack_Mem(void);
extern u32 __get_MSP(void);

#define CheckKey1 (0x12345678)
#define CheckKey2 (0x89abcdef)

int CortexM_StackCheck( void )
{
    static int IsFirstCheck = 1;
    int temp = 0;

    if(0 != IsFirstCheck)
        //��һ��ִ�д˺���.
        //����ʱ�˿��Ƿ�ջ���,����ջ���趨����־.
    {
        if(__get_MSP() <= (u32)Stack_Mem + 8)
            //��Ҫʹ��8�ֽڵļ��ռ�,������ھͲ�������ô��..
        {
            return 1;
        }
        else
            //���㹻�Ŀռ������������õ�����.
        {
            //��������
            *((int *)Stack_Mem + 0) = CheckKey1;
            *((int *)Stack_Mem + 1) = CheckKey2;

            //����Ѿ�����������,�Ժ�Ͳ�����������.
            IsFirstCheck = 0;

            //OK
            return 0;
        }
    }
    else
        //ǰ���Ѿ������˺ö���,�������ڲ���...
    {
        if(CheckKey1 != *((int *)Stack_Mem + 0)
                || CheckKey2 != *((int *)Stack_Mem + 1))
            //�ö���������...
        {
            return 1;
        }
        else
            //�������ڹ�����..
        {
            return 0;
        }
    }
}
