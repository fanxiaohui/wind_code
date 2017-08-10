#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include "net_base.h"
#include "net_json.h"
#include "config.h"
#include "debug.h"

#define SUC_RET 		"success"
#define FAIL_RET		"failure"
#define OTA_MASK    6
#define STRCPY(key) strncpy(key,sub_json->valuestring,sizeof(key)-1)

#define ADD_KEY(obj,key) cJSON_AddItemToObject(obj, msg_key[key].name, \
							cJSON_CreateString(nvram_get(msg_key[key].name)))

#if 1
MSG_KEY msg_key[MAG_VAL_MAX+1] = {
	[MSG_TM_UP] = { .name = "time_updated", .inx = MSG_TM_UP},
	[MSG_REPORT_TIME] = {"report_date",MSG_REPORT_TIME},
	[MSG_SCORE] = {"score",MSG_SCORE},
	[MSG_DAY_SLEEP_HOUR] = {"sleep_time_hour",MSG_DAY_SLEEP_HOUR},
	[MSG_DAY_SLEEP_MINU] = {"sleep_time_minute",MSG_DAY_SLEEP_MINU},
	[MSG_DAY_DEEP] = {"deep",MSG_DAY_DEEP},
	[MSG_DAY_LIGHT] = {"light",MSG_DAY_LIGHT},
	[MSG_DAY_REM] = {"rem",MSG_DAY_REM},
	[MSG_DAY_AWAKE] = {"awake",MSG_DAY_AWAKE},
	[MSG_DAY_BREATH_STOP] = {"breath_stop",MSG_DAY_BREATH_STOP},
	[MSG_PERCENTAGE] = {"percentage",MSG_PERCENTAGE},
	[MSG_VALUE] = {"value",MSG_VALUE},
	[MSG_NUM] = {"num",MSG_NUM},
	[MSG_DURATION] = {"duration",MSG_DURATION},
	[MSG_SUB_REPORT_ARRAY] = {"sub_report",MSG_SUB_REPORT_ARRAY},
	[MSG_SRPT_SLEEP_START] = {"sleep_start",MSG_SRPT_SLEEP_START},
	[MSG_SRPT_SLEEP_END] = {"sleep_end",MSG_SRPT_SLEEP_END},
	[MSG_SRPT_SLEEP_HOUR] = {"sleep_time_hour",MSG_SRPT_SLEEP_HOUR},
	[MSG_SRPT_SLEEP_MINU] = {"sleep_time_minute",MSG_SRPT_SLEEP_MINU},
	[MSG_SRPT_SLEEP_DATA_ARRAY] = {"sleep_data",MSG_SRPT_SLEEP_DATA_ARRAY},
	[MSG_SRPT_SLEEP_DATA2_ARRAY] = {"sleep_data_2",MSG_SRPT_SLEEP_DATA2_ARRAY},
	[MSG_SRPT_SLEEP_DATA_START] = {"start",MSG_SRPT_SLEEP_DATA_START},
	[MSG_SRPT_SLEEP_DATA_END] = {"end",MSG_SRPT_SLEEP_DATA_END},
	[MSG_SRPT_SLEEP_DATA_STATUS] = {"status",MSG_SRPT_SLEEP_DATA_STATUS},
	
	
	[MAG_VAL_MAX] = {"null",MAG_VAL_MAX}
};
#else
char msg_val[MAG_VAL_MAX][32] = {
	[MSG_MAGIC] = "magic",
	[MSG_TYPE] = "msgType3-char",
};
#endif

void get_cur_time(char *cur_time)
{
	time_t timer = time(NULL);
	struct tm *timeinfo = localtime(&timer);
	if(timeinfo) 
		sprintf(cur_time,"%4d-%02d-%02d %02d:%02d:%02d",1900+timeinfo->tm_year, 
                   1+timeinfo->tm_mon,timeinfo->tm_mday,timeinfo->tm_hour,
                    timeinfo->tm_min,timeinfo->tm_sec);
	else
		cur_time = NULL;
}

