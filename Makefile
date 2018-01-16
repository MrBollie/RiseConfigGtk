CFLAGS	= -Wall -lasound `pkg-config --cflags --libs gtk+-3.0` -export-dynamic

all: RiseConfigGtk
debug: CFLAGS += -ggdb
debug: RiseConfigGtk

RiseConfigGtk: rise_config.o
	gcc $(CFLAGS) -o build/RiseConfigGtk build/rise_config.o

rise_config.o: rise_config.c
	gcc $(CFLAGS) -o build/rise_config.o -c rise_config.c

clean:
	rm -f build/*
