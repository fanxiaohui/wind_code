#############################################################
# Makefile for static library.
# make ikcc
#############################################################
#set your own environment option
CC = gcc
C_FLAG += -D_NOMNG -D_FILELINE 

ifeq ($(system), linux)
	LIB += -L../lib/ubuntux64/
	
	EX_FLAG += -lgatewaysdk -lXlinkV4Sdk -lsqlite 
	EX_FLAG += -lpthread -lcjson -llua -lm -ldl -lcurl \
			-Wl,-E	#lua
	
	X86_DEBUG = 1
else 
	C_FLAG += -DOPENWRT -DSYSLOG
	EX_FLAG += -lpthread -lcjson -llua -lm -ldl -Wl,-E
	LIB += -L../lib/mipsel/
	EX_FLAG += -lXlinkV4Sdk -lsqlite -lcurl
endif

 



#JSP_EX_FLAG += -lubox -ljson_script -lblobmsg_json -ljson-c
C_FLAG += -g -c  -O2 #-Werror
C_FLAG += -DLUA_SCRIPT
CFLAGS += $(C_FLAG)

#static library use 'ar' command 
AR = ar

#set your inc and lib
INC = -I ../include/inner\
	-I ../include/common\
	-I ../include/platform/linux\
	-I ../include/thirdparty\
	-I ../src/xlinkgateway\
	-I ../src/xlinkgateway/sqlite\
	-I ../src/xlinkgateway/src\
	-I ../src/xlinkgateway/xlink\
	-I ../src/xlinkgateway/xlink/ic_head\
	-I ../include/inner\
#	-I /usr/include/lua5.1
#LIB = -lpthread -L./ -lsvrtool

#make target lib and relevant obj 
PRG = ../ikcc_service #ikcc_service.a
SRC=$(wildcard ../src/*.c ../src/tool/*.c ../src/xlinkgateway/*.c ../src/xlinkgateway/sqlite/*.c ../src/xlinkgateway/src/*.c)
#SRC=$(wildcard ../src/*.c ../src/tool/*.c ../src/jsp_trigger/*.c)
OBJ=$(patsubst %.c,%.o,$(notdir $(SRC)))


all:$(PRG)
$(PRG):$(OBJ)
	$(CC) $(OBJ) $(EX_FLAG) $(LIB) -o $(PRG) 
$(OBJ):$(SRC)
	$(CC) $(CFLAGS) $(SRC) $(EX_FLAG) $(INC) 

	
.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	rm -f $(OBJ) $(PRG)