int parse_report_file(const char *text)
{
	int ret = -1;
	int array_size = 0, i = 0;
	cJSON *body = NULL,*sub = NULL;
	cJSON *root = cJSON_Parse(text);
	if (!root) {
		ERROR("Error before: [%s]\n",cJSON_GetErrorPtr());
		return -1;
	} else {
		if((body=GET_KEY(root,MSG_REPORT_TIME))) {
			debug(LOG_DEBUG,"KEY %s 's value is %s\n",msg_key[MSG_REPORT_TIME].name,body->valuestring);
		}
		if((body=GET_KEY(root,MSG_SUB_REPORT_ARRAY))) {
			if(body->type != cJSON_Array) {
				debug(LOG_WARNING,"KEY %s 's type is incorrect!",msg_key[MSG_SUB_REPORT_ARRAY].name)
				return -1;
			}
			
		}
		
		if((body_json=GET_KEY(json_fmt,MSG_BODY))) {
			if((sub_json=GET_KEY(body_json,MSG_TTY_BAUD))) {
				DEBUG("get tty baud=%s\n",sub_json->valuestring);
				uart_attr->baud = atoi(sub_json->valuestring);
			} else {
				goto EXIT;
			}
			if((sub_json=GET_KEY(body_json,MSG_TTY_DBT))) {
				DEBUG("get tty data bit=%s\n",sub_json->valuestring);
				uart_attr->databit = atoi(sub_json->valuestring);
			} else {
				goto EXIT;
			}
			
}}}

/* Need to release the returned pointer */
char* create_json_msg_login(const char *info_file,char *seqnum)
{

	cJSON *root = NULL;	/* declare a few. */
	char *out = NULL;
	root = create_root_json_obj();

	//type
	cJSON_AddItemToObject(root, msg_key[MSG_TYPE].name, cJSON_CreateString("login"));
	cJSON_AddItemToObject(root, msg_key[MSG_SEQNUM].name, cJSON_CreateString(seqnum));
	//body
	cJSON *obj_val = create_root_json_obj();
	ADD_KEY(obj_val,MSG_DEV_ID);
	ADD_KEY(obj_val,MSG_MAC);
	ADD_KEY(obj_val,MSG_IMEI);
	ADD_KEY(obj_val,MSG_IMSI);
	ADD_KEY(obj_val,MSG_VERSION);
	//ADD_KEY(obj_val,MSG_CUR_TIME);
	char cur_time[64] = "";
	get_cur_time(cur_time);
	cJSON_AddItemToObject(obj_val, msg_key[MSG_CUR_TIME].name, cJSON_CreateString(cur_time));
	cJSON_AddItemToObject(obj_val, msg_key[MSG_PROTO_VER].name, cJSON_CreateString("V2.7"));
	
	cJSON_AddItemToObject(root, msg_key[MSG_BODY].name, obj_val);

	//switch to string
	out = minimun_json(root);
	cJSON_Delete(root);

	assert(out);
	return out;
}

