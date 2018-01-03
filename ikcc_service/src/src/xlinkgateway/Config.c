/*
 * XDConfig.c
 *
 *  Created on: 2016年12月13日
 *      Author: john
 */

#include "Config.h"
#include "inifile.h"
#include "base64.h"

#include "platform_type.h"
#include "print.h"
#define SECTION  "config"
#define FILE_NAME "./config.ini"

int XDReadBase64Decode(char *key, unsigned char *value, int size) {

	char buffer[1024] = { 0x00 };
	int ret = read_profile_string(SECTION, key, buffer, 1024, "1234", FILE_NAME);
	ret = base64_decode((const char *) buffer, value);
	Info("----------------------------------------XD read key=%s value leng=%d", key, ret);
	return ret;
}

int XDWriteBase64Encode(char *key, unsigned char *value, int size) {
	Info("----------------------------------------XD write key=%s", key);
	char temp[1024] = { 0x00 };
	base64_encode((const unsigned char *) value, temp, size);
	return write_profile_string(SECTION, key, temp, FILE_NAME);
}

int XDReadString(char *key, char *value, int size) {
	int ret = read_profile_string(SECTION, key, value, size, "", FILE_NAME);
	return ret;
}

int XDWriteString(char *key, char *value){
	return write_profile_string(SECTION, key, value, FILE_NAME);
}
