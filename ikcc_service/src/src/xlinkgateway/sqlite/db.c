/*
 * db.c
 *
 *  Created on: Dec 19, 2016
 *      Author: chery
 */

#include <stdio.h>
#include "db.h"
#include <stdlib.h>
//#include "../src/XGDebug.h"
#include "print.h"
#include "platform_type.h"

static unsigned int isinited = 0;

static sqlite3 *dbhandle = NULL;

int dbInit(void) {
	if (isinited == 1) {
		return 0;
	}
	int sret = sqlite3_open("device.db", &dbhandle);
	if (sret != SQLITE_OK) {
		return -1;
	}
	int rets = system("chmod 666 device.db");
	if (rets < 0) {
	}

	isinited = 1;
	return 0;
}

void dbUninit(void) {
	if (isinited == 1) {
		sqlite3_close(dbhandle);
		isinited = 0;
		dbhandle = NULL;
	}
}

sqlite3 *getDbHandle(void) {
	return dbhandle;
}

int execution(const char *cmd) {

	char* szError = NULL;
	if (dbhandle == NULL) {
		return -1;
	}
	int nRet = sqlite3_exec(dbhandle, cmd, 0, 0, &szError);
	if (nRet == SQLITE_OK) {
		return SQLITE_OK;
	}

	if (szError != NULL) {
		Error("db execution  error:%s\r\n", szError);
		sqlite3_free(szError);
	}

	return -1;
}

int checkFeildexisted(const char *field, const char *value, const char *table) {

	char cmd[1024] = { 0x00 };
	int ret = 0;
	sqlite3_stmt *statement = NULL;
	if (dbhandle == NULL) {
		return 0;
	}
	if (field == NULL || value == NULL || table == NULL) {
		return 0;
	}
	snprintf(cmd, 1024, "SELECT * FROM %s where %s='%s'", table, field, value);
	Info("Sql:%s", cmd);
	if (dbprepare_v2(cmd, &statement) != SQLITE_OK) {
		return 0;
	}

	if (sqlite3_step(statement) == SQLITE_ROW) {
		ret = 1;
	}

	sqlite3_finalize(statement);
	return ret;
}

int deleteData(const char *field, const char *value, const char *table) {
	char cmd[1024] = { 0x00 };
	if (dbhandle == NULL) {
		return -1;
	}
	if (checkFeildexisted(field, value, table) != 1) {
		return 0;
	}
	snprintf(cmd, 1024, "delete from %s where %s='%s'", table, field, value);
	int ret = execution(cmd);
	return ret;
}

int updateKey(const char *where, char *wvalue, const char *key, char *kvalue, const char *table) {
	int ret = -1;
	char cmd[1024] = { 0x00 };
	if (dbhandle == NULL) {
		return -1;
	}
	snprintf(cmd, 1024, "update %s set %s='%s' where %s='%s'", table, key, kvalue, where, wvalue);
	ret = execution(cmd);
	return ret;
}

int dbprepare_v2(const char *zSql, sqlite3_stmt **ppStmt) {
	return sqlite3_prepare_v2(dbhandle, zSql, -1, ppStmt, NULL);
}

int dbGetTableCount(const char *table) {
	char cmd[1024] = { 0x00 };
	sqlite3_stmt *statement = NULL;
	int ret = -2;
	snprintf(cmd, 1024, "SELECT COUNT(*) FROM %s", table);
	if (dbprepare_v2(cmd, &statement) != SQLITE_OK) {
		return -1;
	}
	if (sqlite3_step(statement) == SQLITE_ROW) {
		ret = sqlite3_column_int(statement, 0);
	}
	sqlite3_finalize(statement);
	return ret;
}