/* Need to release the returned pointer */
char* create_json_board_info(const char *board_info,char *seqnum)
{
	cJSON *root = create_root_json_obj();
	//type
	cJSON_AddItemToObject(root, msg_key[MSG_TYPE].name, cJSON_CreateString("boardinfo"));
	cJSON_AddItemToObject(root, msg_key[MSG_SEQNUM].name, cJSON_CreateString(seqnum));
	
	//body
	nvram_renew("/tmp/gps_info");
	cJSON *gps_obj = create_root_json_obj();
	ADD_KEY(gps_obj,MSG_GPS_LATI);
	ADD_KEY(gps_obj,MSG_GPS_LONGI);
	ADD_KEY(gps_obj,MSG_GPS_DIR);
	ADD_KEY(gps_obj,MSG_GPS_SPEED);
	ADD_KEY(gps_obj,MSG_GPS_NS);
	ADD_KEY(gps_obj,MSG_GPS_HEIGHT);
	
	//agpsinfo
//	nvram_renew("/tmp/agps_info");
//	cJSON *agps_obj = create_root_json_obj();
//	ADD_KEY(agps_obj,MSG_GPS_LATI);
//	ADD_KEY(agps_obj,MSG_GPS_LONGI);
//	ADD_KEY(agps_obj,MSG_GPS_DIR);
//	ADD_KEY(agps_obj,MSG_GPS_SPEED);
	
	cJSON *net_obj = create_root_json_obj();
	ADD_KEY(net_obj,MSG_NET_SIG);
	ADD_KEY(net_obj,MSG_NET_TYPE);
	
//	cJSON *gpio_obj = create_root_json_obj();
//	ADD_KEY(gpio_obj,MSG_GPIO_NUM);
//	ADD_KEY(gpio_obj,MSG_GPIO_NAME);
//	ADD_KEY(gpio_obj,MSG_GPIO_DIR);
//	ADD_KEY(gpio_obj,MSG_GPIO_VAL);

//	cJSON *gpio_array = cJSON_CreateArray();
//	cJSON_AddItemToArray(gpio_array,gpio_obj);
// 	cJSON_AddItemToArray(gpio_array,gpio_obj2);
// 	cJSON_AddItemToArray(gpio_array,gpio_obj3);
	
	cJSON *body_obj = create_root_json_obj();
	ADD_KEY(body_obj,MSG_DEV_ID);
	char cur_time[64] = "";
	get_cur_time(cur_time);
	cJSON_AddItemToObject(body_obj, msg_key[MSG_CUR_TIME].name, cJSON_CreateString(cur_time));
	cJSON_AddItemToObject(body_obj, msg_key[MSG_GPS].name, gps_obj);
//	cJSON_AddItemToObject(body_obj, msg_key[MSG_AGPS].name, agps_obj);
	cJSON_AddItemToObject(body_obj, msg_key[MSG_NET].name, net_obj);
//	cJSON_AddItemToObject(body_obj, msg_key[MSG_GPIO].name, gpio_array);
	
	cJSON_AddItemToObject(root, msg_key[MSG_BODY].name, body_obj);
	
	char *out = minimun_json(root);	
	cJSON_Delete(root);

	assert(out);
	return out;
}

static void replace_specfic_ascii(char *msg)
{
	char *p = msg;
	while(*p != '\0') {
		if(*p == '"')
			*p = '\'';
		p++;
	}
}

/* Need to release the returned pointer */
char* create_json_msg_rsp(const char *code,const char *msg, const char *seqnum,const char *type)
{
	cJSON *root = create_root_json_obj();
	//type
	cJSON_AddItemToObject(root, msg_key[MSG_RSP_CODE].name, cJSON_CreateString(code));
	cJSON_AddItemToObject(root, msg_key[MSG_SEQNUM].name, cJSON_CreateString(seqnum));
	replace_specfic_ascii(msg);
	cJSON_AddItemToObject(root, msg_key[MSG_RSP_MSG].name, cJSON_CreateString(msg));
	cJSON_AddItemToObject(root, msg_key[MSG_TYPE].name, cJSON_CreateString(type));

	char *out = minimun_json(root);	
	cJSON_Delete(root);

	assert(out);
	return out;
}

int parse_json_ret(const char *text,char *retmsg)
{
	int ret = -1;
	cJSON *sub_json = NULL;
	cJSON *json_fmt = cJSON_Parse(text);
	if (!json_fmt) {
		ERROR("Error before: [%s]\n",cJSON_GetErrorPtr());
		return -1;
	} else {
		if((sub_json=GET_KEY(json_fmt,MSG_RSP_CODE))) {
			if(strcmp(sub_json->valuestring,SUC_RET)) {
				ERROR("[RSP] Failed!:[%s]\n",sub_json->valuestring);
				sub_json=GET_KEY(json_fmt,MSG_RSP_MSG);
				if(sub_json)
					ERROR("[RSP] Failed MSG:[%s]\n",sub_json->valuestring);
				goto EXIT;
			} else {
				if((sub_json=GET_KEY(json_fmt,MSG_SEQNUM))) {
					ret = atoi(sub_json->valuestring);
					DEBUG("[RSP] OK seqnum=[%d]\n",ret);
				}
				if((sub_json=GET_KEY(json_fmt,MSG_RSP_MSG)) && retmsg) {
					strncpy(retmsg,sub_json->valuestring,127);
				}	
				ret = 0;
			}
		}
	}
	cJSON_Delete(json_fmt);
	json_fmt = NULL;
	return ret;
EXIT:
	if(json_fmt) cJSON_Delete(json_fmt);
	return -1;
}

