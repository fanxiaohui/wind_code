#ifndef _CortexM_StackCheck_h_
#define _CortexM_StackCheck_h_
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
//	��startup_stmxxxxxxxx.s��������������,��ջ��ʼ��ַ�����

EXPORT  Stack_Mem

#endif
//��ʼ��ջ��,ջ�׿ռ���Ϊ���Կռ�.
//��Ҫ����Stack_Mem.
//Stack_Mem��ջ��.
//��Ҫ�� 8 bytes�Ŀռ�,����ջʱҪע�����ֵ.


//ջ̫Сʱ,�������������ʱ���Ѿ����˱�ջ,���Ҳ��ص�ԭλ��,
//�������Ч��,��Ϊջָ���Ѿ��ں��海����.

//���ջ���Ƿ񱻸�д,��д�˾�1.
//�����ȷ����0.
int CortexM_StackCheck(void);

#ifdef __cplusplus
}
#endif
#endif

