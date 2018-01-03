#ifndef MSG_H
#define MSG_H

#define MSG_NET 			0x0000000F /* the whole status of network*/
#define MSG_NET_ON_LINE		0x00000001 /* the network had disconnected*/
#define MSG_NET_OFF_LINE	0x00000002 /* the netowrk had connected*/
#define MSG_NET_CONNING		0x00000004 /* the network is connecting*/
#define MSG_NET_ERROR		0x00000008 /* the network is connecting*/

#define MSG_DEV 			0x000000F0
#define MSG_DEV_ON_LINE		0x00000010 /* online or offline*/
#define MSG_DEV_OFF_LINE	0x00000020 /* online or offline*/
#define MSG_DEV_DATA		0x00000040 /* the whole status of device*/
#define MSG_DEV_INFO		0x00000080 /* the attribution of device*/

#define MSG_CTRL 			0x00000F00
#define MSG_CTRL_CMD_DEV	0x00000100
#define MSG_CTRL_DEL_DEV	0x00000200
#define MSG_CTRL_SYNC		0x00000400

#define MSG_SYS				0x0000F000

#endif

