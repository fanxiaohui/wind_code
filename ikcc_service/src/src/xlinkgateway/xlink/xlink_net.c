#include "xlink_net.h"
#include "xlinkv4Udp.h"
#include "xlink_client.h"
#include "xlink_tcp_client.h"
#include "xlink_process_tcp_data.h"
#include "xsdk_config.h"

#if XLINK_USE_SYNC_CONNECT_SERVER

#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
//#include <error.h>
#endif

#if CLIENT_SSL_ENABLE		
CYASSL_METHOD* tq_method = 0;
CYASSL_CTX* tq_ctx = 0;
CYASSL* tq_ssl = 0;
#endif
#if CLIENT_SSL_ENABLE		
#define XLINK_TCP_CONNECT_PORT 23779
#else
//#define XLINK_TCP_CONNECT_PORT 9999
#define XLINK_TCP_CONNECT_PORT 23778
#endif
int Xlink_TCP_ConnectPort = XLINK_TCP_CONNECT_PORT;
static xsdk_time_t last_check_heartbeat = 0;

#ifdef TEST_SERVER
static char g_xlink_hostNameOrIp[64] = {"42.121.122.23"};
#else

#if __XLINK_STAND_HAIMAN__
static char g_xlink_hostNameOrIp[64] = {"io.heiman.com.cn"};
#else
static char g_xlink_hostNameOrIp[64] = { "47.96.38.166" };
#endif

#endif

extern int g_udp_listen_port;

XLINK_FUNC void XlinkSetHost(const char *domain) {
	memset(g_xlink_hostNameOrIp, 0, sizeof(g_xlink_hostNameOrIp));
		strcpy(g_xlink_hostNameOrIp, domain);
}


XLINK_FUNC const char *xlinkNetGetHost(void) {
	return g_xlink_hostNameOrIp;
}

#if (!__LWIP__ESP_8266) && (!__MT7681__) && (!__STM32F107__) && (!__STM32F103_UIP__) && (!__ALL_DEVICE__)

extern G_XLINK_INFO g_xlink_info;

static xlink_set_fd m_fd_set;

static XLINK_SELECT *xlink_event = NULL;
static int g_maxfd = 0;
static unsigned char g_isWork = 0;
static x_uint8 *g_RecvDataBuffer = NULL;

XLINK_FUNC int static xlink_udp_create(x_int32 port);
XLINK_FUNC int static xlink_connect_Port(const int fd, xlink_addr *addr);
XLINK_FUNC unsigned long static IPtoAddr(const char *ip);
XLINK_FUNC int static is_valid_ip(const char *ip);
XLINK_FUNC int xlinkGetHostByName(const char *hostname);
XLINK_FUNC int static xlink_tcp_connect_server(const unsigned int pIpAddr);
XLINK_FUNC void static tcp_http(unsigned char *RecvBuffer);
XLINK_FUNC static int tcp_Connect_server_init(const xsdk_time_t c_time, const unsigned int pIpAddr);

#if REALTEK || REALTEK_8711
void setNoBlock(int s) {
	int has = 0;
	ioctlsocket(s, FIONBIO, &has);
}
#endif


XLINK_FUNC void XlinkNetInit(void) {

	static unsigned char xlink_recvDtatBufer[__XLINK_BUFFER_PIPE__ + 50];

	g_xlink_info.net_info.tcpFlag.Bit.isFirstConnectSuccess = 0;
	g_xlink_info.net_info.tcpFlag.Bit.isLogined = 0;
	g_xlink_info.net_info.tcpFlag.Bit.is80ConnectSuccess = 0;
	g_xlink_info.net_info.tcpFlag.Bit.is80Port = 0;

	g_RecvDataBuffer = xlink_recvDtatBufer;
#if CLIENT_SSL_ENABLE
	CyaSSL_Init();
#endif
}
#if CLIENT_SSL_ENABLE	
void resetSSL(void) {
	if(tq_ssl != NULL) {
		CyaSSL_shutdown(tq_ssl);
		CyaSSL_free(tq_ssl);
		CyaSSL_CTX_free(tq_ctx);
		tq_ssl = NULL;
		tq_ctx = NULL;
		tq_method = NULL;
	}
}
#endif

XLINK_FUNC void XlinkInitEvent(XLINK_SELECT *event) {
	xlink_event = event;
}

