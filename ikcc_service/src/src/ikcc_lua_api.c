
#ifdef LUA_SCRIPT

#include "ikcc_lua_api.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


#include "print.h"
#include "ft_def.h"
#include "platform_type.h"



static lua_State* L = NULL;

static __stack_dump(lua_State *L)
{
	int i;
	int top = lua_gettop(L);
	for (i = 1; i <= top; i++){
		int t = lua_type(L, i);
		switch (t){
			case LUA_TSTRING:{
				Info("string:%s", lua_tostring(L, i));
				break;
			}
			case LUA_TBOOLEAN:{
				Info(lua_toboolean(L, i)?"true":"false");
				break;
			}
			case LUA_TNUMBER:{
				Info("number:%g", lua_tonumber(L, i));
				break;
			}
			default:{
				Info("type:%s", lua_typename(L, i));
				break;
			}
		}
	}
}

int lua_init_lua()
{
	L = lua_open();
	luaL_openlibs(L);
#ifdef OPENWRT
	luaL_dofile(L, "/etc/config/ikcc/script/authentication.lua");
	luaL_dofile(L, "/etc/config/ikcc/script/protocol.lua");
#else
	luaL_dofile(L, "./lib/ubuntux64/authentication.lua");
	luaL_dofile(L, "./lib/ubuntux64/protocol.lua");
#endif /* OPENWRT */
	
}

int lua_destroy_lua()
{
	lua_close(L);
}

int lua_check_user(const char* user, const char* password)
{
	if (user == NULL || password == NULL){
		Warn("user or password error!");
		return 3;
	}
	lua_getglobal(L, __FUNCTION__);
	lua_pushstring(L, user);
	lua_pushstring(L, password);
	lua_pcall(L, 2, 1, 0);
	//__stack_dump(L);
	return lua_tointeger(L, -1);
}

int lua_insert_user(const char* user, const char* password)
{
	
	lua_getglobal(L, __FUNCTION__);
	lua_pushstring(L, user);
	lua_pushstring(L, password);
	lua_pcall(L, 2, 0, 0);
	//__stack_dump(L);
	return lua_tointeger(L, -1);
}

const char* lua_uart_to_json(const char* uart, int len)
{
	lua_getglobal(L, __FUNCTION__);
	lua_pushlstring(L, uart, len);
	lua_pcall(L, 1, 1, 0);
	//__stack_dump(L);
	return lua_tostring(L, -1);
}

const unsigned char* lua_json_to_uart(const char* json, int* len)
{
	lua_getglobal(L, __FUNCTION__);
	lua_pushstring(L, json);
	lua_pcall(L, 1, 2, 0);
	//__stack_dump(L);
	const char* p = lua_tostring(L, -1); 
	*len = lua_tointeger(L, -2);
	//Info("%s:%d", p, *len);
	return p;
}


// test code below
void print_json(const char* text)
{
	if (text)
		Info("%s",text);
	char * out;
	cJSON * json;

	json = cJSON_Parse(text);

	//Info("name:%s, age:%d", cJSON_GetObjectItem(json, "name")->valuestring, cJSON_GetObjectItem(json, "age")->valueint);
	Info("head:%d, len:%d, type:%d, cmd:%d, stat:%d, flag:%d, crc:%d, payload size:%d-%d %d", 
	cJSON_GetObjectItem(json, "head")->valueint, cJSON_GetObjectItem(json, "len")->valueint, cJSON_GetObjectItem(json, "type")->valueint,
	cJSON_GetObjectItem(json, "cmd")->valueint, cJSON_GetObjectItem(json, "stat")->valueint, cJSON_GetObjectItem(json, "flags")->valueint,
	cJSON_GetObjectItem(json, "crc")->valueint, cJSON_GetArraySize(cJSON_GetObjectItem(json, "payload")), cJSON_GetArrayItem(cJSON_GetObjectItem(json, "payload"), 1)->valueint, cJSON_GetArrayItem(cJSON_GetObjectItem(json, "payload"), 0)->valueint);
	out = cJSON_Print(json);
	cJSON_Delete(json);
	//Info("%s", out);
	free(out);
	
}

void  cjson_test()
{
	Info("check result:%d", lua_check_user("cairj", "cairuijia"));
	//Info("insert result:%d", lua_insert_user("fuzy", "fuzhiyuan"));

	//json test
	//char str[]="{\"name\":\"Jack(\\\"Bee\\\")Nimble\", \"age\":32}";
	char str[]={0xf4, 0xf5, 0x00, 0x0a, 0x00, 0x01, 0x06, 0x01, 0x00, 0x00, 0x59, 0x59,0x00, 0x0f};
	const char* res = lua_uart_to_json(str, sizeof(str));
	print_json(res);

	const char* jss = "{\"cmd\":6,\"type\":1,\"crc\":15,\"head\":62709,\"len\":9,\"payload\":[89,90],\"stat\":1,\"flags\":0}";
	int size = 0;
	const unsigned char* js = lua_json_to_uart(jss, &size);
	//unsigned char* js = (unsigned char*)str;
	int i;
	for (i =0; i<size; i++){
		Debug("%02x", js[i]);
	}

}
#endif



