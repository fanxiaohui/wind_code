#!/bin/env lua

require "leomclualib" --lmclua.writeuart("/dev/ttyS1","hello",5)

local json = require "cjson.safe"
local FILE_PREFIX = "/mnt/mmcblk0p1/leoman/"
local FILE_NAME_PRE = "report-"
local comwrite = lmclua.writeuart
local utc2local = lmclua.time_from_utc
local LOG2SYS = lmclua.log2sys or print

reportfile = {file="/mnt/mmcblk0p1/leoman/2017-4-17"}
function reportfile.read()
	local file = reportfile.file
    local f = io.open(file, "r")
	if(f == nil) then
		LOG2SYS("reportfile cannot open file:" .. file)
		return ""
	end
    local s = f:read("*a")
	s = string.gsub(s,"\r","")
	s = string.gsub(s,"\n","")
    f:close()
    return s
end

local function readfile(filepath)
    local f = io.open(filepath, "r")
	if(f == nil) then
		LOG2SYS("readfile cannot open file:" .. filepath)
		return nil
	end
    local s = f:read("*a")
	s = string.gsub(s,"\r","")
	s = string.gsub(s,"\n","")
    f:close()
    return s
end

local function uart_write(comdev,content)
    local f = io.open(comdev, "wb+")
	if(f == nil) then
		LOG2SYS("uart_write cannot open dev:" .. comdev)
		return 0
	end
    local ret = f:write(content)
    f:close()
	return ret
end

local function check_valid(luaval)
	
	return true
end

local function hex2string(hex)
	local retbuf=""
	retbuf = string.gsub(hex,"(.)",
			function (c) return (string.format("%02x",string.byte(c)))
			end)
	return  retbuf
end

local function string2hex(str)
	local retbuf=""
	retbuf = string.gsub(str,"(.)(.)",
			function (c1,c2) return string.char(tonumber(string.format("%s%s",c1,c2),16))
			end)
	return  retbuf
end

local function string_split(str, delimiter)
	if str == nil or str == '' or delimiter == nil then
		return nil
	end
    local result = {}
    for match in (str..delimiter):gmatch("(.-)"..delimiter) do
        table.insert(result, match)
    end
	return result
end

local function get_single_char_from_int2(data_int)
	local tmp = string.format("%04x",data_int)
	return string2hex(tmp)
end

local function get_single_char_from_int4(data_int)
	local tmp = string.format("%08x",data_int)
	return string2hex(tmp)
end

local function int_offset(data_int,offset)
	local tmp = string.format("%08x",data_int)
	tmp = string2hex(tmp)
	return string.byte(tmp,offset) or 0x0
end

local function __head_char_array(head,cmd,length,timestr)
	local chartb = {}
	
	table.insert(chartb,string.char(head)) --head
	table.insert(chartb,string.char(length)) --len
	table.insert(chartb,string.char(cmd)) --cmd
	
	-- int 2017 = 0x07e1
	local charary = get_single_char_from_int2(timestr[1]) --year
	for i=1,#charary,1 do
		table.insert(chartb,string.char(string.byte(charary,i))) -- H_L
	end
	table.insert(chartb,string.char(timestr[2])) --month
	table.insert(chartb,string.char(timestr[3])) --day
	
	return chartb
end