XLINK_FUNC int static xlink_udp_create(x_int32 port) {

#if __MXCHIP__
	struct sockaddr_t mxchip_addr;
#else
	xlink_addr addr;
#endif
	int udp_fd = -1;
	int mOpttmp = 1;
	int ret = 0;
	g_xlink_info.net_info.udp_fd = -1;

#if __MXCHIP__
	udp_fd = xlink_socket(AF_INET, SOCK_DGRM, IPPROTO_UDP);
#else
	udp_fd = xlink_socket(AF_INET, SOCK_DGRAM, 0);
#endif

	if (udp_fd <= 0) {
#if __XDEBUG__

		XSDK_DEBUG_ERROR("create udp socket fail");

#endif
		return -1;

	} else {

#if REALTEK || REALTEK_8711
		setNoBlock(udp_fd);
#endif 

#if __XDEBUG__
		XSDK_DEBUG_DEBUG("create udp socket success");

#endif
	}

#if __MXCHIP__
	xlink_memset((char*) &mxchip_addr, 0, sizeof(mxchip_addr));
	mxchip_addr.s_port = port;
	mxchip_addr.s_ip = INADDR_ANY;
	xlink_setsockopt(udp_fd, SOL_SOCKET, SO_BROADCAST, &mOpttmp, sizeof(mOpttmp));
#else
	xlink_memset((char* ) &addr, 0, sizeof(xlink_addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	xlink_setsockopt(udp_fd, SOL_SOCKET, SO_BROADCAST, &mOpttmp, sizeof(mOpttmp));

	mOpttmp = 1;
	setsockopt(udp_fd, SOL_SOCKET, SO_REUSEADDR, (const void *) &mOpttmp, sizeof(mOpttmp));
#endif

#if    __MXCHIP__
	ret = xlink_bind(udp_fd, &mxchip_addr, sizeof(mxchip_addr));
#else
	ret = xlink_bind(udp_fd, (struct sockaddr*) &addr, sizeof(addr));
#endif
	if (ret < 0) {
		xlink_close(udp_fd);
#if __XDEBUG__
		XSDK_DEBUG_ERROR("local udp server bind failed");
#endif
		return -1;
	}
	g_xlink_info.net_info.udp_fd = udp_fd;
	return udp_fd;
}

XLINK_FUNC int static xlink_connect_Port(const int fd, xlink_addr *addr) {

	int retx = 0;
#if  __MXCHIP__
	struct sockaddr_t mxchip_addr;
	xlink_memset(&mxchip_addr, 0, sizeof(struct sockaddr_t));
	mxchip_addr.s_port = addr->sin_port;
	mxchip_addr.s_ip = addr->sin_addr.s_addr;
	retx = xlink_connect(fd, &mxchip_addr, sizeof(mxchip_addr));
#else
	retx = xlink_connect(fd, (struct sockaddr *) addr, sizeof(xlink_addr));
#endif

	if (retx < 0) {

#if XLINK_USE_SYNC_CONNECT_SERVER
		{
			if (errno != EINPROGRESS) {
				shutdown(fd, SHUT_RDWR);
				XSDK_DEBUG_DEBUG("sync connect server failed errno=%d", errno);
				return -1;
			}
			struct timeval tm;
			fd_set set;
			tm.tv_sec = 3;
			tm.tv_usec = 0;
			FD_ZERO(&set);
			FD_SET(fd, &set);
			if (select(fd + 1, NULL, &set, NULL, &tm) > 0) {
				int error = -1, len = 4;
				getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *) &len);
				if (error == 0) {
					//connect success
					return 0;
				} else {
					shutdown(fd, SHUT_RDWR);
					XSDK_DEBUG_DEBUG("SYNC connect server failed");
					return -1;
				}
			} else {
				shutdown(fd, SHUT_RDWR);
				XSDK_DEBUG_DEBUG("SYNC connect server failed timeout");
				return -1;
			}
		}
#endif

#if __XDEBUG__
		XSDK_DEBUG_DEBUG("connect tcp server failed %d\n", retx);
#endif
		return -1;
	}
	return 0;
}

XLINK_FUNC unsigned long static IPtoAddr(const char *ip) {
	unsigned long ret = 0;
	unsigned int buf[4];
	sscanf(ip, "%3d.%3d.%3d.%3d", &buf[0], &buf[1], &buf[2], &buf[3]);
	ret = (buf[3] << 24) + (buf[2] << 16) + (buf[1] << 8) + buf[0];
	return ret;
}

/**
 * return 1 is ip address
 */