int parse_json_seqnum(const char *text)
{
	int ret = -1;
	cJSON *sub_json = NULL;
	cJSON *json_fmt = cJSON_Parse(text);
	if (!json_fmt) {
		ERROR("Error before: [%s]\n",cJSON_GetErrorPtr());
		return -1;
	} else {
		if(NULL == GET_KEY(json_fmt,MSG_TYPE)) {
			goto EXIT;
		}
		if(NULL == GET_KEY(json_fmt,MSG_BODY)) {
			goto EXIT;
		}
		if((sub_json=GET_KEY(json_fmt,MSG_SEQNUM))) {
			ret = atoi(sub_json->valuestring);
			DEBUG("[Get] seqnum=%d\n",ret);
		} else {
			goto EXIT;
		}
	}
	cJSON_Delete(json_fmt);
	json_fmt = NULL;
	return ret;
EXIT:
	if(json_fmt) cJSON_Delete(json_fmt);
	return -1;
}

int parse_json_svrtime(const char *text,int *seq,char *svrtime)
{
	int ret = -1;
	cJSON *sub_json = NULL;
	cJSON *json_fmt = cJSON_Parse(text);
	if (!json_fmt) {
		ERROR("Error before: [%s]\n",cJSON_GetErrorPtr());
		return -1;
	} else {
		if((sub_json=GET_KEY(json_fmt,MSG_SEQNUM)) && seq) {
			*seq = atoi(sub_json->valuestring);
			DEBUG("[Get] seqnum=%d\n",*seq);
		} else {
			goto EXIT;
		}
		
		if((sub_json=GET_KEY(json_fmt,MSG_BODY))) {
			if((sub_json=GET_KEY(sub_json,MSG_CUR_TIME))) {
				DEBUG("Server time=%s\n",sub_json->valuestring);
				if(svrtime)
					strncpy(svrtime,sub_json->valuestring,127);
				ret = 0;
			}
		}
	}
	cJSON_Delete(json_fmt);
	json_fmt = NULL;
	return ret;
	
EXIT:
	if(json_fmt) cJSON_Delete(json_fmt);
	return -1;
}

int parse_json_timing_gpio_mode(const char *text)
{
	int ret = -1;
	cJSON *sub_json = NULL;
	cJSON *json_fmt = cJSON_Parse(text);
	if (!json_fmt) {
		ERROR("Error before: [%s]\n",cJSON_GetErrorPtr());
		return -1;
	} else {
		if((sub_json=GET_KEY(json_fmt,MSG_BODY))) {
			if((sub_json=GET_KEY(sub_json,MSG_GPIO_MODE))) {
				DEBUG("GPIO Ctrl mode:%s\n",sub_json->valuestring);
				if(strcmp(sub_json->valuestring,"auto") == 0)
					ret = 1;
				else if(strcmp(sub_json->valuestring,"manual") == 0)
					ret = 2;
				else
					ret = 0;
			}
		}
	}
	cJSON_Delete(json_fmt);
	json_fmt = NULL;
	return ret;
}

