#!/usr/lib/lua/5.1/

package.cpath='/usr/lib/lua/?.so;./?.so;/usr/lib/lua/loadall.so'
local cjson = require("cjson")

print("lua script function.lua have been load ")  
local user_list = "/bin/ikcc/user_list.cfg"


function lua_check_user(user, password)
	if (file_exists(user_list)) then
	else
		__write("{}")
		return 3	--no file
	end
	luaprint(__read())
	list = cjson.decode(__read())
	if (list[user] == nil) then
		luaprint("user error!")
		return 1
	else
		if (list[user] ~= password) then
			luaprint("password error!")
			return 2
		end
	end
	luaprint("user check success!")
	return 0
end  


function lua_insert_user(user, password)
	file = __read()
	data = cjson.decode(file)
	data[user] = password	
	luaprint(cjson.encode(data))
	__write(cjson.encode(data))
end


function __read()
	file = io.open(user_list, "r")
	local data = file:read("*a")
	file:close()
	return data
end
  
function __write(str)
	file = io.open(user_list, "w+")
	file:write(str)
	file:close()
end

function file_exists(path)
	local file = io.open(path, "rb")
	if file then
		file:close()
	end
	return file ~= nil
end

function luaprint(str)
	print("[lua]"..str)
end