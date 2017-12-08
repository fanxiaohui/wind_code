
EXE=salepos
all: $(EXE)

$(EXE): fj_main.o uart.o fj_pro.o pos_pro.o handle.o debug.o utils.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf *.o $(EXE)