local function __parse_day_circle_onbed(head,cmd,length,timestr,num,subrept)
	local onbed_item = 0x01
	local headarray = __head_char_array(head,cmd,length,timestr)
	-- insert num,item =0x01 onbed data
	local tailarray = {}
	local onoff_st,onoff_end
	if(type(subrept[1].time_start) ~= "number") then
		onoff_st = 0x00
	else
		onoff_st = subrept[1].time_start
	end
	if(type(subrept[1].time_end) ~= "number") then
		onoff_end = 0x00
	else
		onoff_end = subrept[1].time_end
	end
	
	table.insert(tailarray,string.char(num)) 		-- num
	table.insert(tailarray,string.char(onbed_item))	-- item
	local ye,mo,da,ho,mi,se = utc2local(onoff_st)   -- sleep start time
	print("Debug time start:",ye,mo,da,ho,mi,se)

	local yeartb = get_single_char_from_int2(ye)
	for i=1,#yeartb,1 do
		table.insert(tailarray,string.char(string.byte(yeartb,i)))
	end
	table.insert(tailarray,string.char((mo)))
	table.insert(tailarray,string.char((da)))
	table.insert(tailarray,string.char((ho)))
	table.insert(tailarray,string.char((mi)))
	table.insert(tailarray,string.char((se)))
	
	ye,mo,da,ho,mi,se = utc2local(onoff_end)      -- sleep end time
	print("Debug time end:",ye,mo,da,ho,mi,se)

	local yeartb = get_single_char_from_int2(ye)
	for i=1,#yeartb,1 do
		table.insert(tailarray,string.char(string.byte(yeartb,i)))
	end
	table.insert(tailarray,string.char((mo)))
	table.insert(tailarray,string.char((da)))
	table.insert(tailarray,string.char((ho)))
	table.insert(tailarray,string.char((mi)))
	table.insert(tailarray,string.char((se)))
	--[[
	local charary = get_single_char_from_int4(subrept[1].time_start) -- sleep start time
	for i=1,#charary,1 do
		table.insert(tailarray,string.char(string.byte(charary,i)))
	end
	charary = get_single_char_from_int4(subrept[1].time_end) -- sleep start time
	for i=1,#charary,1 do
		table.insert(tailarray,string.char(string.byte(charary,i)))
	end
	]]--
	local chartb = {}
	for i,v in pairs(headarray) do
		table.insert(chartb,v)
	end
	for i,v in pairs(tailarray) do
		table.insert(chartb,v)
	end
	
	local ret_day_cir_onbed = {}
	table.insert(ret_day_cir_onbed, chartb)
	
	--[[
	-- add ending data
	local ending = {head,0x06,cmd,0xee}
	table.foreachi(ending,function(i,v) ending[i] = string.char(v) end)
	table.insert(ret_day_cir_onbed, ending)
	--]]
	return ret_day_cir_onbed,"OK"
end