int parse_json_ota_msg(const char* text,OTA_ST *ota_info)
{
	int parse_mask = 0;
	cJSON *sub_json = NULL, *body_json = NULL;
	cJSON *json_fmt = cJSON_Parse(text);
	if (!json_fmt) {
		ERROR("Error before: [%s]\n",cJSON_GetErrorPtr());
		return -1;
	} else {
		if(NULL == (body_json=GET_KEY(json_fmt,MSG_BODY)) ) {
			ERROR("OTA do NOT found body msg!\n");
			goto EXIT;
		}
		
		if((sub_json=GET_KEY(body_json,MSG_OTA_URL))) {
			DEBUG("[OTA] url:[%s]\n",sub_json->valuestring);
			parse_mask++;
			STRCPY(ota_info->url);
		}
		if((sub_json=GET_KEY(body_json,MSG_OTA_VER))) {
			DEBUG("[OTA] version:[%s]\n",sub_json->valuestring);
			parse_mask++;
			STRCPY(ota_info->version);
		}
		if((sub_json=GET_KEY(body_json,MSG_OTA_SIZE))) {
			DEBUG("[OTA] size:[%s]\n",sub_json->valuestring);
			parse_mask++;
			ota_info->size = strtol(sub_json->valuestring,NULL,16);
		}
		if((sub_json=GET_KEY(body_json,MSG_OTA_CRC))) {
			DEBUG("[OTA] crc:[%s]\n",sub_json->valuestring);
			parse_mask++;
			ota_info->crc = strtol(sub_json->valuestring,NULL,16);
		}
		if((sub_json=GET_KEY(body_json,MSG_OTA_MD5))) {
			DEBUG("[OTA] md5:[%s]\n",sub_json->valuestring);
			parse_mask++;
			STRCPY(ota_info->md5);
		}
		if((sub_json=GET_KEY(body_json,MSG_OTA_BUILDTIME))) {
			DEBUG("[OTA] buildtime:[%s]\n",sub_json->valuestring);
			parse_mask++;
			STRCPY(ota_info->buildtime);
		}
	}
	if(parse_mask != OTA_MASK) {
		ERROR("Error json format![%s]\n",text);
		goto EXIT;
	}
	cJSON_Delete(json_fmt);
	json_fmt = NULL;
	return 0;
EXIT:
	if(json_fmt) cJSON_Delete(json_fmt);
	return -1;
}

// add after 2016-10-15
int parse_json_readlog(const char *text,int *seq, char *type)
{
	int ret = -1;
	cJSON *sub_json = NULL;
	cJSON *json_fmt = cJSON_Parse(text);
	if (!json_fmt) {
		ERROR("Error before: [%s]\n",cJSON_GetErrorPtr());
		return -1;
	} else {
		if((sub_json=GET_KEY(json_fmt,MSG_SEQNUM))) {
			*seq = atoi(sub_json->valuestring);
			DEBUG("[Get] seqnum=%d\n",*seq);
		} else {
			goto EXIT;
		}

		if((sub_json=GET_KEY(json_fmt,MSG_BODY))) {
			if((sub_json=GET_KEY(sub_json,MSG_SYS_LOG))) {
				strncpy(type,sub_json->valuestring,127);
				DEBUG("get logtype=%s\n",type);
				ret = 0;
			}
		}
	}
EXIT:
	cJSON_Delete(json_fmt);
	json_fmt = NULL;
	return ret;
}


int parse_json_runcmd(const char *text,int *seq, char *cmd)
{
	int ret = -1;
	cJSON *sub_json = NULL;
	cJSON *json_fmt = cJSON_Parse(text);
	if (!json_fmt) {
		ERROR("Error before: [%s]\n",cJSON_GetErrorPtr());
		return -1;
	} else {
		if((sub_json=GET_KEY(json_fmt,MSG_SEQNUM))) {
			*seq = atoi(sub_json->valuestring);
			DEBUG("[Get] seqnum=%d\n",*seq);
		} else {
			goto EXIT;
		}

		if((sub_json=GET_KEY(json_fmt,MSG_BODY))) {
			if((sub_json=GET_KEY(sub_json,MSG_CMD_INFO))) {
				strncpy(cmd,sub_json->valuestring,255);
				DEBUG("get cmd=%s\n",cmd);
				//if(cmd)
					
				ret = 0;
			}
		}
	}
EXIT:
	cJSON_Delete(json_fmt);
	json_fmt = NULL;
	return ret;
}

