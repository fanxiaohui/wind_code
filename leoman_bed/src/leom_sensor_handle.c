#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "leom_sensor_handle.h"
#include "leom_utils.h"
#include "leom_dbg.h"
#include "leom_uart.h"
#include "leoman.h"

#define MIN_FILE_PATH_LEN	(8)
#define FILE_PREFIX			"/tmp/leoman"
#define SEPRA				','
#define BEACON_JSON_HEAD	"{\"config\":{\"timezone\":8},\"input_data\":["
#define BEACON_JSON_TAIL	"]}"
#define CP_DATA_SHELL		"/sbin/leom_cp_data.sh"

extern st_glb_cfg glb_cfg;
static char file_path[128] = {0};
static int aver_ret[MAX_INX+1] = {0};

/* return length had be written to file */
static int _write_file(char *path, char *buf, int len, char *mode)
{
	if(path == NULL || buf == NULL || len <= 0 || mode == NULL)
		return 0;

	FILE *fp = fopen(path,mode);
	if(!fp) {
		debug(LOG_ERR, "===== can NOT create file:%s\n",path);
		return 0;
	}
	int ret = fwrite(buf,1,len,fp);
	if(ret != len)
		debug(LOG_WARNING, "WARN! write file %s! actual size %d<%d\n",path,ret,len);
	fflush(fp);
	fclose(fp);

	return ret;
}

static void _generate_new_file(char *name, int len)
{
	time_t timep;  
    struct tm *p_tm;
	static int index = 0;
	char tmp_path[128] = {0};
	char tmp[8] = {0};
	
	if(!name || len <= 0)
		return;
	
    timep = time(NULL);  
    p_tm = localtime(&timep);
    //p_tm->tm_wday, p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
    snprintf(name,len, "%s/%04d-%d-%d", FILE_PREFIX, (p_tm->tm_year+1900), (p_tm->tm_mon+1), p_tm->tm_mday);
    debug(LOG_NOTICE,"=====> Generate New File:%s\n",name);
	
	strcpy(tmp_path,name);
CHK_AGAIN:
	if(access(tmp_path,F_OK) == 0) {
		debug(LOG_NOTICE,"====== File %s is existed! To++\n",tmp_path);
		index++;
		memset(tmp,0,sizeof(tmp));
		snprintf(tmp,sizeof(tmp),"_%d",index);
		memset(tmp_path,0,sizeof(tmp_path));
		sprintf(tmp_path,"%s%s",name,tmp);
		goto CHK_AGAIN;
	}
	strcpy(name,tmp_path);
}

static int _write_beaconjson_head(char *file)
{
	//char header[] = "{\"config\": {\"timezone\": 8}, \"input_data\": [";
	return _write_file(file,BEACON_JSON_HEAD,strlen(BEACON_JSON_HEAD),"a+");
}

static int _write_beaconjson_tail(char *file)
{
	//char tail[] = "]}";
	return _write_file(file,BEACON_JSON_TAIL,strlen(BEACON_JSON_TAIL),"a+");
}

static int copy_file2sdcard(char *fpth, int report)
{
	char *p = fpth;
	if(fpth == NULL)
		return -1;
	/* the path of file should not has spacing */
	while(*p != '\0') {
		if(*p == ' ')
			return -1;
		p++;
	}
	
	char cmd[256] = {0};
	snprintf(cmd,sizeof(cmd)-1,"%s %d %s",CP_DATA_SHELL,report,fpth);
	return system(cmd);
}

static int _append2_tmpfile(char *content, int len)
{
	if(content == NULL || len <= 0 )
		return -1;
	
	if(strlen(file_path) <= MIN_FILE_PATH_LEN) {
		memset(file_path,0,sizeof(file_path));
		_generate_new_file(file_path,sizeof(file_path)-1);
		_write_beaconjson_head(file_path);
	}

	if(glb_cfg.cp2sd_flag) {
		debug(LOG_INFO, "Only Copy tmp file [%s] to sd card!\n",file_path);
		glb_cfg.cp2sd_flag = 0;
		copy_file2sdcard(file_path,0);
	}
	
	if(glb_cfg.gen_new_flag) {
		glb_cfg.gen_new_flag = 0;
		//content[len-1] = '';
		_write_file(file_path,content,len-1,"a+"); //skip the last ','
		_write_beaconjson_tail(file_path);
		copy_file2sdcard(file_path,1);
		debug(LOG_NOTICE, "==>Move tmp file [%s] to sd card, then delete it!\n",file_path);
		//unlink(file_path);  //do in shell 
		
		memset(file_path,0,sizeof(file_path));
		_generate_new_file(file_path,sizeof(file_path)-1);
		_write_beaconjson_head(file_path);
	}
	_write_file(file_path,content,len,"a+");

	return 0;
}

