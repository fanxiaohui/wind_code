/**
* @file ikcc_lua_api.h
* @brief 
* @version 1.1
* @author cairj
* @date 2016/11/15
*/

#ifndef IKCC_LUA_API_H
#define IKCC_LUA_API_H
#include <stdio.h>
#include <stdlib.h>

#include "cJSON.h"


#ifdef LUA_SCRIPT
int lua_init_lua();
int lua_destroy_lua();

int lua_check_user(const char* user, const char* password);
int lua_insert_user(const char* user, const char* password);

/**
* @ Description: uart to json format
* @ param: 	uart: uart stream	
* @ param:	len: uart len
* @ return: json data
*/
const char* lua_uart_to_json(const char* uart, int len);
/**
* @ Description: json to uart fromat
* @ param: 		json: json data
* @ param out:	size: uart size
* @ return: 	uart stream
*/
const unsigned char* lua_json_to_uart(const char* json, int* len);

//test 
void cjson_test();
#endif /* LUA_SCRIPT */

#endif /* IKCC_LUA_API_H */