int parse_json_remote_cfg(const char *text,int *seq,char *devid,char *ipaddr,char *port)
{
	int ret = -1;
	cJSON *sub_json = NULL, *body_json = NULL;
	cJSON *json_fmt = cJSON_Parse(text);
	if (!json_fmt) {
		ERROR("Error before: [%s]\n",cJSON_GetErrorPtr());
		return -1;
	} else {
		if((sub_json=GET_KEY(json_fmt,MSG_SEQNUM))) {
			*seq = atoi(sub_json->valuestring);
			DEBUG("[Get] seqnum=%d\n",*seq);
		} else {
			goto EXIT;
		}

		if((body_json=GET_KEY(json_fmt,MSG_BODY))) {

			if((sub_json=GET_KEY(body_json,MSG_REMOTE_ID))) {
				strncpy(devid,sub_json->valuestring,127);
				DEBUG("get cfg devid=%s\n",devid);
				//if(devid)
					
			} else {
				goto EXIT;
			}

			if((sub_json=GET_KEY(body_json,MSG_REMOTE_IP))) {
				strncpy(ipaddr,sub_json->valuestring,127);
				DEBUG("get cfg ip=%s\n",ipaddr);
				//if(ipaddr)
					
			} else {
				goto EXIT;
			}

			if((sub_json=GET_KEY(body_json,MSG_REMOTE_PORT))) {
				strncpy(port,sub_json->valuestring,127);
				DEBUG("get cfg port=%s\n",port);
				//if(port)
					
			} else {
				goto EXIT;
			}
			ret = 0;
		}
	}
EXIT:
	cJSON_Delete(json_fmt);
	json_fmt = NULL;
	return ret;
}

int parse_json_ppp_cfg(const char *text,int *seq,char *dialtype,char *apn,char *user,char *pwd,char *dialnum)
{
	int ret = -1;
	cJSON *sub_json = NULL,*body_json = NULL;
	cJSON *json_fmt = cJSON_Parse(text);
	if (!json_fmt) {
		ERROR("Error before: [%s]\n",cJSON_GetErrorPtr());
		return -1;
	} else {
		if((sub_json=GET_KEY(json_fmt,MSG_SEQNUM))) {
			*seq = atoi(sub_json->valuestring);
			DEBUG("[Get] seqnum=%d\n",*seq);
		} else {
			goto EXIT;
		}
		
		if((body_json=GET_KEY(json_fmt,MSG_BODY))) {

			if((sub_json=GET_KEY(body_json,MSG_PPP_4GDIAL))) {
				DEBUG("get ppp 4gdial=%s\n",sub_json->valuestring);
				if(dialtype)
					strncpy(dialtype,sub_json->valuestring,127);
			} else {
				goto EXIT;
			}

			if((sub_json=GET_KEY(body_json,MSG_PPP_APN))) {
				DEBUG("get ppp apn=%s\n",sub_json->valuestring);
				if(apn)
					strncpy(apn,sub_json->valuestring,127);
			} else {
				goto EXIT;
			}

			if((sub_json=GET_KEY(body_json,MSG_PPP_USER))) {
				DEBUG("get ppp user=%s\n",sub_json->valuestring);
				if(user)
					strncpy(user,sub_json->valuestring,127);
			} else {
				goto EXIT;
			}

			if((sub_json=GET_KEY(body_json,MSG_PPP_PASD))) {
				DEBUG("get ppp pwd=%s\n",sub_json->valuestring);
				if(pwd)
					strncpy(pwd,sub_json->valuestring,127);
			} else {
				goto EXIT;
			}

			if((sub_json=GET_KEY(body_json,MSG_PPP_DIALNUM))) {
				DEBUG("get ppp dialnum=%s\n",sub_json->valuestring);
				if(dialnum)
					strncpy(dialnum,sub_json->valuestring,127);
			} else {
				goto EXIT;
			}
			ret = 0;
		}
	}
EXIT:
	cJSON_Delete(json_fmt);
	json_fmt = NULL;
	return ret;
}

