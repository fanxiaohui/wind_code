/*
 * XGCore.c
 *
 *  Created on: 2016年11月30日
 *      Author: john
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>

#include "XGCore.h"
#include "XGThread.h"
#include "XGDebug.h"
#include "XGMem.h"
#include "XGTime.h"
#include "XGageway.h"
#include "ConnectTask.h"

//
static int is_valid_ip(const char *ip) {
	int section = 0;
	int dot = 0;
	int last = -1;
	while (*ip) {
		if (*ip == '.') {
			dot++;
			if (dot > 3) {
				return 0;
			}
			if (section >= 0 && section <= 255) {
				section = 0;
			} else {
				return 0;
			}
		} else if (*ip >= '0' && *ip <= '9') {
			section = section * 10 + *ip - '0';
			if (last == '0') {
				return 0;
			}
		} else {
			return 0;
		}
		last = *ip;
		ip++;
	}

	if (section >= 0 && section <= 255) {
		if (3 == dot) {
			section = 0;
			return 1;
		}
	}
	return 0;
}

static in_addr_t XGGetIpaddr(XGCoreCtx core) {
	if (is_valid_ip(core->Host)) {
		XGDEBUG("Get Host Remote Server %s\r\n", core->Host);
		return inet_addr(core->Host);
	}

	struct hostent * phost = gethostbyname(core->Host);
	if (phost) {
		char buffer[64] = { 0x00 };
		sprintf(buffer, "%s", inet_ntoa(*((struct in_addr *) phost->h_addr)));
		XGDEBUG("Get Host Remote Server %s\r\n", buffer);
		return inet_addr(buffer);
	} else {
		XGDEBUG("Get Host Remote Server %s\r\n", core->Host);
	}

	return 0;
}

static int XGConnectServer(XGCoreCtx core, ConnectTask_t task) {

	struct sockaddr_in sin;
	int connectret = 0;
	memset(&sin, 0, sizeof(sin));

	in_addr_t ip = XGGetIpaddr(core);
	if (ip == 0) {
		XGDEBUG("get ip addr %d\r\n", ip);
		return -1;
	}
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd <= 0) {
		XGDEBUG("connect server create socket failed\r\n");
		return -1;
	}
	XGDEBUG("Start Connect Remote Server address %s port %d sock %d\r\n", core->Host, core->ServerPort, sockfd);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = ip;
	sin.sin_port = htons(core->ServerPort);
	int ul = 1;
	ioctl(sockfd, FIONBIO, &ul); //设置为非阻塞模式
	fcntl(sockfd, F_SETFL, O_NDELAY);
	struct timeval tm;
	fd_set set;

	connectret = connect(sockfd, (struct sockaddr *) &sin, sizeof(sin));
	if (connectret == -1) {
		if (errno != EINPROGRESS) {
			shutdown(sockfd, SHUT_RDWR);
			close(sockfd);
			XGDEBUG("sync connect server failed errno=%d\r\n", errno);
			return -1;
		}
		tm.tv_sec = 5;
		tm.tv_usec = 0;
		FD_ZERO(&set);
		FD_SET(sockfd, &set);
		if (select(sockfd + 1, NULL, &set, NULL, &tm) > 0) {
			int error = -1, len = 4;
			getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *) &len);
			if (error == 0) {
				//connect success
			} else {
				shutdown(sockfd, SHUT_RDWR);
				close(sockfd);
				XGDEBUG("SYNC connect server failed\r\n");
				return -1;
			}
		} else {
			shutdown(sockfd, SHUT_RDWR);
			close(sockfd);
			XGDEBUG("SYNC connect server failed timeout\r\n");
			return -1;
		}
	}
	XGDEBUG("SYNC connect server success\r\n");
	task->fd = sockfd;
	return 0;
}

static void XGAcceptConnect(XGCoreCtx core) {
	struct sockaddr_in stClientAddr={0};
	socklen_t socketAddrLen;
	int sock = 0;
	socketAddrLen = sizeof(struct sockaddr_in);

	sock = accept(core->ServerTcpSocketHandle, (struct sockaddr*) &stClientAddr, &socketAddrLen);
	if (sock <= 0) {
		XGERROR("accept errno=%d\r\n", errno);
		return;
	}

	XGDEBUG("New Connect socket=%d\r\n", sock);

	Connect_t conn = XGConnectFactory();
	if (conn == NULL) {
		close(sock);
		XGERROR("new connect malloc(connect) failed \r\n");
		return;
	}
	int mOpttmp = 1;
	setsockopt(sock, SOL_SOCKET,SO_REUSEADDR, (const void *) &mOpttmp, sizeof(mOpttmp));
	memcpy(&conn->client.Addr, &stClientAddr, sizeof(stClientAddr));
	int flags = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, flags | O_NONBLOCK);
	conn->isActive = 1;
	conn->client.SocketHandle = sock;
	conn->server.status = E_SER_WAIT_CLIENT_LOGIN; //等待设备的登录包
	conn->client.status = E_CLI_WAIT_LOGIN; //等待设备登录
	conn->client.time = XGGetLocalTime();
	conn->next = core->Connects.next;
	core->Connects.next = conn;
}

#include "XGProtocolUdp.h"
static void XGRecvUdpData(XGCoreCtx core, char *buffer) {
	struct sockaddr_in clientAddr;
	socklen_t len = sizeof(clientAddr);
	int bytes = recvfrom(core->ServerUdpSocketHandle, buffer, BQ_BUFFER_SIZE, 0, (struct sockaddr*) &clientAddr, &len);
	if (bytes > 0) {
		XGDEBUG("Recv udp data length=%d\r\n", bytes);
		XGStartProtocolUdp(core, (unsigned char *) buffer, bytes, &clientAddr);
	}
}

static void XGRecvConnectClient(Connect_t conn, char *buffer) {

	int used = XGByteQueueSize(&conn->client.recvBuffer);
	int size = recv(conn->client.SocketHandle, buffer, BQ_BUFFER_SIZE - used, 0);
	if (size == 0) {
		XGDEBUG("Client Disconnect  socket=%d ret=%d errno=%d deviceid=%d\r\n",conn->client.SocketHandle, size, errno,conn->device_id);
		conn->disconnectReason = E_DIS_CLIENT_CLOSE;
		conn->isActive = 0;
		shutdown(conn->client.SocketHandle, SHUT_RDWR);
		close(conn->client.SocketHandle);
		conn->client.SocketHandle = 0;
		return;
	}

	if (size < 0)
		return;

	XGDEBUG("Recv Client[%d] Data length=%d\r\n", conn->client.SocketHandle, size);
	XGByteQueuePush(&conn->client.recvBuffer, (unsigned char *) buffer, size);

}

static void XGRecvConnectServer(Connect_t conn, char *buffer) {

	int used = XGByteQueueSize(&conn->server.recvBuffer);
	int size = recv(conn->server.SocketHandle, buffer, BQ_BUFFER_SIZE - used, 0);
	if (size == 0) {
		XGDEBUG("Server Disconnect socket=%d ret=%d errno=%d deviceid=%d\r\n", conn->server.SocketHandle, size, errno,conn->device_id);
		conn->server.status = E_SER_WAIT_CONECT;
		conn->server.time = XGGetLocalTime();
		shutdown(conn->server.SocketHandle, SHUT_RDWR);
		close(conn->server.SocketHandle);
		conn->server.SocketHandle = 0;

		return;
	}
	if (size < 0)
		return;
	XGDEBUG("Recv Server[%d] Data length=%d\r\n", conn->server.SocketHandle, size);
	XGByteQueuePush(&conn->server.recvBuffer, (unsigned char *) buffer, size);

}

static void XGConnectSend(Connect_t conn, int server) {
	XGQueueItem temp = NULL;
	int sock = 0;
	if (server) {
		temp = &conn->server.sendQueue;
		sock = conn->server.SocketHandle;
	} else {
		temp = &conn->client.sendQueue;
		sock = conn->client.SocketHandle;
	}

	XGQueueItem data = temp->next;
	temp->next = data->next;
	data->next = NULL;
	int ret = send(sock, data->data, data->dataSize, 0);
	XGDEBUG("Send Tcp socket[%d] Datalenght=%d\r\n", sock, ret);
	XGDataQueueRelease(data);
}
//移除不活动的客户端连接
static void XGRemoveNotActive(XGCoreCtx core) {
	Connect_t conn = NULL;
	Connect_t head = &core->Connects;
	conn = head->next;
	while (conn != NULL) {
		if (conn->isActive == 0) {
			XGDEBUG("Remove Client socket=%d server socket=%d\r\n", conn->client.SocketHandle, conn->server.SocketHandle);
			shutdown(conn->client.SocketHandle, SHUT_RDWR);
			shutdown(conn->server.SocketHandle, SHUT_RDWR);
			close(conn->client.SocketHandle);
			close(conn->server.SocketHandle);
			conn->client.SocketHandle = 0;
			conn->server.SocketHandle = 0;
			if (core->config && conn->device_id > 0 && conn->client.islogined) {
				if (core->config->OnLoginout) {
					core->config->OnLoginout(conn->device_id, conn->disconnectReason);
				}
			}
			head->next = conn->next;
			XGConnectRelease(conn);
			conn = head->next;
			continue;
		}
		head = conn;
		conn = head->next;
	}
}

#include "XGProtocol.h"
//开始解析数据协议
static void XGProtocolStart(XGCoreCtx core) {
	Connect_t conn = NULL;
	conn = core->Connects.next;
	int size = 0;
	while (conn != NULL) {
		size = XGByteQueueSize(&conn->client.recvBuffer);
		size += XGByteQueueSize(&conn->server.recvBuffer);
		if (size > 0) {
			XGProtocol(conn, core);
		}
		conn = conn->next;
	}
}

static inline int XGSendQueueClient(XGCoreCtx core, XGQueueItem item) {
	Connect_t conn = core->Connects.next;
	while (conn != NULL) {
		if (conn->device_id == item->deviceid)
			break;
		conn = conn->next;
	}
	if (conn == NULL || conn->client.status != E_CLI_LOGINED) {
		return -1;
	}
	XGQueueItem temp = &conn->client.sendQueue;
	while (temp->next != NULL) {
		temp = temp->next;
	}
	temp->next = item;
	return 0;
}
//发送数据到云端
static inline int XGSendQueueCloud(XGCoreCtx core, XGQueueItem item) {
	Connect_t conn = core->Connects.next;
	while (conn != NULL) {
		if (conn->device_id == item->deviceid)
			break;
		conn = conn->next;
	}
	//找到这个deviceid 或者 没有登录服务器不能发送数据
	if (conn == NULL || conn->server.status != E_SER_LOGINED) {
		return -1;
	}
	XGQueueItem temp = &conn->server.sendQueue;
	while (temp->next != NULL) {
		temp = temp->next;
	}
	temp->next = item;
	return 0;
}
//处理需要发送的数据
static void XGPSendQueeu(XGCoreCtx core) {
	if ((core->ClientSendQueue.next == NULL) && (core->CloudSendQueue.next == NULL)) {
		return;
	}

	int lock = pthread_mutex_trylock(&core->lockSendData);
	if (lock != 0)
		return;
	XGDEBUG("--LOCK\r\n");
	int ret = 0;
	XGQueueItem item = core->ClientSendQueue.next;
	while (item != NULL) {
		core->ClientSendQueue.next = item->next;
		item->next = NULL;
		ret = XGSendQueueClient(core, item);
		if (ret != 0) {
			XGDataQueueRelease(item);
		}
		item = core->ClientSendQueue.next;
	}

	item = core->CloudSendQueue.next;
	while (item != NULL) {
		core->CloudSendQueue.next = item->next;
		item->next = NULL;
		ret = XGSendQueueCloud(core, item);
		if (ret != 0) {
			XGDataQueueRelease(item);
		}
		item = core->CloudSendQueue.next;
	}

	pthread_mutex_unlock(&core->lockSendData);
	XGDEBUG("--unLOCK\r\n");
}
//释放发送数据
static void XGReleaseDataQueue(struct XGQueue *queue) {
	XGQueueItem item = queue->next;
	while (item != NULL) {
		queue->next = item->next;
		item->next = NULL;
		XGReleaseDataQueue(item);
		item = queue->next;
	}
}
//处理连接服务器任务
static void XGConnectTask(XGCoreCtx core) {

	ConnectTask_t head = NULL;
	Connect_t conn = core->Connects.next;
	while (conn != NULL) {
		if (conn->server.status == E_SER_WAIT_CONECT && conn->isActive) {
			if ((XGGetLocalTime() - conn->server.time) > 3000) {
				XGReleaseDataQueue(&conn->server.sendQueue);
				ConnectTask_t task = XGConnectTaskFactory();
				if (task != NULL) {
					conn->server.status = E_SER_CONNECTING;
					task->device_id = conn->device_id;
					task->fd = 0;
					task->next = NULL;
					XGDEBUG("Add connect task devicdid=%d\r\n", conn->device_id);
					if (head == NULL) {
						head = task;
					} else {
						ConnectTask_t tail = head;
						while (tail->next != NULL) {
							tail = tail->next;
						}
						tail->next = task;
					}
				}
			}
		}
		conn = conn->next;
	}
	if (head == NULL) {
		return;
	}
	pthread_mutex_lock(&core->lockConnectServerTask);
	ConnectTask_t tail = &core->ConnectTasks;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = head;
	pthread_mutex_unlock(&core->lockConnectServerTask);
}

static Connect_t XGGetConnectByDeviceid(XGCoreCtx core, int deviceid) {
	Connect_t con = &core->Connects;
	while (con != NULL) {
		if (con->device_id == deviceid) {
			return con;
		}
		con = con->next;
	}
	return NULL;
}
//处理连接服务器的返回结果
static void XGConnectTaskResult(XGCoreCtx core) {

	if (core->ConnectTasksResult.next == NULL) {
		return;
	}
	//取出任务
	int lock = pthread_mutex_trylock(&core->lockConnectServerResult);
	if (lock != 0) {
		return;
	}
	ConnectTask_t item = core->ConnectTasksResult.next;
	core->ConnectTasksResult.next = NULL;
	pthread_mutex_unlock(&core->lockConnectServerResult);

	while (item != NULL) {
		ConnectTask_t temp = item;
		XGDEBUG("get connect task result device id=%d\r\n", temp->device_id);
		//找到任务对应的设备
		Connect_t conn = XGGetConnectByDeviceid(core, temp->device_id);
		if (conn != NULL) {
			conn->server.status = E_SER_WAIT_CONECT;
			conn->server.time = XGGetLocalTime();
			//连接服务器成功
			if (temp->fd > 0) {
				XGDEBUG("get connect task result device id=%d connect server success\r\n", temp->device_id);
				//发送登录信息
				int sendcount = send(temp->fd, conn->loginData, conn->loginDatalength, 0);
				if (sendcount > 0) {
					conn->server.SocketHandle = temp->fd;
					conn->server.status = E_SER_WAIT_LOGIN_RESULT; //等待登录返回结果
					conn->server.time = XGGetLocalTime();
				} else { //发送登录包失败
					shutdown(temp->fd, SHUT_RDWR);
					close(temp->fd);
					temp->fd = 0;
					XGDEBUG("get connect task result device id=%d connect server failed\r\n", temp->device_id);
				}
			} else { //连接服务器失败
				XGDEBUG("get connect task result device id=%d connect server failed\r\n", temp->device_id);
			}
		} else { //没有找到对应的设备，设备已经断开连接
			if (temp->fd > 0) {
				shutdown(temp->fd, SHUT_RDWR);
				close(temp->fd);
				temp->fd = 0;
			}
			XGDEBUG("get connect task result device id=%d not find device\r\n", temp->device_id);
		}
		item = temp->next;
		temp->next = NULL;
		//删除任务
		XGConnectTaskRelease(temp);
	}
}

//检测超时时间
static void XGTimeout(XGCoreCtx core) {

	Connect_t con = &core->Connects;
	XGTimeMs current_time = XGGetLocalTime();
	while (con != NULL) {

		if (current_time < con->server.time) {
			con->server.time = current_time;
		}
		//登录登录云端的返回包超时检测。15秒内需要返回登录结果。
		if (con->server.status == E_SER_WAIT_LOGIN_RESULT) {
			if ((current_time - con->server.time) > 15000) {
				if (con->server.SocketHandle > 0) {
					shutdown(con->server.SocketHandle, SHUT_RDWR);
					close(con->server.SocketHandle);
					con->server.SocketHandle = 0;
					con->server.status = E_SER_WAIT_CONECT;
					con->server.time = XGGetLocalTime();
				}
			}
		}

		if (con->client.time > current_time) {
			con->client.time = current_time;
		}
		//子设备的登录包检测,20 秒超时
		if (con->client.status == E_CLI_WAIT_LOGIN) {

			if ((current_time - con->client.time) > 20000) {
				con->disconnectReason = E_DIS_CLIENT_LOGIN_TIMEOUT;
				con->isActive = 0;
			}
		}
		//检测子设备心跳超时
		else if (con->client.status == E_CLI_LOGINED) {
			if ((current_time - con->client.time) > 180000) {
				con->disconnectReason = E_DIS_CLIENT_PING_TIMEOUT;
				con->isActive = 0;
			}
		}

		con = con->next;
	}
}

#if 0
static void XGPReconnectServer(XGCoreCtx core) {

	Connect_t conn = NULL;
	Connect_t head = &core->Connects;
	conn = head->next;
	while (conn != NULL) {
		if (conn->isActive == 1) {
			if(conn->isLocalConnect == E_LOCAL_CONNECT) {
				conn->isActive = 0;
			}
		}
		head = conn;
		conn = head->next;
	}
}

THREAD_HANDLE(XGCoreNetworkDaemonThread) {
	XGCoreCtx core = (XGCoreCtx) args;
	char szaddr[128] = {0};
	strcpy(szaddr, core->Host);
	int isIpAddr = 0;
	if (is_valid_ip(szaddr)) {
		isIpAddr = 1;
		memset(core->ServerIpAddress, 0, CORE_IP_ADDRESS_BUFFER_SIZE);
		strcpy(core->ServerIpAddress, szaddr);
	}
	time_t LastCheckTime = 0;
	core->isNeedCheckNet = E_IS_NEED;
	core->netStatus = E_NET_WAIT_CHECK;
	while (!core->isQuit) {
		time_t c = time(NULL);
		if (((c - LastCheckTime) >= 30) && core->isNeedCheckNet == E_IS_NEED) {
			core->netStatus = E_NET_CHECK_ING;
			LastCheckTime = c;
			XGDEBUG("Start check network time=%d\r\n", time(NULL));
			if (isIpAddr) {
				struct sockaddr_in sin;
				int sockfd = socket(AF_INET, SOCK_STREAM, 0);
				if (sockfd <= 0) {
					XGDEBUG("network connect server create socket failed\r\n",
							szaddr);
					usleep(100000);
					continue;
				}
				sin.sin_family = AF_INET;
				sin.sin_addr.s_addr = inet_addr(szaddr);
				sin.sin_port = htons(23778);
				fcntl(sockfd, F_SETFL, O_NDELAY);
				int ul = 1;
				ioctl(sockfd, FIONBIO, &ul); //设置为非阻塞模式
				struct timeval tm;
				fd_set set;
				if (connect(sockfd, (struct sockaddr *) &sin, sizeof(sin))
						== -1) {
					if (errno != EINPROGRESS) {
						core->netStatus = E_NET_FAILED;
						XGDEBUG("check network failed time=%d error=%d\r\n",
								time(NULL), errno);
					}
					tm.tv_sec = 10;
					tm.tv_usec = 0;
					FD_ZERO(&set);
					FD_SET(sockfd, &set);
					if (select(sockfd + 1, NULL, &set, NULL, &tm) > 0) {
						int error = -1, len = 4;

						int nret = getsockopt(sockfd, SOL_SOCKET, SO_ERROR,
								&error, (socklen_t *) &len);
						if (error == 0) {
							//connect success
							core->netStatus = E_NET_OK;
							core->isNeedCheckNet = E_IS_NOT_NEED;

							XGDEBUG("check network ok \r\n");
						} else {
							core->netStatus = E_NET_FAILED;
							XGDEBUG(
									"check network failed time=%d geterror=%d nret=%d error=%d\r\n",
									time(NULL), error, nret, errno);
						}
					} else {
						core->netStatus = E_NET_FAILED;
						XGDEBUG("check network failed timeout time=%d\r\n",
								time(NULL));
					}

				} else {
					core->netStatus = E_NET_OK;
					core->isNeedCheckNet = E_IS_NOT_NEED;
					XGDEBUG("check network ok\r\n");
				}
				shutdown(sockfd, SHUT_RDWR);
				close(sockfd);
			} else {
				struct hostent * phost = gethostbyname(szaddr);
				if (phost) {
					memset(core->ServerIpAddress, 0,
							CORE_IP_ADDRESS_BUFFER_SIZE);
					sprintf(core->ServerIpAddress, "%s",
							inet_ntoa(*((struct in_addr *) phost->h_addr)));
					XGDEBUG("Get Host Remote Server %s\r\n", szaddr);
					struct sockaddr_in sin;
					int sockfd = socket(AF_INET, SOCK_STREAM, 0);
					if (sockfd <= 0) {
						XGDEBUG(
								"network connect server create socket failed\r\n",
								szaddr);
						usleep(100000);
						continue;
					}
					sin.sin_family = AF_INET;
					sin.sin_addr.s_addr = inet_addr(core->ServerIpAddress);
					sin.sin_port = htons(23778);
					fcntl(sockfd, F_SETFL, O_NDELAY);
					//ioctl(sockfd, FIONBIO, &ul); //设置为非阻塞模式
					struct timeval tm;
					fd_set set;
					if (connect(sockfd, (struct sockaddr *) &sin, sizeof(sin))
							== 0) {
						if (errno != EINPROGRESS) {
							core->netStatus = E_NET_FAILED;
							XGDEBUG("check network failed time=%d error=%d\r\n",
									time(NULL), errno);
						}
						tm.tv_sec = 3;
						tm.tv_usec = 0;
						FD_ZERO(&set);
						FD_SET(sockfd, &set);
						if (select(sockfd + 1, NULL, &set, NULL, &tm) > 0) {
							int error = 0, len = 4;
							int nret = getsockopt(sockfd, SOL_SOCKET, SO_ERROR,
									&error, (socklen_t *) &len);
							if (error == 0) {
								//connect success
								core->netStatus = E_NET_OK;
								core->isNeedCheckNet = E_IS_NOT_NEED;
								XGDEBUG("check network ok\r\n");
							} else {
								core->netStatus = E_NET_FAILED;
								XGDEBUG(
										"check network failed time=%d error=%d nret=%d\r\n",
										time(NULL), error, nret);
							}
						} else {
							core->netStatus = E_NET_FAILED;
							XGDEBUG("check network failed timeout time=%d\r\n",
									time(NULL));
						}

					} else {
						core->netStatus = E_NET_OK;
						core->isNeedCheckNet = E_IS_NOT_NEED;
						XGDEBUG("check network ok\r\n");
					}
					shutdown(sockfd, SHUT_RDWR);
					close(sockfd);
				} else {
					XGDEBUG("Get Host Remote Server failed host=%s\r\n",
							szaddr);
				}
			}
		}

		usleep(100000);
	}
	return 0;
}

#endif

THREAD_HANDLE(XGCoreConnectServerThread) {
	XGCoreCtx core = (XGCoreCtx) args;
	time_t oldlog = 0;
	while (!core->isQuit) {

		if (time(NULL) - oldlog > 5) {
			oldlog = time(NULL);
			XGDEBUG("-----------------------------connect task thread keeplive------------------------\r\n");
		}

		if (core->ConnectTasks.next == NULL) {
			XGSleepMs(300);
			continue;
		}
		int lock = pthread_mutex_trylock(&core->lockConnectServerTask);
		if (lock != 0) {
			XGSleepMs(100);
			continue;
		}
		ConnectTask_t task = core->ConnectTasks.next;
		core->ConnectTasks.next = task->next;
		task->next = NULL;
		pthread_mutex_unlock(&core->lockConnectServerTask);
		task->fd = 0;
		XGDEBUG("Get connect task device id=%d\r\n", task->device_id);
		XGConnectServer(core, task);
		pthread_mutex_lock(&core->lockConnectServerResult);
		ConnectTask_t temp = &core->ConnectTasksResult;
		while (temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = task;
		pthread_mutex_unlock(&core->lockConnectServerResult);

		XGSleepMs(30);
	}
	return 0;
}

THREAD_HANDLE(XGCoreThread) {
	XGCoreCtx core = (XGCoreCtx) args;
	fd_set fdread, fdwrite;
	int maxfd = 0;
	int isWrite = 0;
	Connect_t conn = NULL;
	struct timeval timeout = { 3, 0 };
	int selectret = 0;
	core->isInited = 1;
	core->isRuning = 1;

	char RecvBuffer[BQ_BUFFER_SIZE] = { 0x00 };
	time_t oldlog = 0;
	while (!core->isQuit) {
		isWrite = 0;
		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);
		FD_SET(core->ServerTcpSocketHandle, &fdread);
		FD_SET(core->ServerUdpSocketHandle, &fdread);
		maxfd = core->ServerTcpSocketHandle > core->ServerUdpSocketHandle ? core->ServerTcpSocketHandle : core->ServerUdpSocketHandle;
		XGRemoveNotActive(core);
		XGProtocolStart(core);
		XGPSendQueeu(core);
		XGConnectTask(core);
		XGConnectTaskResult(core);
		XGTimeout(core);
		conn = core->Connects.next;
		while (conn != NULL) {
			if (conn->client.SocketHandle > 0 && conn->isActive) {
				FD_SET(conn->client.SocketHandle, &fdread);
				if (conn->client.sendQueue.next != NULL) {
					FD_SET(conn->client.SocketHandle, &fdwrite);
					isWrite = 1;
				}
				maxfd = maxfd < conn->client.SocketHandle ? conn->client.SocketHandle : maxfd;
			} else {
				conn->isActive = 0;
			}

			if ((conn->server.SocketHandle > 0) && conn->isActive) {
				FD_SET(conn->server.SocketHandle, &fdread);
				if (conn->server.sendQueue.next != NULL) {
					FD_SET(conn->server.SocketHandle, &fdwrite);
					isWrite = 1;
				}
				maxfd = maxfd < conn->server.SocketHandle ? conn->server.SocketHandle : maxfd;
			}
			conn = conn->next;
		}

		//打印一个线程心跳 5 秒
		if (time(NULL) - oldlog > 5) {
			oldlog = time(NULL);
			XGDEBUG("-----------------------------XGCore thread keeplive------------------------\r\n");
		}
		timeout.tv_sec = 0;
		timeout.tv_usec = 1000 * 300;
		if (isWrite) {
			selectret = select(maxfd + 1, &fdread, &fdwrite, NULL, &timeout);
		} else {
			selectret = select(maxfd + 1, &fdread, NULL, NULL, &timeout);
		}

		if (selectret <= 0) {
			continue;
		}

		if (FD_ISSET(core->ServerTcpSocketHandle, &fdread)) {
			XGAcceptConnect(core);
		}

		if (FD_ISSET(core->ServerUdpSocketHandle, &fdread)) {
			memset(RecvBuffer, 0, BQ_BUFFER_SIZE);
			XGRecvUdpData(core, RecvBuffer);
		}

		if (isWrite) {
			conn = core->Connects.next;
			while (conn != NULL) {
				if (FD_ISSET(conn->client.SocketHandle, &fdwrite)) {
					XGConnectSend(conn, 0);
				}
				if (FD_ISSET(conn->server.SocketHandle, &fdwrite)) {
					XGConnectSend(conn, 1);
				}
				conn = conn->next;
			}
		}

		conn = core->Connects.next;
		while (conn != NULL) {
			if (FD_ISSET(conn->client.SocketHandle, &fdread)) {
				memset(RecvBuffer, 0, BQ_BUFFER_SIZE);
				XGRecvConnectClient(conn, RecvBuffer);
			}
			if (FD_ISSET(conn->server.SocketHandle, &fdread)) {
				memset(RecvBuffer, 0, BQ_BUFFER_SIZE);
				XGRecvConnectServer(conn, RecvBuffer);
			}
			conn = conn->next;
		}

	}
	XGINFO("XGCore exit...\r\n");
	core->isRuning = 0;
	shutdown(core->ServerTcpSocketHandle, SHUT_RDWR);
	shutdown(core->ServerUdpSocketHandle, SHUT_RDWR);
	close(conn->client.SocketHandle);
	close(core->ServerUdpSocketHandle);

	return 0;
}

static int XGCreateTcpServer(int port) {

	int listenfd = 0;
	struct sockaddr_in server;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		XGERROR("Create tcp handle failed\r\n");
		perror("socket() error. Failed to initiate a socket");
		return -1;
	}

	int opt = SO_REUSEADDR;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	bzero(&server, sizeof(server));

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(listenfd, (struct sockaddr *) &server, sizeof(server)) == -1) {
		XGERROR("Bind failed\r\n");
		perror("Bind() error.");
		return -1;
	}

	if (listen(listenfd, 32) == -1) {
		XGERROR("listen failed\r\n");
		perror("listen() error. \n");
		return -1;
	}
	XGDEBUG("Create Tcp Server Success port=%d\r\n", port);
	return listenfd;
}

static int XGCreatUdpServer(int port) {
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int sock;
	int opt = SO_REUSEADDR;

	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *) &opt, sizeof(opt));

	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("bind");
		return -1;
	}

	return sock;
}

int XGStartCore(XGCoreCtx ctx) {
	if (ctx->isInited) {
		ctx->ErrorCode = EC_ALREADY_INIT;
		return -1;
	}
	XGIgoreSignal();
	ctx->ErrorCode = EC_NO_ERROR;
	ctx->isQuit = 0;
	int sock = XGCreateTcpServer(ctx->LocalTcpServerPort);
	if (sock <= 0) {
		ctx->ErrorCode = EC_CREATE_TCP_SERVER_FAILED;
		return EC_CREATE_TCP_SERVER_FAILED;
	}

	ctx->ServerTcpSocketHandle = sock;
	sock = XGCreatUdpServer(ctx->LocalUdpServerPort);
	if (sock <= 0) {
//		ctx->ErrorCode = EC_CREATE_UDP_SERVER_FAILED;
//		return EC_CREATE_UDP_SERVER_FAILED;
	}
	ctx->ServerUdpSocketHandle = sock;
	pthread_mutex_init(&ctx->lockSendData, NULL);
	pthread_mutex_init(&ctx->lockConnectServerTask, NULL);
	pthread_mutex_init(&ctx->lockConnectServerResult, NULL);
	//pthread_cond_init(&(ctx->condConnectServerTask), NULL);
	CREATE_THREAD(XGCoreThread, ctx);
	CREATE_THREAD(XGCoreConnectServerThread, ctx);
//CREATE_THREAD(XGCoreNetworkDaemonThread, ctx);
	return EC_NO_ERROR;
}

int XGStopCore(XGCoreCtx ctx) {
	ctx->isQuit = 1;
	while (ctx->isRuning)
		;
	pthread_mutex_destroy(&ctx->lockSendData);
	pthread_mutex_destroy(&ctx->lockConnectServerTask);
	pthread_mutex_destroy(&ctx->lockConnectServerResult);
	// pthread_cond_destroy(&(ctx->condConnectServerTask));
//释放connects
	Connect_t conn = ctx->Connects.next;
	while (conn != NULL) {
		XGConnectRelease(conn);
		ctx->Connects.next = conn->next;
		conn = ctx->Connects.next;
	}
	return 0;
}

void XGIgoreSignal(void) {
	signal(SIGPIPE, SIG_IGN);
}

XGCoreCtx XGCoreFactory() {
	XGCoreCtx ret = (XGCoreCtx) XGMemFactory(XGCORE_SIZE);
	memset(ret, 0, sizeof(struct XGCore));
	return ret;
}
void XGCoreRelease(XGCoreCtx ctx) {
	XGMemRelease(ctx);
	ctx = NULL;
}

int XGCoreSendData(XGCoreCtx ctx, XGQueueItem item, int where) {
	int ret = 0;
	pthread_mutex_lock(&ctx->lockSendData);
	XGDEBUG("--LOCK\r\n");
	if (where == E_TO_DEVICE) {
		XGQueueItem temp = &ctx->ClientSendQueue;
		while (temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = item;
	} else if (where == E_TO_CLOUD) {
		XGQueueItem temp = &ctx->CloudSendQueue;
		while (temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = item;
	} else {
		ret = -1;
	}
	pthread_mutex_unlock(&ctx->lockSendData);
	XGDEBUG("--unLOCK\r\n");
	return ret;
}

