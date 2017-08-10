LEOMAN_BED = tcptty

LEOM_CLUASO	= leomclualib.so
BED_OBJ = leom_dbg.o leom_utils.o leom_pro.o leom_uart.o leom_sensor_handle.o leom_net_mch.o leoman.o
		# cjson.o net_json.o net_base.o

UCITST	= leom_uci
UCI_OBJ	= leom_uci.o

BED_CFLAGS       = $(CFLAGS) -Wall -std=gnu99 
BED_LDFLAGS      = $(LDFLAGS) -lm -lpthread

all: clean $(LEOMAN_BED)

$(UCITST):$(UCI_OBJ)
	$(CC) -o $@ $^ -luci

$(LEOMAN_BED):$(BED_OBJ)
	$(CC) -o $@ $^ $(BED_LDFLAGS) -llua -lubox
	
$(LEOM_CLUASO):leom_dbg.o leom_clua.o
	$(CC) $(LDFLAGS) -shared -o $@ $^  -llua -lm -ldl

%.o : %.c
	$(CC) $(BED_CFLAGS) -fPIC -c $< -o $@

# remove object files and executable when user executes "make clean"
clean:
	rm -rf *.[oa]  $(LEOMAN_BED) $(UCITST) $(LEOM_CLUASO) *.~ *~





