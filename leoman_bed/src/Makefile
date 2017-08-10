LEOMAN_BED = leoman_bed

LEOM_CLUASO	= leomclualib.so
BED_OBJ = leom_dbg.o leom_utils.o leom_obj.o leom_uart.o leom_sensor_handle.o leoman.o
		# cjson.o net_json.o net_base.o

UCITST	= uci_test
UCI_OBJ	= uci_test.o

BED_CFLAGS       = $(CFLAGS) -Wall -std=gnu99 
BED_LDFLAGS      = $(LDFLAGS) -lm -lpthread

all: clean  $(LEOMAN_BED) $(LEOM_CLUASO)

$(UCITST):$(UCI_OBJ)
	$(CC) -o $@ $^ -luci
	
$(LEOMAN_BED):$(BED_OBJ)
	$(CC) -o $@ $^ $(BED_LDFLAGS) -llua -lubox
	
$(LEOM_CLUASO):leom_dbg.o leom_clua.o
	$(CC) $(LDFLAGS) -shared -o $@ $^  -llua -lm -ldl
	
$(CLI_UART):cli_uart.o
	$(CC) -o $@ $^ -lpthread
	
%.o : %.c
	$(CC) $(BED_CFLAGS) -fPIC -c $< -o $@

# remove object files and executable when user executes "make clean"
clean:
	rm -rf *.[oa]  $(LEOMAN_BED) $(UCITST) $(LEOM_CLUASO) *.~ *~





