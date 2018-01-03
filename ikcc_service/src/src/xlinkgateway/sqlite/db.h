/*
 * db.h
 *
 *  Created on: Dec 19, 2016
 *      Author: chery
 */

#ifndef DB_H_
#define DB_H_

#include "sqlite3.h"

//return 0 ok , -1 error
extern int dbInit(void);
extern void dbUninit(void);
//return 0 ok , -1 error
extern int execution(const char *cmd);
//return 1 existed, 0 not existed
extern int checkFeildexisted(const char *field, const char *value, const char *table);
//return 0 ok, -1 error
extern int deleteData(const char *field, const char *value, const char *table);

extern int updateKey(const char *where, char *wvalue, const char *key, char *kvalue, const char *table);

extern int dbprepare_v2(	const char *zSql,	sqlite3_stmt **ppStmt);
extern int dbGetTableCount(const char *table);
#endif /* DB_H_ */
