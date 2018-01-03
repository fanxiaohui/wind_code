#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ft_map.h"
#include "print.h"
#include "ikcc_common.h"


static st_link_list *s_head = NULL;
//---------------------------------------
//定义处理函数--初始化
int init_list(st_link_list* head)
{
	if(head == NULL )
	{
		return FT_FAILURE;
	}
	//head->dev_status = NULL;
	head->next = NULL;
	head->size = 0;
	s_head = head;
	FT_LOCK_INIT(&head->lock);
	return FT_SUCCESS;
}

int destroy_list(st_link_list* head)
{
	if(head == NULL )
	{
		return FT_FAILURE;
	}
	//todo  free node
	head->next = NULL;
	head->size = 0;
	FT_LOCK_DESTORY(&head->lock);
	return FT_SUCCESS;
}


//---------------------------------------
//定义处理函数--添加(如果存在就修改，否则添加)
int add_device_to_list(st_link_list *head, st_client_device* node)
{
	if (head == NULL)
	{
		return FT_FAILURE;
	}
	st_link_list *p, *q, *t;
	q = (st_link_list*)malloc(sizeof(st_link_list));
	if (q == NULL)
	{
		return FT_FAILURE;
	}
	FT_MEMCPY(&q->dev_status, node, sizeof(st_client_device));
	q->next = NULL;
	//q.size = 0;		//节点的size不关心
	FT_LOCK(&head->lock);
	p = head;
	while (p != NULL && p->next != NULL){
		t = p;
		p = t->next;

		//链表中找到相同的mac 替换掉
		if(FT_STRCMP(p->dev_status.mac, q->dev_status.mac) == 0){
			t->next = q;
			q->next = p->next;
			
			FT_FREE(p);
			p = NULL;
			
			FT_UNLOCK(&head->lock);
			return FT_SUCCESS;
		}
	}
	p->next = q;
	head->size++;
	FT_UNLOCK(&head->lock);
	//Debug("head->size[%d]", head->size);
	return FT_SUCCESS;
}
//---------------------------------------
//定义处理函数--删除
int del_from_list_by_mac(st_link_list *head, const char* mac)
{
	st_link_list *p, *q;
	if (head == NULL)
	{
		return FT_FAILURE;
	}
	FT_LOCK(&head->lock);
	p = head;
	while (p != NULL && p->next != NULL){
		q = p->next;
		if(FT_STRCMP(q->dev_status.mac, mac) == 0){
			p->next = q->next;
			FT_FREE(q);
			q = NULL;
		}
		p = p->next;
	}
	head->size--;
	FT_UNLOCK(&head->lock);
	//Debug("head->size[%d]", head->size);
	return FT_SUCCESS;
}
//---------------------------------------
//定义处理函数--查找
st_client_device *  find_device_by_mac(st_link_list *head, const char* mac)		//todo
{
	st_link_list *p;
	st_client_device *q = NULL;
	if (head == NULL)
	{
		return NULL;
	}
	FT_LOCK(&head->lock);
	p = head;
	while (p != NULL && p->next != NULL){
		p = p->next;
		if(strncmp(p->dev_status.mac, mac, FT_STRING_LEN) == 0){
			q = &p->dev_status;
		}
	}
	FT_UNLOCK(&head->lock);
	return q;
}

//定义处理函数--查找
st_client_device *  find_device_by_fd(st_link_list *head, const int fd)		//todo
{
	st_link_list *p;
	st_client_device *q = NULL;
	if (head == NULL)
	{
		return NULL;
	}
	//Debug("enter");
	FT_LOCK(&head->lock);
	p = head;
	while (p != NULL && p->next != NULL){
		p = p->next;
		if(p->dev_status.fd == fd){
			q = &p->dev_status;
		}
	}
	FT_UNLOCK(&head->lock);
	//Debug("leave");
	return q;
}
//
int dump_device_list()
{
	if(!s_head)
	{
		Warn("s_head is null!");
		return FALSE;
	}
	st_link_list *p = s_head;
	FT_LOCK(&s_head->lock);
	p = s_head;
	Info("size[%d]", p->size);
	while (p != NULL && p->next != NULL){
		p = p->next;
		Info("mac[%s] fd[%d] heartbeat[%d] online[%d]", p->dev_status.mac, p->dev_status.fd, p->dev_status.heartbeat_count, p->dev_status.online);
	}
	FT_UNLOCK(&s_head->lock);
}