void leom_pre_check_tail_for_report()
{
	char obj_str[512] = {0};
	if(strlen(file_path) <= MIN_FILE_PATH_LEN) {
		//debug(LOG_INFO,"====> Had Never Sensor data happend");
		return;
	}
	
	if(glb_cfg.gen_new_flag) {
		glb_cfg.gen_new_flag = 0;
		snprintf(obj_str,sizeof(obj_str)-1,"{\"heart_rate\":%d,\"breath_rate\":%d,\
\"gatem\":%d,\"gateo\":%d,\"is_on_bed\":%d,\"is_body_move\":%d,\
\"timestamp\":%ld,\"celsius\":0,\"user_id\":0,\"device_id\":\"0\",\"time_diff\":0}",
aver_ret[HEART_I],aver_ret[BREATH_I],aver_ret[GATEM],aver_ret[GATEO_H],aver_ret[ON_BED],aver_ret[MOVING],time(NULL));  // no tail ,
		
		_write_file(file_path,obj_str,strlen(obj_str),"a+");
		_write_beaconjson_tail(file_path);
		
		copy_file2sdcard(file_path,1);
		debug(LOG_NOTICE, "==>TT Move tmp file [%s] to sd card, then delete it!\n",file_path);
		//unlink(file_path);  //do in shell 
		memset(file_path,0,sizeof(file_path));
		_generate_new_file(file_path,sizeof(file_path)-1);
		_write_beaconjson_head(file_path);
	}
}

int leom_handle_sensor_data(unsigned char data[][MAX_INX+1])
{
	static int last_on_bed = 1; /* initialized '1' to ensure the first off_bed can be recored */
	int i = 0, j = 0;
	int tmp_f = 0;
	int move_num = MAX_NUM, on_num = MAX_NUM;
	int gto  = 0, gtm = 0;
//	int aver_ret[MAX_INX+1] = {0};  -- to global
	char obj_str[512] = {0};

	if(glb_cfg.start_sensor_flag != 1) {
		debug(LOG_NOTICE, "Please set system time firstly!\n");
		return 0;
	}
#if 0
	for(i=0;i<=MAX_INX;i++)
		debug(LOG_DEBUG, "char[0][%d]=%d,0x%x",i,data[0][i],data[0][i]);
#endif

	/* find 1 or 0 */
	for(j = 0; j < MAX_NUM; j++) {
		if(data[j][MOVING] != 0)
			move_num++;
		else
			move_num--;

		if(data[j][ON_BED] != 0)
			on_num++;
		else
			on_num--;
	}

	if(on_num >= MAX_NUM) {
		aver_ret[ON_BED] = 1;
	} else {
		aver_ret[ON_BED] = 0;
		if(last_on_bed == 0) {
			debug(LOG_INFO,"==== Not in Bed ====\n");
			return 0;		/* when not in bed only record once */
		}
	}

	last_on_bed = aver_ret[ON_BED];

	if(move_num >= MAX_NUM)
		aver_ret[MOVING] = 1;
	else 
		aver_ret[MOVING] = 0;

	/* get average */
	tmp_f = 0;
	for(j = 0; j < MAX_NUM; j++)
		tmp_f += data[j][HEART_I];
	
	aver_ret[HEART_I] = tmp_f / MAX_NUM;
	
	tmp_f = 0;
	for(j = 0; j < MAX_NUM; j++)
		tmp_f += data[j][BREATH_I];
	
	aver_ret[BREATH_I] = tmp_f / MAX_NUM;

	/* find MAX */
	gto = data[0][GATEO_H] * 256 + data[0][GATEO_L];
	gtm = data[0][GATEM];
	j = 1;
	for(j = 1; j < MAX_NUM; j++) {
		if((data[j][GATEO_H]*256+data[j][GATEO_L]) > gto)
			gto = data[j][GATEO_H]*256+data[j][GATEO_L];

		if(data[j][GATEM] > gtm)
			gtm = data[j][GATEM];
	}
	aver_ret[GATEO_H] = gto;
	aver_ret[GATEM] = gtm;

	snprintf(obj_str,sizeof(obj_str)-1,"{\"heart_rate\":%d,\"breath_rate\":%d,\
\"gatem\":%d,\"gateo\":%d,\"is_on_bed\":%d,\"is_body_move\":%d,\
\"timestamp\":%ld,\"celsius\":0,\"user_id\":0,\"device_id\":\"0\",\"time_diff\":0}%c",
aver_ret[HEART_I],aver_ret[BREATH_I],aver_ret[GATEM],aver_ret[GATEO_H],aver_ret[ON_BED],aver_ret[MOVING],time(NULL),SEPRA);
	
	debug(LOG_DEBUG,"handle sensor data:%s\n",obj_str);

	_append2_tmpfile(obj_str, strlen(obj_str));
	
	return 0;
}

static int _is_crc_valid(unsigned char *cmdbuf,int length)
{
	unsigned int crc = 0;
	unsigned int buf_crc = 0;
	if(length < 2)
		return 1;
	buf_crc += cmdbuf[length-1];
	buf_crc += cmdbuf[length-2] << 8;
	crc = calc_crc(cmdbuf,length);
	if(buf_crc != crc) {
		debug(LOG_WARNING, "=!=!=!= bufcrc=%x,calc-crc=%x!",buf_crc,crc);
		return 1;
	}
	else
		return 0;
}

