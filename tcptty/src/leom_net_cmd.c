/*
1	IDM->设备	复位请求	0x0001
2	IDM->设备	获取设备状态请求	0x0002
3	IDM->设备	占用请求	0x0003
4	IDM->设备	释放请求	0x0004
5	IDM->设备	开始请求	0x0005
6	IDM->设备	停止请求	0x0006
7	IDM->设备	设置工作参数请求	0x0007
8	IDM->设备	删除工作参数请求	0x0008
9	设备->IDM	心跳包	0x0100
10	设备->IDM	设备加入请求	0x0101
11	设备->IDM	设备状态报告	0x0102
12	设备->IDM	异常报告	0x0103
13	设备->IDM	执行结果报告	0x0104
14	设备->IDM	进度信息报告	0x0105
*/

enum _svr_cmd_list {
	RESV = 0,
	IDM_RESET,
	IDM_GETDEV,
	IDM_TAKEUPDEV,
	IDM_RELEASEDEV,
	IDM_DOSTART,
	IDM_DOSTOP,
	IDM_SETPARS,
	IDM_DELPARS,
	
	IDM_DEV_HEART = 0x0100,
	IDM_DEV_LOGIN,
	IDM_DEV_STATUS,
	IDM_DEV_ERROR,
	IDM_DEV_TASKRET,
	IDM_DEV_TASKPRO,
	
	IDM_CMD_END = 0xF000
};

#define PRO_HEAD	0xa5a5
#define PRO_VER		0x01
#define PRO_REQ		0x01
#define PRO_RSP		0x02
#define PRO_RESV	0x00000000

volatile uchar deviceid[32] = {0x00,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x31,0x32};
uchar taskid[32] = {0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30};
volatile uchar devicename[32] = {0x00,0x38,0x37,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x32,0x31,0x30};
				
volatile uchar devicemac[6] = {0x02,0x04,0x06,0x08,0x0a,0x0c};

#define ARRAY_NUM(ary)	(sizeof(ary)/sizeof(ary[0]))

int sock_send(int sock, unsigned char *send_buf, int send_len)
{
	int ret = 0;
	if((ret=send(sock,send_buf,send_len,0)) <= 0) {
		debug(LOG_ERR, "socket send error!%d:%s\n",errno,strerror(errno));
		return -1;
	}
	if(ret != send_len)
		debug(LOG_WARNING, "Warning! socket send %d < %d\n",ret,send_len);

	return ret;
}

static int append_common(uchar *payload, uchar *devid, uchar *taskid)
{
	uchar *ptr = payload;
	memcpy(ptr, devid, 32);
	ptr += 32;
	memcpy(ptr, taskid, 32);
	return 64;
}

int send_board_info(int socket, int index)
{
	char info[1024] = {0};
	time_t tt;
    time(&tt);

   	snprintf(info,sizeof(info)-1,"\n[%d] Board's Time: %s\n",index,ctime(&tt));
   	return sock_send(socket,info,strlen(info));
}

int send_heart_info(int socket,PRO_HD *phd, int index)
{
	if(!phd || socket <= 0)
		return -1;
	
	phd->header = PRO_HEAD;
	phd->len = 0x0;
	phd->ver = PRO_VER;
	phd->cmd = IDM_DEV_HEART;
	phd->stat = PRO_REQ;
	phd->seq = glb_seq++;
	phd->reserve = PRO_RESV;
	pro_net2host(phd);
	
	debug(LOG_DEBUG, "---> send [%u] heartbeat %d...\n", glb_seq, sizeof(PRO_HD));
   	return sock_send(socket, (unsigned char*)phd, sizeof(PRO_HD));
}

