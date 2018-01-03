/**

*@  Copyright (C), 2001-2016,Fotile.
*
* @ **
* @ File Name     : ft_queue.h
* @ Version       : Initial Draft
* @ Author        :wangzhengxue
* @ Created       : 2016/9/18
* @ Last Modified :
  brief   : ʵ�ֶ��л�������
* @ Function List :
* @ History       :
* @ 1.Date        : 2016/9/18
* @   Author      : wangzhengxue
* @   Modification: Created file

*/


#ifndef FT_QUEUE_H
#define FT_QUEUE_H

/**�������У���ʼ�����Ͷ���ͷ�ڵ�*/
#define CREATE_QUEUE(head) {\
	(head)->length = 0;\
	FT_LOCK_INIT(&(head)->lock);\
	INIT_LIST_HEAD(&(head)->list);\
}

/**ͷ���ɾ��,���³�ʼ��������ͷ,��������Դ*/
#define DESTORY_QUEUE(head) {\
	FT_LOCK(&(head)->lock); \
	list_del_init(&(head)->list);\
	(head)->length = 0;\
	FT_UNLOCK(&(head)->lock);\
	FT_LOCK_DESTORY(&(head)->lock);\
}

/**�����queue���node�ڵ�,memberΪnode�����ṹ�ĳ�Ա����,������������node*/
#define PUSH_ENQUEUE(node, member, head) {\
	ASSERT(node);\
	ASSERT(head);\
	FT_LOCK(&(head)->lock);\
	list_add_tail(&(node)->member, &(head)->list);\
	(head)->length++;\
	FT_UNLOCK(&(head)->lock);\
}

/**��queueȡ��һ������,posָ��ö���,memberΪnode�����ṹ��һ����Ա,������Ӷ�����ɾ��
�������Ϊ�գ���ֱ�ӷ���NULL��˵��ȡ����ʧ��
*/
#define POP_FROMQUEUE(pos, member, head) {\
	ASSERT(head);\
	FT_LOCK(&(head)->lock);\
	if (!list_empty_careful(&(head)->list)) {\
		(pos) = list_entry((head)->list.next, typeof(*(pos)),member);\
		list_del((head)->list.next);\
		(head)->length--;\
	} else {\
		(pos) = NULL;\
	}\
	FT_UNLOCK(&(head)->lock);\
}

/**����Ϊ��Ϊ��:1���ǿ�Ϊ��:0*/
#define QUEUE_ISEMPTY(head_node) list_empty_careful(head_node)

#endif /*FT_QUEUE_H*/
