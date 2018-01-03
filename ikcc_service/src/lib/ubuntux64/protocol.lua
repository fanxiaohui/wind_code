#!/usr/local/lib/lua/5.1/
local cjson = require("cjson")

print("lua script prorocol.lua have been load ")  
function lua_uart_to_json(uart)
	if (string.len(uart)<12) then
		return "error!"
	end
	json = {}
	json["head"] = at(uart, 1)*256 + at(uart, 2)
	json["len"]= at(uart, 3)*256 + at(uart, 4)
	len = string.len(uart) - 12
	luaprint(len)
	json["type"]= at(uart, 5)*256 + at(uart, 6)
	json["cmd"]= at(uart, 7)
	json["stat"]= at(uart, 8)
	json["flags"]= at(uart, 9)*256 + at(uart, 10)
	json["payload"]={}
	for j=1,len,1 do
		json["payload"][j] = at(uart, 10+j)
	end
    json["crc"]= at(uart, 11+len)*256 + at(uart, 12+len)
	ss = cjson.encode(json)
	--luaprint(ss)
	return ss
end

function lua_json_to_uart(js)
	json = cjson.decode(js)
	
	local dst_str = ""
	--head
	dst_str = dst_str..string.char(json["head"]/256)
	dst_str = dst_str..string.char(json["head"]%256)
	--len
	dst_str = dst_str..string.char(json["len"]/256)
	dst_str = dst_str..string.char(json["len"]%256)
	--type
	dst_str = dst_str..string.char(json["type"]/256)
	dst_str = dst_str..string.char(json["type"]%256)
	--cmd
	dst_str = dst_str..string.char(json["cmd"])
	--stat
	dst_str = dst_str..string.char(json["stat"])
	--flags
	dst_str = dst_str..string.char(json["flags"]/256)
	dst_str = dst_str..string.char(json["flags"]%256)
	--payload
	payload_len = json["len"]-8
	for i=1,payload_len,1 do
		dst_str = dst_str..string.char(json["payload"][i])
	end
	--crc
	dst_str = dst_str..string.char(json["crc"]/256)
	dst_str = dst_str..string.char(json["crc"]%256)
	return json["len"]+4, dst_str
end

function at(str,i)
	return string.byte(string.sub(str, i, i))
end