int send_login_info(int socket,PRO *pro, int index)
{
#define LOGIN_PAYLEN	(32+32+6+32)
	if(!pro || socket <= 0)
		return -1;
	
	int ret = 0;
	pro->hd.header = PRO_HEAD;
	pro->hd.len = LOGIN_PAYLEN;
	pro->hd.ver = PRO_VER;
	pro->hd.cmd = IDM_DEV_LOGIN;
	pro->hd.stat = PRO_REQ;
	pro->hd.seq = glb_seq++;
	pro->hd.reserve = PRO_RESV;
	
#if 0
	DEV_JOIN devjoin;
	memset(&devjoin, 0, sizeof(DEV_JOIN));
	strcpy(devjoin.com.dev_id, "#012345678901234567890123456789#");
	devjoin.mac[0] = 0x02;
	devjoin.mac[1] = 0x04;
	devjoin.mac[2] = 0x06;
	devjoin.mac[3] = 0x08;
	devjoin.mac[4] = 0x0A;
	devjoin.mac[5] = 0x0C;
	strcpy(devjoin.name, "?987654321098765432109876543210?");
	uchar *ptr = pro->payload;
	memcpy(ptr, devjoin.com.dev_id, sizeof(devjoin.com.dev_id));
	ptr += sizeof(devjoin.com.dev_id);
	memcpy(ptr, devjoin.com.task_id, sizeof(devjoin.com.task_id));
	ptr += sizeof(devjoin.com.task_id);
	memcpy(ptr, devjoin.mac, sizeof(devjoin.mac));
	ptr += sizeof(devjoin.mac);
	memcpy(ptr, devjoin.name, sizeof(devjoin.name));
	ptr += sizeof(devjoin.name);
#else
	uchar *ptr = pro->payload;
	ret = append_common(ptr,deviceid,taskid);
	ptr += ret;
	
	memcpy(ptr, devicemac, ARRAY_NUM(devicemac));
	ptr += ARRAY_NUM(devicemac);
	
	memcpy(ptr, devicename, ARRAY_NUM(devicename));
#endif
	/* malloc ONLINE_MAX_LEN for payload 
	 * But acture size maybe < ONLINE_MAX_LEN!
	*/
	uchar hexbuf[ONE_TCP_MAX_LEN] = {0};
	int hexlen = 0;
	pro_pro2hexbuf(pro, hexbuf, &hexlen);
	
	debug(LOG_DEBUG, "---> send login info %d...\n", hexlen);
   	return sock_send(socket, hexbuf, hexlen);
}

int send_status_info(int socket, PRO *pro, uchar status, int index)
{
#define STATUS_PAYLEN	(32+32+1+32)
	if(!pro || socket <= 0)
		return -1;
	
	pro->hd.header = PRO_HEAD;
	pro->hd.len = STATUS_PAYLEN;
	pro->hd.ver = PRO_VER;
	pro->hd.cmd = IDM_DEV_STATUS;
	pro->hd.stat = PRO_REQ;
	pro->hd.seq = glb_seq++;
	pro->hd.reserve = PRO_RESV;
	
	uchar *ptr = pro->payload;
	int ret = append_common(ptr,deviceid,taskid);
	ptr += ret;
	*ptr++ = status;
	// 32B reserve; def 0
	
	/* malloc ONLINE_MAX_LEN=1024 for payload 
	 * But acture size hd.len maybe < ONLINE_MAX_LEN!
	*/
	uchar hexbuf[ONE_TCP_MAX_LEN] = {0};
	int hexlen = 0;
	pro_pro2hexbuf(pro, hexbuf, &hexlen);
	
	debug(LOG_DEBUG, "---> send status info %d...\n", hexlen);
   	return sock_send(socket, hexbuf, hexlen);
}

int send_busy_info(int socket, PRO *pro, uchar status, int index)
{
#define BUSY_PAYLEN	(32+32+1+32)
	if(!pro || socket <= 0)
		return -1;
	
	pro->hd.header = PRO_HEAD;
	pro->hd.len = BUSY_PAYLEN;
	pro->hd.ver = PRO_VER;
	pro->hd.cmd = IDM_DEV_STATUS;  /* status info : busy*/
	pro->hd.stat = PRO_REQ;
	pro->hd.seq = glb_seq++;
	pro->hd.reserve = PRO_RESV;
	
	uchar *ptr = pro->payload; /* ONLINE_MAX_LEN */
	uchar nullid[32] = {0};
	int ret = append_common(ptr, deviceid, nullid);
	ptr += ret;
	*ptr++ = status;
	// 32B current taskid
	memcpy(ptr, taskid, sizeof(taskid));
	
	/* had malloc ONLINE_MAX_LEN=1024 for payload 
	 * But acture size hd.len maybe < ONLINE_MAX_LEN!
	*/
	uchar hexbuf[ONE_TCP_MAX_LEN] = {0};
	int hexlen = 0;
	pro_pro2hexbuf(pro, hexbuf, &hexlen);
	
	debug(LOG_DEBUG, "---> send busy info %d...\n", hexlen);
   	return sock_send(socket, hexbuf, hexlen);
}

