/**

*@  Copyright (C), 2001-2016,Fotile.
*
* @ **
* @ File Name     : ft_queue.h
* @ Version       : Initial Draft
* @ Author        :wangzhengxue
* @ Created       : 2016/9/18
* @ Last Modified :
  brief   : 实现队列基本操作
* @ Function List :
* @ History       :
* @ 1.Date        : 2016/9/18
* @   Author      : wangzhengxue
* @   Modification: Created file

*/


#ifndef FT_QUEUE_H
#define FT_QUEUE_H

/**创建队列，初始化锁和队列头节点*/
#define CREATE_QUEUE(head) {\
	(head)->length = 0;\
	FT_LOCK_INIT(&(head)->lock);\
	INIT_LIST_HEAD(&(head)->list);\
}

/**头结点删除,重新初始化话队列头,销毁锁资源*/
#define DESTORY_QUEUE(head) {\
	FT_LOCK(&(head)->lock); \
	list_del_init(&(head)->list);\
	(head)->length = 0;\
	FT_UNLOCK(&(head)->lock);\
	FT_LOCK_DESTORY(&(head)->lock);\
}

/**向队列queue添加node节点,member为node所属结构的成员变量,用于连接两个node*/
#define PUSH_ENQUEUE(node, member, head) {\
	ASSERT(node);\
	ASSERT(head);\
	FT_LOCK(&(head)->lock);\
	list_add_tail(&(node)->member, &(head)->list);\
	(head)->length++;\
	FT_UNLOCK(&(head)->lock);\
}

/**从queue取出一个对象,pos指向该对象,member为node所属结构的一个成员,将对象从队列里删除
如果队列为空，则直接返回NULL，说明取对象失败
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

/**队列为空为真:1，非空为假:0*/
#define QUEUE_ISEMPTY(head_node) list_empty_careful(head_node)

#endif /*FT_QUEUE_H*/
