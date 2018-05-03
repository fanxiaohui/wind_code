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
        //第一次执行此函数.
        //检查此时此刻是否栈溢出,并在栈底设定检查标志.
    {
        if(__get_MSP() <= (u32)Stack_Mem + 8)
            //需要使用8字节的检查空间,如果现在就不够了肿么办..
        {
            return 1;
        }
        else
            //有足够的空间用于填入检查用的数据.
        {
            //填入数据
            *((int *)Stack_Mem + 0) = CheckKey1;
            *((int *)Stack_Mem + 1) = CheckKey2;

            //标记已经填入了数据,以后就不来这里玩了.
            IsFirstCheck = 0;

            //OK
            return 0;
        }
    }
    else
        //前面已经埋下了好东西,看看还在不在...
    {
        if(CheckKey1 != *((int *)Stack_Mem + 0)
                || CheckKey2 != *((int *)Stack_Mem + 1))
            //好东西不在了...
        {
            return 1;
        }
        else
            //东西还在哈哈哈..
        {
            return 0;
        }
    }
}
