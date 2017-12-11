CFLAGS=-O2 -g -Wall
LIBS=-lrt -lpthread

CFILES_3=lift_3.c main_3.c si_ui.c si_comm.c draw_3.c debug.c
HEADERS_3=lift_3.h si_ui.h si_comm.h draw_3.h debug.h

CFILES_3_opt=lift_3_opt.c main_3_opt.c si_ui.c si_comm.c draw_3.c debug.c
HEADERS_3_opt=lift_3_opt.h si_ui.h si_comm.h draw_3.h debug.h

CFILES_3_mod=lift_3_mod.c main_3_mod.c si_ui.c si_comm.c draw_3.c debug.c
HEADERS_3_mod=lift_3_mod.h si_ui.h si_comm.h draw_3.h debug.h

CFILES_3_mod_opt=lift_3_mod_opt.c main_3_mod_opt.c si_ui.c si_comm.c draw_3.c debug.c
HEADERS_3_mod_opt=lift_3_mod_opt.h si_ui.h si_comm.h draw_3.h debug.h



CFILES_4=lift_4.c main_4.c si_ui.c si_comm.c draw_4.c messages.c
HEADERS_4=lift_4.h si_ui.h si_comm.h draw_4.h messages.h

CFILES_4_mod=lift_4_mod.c main_4_mod.c si_ui.c si_comm.c draw_4.c messages.c
HEADERS_4_mod=lift_4_mod.h si_ui.h si_comm.h draw_4.h messages.h

3: $(CFILES_3) $(HEADERS_3)
	gcc $(CFLAGS) $(CFILES_3) -o 3 $(LIBS)

3_opt: $(CFILES_3_opt) $(HEADERS_3_opt)
	gcc $(CFLAGS) $(CFILES_3_opt) -o 3_opt $(LIBS)

3_mod: $(CFILES_3_mod) $(HEADERS_3_mod)
	gcc $(CFLAGS) $(CFILES_3_mod) -o 3_mod $(LIBS)

3_mod_opt: $(CFILES_3_mod_opt) $(HEADERS_3_mod_opt)
	gcc $(CFLAGS) $(CFILES_3_mod_opt) -o 3_mod $(LIBS)



4: $(CFILES_4) $(HEADERS_4)
	gcc $(CFLAGS) $(CFILES_4) -o 4 $(LIBS)

4_mod: $(CFILES_4_mod) $(HEADERS_4_mod)
	gcc $(CFLAGS) $(CFILES_4_mod) -o 4_mod $(LIBS)

clean:
	rm -f 3 *~ *.o
	rm -f 3_mod *~ *.o
	rm -f 3_opt *~ *.o
	rm -f 4 *~ *.o
	rm -f 4_mod *~ *.o
	rm -f 4_opt *~ *.o
