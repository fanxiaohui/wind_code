CC ?= gcc
C_FLAG += -D_NOMNG -D_FILELINE 

EX_FLAG += -lpthread

LIB += -L../../lib/ubuntux64/
#LIB += -L ./mipse
EX_FLAG +=$(LIB) -lsqlite -ldl
#-lgatewaysdk   -lXlinkV4Sdk 



#JSP_EX_FLAG += -lubox -ljson_script -lblobmsg_json -ljson-c
C_FLAG += -g -c  -O2 
CFLAGS += $(C_FLAG)

#static library use 'ar' command 

#set your inc and lib
INC = -I ./sqlite\
	-I ../../include/inner\
	-I ../../include/common\
	-I ../../include/platform/linux\
	-I ../../include/thirdparty\
	-I ../../src/xlinkgateway\
	-I ../../src/xlinkgateway/sqlite\
	-I ../../src/xlinkgateway/src\
	-I ../../src/xlinkgateway/xlink\
	-I ../../src/xlinkgateway/xlink/ic_head\
	
#	-I /usr/include/lua5.1
#LIB = -lpthread -L./ -lsvrtool

#make target lib and relevant obj 
PRG = ./ikcc_service

SRC=$(wildcard ./*.c ./sqlite/*.c ./src/*.c ./xlink/*.c)
#SRC=$(wildcard ../src/*.c ../src/tool/*.c ../src/jsp_trigger/*.c)
OBJ=$(patsubst %.c,%.o,$(notdir $(SRC)))

all:$(PRG)
$(PRG):$(OBJ)
	$(CC) $(OBJ) $(EX_FLAG) -o $(PRG) 
$(OBJ):$(SRC)
	$(CC) $(CFLAGS) $(SRC) $(EX_FLAG) $(INC) 

	
.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	rm -f $(OBJ) $(PRG)