int send_taskerr_info(int socket,PRO *pro, uchar *errinfo)
{
#define TASKERR_PAYLEN	(32+32+1+32)
	if(!pro || socket <= 0)
		return -1;
	
	pro->hd.header = PRO_HEAD;
	pro->hd.len = TASKERR_PAYLEN;
	pro->hd.ver = PRO_VER;
	pro->hd.cmd = IDM_DEV_ERROR;
	pro->hd.stat = PRO_REQ;
	pro->hd.seq = glb_seq++;
	pro->hd.reserve = PRO_RESV;
	
	uchar *ptr = pro->payload;
	int ret = append_common(ptr,deviceid,taskid);
	ptr += ret;
	*ptr++ = 0x0e; //err info
	memcpy(ptr, errinfo, 32); //err content
	
	/* malloc ONLINE_MAX_LEN=1024 for payload 
	 * But acture size hd.len maybe < ONLINE_MAX_LEN!
	*/
	uchar hexbuf[ONE_TCP_MAX_LEN] = {0};
	int hexlen = 0;
	pro_pro2hexbuf(pro, hexbuf, &hexlen);
	
	debug(LOG_DEBUG, "---> send task err info %d...\n", hexlen);
   	return sock_send(socket, hexbuf, hexlen);
}

int response_status_info(int socket, PRO *pro, uchar status, int index)
{
#define RSP_DEV_PAYLEN	(32+32+1+32)
	if(!pro || socket <= 0)
		return -1;
	
//	pro->hd.header = PRO_HEAD;  //keep original
//	pro->hd.len = RSP_DEV_PAYLEN;
//	pro->hd.ver = PRO_VER;
//	pro->hd.cmd = IDM_GETDEV; 
	pro->hd.stat = PRO_RSP;  // set response
//	pro->hd.seq = glb_seq++; 
//	pro->hd.reserve = PRO_RESV;
	
	uchar *ptr = pro->payload;
	int ret = append_common(ptr,deviceid,taskid);
	ptr += ret;
	*ptr++ = status; //online
	// 32B reserve; def 0
	
	/* malloc ONLINE_MAX_LEN=1024 for payload 
	 * But acture size hd.len maybe < ONLINE_MAX_LEN!
	*/
	uchar hexbuf[ONE_TCP_MAX_LEN] = {0};
	int hexlen = 0;
	pro_pro2hexbuf(pro, hexbuf, &hexlen);
	
	debug(LOG_DEBUG, "---> send response GET_DEV info %d...\n", hexlen);
   	return sock_send(socket, hexbuf, hexlen);
}


int send_taskret_info(int socket,PRO *pro, unsigned char *devret)
{
#define TASKRET_PAYLEN	(32+32+4)
	if(!pro || socket <= 0)
		return -1;
	
	pro->hd.header = PRO_HEAD;
	pro->hd.len = TASKRET_PAYLEN;
	pro->hd.ver = PRO_VER;
	pro->hd.cmd = IDM_DEV_TASKRET;
	pro->hd.stat = PRO_REQ;
	pro->hd.seq = glb_seq++;
	pro->hd.reserve = PRO_RESV;
	
	uchar *ptr = pro->payload;
	int ret = append_common(ptr,deviceid,taskid);
	ptr += ret;
	
	*ptr++ = devret[0];
	*ptr++ = devret[1];
	*ptr++ = devret[2];
	*ptr++ = devret[3];
	
	/* malloc ONLINE_MAX_LEN=1024 for payload 
	 * But acture size hd.len maybe < ONLINE_MAX_LEN!
	*/
	uchar hexbuf[ONE_TCP_MAX_LEN] = {0};
	int hexlen = 0;
	pro_pro2hexbuf(pro, hexbuf, &hexlen);
	
	debug(LOG_DEBUG, "---> send task result info %d...\n", hexlen);
   	return sock_send(socket, hexbuf, hexlen);
}

int send_taskperc_info(int socket,PRO *pro, unsigned char *devret)
{
#define PERC_PAYLEN	(32+32+2+2)
	if(!pro || socket <= 0)
		return -1;
	
	pro->hd.header = PRO_HEAD;
	pro->hd.len = PERC_PAYLEN;
	pro->hd.ver = PRO_VER;
	pro->hd.cmd = IDM_DEV_TASKPRO;
	pro->hd.stat = PRO_REQ;
	pro->hd.seq = glb_seq++;
	pro->hd.reserve = PRO_RESV;
	
	uchar *ptr = pro->payload;
	int ret = append_common(ptr,deviceid,taskid);
	ptr += ret;
	
	*ptr++ = devret[0];
	*ptr++ = devret[1];
	*ptr++ = devret[2];
	*ptr++ = devret[3];

	/* malloc ONLINE_MAX_LEN=1024 for payload 
	 * But acture size hd.len maybe < ONLINE_MAX_LEN!
	*/
	uchar hexbuf[ONE_TCP_MAX_LEN] = {0};
	int hexlen = 0;
	pro_pro2hexbuf(pro, hexbuf, &hexlen);
	
	debug(LOG_DEBUG, "---> send task percentage info %d...\n", hexlen);
   	return sock_send(socket, hexbuf, hexlen);
}


