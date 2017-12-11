CFLAGS=-O2 -g -Wall
LIBS=-lrt -lpthread

CFILES_3=lift_3.c main_3.c si_ui.c si_comm.c draw_3.c debug.c
HEADERS_3=lift_3.h si_ui.h si_comm.h draw_3.h debug.h

CFILES_4=lift_4.c main_4.c si_ui.c si_comm.c draw_4.c messages.c
HEADERS_4=lift_4.h si_ui.h si_comm.h draw_4.h messages.h

3: $(CFILES_3) $(HEADERS_3)
	gcc $(CFLAGS) $(CFILES_3) -o 3 $(LIBS)


4: $(CFILES_4) $(HEADERS_4)
	gcc $(CFLAGS) $(CFILES_4) -o 4 $(LIBS)

clean:
	rm -f 3 *~ *.o
	rm -f 4 *~ *.o
