TCPTTY = tcptty

BED_OBJ = leom_dbg.o leom_utils.o leom_pro.o leom_uart.o leom_sensor_handle.o leom_net_mch.o leoman.o

BED_CFLAGS       = $(CFLAGS) -Wall -std=gnu99 
BED_LDFLAGS      = $(LDFLAGS) -lm -lpthread

all: clean $(TCPTTY)

$(TCPTTY):$(BED_OBJ)
	$(CC) -o $@ $^ $(BED_LDFLAGS) -llua -lubox

%.o : %.c
	$(CC) $(BED_CFLAGS) -fPIC -c $< -o $@

# remove object files and executable when user executes "make clean"
clean:
	rm -rf *.[oa]  $(TCPTTY)  *.~ *~