void lua_call_func(const char *filename, const char *func,
				   const char *param,int len,char *ret)
{
#define MAX_LUA_RET_LEN	(256)

	if(!filename || !func || !param || len < 0 || !ret)
		return;
	
	lua_State *L = lua_open();
	luaL_openlibs(L);
	if(luaL_loadfile(L,filename)|| lua_pcall(L,0,0,0)) {
		debug(LOG_ERR,"luaL_loadfile err:%s\n",lua_tostring(L,-1));
		goto EXIT;
		//lua_error(L);
	}
	
	lua_getglobal(L,func);
	lua_pushlstring(L,param,len);
	
	if(lua_pcall(L,1,1,0)){
		debug(LOG_ERR,"lua_pcall func '%s' err:%s\n",func,lua_tostring(L,-1));
		goto EXIT;
	}
	
	if(!lua_isstring(L,-1)) {
		debug(LOG_ERR,"lua return type error:%s\n",lua_tostring(L,-1));
		goto EXIT;
	}
	
	strncpy(ret,lua_tostring(L,-1),MAX_LUA_RET_LEN-1);
	lua_pop(L,1);
// 	debug(LOG_DEBUG,"lua_call_func-ret[%ld]:%s\n",strlen(ret),ret);
EXIT:
 	lua_close(L);
#undef MAX_LUA_RET_LEN
}

static int is_valid_cmd(unsigned char *cmdbuf, int length)
{
#define CMD_HEAD	0x55
	if(!cmdbuf || length < 1)
		return 1;
	if(cmdbuf[0] != CMD_HEAD)
		return 1;
	//check length
	if(cmdbuf[1] <= 0 || cmdbuf[1] != length)
		return 1;
	
	if(_is_crc_valid(cmdbuf,length) != 0)
		debug(LOG_WARNING, "ui cmd buf crc error!");
	
	return 0;
#undef CMD_HEAD
}

int leom_handle_ui_cmd(unsigned char *cmdbuf, int length)
{
	time_t timep;  
	struct tm *p_tm;
	char retbuf[512] = {0};
	unsigned char resphex[16] = {0};
	int i = 0;
	int crc,fd;
	unsigned char cmd = 0x00;
	if(is_valid_cmd(cmdbuf,length) != 0) {
		debug(LOG_WARNING,"=!=!=!= UI cmd is invalid!");
		return 1;
	}
#if 1
	for(i=0;i<length;i++)
		debug(LOG_DEBUG, "char[%d]=0x%x",i,cmdbuf[i]);
#endif
	// check cmd
	cmd = cmdbuf[2];
	if(cmd == 0x01) {
		// runtime data
		snprintf(retbuf,sizeof(retbuf)-1,"{\"heart_rate\":%d,\"breath_rate\":%d,\
\"gatem\":%d,\"gateo\":%d,\"is_on_bed\":%d,\"is_body_move\":%d,\
\"timestamp\":%ld,\"celsius\":0,\"user_id\":0,\"device_id\":\"0\",\"time_diff\":0}",
aver_ret[HEART_I],aver_ret[BREATH_I],aver_ret[GATEM],aver_ret[GATEO_H],aver_ret[ON_BED],aver_ret[MOVING],time(NULL));  // no tail ,
		
		debug(LOG_DEBUG,"Current RAW:%s",retbuf);
		//上行数据：0X55 0X09  0XA1    Heart  respiratoryrate    Movement    OnBed  CRCH CRCL  
		resphex[0] = 0x55;
		resphex[1] = 0x09;
		resphex[2] = 0xa1;
		resphex[3] = aver_ret[HEART_I];
		resphex[4] = aver_ret[BREATH_I];
		resphex[5] = aver_ret[MOVING];
		resphex[6] = aver_ret[ON_BED];
		crc = calc_crc(resphex,7);
		resphex[7] = (crc & 0xff00) >> 8;
		resphex[8] = crc & 0x00ff;
		fd = open("/dev/ttyS2",O_RDWR);
		if(fd) {
			write(fd,resphex,9);
			close(fd);
		}
	} else {
		// 02 - 07
		memset(retbuf,0,sizeof(retbuf));
		lua_call_func("/sbin/parse_report.lua","parse_ui_cmd",cmdbuf,length,retbuf);
		debug(LOG_DEBUG,"lua_call_func-ret[%ld]:%s\n",strlen(retbuf),retbuf);
		if(cmd == 0x07 && strstr(retbuf,"OK:SetTime")) {
			glb_cfg.start_sensor_flag = 1;
			timep = time(NULL);
			p_tm = localtime(&timep);
			debug(LOG_NOTICE,"Set System Time:%04d-%d-%d %d-%d-%d\n",(p_tm->tm_year+1900), (p_tm->tm_mon+1), p_tm->tm_mday,
				  p_tm->tm_hour,p_tm->tm_min,p_tm->tm_sec);
	
			system("hwclock --systohc --noadjfile --localtime");
			system("/etc/init.d/cron restart");
		}
	}
	
	return 0;
}