local function __parse_day_circle_sleepdata(head,cmd,length,
											timestr,num,slpdt,slpdt2)
	local ret_day_cir_array = {}
	LOG2SYS("Debug:sleep_data array size is " .. #slpdt .. " and " .. #slpdt2)
	
	if(#slpdt == 1 and type(slpdt[1]) == "userdata") then
	--[[	
		local chartb = {}
		local headarray = __head_char_array(head,cmd,length,timestr)
		local tailarray = {num,0x00,
							0x00,0x0,0x00,0x0,0x0,0x00,
							0x00,0x0,0x00,0x0,0x0,0x00,}
		
		for i,v in pairs(headarray) do
			table.insert(chartb,v)
		end
		-- switch hex to char
		table.foreachi(tailarray,function(i,v) tailarray[i] = string.char(v) end)
		for i,v in pairs(tailarray) do
			table.insert(chartb,v)
		end
		
		table.insert(ret_day_cir_array, chartb)
		print("sub_report.sleep_data,",num,"num ret len",#chartb)
	]]--
	
		-- add ending data
		local ending = {head,0x06,cmd,0xee}
		table.foreachi(ending,function(i,v) ending[i] = string.char(v) end)
		table.insert(ret_day_cir_array, ending)
	
		return ret_day_cir_array,"OK"
	end

	local slp_status = 0x00
	local slp_start = 0x00
	local slp_end = 0x00
	
	-- start NO.3
	
	local v
	if(num-2 <= #slpdt) then
		v = slpdt[num-2]
	else
		if(#slpdt2 == 1 and type(slpdt2[1]) == "userdata") then
			LOG2SYS("sub_report.sleep_data_2 is null")
			-- add ending data
			local ending = {head,0x06,cmd,0xee}
			table.foreachi(ending,function(i,v) ending[i] = string.char(v) end)
			table.insert(ret_day_cir_array, ending)
			
			return ret_day_cir_array,"OK"
		end
		num = num - 2 - #slpdt
		v = slpdt2[num]
	end
	--for _,v in pairs(slpdt) 
	if(v) then 
		local chartb = {}
		local headarray = __head_char_array(head,cmd,length,timestr)
		local tailarray = {}
		
		if(not v.status or type(v.status) ~= "number" or v.status > 0xff)
		then
			slp_status = 0x00
		else
			slp_status = v.status
		end
		if(not v["start"] or type(v["start"]) ~= "number")
		then
			slp_start = 0x00
		else
			slp_start = v["start"]
		end
		if(not v["end"] or type(v["end"]) ~= "number")
		then
			slp_end = 0x00
		else
			slp_end = v["end"]
		end
		--print("Debug st->end",string.format("%08x",v["start"]),string.format("%08x",v["end"]))
		--print("Debug st->end",v["start"],v["end"])
		
		table.insert(tailarray,string.char(num)) --num
		table.insert(tailarray,string.char(slp_status)) --item
		local ye,mo,da,ho,mi,se = utc2local(slp_start)  -- start time
		LOG2SYS("Debug time start:" .. ye .. mo .. da .. ho .. mi .. se)

		local yeartb = get_single_char_from_int2(ye)
		for i=1,#yeartb,1 do
			table.insert(tailarray,string.char(string.byte(yeartb,i)))
		end
		table.insert(tailarray,string.char((mo)))
		table.insert(tailarray,string.char((da)))
		table.insert(tailarray,string.char((ho)))
		table.insert(tailarray,string.char((mi)))
		table.insert(tailarray,string.char((se)))
		
		ye,mo,da,ho,mi,se = utc2local(slp_end)   -- end time
		LOG2SYS("Debug time end:" .. ye .. mo .. da .. ho .. mi .. se)

		local yeartb = get_single_char_from_int2(ye)
		for i=1,#yeartb,1 do
			table.insert(tailarray,string.char(string.byte(yeartb,i)))
		end
		table.insert(tailarray,string.char((mo)))
		table.insert(tailarray,string.char((da)))
		table.insert(tailarray,string.char((ho)))
		table.insert(tailarray,string.char((mi)))
		table.insert(tailarray,string.char((se)))
		--[[
		local charary = get_single_char_from_int4(v["start"]) --start time
		for i=1,#charary,1 do
			table.insert(tailarray,string.char(string.byte(charary,i)))
		end
		
		charary = get_single_char_from_int4(v["end"]) -- end time
		for i=1,#charary,1 do
			table.insert(tailarray,string.char(string.byte(charary,i)))
		end
		]]--
		
		for i,v in pairs(headarray) do
			table.insert(chartb,v)
		end
		for i,v in pairs(tailarray) do
			table.insert(chartb,v)
		end
		--table.foreachi(chartb,function(i,v) chartb[i] = string.char(v) end)
		LOG2SYS("sub_report.sleep_data,num is " .. num .. ",ret len " .. #chartb)
		table.insert(ret_day_cir_array, chartb)
		
		--for i,v in pairs(chartb) do
		--	print("Debug i-v",i,string.format("%02x",string.byte(v)))
		--end
		--comwrite("/dev/ttyS1",hexstr,#hexstr,0x02) -- call c func in clualib
	else
		-- add ending data
		local ending = {head,0x06,cmd,0xee}
		table.foreachi(ending,function(i,v) ending[i] = string.char(v) end)
		table.insert(ret_day_cir_array, ending)
	
		return ret_day_cir_array,"OK"
	end  --end if(v)
	
	return ret_day_cir_array,"OK"
end

local function __parse_day_circle_score(head,cmd,length,timestr,num,subrept)
	local score_item = 0x07
	local headarray = __head_char_array(head,cmd,length,timestr)
	-- insert num,item =0x01 onbed data
	local tailarray = {}
	local rept_score = 0x00
	if(type(subrept[1].score) ~= "number" or subrept[1].score > 0xff) then
		rept_score = 0x00
	else
		rept_score = subrept[1].score
	end
	
	table.insert(tailarray,string.char(num)) 		-- num
	table.insert(tailarray,string.char(score_item))	-- item
	for i=1,13,1 do
		table.insert(tailarray,string.char(0x00))
	end
	
	table.insert(tailarray,string.char(rept_score))	-- score
	
	local chartb = {}
	for i,v in pairs(headarray) do
		table.insert(chartb,v)
	end
	for i,v in pairs(tailarray) do
		table.insert(chartb,v)
	end
	
	local ret_day_cir_score = {}
	table.insert(ret_day_cir_score, chartb)
	
	--[[
	-- add ending data
	local ending = {head,0x06,cmd,0xee}
	table.foreachi(ending,function(i,v) ending[i] = string.char(v) end)
	table.insert(ret_day_cir_score, ending)
	--]]
	
	return ret_day_cir_score,"OK"
end

local function errzero_return_time_sleep()
	local chartb = {0x55,0x19,0xa3,0x00,0x00,0x00,0x00,0x00,0x00,
					0x00,0x00,0x00,0x00,0x00,0x00,0x00,
					0x00,0x00,0x00,0x00,0x00,0x00,0x00} -- 23 Bytes, crc in .c send
	
	table.foreachi(chartb,function(i,v) chartb[i] = string.char(v) end)
	local ret_day_circle = {}
	table.insert(ret_day_circle, chartb)
	return ret_day_circle,"OK;errZero"
end

local function _parse_day_circle(luaval,timestr,num)
	local slpdt = luaval.sub_report and luaval.sub_report[1] and luaval.sub_report[1].sleep_data
	local slpdt2 = luaval.sub_report and luaval.sub_report[1] and luaval.sub_report[1].sleep_data_2
	
	if(type(slpdt) ~= "table") then
		LOG2SYS("_parse_day_circle sub_report.sleep_data Format error!")
		return errzero_return_time_sleep()
	end
	
	if(num == 0x01) then
		-- on or leave bed
		-- only one item
		return __parse_day_circle_onbed(0x55,0xa3,0x19,timestr,num,luaval.sub_report)
		
	elseif(num == 0x02) then
		--sleep score
		-- only one item
		return __parse_day_circle_score(0x55,0xa3,0x19,timestr,num,luaval.sub_report)
			
	elseif(num >= 0x03) then
		-- sleep,deep,light,rem,awake
		-- multiply items; need end cmd
		return __parse_day_circle_sleepdata(0x55,0xa3,0x19,timestr,num,slpdt,slpdt2)
	else
		-- error num
		return errzero_return_time_sleep()
	end
end

local function _parse_month_report(luaret,head,cmd,length,timestr)
	local deep = luaret.deep and luaret.deep["value"]
	local light = luaret.light and luaret.light["value"]
	local deep_val ,light_val
	if(not deep or type(deep) ~= "number" or deep > 0xffff) then
		deep_val = 0x00
	else
		deep_val = deep
	end
	if(not light or type(light) ~= "number" or light > 0xffff) then
		light_val = 0x00
	else
		light_val = light
	end
	
	local headarray = __head_char_array(head,cmd,length,timestr)
	-- deep and light is minutes , > 255
	local tailarray = {}
	local charary = get_single_char_from_int2(deep_val) -- deep sleep time
	for i=1,#charary,1 do
		table.insert(tailarray,string.char(string.byte(charary,i)))
	end
	charary = get_single_char_from_int2(light_val) -- light sleep time
	for i=1,#charary,1 do
		table.insert(tailarray,string.char(string.byte(charary,i)))
	end
	
	local chartb = {}
	for i,v in pairs(headarray) do
		table.insert(chartb,v)
	end
	for i,v in pairs(tailarray) do
		table.insert(chartb,v)
	end
	
	local ret_day_of_month = {}
	table.insert(ret_day_of_month, chartb)
	
	return ret_day_of_month,"OK"
end

local function _parse_day_common(attr_array,time_array,athour)
	if(not attr_array or type(attr_array) ~= "table") then
		return 0x00,"key sub_report.attr array is error"
	end
	if(not time_array or type(time_array) ~= "table") then
		return 0x00,"key sub_report.time array is error"
	end
	if(#attr_array ~= #time_array) then
		return 0x00,"move array len != time array len"
	end
	-- move at time
	local rate = 0x00
	for i,v in pairs(time_array) do
		if(v == athour) then
			rate = attr_array[i]
			break
		end
	end
	
	return rate,"+OK"
end

local function _parse_day_heart(luaret,head,cmd,length,timestr,athour)
	local subrpt = luaret.sub_report and luaret.sub_report[1]
	local heart_array = subrpt["heart_beat"] and subrpt["heart_beat"]["num"]
	local time_array = subrpt["heart_beat"] and subrpt["heart_beat"]["time"]
	local rate,msg = _parse_day_common(heart_array,time_array,athour)
	if(not rate) then 
		LOG2SYS("sub_report.heart_beat or .time array is error")
		rate = 0x00
	end 
	
	local headarray = __head_char_array(head,cmd,length,timestr)
	-- heart at time
	LOG2SYS("Debug get heart_beat " .. rate .. " at time " .. athour)
	
	local tailarray = {}
	table.insert(tailarray,string.char(athour))     --hour
	local charary = get_single_char_from_int2(rate) -- rate at time
	for i=1,#charary,1 do
		table.insert(tailarray,string.char(string.byte(charary,i)))
	end
	
	local chartb = {}
	for i,v in pairs(headarray) do
		table.insert(chartb,v)
	end
	for i,v in pairs(tailarray) do
		table.insert(chartb,v)
	end
	
	local ret_heart_at_hour = {}
	table.insert(ret_heart_at_hour,chartb)
	
	return ret_heart_at_hour,"OK"
end

local function _parse_day_breath(luaret,head,cmd,length,timestr,athour)
	local subrpt = luaret.sub_report and luaret.sub_report[1]
	local breath_array = subrpt["breath"] and subrpt["breath"]["num"]
	local time_array = subrpt["breath"] and subrpt["breath"]["time"]
	local rate,msg = _parse_day_common(breath_array,time_array,athour)
	if(not rate) then 
		LOG2SYS("sub_report.breath or .time array is error")
		rate = 0x00
	end 

	local headarray = __head_char_array(head,cmd,length,timestr)
	-- breath at time
	LOG2SYS("Debug get Breath Rate " .. rate .. " at time " .. athour)
	
	local tailarray = {}
	table.insert(tailarray,string.char(athour))     --hour
	local charary = get_single_char_from_int2(rate) -- breath at time
	for i=1,#charary,1 do
		table.insert(tailarray,string.char(string.byte(charary,i)))
	end
	
	local chartb = {}
	for i,v in pairs(headarray) do
		table.insert(chartb,v)
	end
	for i,v in pairs(tailarray) do
		table.insert(chartb,v)
	end
	
	local ret_breath_at_hour = {}
	table.insert(ret_breath_at_hour,chartb)
	
	return ret_breath_at_hour,"OK"
end

local function _parse_day_movement(luaret,head,cmd,length,timestr,athour)
	local subrpt = luaret.sub_report and luaret.sub_report[1]
	local move_array = subrpt["move"] and subrpt["move"]["num"]
	local time_array = subrpt["move"] and subrpt["move"]["time"]
	local rate,msg = _parse_day_common(move_array,time_array,athour)
	if(not rate) then 
		LOG2SYS("sub_report.move or .time array is error")
		rate = 0x00
	end 

	local headarray = __head_char_array(head,cmd,length,timestr)
	-- move at time
	LOG2SYS("Debug get move Rate " .. rate .. " at time " .. athour)
	
	local tailarray = {}
	table.insert(tailarray,string.char(athour))     --hour
	local charary = get_single_char_from_int2(rate) -- move at time
	for i=1,#charary,1 do
		table.insert(tailarray,string.char(string.byte(charary,i)))
	end
	
	local chartb = {}
	for i,v in pairs(headarray) do
		table.insert(chartb,v)
	end
	for i,v in pairs(tailarray) do
		table.insert(chartb,v)
	end
	
	local ret_move_at_hour = {}
	table.insert(ret_move_at_hour,chartb)
	
	return ret_move_at_hour,"OK"
end

local function check_key_keys(luaret)
	local subrpt = luaret.sub_report and luaret.sub_report[1]
	if not subrpt or type(subrpt) ~= "table" then return nil,"sub_report key err" end
	return true,"checkkey ok"
end

local function err_resp_hex_2uart()
	local chartb = {0x55,0x06,0xee,0xee} -- 4 Bytes, crc in .c send
	table.foreachi(chartb,function(i,v) chartb[i] = string.char(v) end)
	local resp = table.concat(chartb,"")
	comwrite("/dev/ttyS2",resp,#resp)
end

local function parse_report_json(content,cmd,num)
	local luaval = json.decode(content)
	local key,uartresp
	local ret,msg
	if(luaval == nil) then
		err_resp_hex_2uart()
		return nil,"Json Format error"
	else
		local reporttime = string_split(luaval["report_date"],"-")
		if(not reporttime or #reporttime ~= 3) then
			err_resp_hex_2uart()
			return nil,"Json KEY report_date error!"
		end
		for _,v in pairs(reporttime) do
			-- 0x270f = 9999 ,this code can only run util 9999 year! 
			-- Can you review this code at 9999 year ? HaHaHa...
			if(tonumber(v) > 0x270f) then
				err_resp_hex_2uart()
				return nil,"Json report_date value error!"
			end
		end
	
		-- check some key keys
		ret,msg = check_key_keys(luaval)
		if(not ret) then 
			err_resp_hex_2uart()
			return nil,"JsonErr" .. msg
		end
		-- check end
		
		if(cmd == 0x02) then
			-- month data
			return _parse_month_report(luaval,0x55,0xa2,0x0d,reporttime)
		elseif(cmd == 0x03) then
			-- day circle
			return _parse_day_circle(luaval,reporttime,num)
		elseif(cmd == 0x04) then
			-- day heart
			return _parse_day_heart(luaval,0x55,0xa4,0x0c,reporttime,num)
		elseif(cmd == 0x05) then
			-- day breath
			return _parse_day_breath(luaval,0x55,0xa5,0x0c,reporttime,num)
		elseif(cmd == 0x06) then
			-- dat movement
			return _parse_day_movement(luaval,0x55,0xa6,0x0c,reporttime,num)
		end
	end
	return uartresp,msg
end

local function find_dayfile_in_sdcard(year,month,day)
	if(not year or not month or not day) then return nil,"date nil" end
	
	local path = FILE_PREFIX .. FILE_NAME_PRE .. year .. "-" .. month .. "-" .. day
	LOG2SYS("Debug:find day file:" .. path)
	return path
end

local function test_sub_report_common(year,month,day,cmd,num)
	local sdfile = find_dayfile_in_sdcard(year,month,day)
	local filecont = readfile(sdfile)
	if(not filecont) then 
		err_resp_hex_2uart()
		return nil,sdfile .. " File Not exist" 
	end
	
	local ret,msg = parse_report_json(filecont,cmd,num)
	if(not ret) then 
		LOG2SYS(msg)
		return nil,msg
	end
	LOG2SYS("retdata array type is " .. type(ret) .. " size is " .. #ret)
	
	for i,onedayresp in pairs(ret) do
		--print("day sleep in array type",type(onedayresp),onedayresp)
		if(not onedayresp or type(onedayresp) ~= "table") then
			LOG2SYS("retdata item array is error")
			return nil,"retdata item array is error"
		end
		LOG2SYS("inx " .. i .. " single item length is " .. #onedayresp)
		local hexbuf = table.concat(onedayresp,"")
		LOG2SYS("sent to uart len " .. #hexbuf)
		comwrite("/dev/ttyS2",hexbuf,#hexbuf)
		--[[
		local tmpcharary = {}
		--table.foreachi(onedayresp,function(i,v) tmpchar[i] = string.char(v) end) if v>255 then error
		--table.foreachi(onedayresp,function(i,v) print(i,v,string.format("%08x",v)) end)
		table.foreachi(onedayresp,function(i,v) table.insert(tmpcharary,string.format("%02x",string.byte(v))) end)
		local tmpstr = table.concat(tmpcharary,"")
		LOG2SYS("hex switch to char-char:")
		LOG2SYS("===>" .. tmpstr,"len is " .. #tmpstr) --80
		
		local tmphex = string2hex(tmpstr)
		comwrite("/dev/ttyS2",tmphex,#tmphex)
		--LOG2SYS("tmphex type is " .. type(tmphex)) --len 40
		--LOG2SYS("4char-item array to hex len is " .. #tmphex)
		--uart_write("/mnt/mmcblk0p1/leoman/sleep_data",tmphex)
		]]
	end
	return true,"Response to uart ok"
end

function parse_ui_cmd(hexcmd)
	LOG2SYS("parse_ui_cmd at lua-time:" .. os.clock())
	if(not hexcmd or string.len(hexcmd) < 5 ) then
		LOG2SYS("[LuaERR]Hex Length is " .. string.len(hexcmd))
		LOG2SYS("[LuaERR]Hex to string:" .. hex2string(hexcmd))
		return "+ERR: ui cmd error!"
	end
	
	local data = hexcmd
	--local length = string.byte(data,2)
	local cmd = string.byte(data,3)
	local length = string.len(hexcmd)
	local sdfile,resp_ret
	local year,year_h,year_l,tmp,month,day,num_hour
	
	if(cmd ~= 0x01 and length > 6) then
		year_h = string.byte(data,4)
		year_l = string.byte(data,5)
		month = string.byte(data,6)
		day = string.byte(data,7)
		num_hour = string.byte(data,8)
		if not num_hour then 
			return "+ERR:ui cmd num or hour error!"
		end
		
		tmp = string.format("0x%02x%02x",year_h,year_l)
		year = tonumber(tmp)
		--sdfile = find_dayfile_in_sdcard(year,month,day)
		--if(not sdfile) then return "+ERR:can not find file in sdcard!" end
	end
	
	if(cmd == 0x01) then
		-- runtime data
		-- handle in .c file
	elseif(cmd == 0x07) then
		-- check time
		local hour,minute,second
		hour = string.byte(data,8)
		minute = string.byte(data,9)
		second = string.byte(data,10)
		
		local settime = string.format("\'%04d-%d-%d %d:%d:%d\'",year,month,day,hour,minute,second)
		LOG2SYS("Debug: setting current time " .. settime)
		resp_ret = os.execute("/bin/date -s " .. settime)
		--date -s "2008-05-23 01:01:01"
		local tb = os.date("*t")
		local charary = get_single_char_from_int2(tb.year) -- move at time
		local chartb = {0x55,0x0c,0xa7,string.byte(charary,1),string.byte(charary,2),
						tb.month, tb.day, tb.hour, tb.min, tb.sec,}
		table.foreachi(chartb,function(i,v) chartb[i] = string.char(v) end)
		local resp = table.concat(chartb,"")
		--LOG2SYS("resp str " , resp) -- only first param is valid
		comwrite("/dev/ttyS2",resp,#resp)
		return "+OK:SetTime"
	else
		resp_ret,tmp = test_sub_report_common(year,month,day,cmd,num_hour)
	end
	if(not resp_ret) then
		return "+ERR:" .. tmp 
	else
		return "+OK:" .. tmp 
	end
end

function test_get_sleep_data(mode)
	test_sub_report_common("2017","4","17",0x03,mode)
end

function test_get_day_of_month_data()
	test_sub_report_common("2017","4","17",0x02,0x00)
end

function test_get_heart_at_hour(hour)
	test_sub_report_common("2017","4","17",0x04,hour)
end

function test_get_breath_at_hour(hour)
	test_sub_report_common("2017","4","17",0x05,hour)
end

function test_get_move_at_hour(hour)
	test_sub_report_common("2017","4","17",0x06,hour)
end

function test_all()
	print ("args:",arg[0],arg[1],arg[2],arg[3])
	if(arg[1] ~= nil ) then
		reportfile.file = arg[1]
	end

	comwrite(arg[2],"hello\n",6)

	print("===get sleep data ,on or leave bed:")
	test_get_sleep_data(0x01)
	print("\n===get sleep data ,sleep data(deep,ligth,rem,awake):")
	test_get_sleep_data(0x02)
	print("\n===get sleep data ,sleep score:")
	test_get_sleep_data(0x03)

	print("\n===get day of month:")
	test_get_day_of_month_data()

	--[[
	print("\n===get heart at time:")
	for i=0,23,1 do
		print("\n===get heart at time " .. i)
		test_get_heart_at_hour(i)
	end

	for i=0,23,1 do
		print("\n===get breath at time " .. i)
		test_get_breath_at_hour(i)
	end

	for i=0,23,1 do
		print("\n===get move at time " .. i)
		test_get_move_at_hour(i)
	end
	]]--
end

--test_all()



