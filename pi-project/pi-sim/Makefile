CC = gcc
CFLAGS = -Wall -O2 -g
LFLAGS = -lm
AR = ar

SDL_LIBS= `sdl-config --libs`
SDL_INCLUDE= `sdl-config --cflags`

all:	sparkle pi_opener chiptune mode7_demo vmw_open


####

sparkle:	sparkle.o svmwgraph.o pi-sim.o apple2.o
	$(CC) -o sparkle sparkle.o svmwgraph.o pi-sim.o apple2.o \
		$(SDL_LIBS) $(LFLAGS)

sparkle.o:	sparkle.c svmwgraph.h pi-sim.h
	$(CC) $(CFLAGS) -c sparkle.c

####

chiptune:	chiptune.o ay8912.o pt3_lib.o
	$(CC) -o chiptune ay8912.o chiptune.o pt3_lib.o $(LFLAGS)

####

chiptune.o:	chiptune.c pt3_lib.h
	$(CC) $(CFLAGS) -c chiptune.c

ay8912.o:	ay8912.c ayemu.h
	$(CC) $(CFLAGS) -c ay8912.c

pt3_lib.o:	pt3_lib.c pt3_lib.h
	$(CC) $(CFLAGS) -c pt3_lib.c

####

mode7_demo:	mode7_demo.o pi-sim.o svmwgraph.o apple2.o
	$(CC) -o mode7_demo mode7_demo.o svmwgraph.o apple2.o\
		pi-sim.o $(SDL_LIBS) $(LFLAGS)

mode7_demo.o:	mode7_demo.c svmwgraph.h pi-sim.h
	$(CC) $(CFLAGS) -c mode7_demo.c

####

vmw_open:	vmw_open.o pi-sim.o svmwgraph.o default_font.o print_string.o
	$(CC) -o vmw_open vmw_open.o svmwgraph.o default_font.o print_string.o \
		pi-sim.o $(SDL_LIBS) $(LFLAGS)

vmw_open.o:	vmw_open.c pi-sim.h
	$(CC) $(CFLAGS) -c vmw_open.c


####

pi_opener:	pi_opener.o pcx_load.o default_font.o \
		pi_boot.o pi-sim.o print_string.o svmwgraph.o
	$(CC) -o pi_opener pi_opener.o pcx_load.o \
		pi_boot.o print_string.o default_font.o \
		pi-sim.o svmwgraph.o $(SDL_LIBS) $(LFLAGS)

pi_opener.o:	pi_opener.c pi-sim.h
	$(CC) $(CFLAGS) -c pi_opener.c


####

pi_boot.o:	pi_boot.pcx
	ld -r -b binary pi_boot.pcx -o pi_boot.o

####

#pi-sim.a:	pi-sim.o apple2.o lowlevel_graphics.o print_string.o default_font.o
#	$(AR) crs pi-sim.a pi-sim.o apple2.o lowlevel_graphics.o print_string.o default_font.o


###

pi-sim.o:	pi-sim.c pi-sim.h
	$(CC) $(CFLAGS) $(SDL_INCLUDE) -c pi-sim.c

svmwgraphics.o:	svmwgraphics.c svmwgraphics.h
	$(CC) $(CFLAGS) $(SDL_INCLUDE) -c svmwgraphics.c

apple2.o:	apple2.c pi-sim.h
	$(CC) $(CFLAGS) $(SDL_INCLUDE) -c apple2.c

pcx_load.o:	pcx_load.c pcx_load.h svmwgraph.h
	$(CC) $(CFLAGS) -c pcx_load.c

print_string.o:	print_string.c default_font.h
	$(CC) $(CFLAGS) -c print_string.c

default_font.o:	default_font.c default_font.h
	$(CC) $(CFLAGS) -c default_font.c

clean:
	rm -f chiptune sparkle pi_opener vmw_open mode7_demo *~ *.o *.a

