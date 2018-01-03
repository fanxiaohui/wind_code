/*
 * dbDevice.c
 *
 *  Created on: 2016年12月20日
 *      Author: john
 */

#include <string.h>
#include <stdio.h>
#include "dbDevice.h"
#include "db.h"
#include "../base64.h"
#include "print.h"

const char *__table__ = "device";

int dbCreateDeviceTable(void) {
	int sret = execution("CREATE TABLE IF NOT EXISTS device(id INTEGER PRIMARY KEY ,productid varchar(64),mac varchar(128),address varchar(255),status INTEGER)");
	//初始化，将设备全部离线
	updateKey("status", "1", "status", "0", __table__);
	return sret;
}

int dbAddDevice(unsigned int deviceid, char *productid, unsigned char *mac, int maclength, struct sockaddr_in *address) {
	char cmd[1024] = { 0x00 };
	char macstring[128] = { 0x00 };
	char addr[128] = { 0x00 };
	sprintf(cmd, "%d", deviceid);
	if (checkFeildexisted("id", cmd, __table__) == 1) {
		return -2;
	}
	//base64_encode(mac, macstring, maclength);
	sprintf(macstring, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );
	base64_encode((unsigned char *) address, addr, sizeof(struct sockaddr_in));
	snprintf(cmd, 1024, "INSERT INTO device VALUES('%d','%s','%s','%s','1')", deviceid, productid, macstring, addr);
	int ret = execution(cmd);
	return ret;
}

int dbDeleteDevice(unsigned int deviceid) {
	char cmd[64] = { 0x00 };
	sprintf(cmd, "%d", deviceid);
	if (checkFeildexisted("id", cmd, __table__) != 1) {
		return 0;
	}
	return deleteData("id", cmd, __table__);
}

int dbDeviceExisted(unsigned int deviceid) {
	char cmd[64] = { 0x00 };
	sprintf(cmd, "%d", deviceid);
	return checkFeildexisted("id", cmd, __table__);
}

int dbUpdateDeviceAddress(unsigned int deviceid, struct sockaddr_in *address) {
	char idstring[32] = { 0x00 };
	char addr[128] = { 0x00 };
	sprintf(idstring, "%d", deviceid);
	if (checkFeildexisted("id", idstring, __table__) != 1) {
		return -2;
	}
	base64_encode((unsigned char *) address, addr, sizeof(struct sockaddr_in));

	return updateKey("id", idstring, "address", addr, __table__);
}

int dbSetDeviceStatus(unsigned int deviceid, int status) {
	char idstring[32] = { 0x00 };

	sprintf(idstring, "%d", deviceid);
	if (checkFeildexisted("id", idstring, __table__) != 1) {
		return -2;
	}
	if (status == 0) {
		return updateKey("id", idstring, "status", "0", __table__);
	} else {
		return updateKey("id", idstring, "status", "1", __table__);
	}
}

int dbGetDeviceStatus(unsigned int deviceid, int *status) {
	char cmd[1024] = { 0x00 };
	sqlite3_stmt *statement = NULL;
	int ret = -2;
	snprintf(cmd, 1024, "SELECT * FROM %s where id='%d'", __table__, deviceid);
	if (dbprepare_v2(cmd, &statement) != SQLITE_OK) {
		return -1;
	}
	if (sqlite3_step(statement) == SQLITE_ROW) {
		*status = sqlite3_column_int(statement, 4);
		ret = 0;
	}
	sqlite3_finalize(statement);
	return ret;
}

int dbGetDeviceCount() {
	return dbGetTableCount(__table__);
}

int dbGetAllDeviceID(unsigned char *buffer, int buffersize) {
	sqlite3_stmt *statement = NULL;
	if (dbprepare_v2("SELECT * FROM device", &statement) != SQLITE_OK) {
		return -1;
	}
	int deviceid = 0;
	int index = 0;
	while (sqlite3_step(statement) == SQLITE_ROW) {
		deviceid = sqlite3_column_int(statement, 0);
		if ((index + 4) >= buffersize)
			break;
		buffer[index++] = (unsigned char) (((deviceid) & 0xFF000000) >> 24);
		buffer[index++] = (unsigned char) (((deviceid) & 0x00FF0000) >> 16);
		buffer[index++] = (unsigned char) (((deviceid) & 0x0000FF00) >> 8);
		buffer[index++] = (unsigned char) (((deviceid) & 0x000000FF));
	}
	sqlite3_finalize(statement);
	return index;
}

extern int dbGetDeviceInfo(unsigned int deviceid, char *outProductID, unsigned char *outMac, int *outMaclen, struct sockaddr_in *outAddress) {
	char cmd[1024] = { 0x00 };
	sqlite3_stmt *statement = NULL;
	int ret = -2;
	snprintf(cmd, 1024, "SELECT * FROM %s where id='%d'", __table__, deviceid);
	if (dbprepare_v2(cmd, &statement) != SQLITE_OK) {
		return -1;
	}
	if (sqlite3_step(statement) == SQLITE_ROW) {
		//*status = sqlite3_column_int(statement, 4);
		//product id
		if (outProductID != NULL) {
			char *productid = (char *)sqlite3_column_text(statement, 1);
			if (productid != NULL) {
				memcpy(outProductID, productid, 32);
			}
		}

		if (outMac != NULL && outMaclen != NULL) {
			char *temp = (char *)sqlite3_column_text(statement, 2);
			if (temp != NULL)
			{
				//*outMaclen = base64_decode(temp, outMac);
				*outMaclen=sscanf(temp, "%x:%x:%x:%x:%x:%x", outMac, outMac+1, outMac+2, outMac+3, outMac+4, outMac+5);
				//Debug("outmanlen[%d], %x %x %x %x %x %x", *outMaclen, outMac[0], outMac[1], outMac[2],outMac[3], outMac[4], outMac[5]);
			}
		}

		if (outAddress != NULL) {
			char *temp = (char *)sqlite3_column_text(statement, 3);
			if(temp != NULL)
				base64_decode(temp, (unsigned char *)outAddress);
		}
		ret = 0;
	}
	sqlite3_finalize(statement);
	return ret;
}
