CFLAGS	= -Wall -lasound `pkg-config --cflags --libs gtk+-3.0` -export-dynamic
RiseConfigGtk: main.o
	gcc $(CFLAGS) -o build/RiseConfigGtk build/main.o build/rise_ctl.o

main.o: main.c
	gcc $(CFLAGS) -o build/main.o -c main.c