XLINK_FUNC int static is_valid_ip(const char *ip) {
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

XLINK_FUNC int xlinkGetHostByName(const char *hostname) {

	unsigned int RetServerIpAddr = 0;
#if HF_SDK
	ip_addr_t dest_addr;
	if (hfnet_gethostbyname(hostname, &dest_addr) != HF_SUCCESS) {
		RetServerIpAddr = 0;
	} else {
		RetServerIpAddr = dest_addr.addr;
	}
#elif HF_SDK_A21
	struct hostent *h = NULL;
	h = gethostbyname(hostname);
	if (h != NULL) {
		RetServerIpAddr = *((unsigned int*) (h->h_addr_list[0]));
	}
#elif (MARVELL_SDK)
	struct hostent *entry = NULL;
	net_gethostbyname(hostname, &entry);
	if (entry) {
		memcpy(&RetServerIpAddr, entry->h_addr_list[0], entry->h_length);
	}

#elif (QCA4004)

	x_uint32 ipAddress = 0;
	if (qcom_dnsc_get_host_by_name((char*) hostname, &ipAddress) == 0) {
		RetServerIpAddr = LE_BE_32(ipAddress);
	}

#elif  (REALTEK)

	struct hostent *host = lwip_gethostbyname(hostname);
	if(host != NULL) {
		RetServerIpAddr = *((unsigned int*)(host->h_addr_list[0]));
	}
#elif REALTEK_8711
	struct hostent *host = lwip_gethostbyname(hostname);
	if (host != NULL ) {
		RetServerIpAddr = *((unsigned int*) (host->h_addr_list[0]));
	}
#elif (NL6621)

	x_uint32 ipAddress = 0;
	s8_t err;
	err = netconn_gethostbyname(hostname, &ipAddress);
	if (err != ERR_OK)
	{
	}
	else
	{
		RetServerIpAddr = ipAddress;
	}
#elif (__HED_10W07SN__)
	x_uint32 ipAddress=0;
	s8_t err = netconn_gethostbyname(hostname, &ipAddress);
	if (err != ERR_OK)
	{

	}
	else
	{
		RetServerIpAddr = ipAddress;
	}
#elif __LINUX__
	struct hostent *h = NULL;
	h = gethostbyname(hostname);
	if (h != NULL) {
		RetServerIpAddr = *((unsigned int*) (h->h_addr_list[0]));
	}
#elif __LSD4WF_2MD05101__
	//struct hostent* h = NULL;
	char *h = lsdnet_gethostbyname_ip(hostname);
	if (h != NULL ) {
		RetServerIpAddr = IPtoAddr(h);
	}
#elif __RTOS_STM32F103__
	struct ip_addr ipAddress;
	s8_t err = netconn_gethostbyname(hostname, &ipAddress);
	if (err != ERR_OK) {

	} else {
		RetServerIpAddr = ipAddress.addr;
	}
#elif __RTOS_STM32F107__

	struct ip_addr ipAddress;
	s8_t err = netconn_gethostbyname(hostname, &ipAddress);
	if (err != ERR_OK) {

	} else {
		RetServerIpAddr = ipAddress.addr;
	}

	//        struct hostent* h = NULL;
	//	h = lwip_gethostbyname(hostname);
	//	if (h != NULL ) {
	//		RetServerIpAddr = *((unsigned int*) (h->h_addr_list[0]));
	//	}
#elif __MXCHIP__
	OSStatus err = kUnknownErr;
	char ipstr[16];
	err = gethostbyname(hostname, (uint8_t*)ipstr,16);
	if(err == kGeneralErr) {
		return RetServerIpAddr;
	} else {
		RetServerIpAddr = inet_addr(ipstr);
	}
#endif

#if __XDEBUG__
	XSDK_DEBUG_DEBUG(" get host ip %d.%d.%d.%d", (RetServerIpAddr & 0xff000000) >> 24, (RetServerIpAddr & 0x00ff0000) >> 16, (RetServerIpAddr & 0x0000ff00) >> 8,
			RetServerIpAddr & 0x000000ff);

#endif

	return RetServerIpAddr;
}

XLINK_FUNC int static xlink_tcp_connect_server(const unsigned int pIpAddr) {

	int tcpfd = -1;
#if __MXCHIP__==0
	int tmp = 1;
#endif
	int cRet = 0;
#if  CLIENT_SSL_ENABLE
	int ret = 0;
	int err = 0;
	char buffer[80];
#endif
	xlink_addr addr;
	g_xlink_info.net_info.tcpFlag.Bit.isLogined = 0;
	g_xlink_info.net_info.tcp_fd = 0;

#if __XDEBUG__
	XSDK_DEBUG_DEBUG(" tcp  start connect server %d.%d.%d.%d", (pIpAddr & 0xff000000), (pIpAddr & 0x00ff0000) >> 8, (pIpAddr & 0x0000ff00) >> 16,
		 (pIpAddr & 0xff000000) >> 24);

#endif

	xlink_memset((char* ) &addr, 0, sizeof(xlink_addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = pIpAddr;

#if __MXCHIP__
	tcpfd = xlink_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
	tcpfd = xlink_socket(AF_INET, SOCK_STREAM, 0);
#endif

	if (tcpfd <= 0) {

#if __XDEBUG__
		XSDK_DEBUG_ERROR("xlink create tcp socket failed %d", tcpfd);

#endif
		return -1;
	} else {
#if __XDEBUG__
		XSDK_DEBUG_DEBUG("xlink create tcp socket success %d", tcpfd);

#endif

	}

#if XLINK_USE_SYNC_CONNECT_SERVER
	{
		int ulsync = 1;
		ioctl(tcpfd, FIONBIO, &ulsync); //设置为非阻塞模式
		fcntl(tcpfd, F_SETFL, O_NDELAY);
	}
#endif

#if REALTEK || REALTEK_8711
	setNoBlock(tcpfd);
#endif

#if  __MXCHIP__
	set_tcp_keepalive(3, 60);
#else
	tmp = 1;
	tmp = 180;
	if (xlink_setsockopt(tcpfd, SOL_SOCKET, SO_KEEPALIVE, &tmp, sizeof(tmp)) < 0) {
	}
#endif

#if (HF_SDK)
	tmp = 60; //60s
	if (xlink_setsockopt(tcpfd, IPPROTO_TCP, TCP_KEEPIDLE, &tmp, sizeof(tmp)) < 0) {
	}

	tmp = 60;
	if (xlink_setsockopt(tcpfd, IPPROTO_TCP, TCP_KEEPINTVL, &tmp, sizeof(tmp)) < 0) {
	}

	tmp = 60;
	if (xlink_setsockopt(tcpfd, IPPROTO_TCP, TCP_KEEPCNT, &tmp, sizeof(tmp)) < 0) {
	}
#endif

	if (g_xlink_info.net_info.ServerPort == 80) {
		g_xlink_info.net_info.ServerPort = Xlink_TCP_ConnectPort;
		g_xlink_info.net_info.tcpFlag.Bit.is80Port = 0;
		g_xlink_info.net_info.tcpFlag.Bit.is80ConnectSuccess = 1;
	} else if (g_xlink_info.net_info.ServerPort == Xlink_TCP_ConnectPort) {
#ifdef TEST_SERVER
		g_xlink_info.net_info.ServerPort = 808;
#else
		g_xlink_info.net_info.ServerPort = 80;
#endif
		g_xlink_info.net_info.tcpFlag.Bit.is80Port = 1;
		g_xlink_info.net_info.tcpFlag.Bit.is80ConnectSuccess = 0;
	} else {
		g_xlink_info.net_info.ServerPort = Xlink_TCP_ConnectPort;
		g_xlink_info.net_info.tcpFlag.Bit.is80Port = 0;
		g_xlink_info.net_info.tcpFlag.Bit.is80ConnectSuccess = 1;
	}

#if __MXCHIP__
	addr.sin_port = g_xlink_info.net_info.ServerPort;
#else
	addr.sin_port = htons(g_xlink_info.net_info.ServerPort);
#endif

	cRet = xlink_connect_Port(tcpfd, &addr);
	if (cRet != 0) {
#if  CLIENT_SSL_ENABLE
		CyaSSL_shutdown(tq_ssl);
		CyaSSL_free(tq_ssl);
#endif
		xlink_close(tcpfd);
#if __XDEBUG__
		XSDK_DEBUG_DEBUG("xlink_connect_Port cRet=%d", cRet);
#endif
		return -1;
	}

#if  CLIENT_SSL_ENABLE
	tq_method=CyaTLSv1_2_client_method();

	if (tq_method == NULL) {
#if __XDEBUG__	
		XSDK_DEBUG_DEBUG("unable to get method");
#endif
		xlink_close(tcpfd);
		return -1;
	}

#if __XDEBUG__ 
	XSDK_DEBUG_DEBUG(" ssl=====  CyaSSL_CTX_new");
#endif	

	tq_ctx = CyaSSL_CTX_new(tq_method);
	if (tq_ctx == NULL) {
#if __XDEBUG__ 		
		XSDK_DEBUG_DEBUG("unable to get ctx");
#endif
		xlink_close(tcpfd);
		return -1;
	}

	CyaSSL_CTX_set_verify(tq_ctx, SSL_VERIFY_NONE, 0); //disable verify certificates

#if __XDEBUG__ 	
	XSDK_DEBUG_DEBUG(" ssl=====  CyaSSL_new");
#endif

	tq_ssl = CyaSSL_new(tq_ctx);
	if (tq_ssl == NULL) {
#if __XDEBUG__		
		XSDK_DEBUG_DEBUG("unable to get SSL object");
#endif		
		CyaSSL_CTX_free(tq_ctx);
		xlink_close(tcpfd);
		return -1;
	}

	CyaSSL_set_fd(tq_ssl, tcpfd);

#if __XDEBUG__	
	XSDK_DEBUG_DEBUG(" ssl=====  xlink_ssl_connect fd=%d",tcpfd);
#endif	

	ret = CyaSSL_connect(tq_ssl);

#if __XDEBUG__	
	XSDK_DEBUG_DEBUG(" ssl=====  CyaSSL_connect start.");
#endif

	if(ret != SSL_SUCCESS) {
#if __XDEBUG__		
		XSDK_DEBUG_DEBUG("==============error = %d, %s==============\n",err,CyaSSL_ERR_error_string(err,buffer));
#endif				
		err = CyaSSL_get_error(tq_ssl,ret);

		CyaSSL_shutdown(tq_ssl);
		CyaSSL_free(tq_ssl);
		CyaSSL_CTX_free(tq_ctx);
		xlink_close(tcpfd);
		return -1;
	} else {
		XSDK_DEBUG_DEBUG(" =========xlink_ssl_connect SSL_SUCCESS================");
	}
#endif

	g_xlink_info.net_info.tcp_fd = tcpfd;

	if (g_xlink_user_config->OnStatus && cRet == 0) {
		g_xlink_user_config->OnStatus(XLINK_WIFI_STA_CONNECT_SERVER);
	}

#if __XDEBUG__

	XSDK_DEBUG_DEBUG("xlink connect server ok! port %d ", g_xlink_info.net_info.ServerPort);

#endif

	return tcpfd;
}

XLINK_FUNC void static tcp_http(unsigned char *RecvBuffer) {

#if CLIENT_SSL_ENABLE
	int recv_http = xlink_ssl_recv(tq_ssl, (char*) RecvBuffer, __XLINK_BUFFER_PIPE__ + 50);
#else
	int recv_http = xlink_recv(g_xlink_info.net_info.tcp_fd, (char*) RecvBuffer, __XLINK_BUFFER_PIPE__ + 50, 0);
#endif
	if (recv_http == 0) {
		g_xlink_info.net_info.tcpFlag.Bit.isLogined = 0;
		xsdk_closeTCP(1);
	} else if (recv_http > 0) {
		RecvBuffer[recv_http] = '\0';
		XlinkTcpInit(TCP_ACTIVATION);
		g_xlink_info.net_info.tcpFlag.Bit.is80ConnectSuccess = 1;
	}
}

XLINK_FUNC void event_tcp_recv_data(unsigned char *RecvBuffer) {

	int recv_num = 0;
	x_int32 bodylen;
	x_int16 ind = 0;
	unsigned char isTypeOk = 0;
	if (g_xlink_info.net_info.tcpFlag.Bit.is80Port == 1) {
		if (g_xlink_info.net_info.tcpFlag.Bit.is80ConnectSuccess == 0) {
			tcp_http(RecvBuffer);
			return;
		}
	}

#if CLIENT_SSL_ENABLE	
	recv_num = xlink_ssl_recv(tq_ssl, (char*) RecvBuffer, 5);
#else
	recv_num = xlink_recv(g_xlink_info.net_info.tcp_fd, (char*) RecvBuffer, 5, 0);
#endif
	if (recv_num == 0) {
#if CLIENT_SSL_ENABLE	
		CyaSSL_shutdown(tq_ssl);
		CyaSSL_free(tq_ssl);
		CyaSSL_CTX_free(tq_ctx);
#endif		
		xsdk_closeTCP(1);
#if __XDEBUG__
		XSDK_DEBUG_ERROR("tcp disconnectd recv head! error %d", recv_num);
#endif
	} else if (recv_num > 0) {
		bodylen = UTIL_INT32_SET(RecvBuffer[1], RecvBuffer[2], RecvBuffer[3], RecvBuffer[4]);
#if __XDEBUG__
		XSDK_DEBUG_DEBUG("recv tcp data body len %d", bodylen);
#endif
		//data is to long
		//		if (bodylen > __XLINK_BUFFER_PIPE__) {
		//#if __XDEBUG__
		//			XSDK_DEBUG_WARING("recv tcp data length %d > BufferLength %d ", bodylen, __XLINK_BUFFER_PIPE__);
		//#endif
		//			return;
		//		}
		isTypeOk = 0;
		isTypeOk = check_typeOk(RecvBuffer[0]);
		if (isTypeOk == 0) {
#if __XDEBUG__
			XSDK_DEBUG_ERROR("tcp recv data error not type");
#endif
#if CLIENT_SSL_ENABLE	
			CyaSSL_shutdown(tq_ssl);
			CyaSSL_free(tq_ssl);
			CyaSSL_CTX_free(tq_ctx);
#endif
			xsdk_closeTCP(1);
			return;
		}

		if (bodylen > (__XLINK_BUFFER_PIPE__ + 10)) {
#if __XDEBUG__
			XSDK_DEBUG_ERROR("tcp recv data error body large");
#endif
			xsdk_closeTCP(1);
			return;
		}

		if (bodylen >= 0) {

			while (bodylen > ind) {
#if CLIENT_SSL_ENABLE				
				x_uint16 num = xlink_ssl_recv(tq_ssl, (char*) (RecvBuffer + 5 + ind), bodylen - ind);
#else
				x_uint16 num = xlink_recv(g_xlink_info.net_info.tcp_fd, (char*) (RecvBuffer + 5 + ind), bodylen - ind, 0);
#endif	
				if (num > 0) {
					ind += num;
				} else {
#if __XDEBUG__
					XSDK_DEBUG_ERROR("tcp recv data error %d", num);
#endif
					if (num == 0) { //tcp closed
#if CLIENT_SSL_ENABLE	
							CyaSSL_shutdown(tq_ssl);
							CyaSSL_free(tq_ssl);
							CyaSSL_CTX_free(tq_ctx);
#endif
						xsdk_closeTCP(1);
					}
					break;
				}
			}

			if (ind == bodylen) {
				RecvBuffer[ind + 5] = '\0';
				xlink_process_tcp_data(RecvBuffer, ind + 5, ind);
			}
		}
	}

}

XLINK_FUNC void event_udp_recv_data(unsigned char *RecvBuffer) {

	int alen = sizeof(struct sockaddr_in);
	int recv_num = 0;
	static struct sockaddr_in addr;
#if __XDEBUG__	
	volatile union {
		unsigned int ipAddr;
		struct {
			unsigned char IP3 :8;
			unsigned char IP2 :8;
			unsigned char IP1 :8;
			unsigned char IP0 :8;
		} byte;
	} ip_byte;
#endif

#if HF_SDK
	static x_uint8 mac_addr[6];
	recv_num = hfnet_recvfrom(g_xlink_info.net_info.udp_fd, (char*) RecvBuffer, __XLINK_BUFFER_PIPE__ + 50, 0, (struct sockaddr*) &addr, (socklen_t*) &alen, (char*) mac_addr);

#elif __MXCHIP__
	static struct sockaddr_t maddr;
	alen = sizeof(maddr);
	recv_num = xlink_recvfrom(g_xlink_info.net_info.udp_fd, (char*) RecvBuffer, __XLINK_BUFFER_PIPE__ + 50, 0, (struct sockaddr_t *)&maddr, (socklen_t*) &alen);
	addr.sin_addr.s_addr = maddr.s_ip;
	addr.type = maddr.s_type;
	addr.sin_port = maddr.s_port;
	xlink_memcpy(addr.spares,maddr.s_spares,6);

#else

	recv_num = xlink_recvfrom(g_xlink_info.net_info.udp_fd, (char*) RecvBuffer,
	__XLINK_BUFFER_PIPE__ + 50, 0, (struct sockaddr*) &addr, (socklen_t*) &alen);

#endif
	if (recv_num > 0) {
		RecvBuffer[recv_num] = '\0';

#if __XDEBUG__
		ip_byte.ipAddr = addr.sin_addr.s_addr;
		XSDK_DEBUG_DEBUG("recv udp data  length %d IP=%d.%d.%d.%d port %d", recv_num, ip_byte.byte.IP0, ip_byte.byte.IP1, ip_byte.byte.IP2, ip_byte.byte.IP3, addr.sin_port);
#endif
		//XlinkProcessUdpData(RecvBuffer, recv_num, &addr);
//		printf("Recv Udp:");
//		int i = 0;
//		for (i = 0; i < recv_num; i++) {
//			printf("%02X ", RecvBuffer[i]);
//		}
//		printf("");

		XlinkProcessUdpDataV4(RecvBuffer, recv_num, &addr);

	} else {
#if __XDEBUG__
		XSDK_DEBUG_ERROR("udp udp data error %d", recv_num);

#endif
	}

}

XLINK_FUNC static x_uint8 _net_event(int maxfd, x_int32 timeout, xsdk_time_t c_time) {
	//Receive data buffer

	int ret = 0;

#if __MXCHIP__
	static struct timeval_t timeoutal;
#else
	static struct timeval timeoutal;
#endif

	timeoutal.tv_sec = timeout / 1000;
	timeoutal.tv_usec = (timeout % 1000) * 1000;

	ret = xlink_select(maxfd + 1, &m_fd_set, NULL, NULL, &timeoutal);

	if (ret <= 0) {
		return XLINK_ERROR_EVENT_TIMEOUT;
	}

	xlink_memset(g_RecvDataBuffer, 0, __XLINK_BUFFER_PIPE__ + 50);

	if (g_xlink_user_config->in_internet) {
		if (g_xlink_info.net_info.tcp_fd > 0) {
			if (FD_ISSET(g_xlink_info.net_info.tcp_fd, &m_fd_set)) {
				event_tcp_recv_data(g_RecvDataBuffer);
			}
		}
	}

	if (g_xlink_info.net_info.udp_fd > 0) {
		if (FD_ISSET(g_xlink_info.net_info.udp_fd, &m_fd_set)) {
			event_udp_recv_data(g_RecvDataBuffer);
		}
	}

	if (xlink_event != NULL) {
		if (xlink_event->fd > 0) {
			if (FD_ISSET(xlink_event->fd, &m_fd_set)) {
				if (xlink_event->recv_event) {
					xlink_event->recv_event(xlink_event->fd);
				}
			}
		}
	}

	return xlink_success;
}

XLINK_FUNC static int tcp_Connect_server_init(const xsdk_time_t c_time, const unsigned int pIpAddr) {

	xlink_tcp_connect_server(pIpAddr);
	if (g_xlink_info.net_info.tcp_fd > 0) {
		g_xlink_info.net_info.tcp_last_connect_server = c_time;
		g_xlink_info.net_info.tcpPingCount = 0;
		if (g_xlink_info.net_info.tcpFlag.Bit.is80Port == 1) {
			XlinkTcpInit(TCP_SEND_HTTPCONNECT);
		} else {
			XlinkTcpInit(TCP_ACTIVATION);
		}
		g_xlink_info.net_info.tcpFlag.Bit.isFirstConnectSuccess = 1;
		return 1;
	} else {
		g_xlink_info.net_info.tcp_last_connect_server = c_time;
		g_xlink_user_config->OnStatus(XLINK_WIFI_STA_CONNECT_SERVER_FAILED);
		return 0;
	}

	//	return 0;
}

XLINK_FUNC static void set_event_tcp(const xsdk_time_t c_time) {

	if (g_xlink_info.net_info.tcpFlag.Bit.isConnectWifi == 0) {
		if (g_xlink_info.net_info.tcp_fd > 0) {
			xsdk_closeTCP(1);
		}
	}

	if (g_xlink_info.net_info.tcp_fd > 0) {
		XlinkTcpLoop(c_time);
		if (g_maxfd < g_xlink_info.net_info.tcp_fd) {
			g_maxfd = g_xlink_info.net_info.tcp_fd;
		}
		FD_SET(g_xlink_info.net_info.tcp_fd, &m_fd_set);
		g_isWork = 1;
	}

}

XLINK_FUNC static void set_event_udp(xsdk_time_t c_time) {
	int ic = 0;

	if (g_xlink_info.net_info.udp_fd > 0) {
		g_maxfd = g_xlink_info.net_info.udp_fd;

		FD_SET(g_xlink_info.net_info.udp_fd, &m_fd_set);

		g_isWork = 1;
		ic++;

	} else if (g_udp_listen_port > 0) {
		xlink_udp_create(g_udp_listen_port);
		if (g_xlink_info.net_info.udp_fd > 0) {
			g_maxfd = g_xlink_info.net_info.udp_fd;
			FD_SET(g_xlink_info.net_info.udp_fd, &m_fd_set);
			g_isWork = 1;
			ic++;
		}
	}

}

#endif

XLINK_FUNC x_uint8 xlink_net_loop(xsdk_time_t c_time, x_int32 timeout) {

	x_uint32 time_temp = 0;
	if (c_time < last_check_heartbeat) {
		last_check_heartbeat = c_time;
		time_temp = 4;
	} else {
		time_temp = c_time - last_check_heartbeat;
	}
	if (time_temp > 3) {
		last_check_heartbeat = c_time;
		XlinkClientCheckHeartbeat(c_time);
	}

#if (!__LWIP__ESP_8266) && !__MT7681__ && !__STM32F107__ && !__STM32F103_UIP__ && !__ALL_DEVICE__

	FD_ZERO(&m_fd_set);
	g_maxfd = 0;
	g_isWork = 0;

	set_event_udp(c_time);

	if (g_xlink_user_config->in_internet) {
		set_event_tcp(c_time);
	}

	if (xlink_event != NULL) {
		if (xlink_event->fd > 0) {
			if (g_maxfd < xlink_event->fd) {
				g_maxfd = xlink_event->fd;
			}
			g_isWork++;
			FD_SET(xlink_event->fd, &m_fd_set);
		}
	}

	if (g_isWork == 0 || g_maxfd <= 0) {
		xlink_msleep(timeout);
		return XLINK_ERROR_NOT_WORK_TATUS;
	}
	//XlinkResend();
	_net_event(g_maxfd, timeout, c_time);

#else
	if(g_xlink_info.net_info.tcpFlag.Bit.isConnectWifi ==1 ) {
		//XlinkPorductTestCheck(c_time);
		XlinkTcpLoop(c_time);

	}
#endif

	return 0;
}
#if (!__LWIP__ESP_8266) && !__MT7681__ && !__STM32F107__ && !__STM32F103_UIP__ && !__ALL_DEVICE__

static unsigned int g_tcpConnectDelayTime = 5;

XLINK_FUNC int xlink_net_loop_Domain_name(void) {

	const char *hostIP = g_xlink_hostNameOrIp;

	int local_serverIp = 0, ret = 0;

	unsigned int temp = 0;

	char domainip[XLINK_DOMAIN_IP_MAX_LEN + 1];

	if (g_xlink_info.net_info.tcpFlag.Bit.isConnectWifi == 0) {
		return 0;
	}

	if (g_xlink_info.net_info.tcp_fd <= 0 && g_xlink_user_config->in_internet > 0) {

		xlink_memset(domainip, 0, XLINK_DOMAIN_IP_MAX_LEN+1);

		xsdk_read_config(XLINK_CONFIG_INDEX_DOMAIN_IP, domainip,
		XLINK_DOMAIN_IP_MAX_LEN);

		if (domainip[2] == 0) {

			hostIP = g_xlink_hostNameOrIp;
			Xlink_TCP_ConnectPort = XLINK_TCP_CONNECT_PORT;


			XSDK_DEBUG_DEBUG("TCP connect host use default port=%d ip=%s", Xlink_TCP_ConnectPort, hostIP);


		} else {

			hostIP = domainip + 2;
			Xlink_TCP_ConnectPort = domainip[0];
			Xlink_TCP_ConnectPort <<= 8;
			Xlink_TCP_ConnectPort += domainip[1];


			XSDK_DEBUG_DEBUG("TCP connect host use domain port=%d ip=%s", Xlink_TCP_ConnectPort, hostIP);



			if (domainip[2] != 0) {
				g_xlink_info.domain_connect_times++;
				if (g_xlink_info.domain_connect_times > 3) {
					xlink_memset(domainip, 0, 1+XLINK_DOMAIN_IP_MAX_LEN);
					xsdk_write_config(XLINK_CONFIG_INDEX_DOMAIN_IP, domainip, 2 + XLINK_DOMAIN_IP_MAX_LEN);
					xsdk_config_save();
				}
			}
		}

		if (g_xlink_info.g_XlinkSdkTime < g_xlink_info.net_info.tcp_last_connect_server) {
			g_xlink_info.net_info.tcp_last_connect_server = g_xlink_info.g_XlinkSdkTime;
		}
		temp = g_xlink_info.g_XlinkSdkTime - g_xlink_info.net_info.tcp_last_connect_server;

		if ((g_xlink_info.net_info.tcpFlag.Bit.isFirstConnectSuccess == 0) || temp > g_tcpConnectDelayTime) {

			if (is_valid_ip(hostIP) != 1) {
				local_serverIp = xlinkGetHostByName(hostIP);
			} else {

#if __MXCHIP__
				local_serverIp = inet_addr((char*) hostIP);
#else
				local_serverIp = IPtoAddr(hostIP);
#endif
			}

			if (local_serverIp != 0) {

				ret = tcp_Connect_server_init(g_xlink_info.g_XlinkSdkTime, local_serverIp);
				if (ret == 1) { //connect
					g_tcpConnectDelayTime = 0;
				} else {

					if (g_tcpConnectDelayTime == 0) {
						g_tcpConnectDelayTime = (g_xlink_info.g_XlinkSdkTime % 6);
					}

					g_tcpConnectDelayTime += 1;
					if (g_tcpConnectDelayTime > 10) {
						g_tcpConnectDelayTime = 10;
					}
				}
			} else {
				xlink_msleep(2);
			}
		}
	}
	//xlink_msleep(1000);
	return local_serverIp;
}
#endif

XLINK_FUNC void xsdk_closeUDP(void) {
	if (g_xlink_info.net_info.udp_fd > 0) {
		xlink_close(g_xlink_info.net_info.udp_fd);
		g_xlink_info.net_info.udp_fd = 0;
	}
}

XLINK_FUNC void xsdk_closeTCP(int flag) {

	if (g_xlink_info.net_info.tcp_fd > 0) {
#if __XDEBUG__
		XSDK_DEBUG_ERROR("xlink close tcp socket fd %d", g_xlink_info.net_info.tcp_fd);
#endif
		xlink_close(g_xlink_info.net_info.tcp_fd);
		g_xlink_info.net_info.tcp_fd = 0;

	}
	xlinkChangeWork(TCP_NOT_CONNECT);
	g_xlink_info.net_info.tcpFlag.Bit.isLogined = 0;
	if (g_xlink_user_config->OnStatus && flag) {
		g_xlink_user_config->OnStatus(XLINK_WIFI_STA_DISCONNCT_SERVER);
	}
#if CLIENT_SSL_ENABLE
	resetSSL();
#endif
}

XLINK_FUNC void XlinkCloseNet(void) {
	xsdk_closeTCP(1);
	xsdk_closeUDP();
}

XLINK_FUNC void XlinkCloseTcp(void) {
	xsdk_closeTCP(0);
}

XLINK_FUNC unsigned char check_typeOk(unsigned char type) {
	xlink_Message header;
	header.byte = type;
	switch (header.bits.type) {
	case TCP_TYPE_ACTIVATE:

	case TCP_TYPE_CONNECT:
	case TCP_TYPE_SET:
	case TCP_TYPE_SYNC:
	case TCP_TYPE_SETPWD:
	case TCP_TYPE_COLLECT:
	case TCP_TYPE_PIPE:
	case TCP_TYPE_PIPE_2:
	case TCP_TYPE_PROBE:
	case TCP_TYPE_SUBSCRIBE:
	case TCP_TYPE_PING:
	case TCP_TYPE_DISCONNECT:
	case TCP_TYPE_EVENT:
		return 1;
	default:
		break;
	}
	return 0;
}

XLINK_FUNC unsigned char check_typeOk_t(unsigned char type) {
	xlink_Message header;
	header.byte = type;
	switch (header.bits.type) {
	case TCP_TYPE_ACTIVATE:
	case TCP_TYPE_CONNECT:
	case TCP_TYPE_SET:
	case TCP_TYPE_PIPE:
	case TCP_TYPE_PROBE:
	case TCP_TYPE_PING:
	case TCP_TYPE_EVENT:
		return 1;
	default:
		break;
	}
	return 0;
}
