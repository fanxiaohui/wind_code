#ifndef MAP_H
#define MAP_H
#include "ft_ikcc_net_server.h"

//定义结构
typedef struct _st_link_node{
	st_client_device dev_status;
	int size;
	FT_MUTEX_TYPE lock;
	struct _st_link_node* next;
}st_link_list;

int init_list(st_link_list* head);
int destroy_list(st_link_list* head);
int dump_device_list();
int add_device_to_list(st_link_list *head, st_client_device* node);
int del_from_list_by_mac(st_link_list *head, const char* mac);
st_client_device *  find_device_by_mac(st_link_list *head, const char* mac);
st_client_device *  find_device_by_fd(st_link_list *head, const int fd);


#endif /* MAP_H */