int parse_json_tty_trans(const char *text, int *seq, char *outcmd)
{
	int ret = -1;
	cJSON *sub_json = NULL,*body_json = NULL;
	cJSON *json_fmt = cJSON_Parse(text);
	if (!json_fmt) {
		ERROR("Error before: [%s]\n",cJSON_GetErrorPtr());
		return -1;
	} else {
		if((sub_json=GET_KEY(json_fmt,MSG_SEQNUM))) {
			*seq = atoi(sub_json->valuestring);
			DEBUG("[Get] seqnum=%d\n",*seq);
		} else {
			goto EXIT;
		}
		
		if((body_json=GET_KEY(json_fmt,MSG_BODY))) {

			if((sub_json=GET_KEY(body_json,MSG_CMD_INFO))) {
				DEBUG("get tty cmd=%s\n",sub_json->valuestring);
				if(outcmd)
					strncpy(outcmd,sub_json->valuestring,1023);
			} else {
				goto EXIT;
			}
			if((sub_json=GET_KEY(body_json,MSG_TTY_CMDTP))) {
				DEBUG("get tty cmd type=%s\n",sub_json->valuestring);
				
			} else {
				goto EXIT;
			}
			ret = 0;
		}
	}
EXIT:
	cJSON_Delete(json_fmt);
	json_fmt = NULL;
	return ret;
}

int parse_json_tty_cfg(const char *text,int *seq, ST_UART *uart_attr)
{
	int ret = -1;
	cJSON *sub_json = NULL,*body_json = NULL;
	cJSON *json_fmt = cJSON_Parse(text);
	if (!json_fmt) {
		ERROR("Error before: [%s]\n",cJSON_GetErrorPtr());
		return -1;
	} else {
		if((sub_json=GET_KEY(json_fmt,MSG_SEQNUM))) {
			*seq = atoi(sub_json->valuestring);
			DEBUG("[Get] seqnum=%d\n",*seq);
		} else {
			goto EXIT;
		}
		
		if((body_json=GET_KEY(json_fmt,MSG_BODY))) {
			if((sub_json=GET_KEY(body_json,MSG_TTY_BAUD))) {
				DEBUG("get tty baud=%s\n",sub_json->valuestring);
				uart_attr->baud = atoi(sub_json->valuestring);
			} else {
				goto EXIT;
			}
			if((sub_json=GET_KEY(body_json,MSG_TTY_DBT))) {
				DEBUG("get tty data bit=%s\n",sub_json->valuestring);
				uart_attr->databit = atoi(sub_json->valuestring);
			} else {
				goto EXIT;
			}
			if((sub_json=GET_KEY(body_json,MSG_TTY_STPB))) {
				DEBUG("get tty stop bit=%s\n",sub_json->valuestring);
				uart_attr->stopbit = atoi(sub_json->valuestring);
			} else {
				goto EXIT;
			}
			if((sub_json=GET_KEY(body_json,MSG_TTY_PARI))) {
				DEBUG("get tty crc=%s\n",sub_json->valuestring);
				uart_attr->parity = atoi(sub_json->valuestring);
			} else {
				goto EXIT;
			}
			if((sub_json=GET_KEY(body_json,MSG_TTY_FLOWCT))) {
				DEBUG("get tty flowcontrol=%s\n",sub_json->valuestring);
				uart_attr->flowct = atoi(sub_json->valuestring);
			} else {
				goto EXIT;
			}
			if((sub_json=GET_KEY(body_json,MSG_TTY_WKMD))) {
				DEBUG("get tty workmode=%s\n",sub_json->valuestring);
				uart_attr->workmode = atoi(sub_json->valuestring);
			} else {
				goto EXIT;
			}
			if((sub_json=GET_KEY(body_json,MSG_TTY_TINTER))) {
				DEBUG("get tty time interval=%s\n",sub_json->valuestring);
				uart_attr->interval = atoi(sub_json->valuestring);
			} else {
				goto EXIT;
			}
			
			ret = 0;
		}
	}
EXIT:
	cJSON_Delete(json_fmt);
	json_fmt = NULL;
	return ret;
}
