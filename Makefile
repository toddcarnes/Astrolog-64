# Astrolog (Version 6.10) File: Makefile (Unix version)
#
# IMPORTANT NOTICE: Astrolog and all chart display routines and anything
# not enumerated elsewhere in this program are Copyright (C) 1991-2016 by
# Walter D. Pullen (Astara@msn.com, http://www.astrolog.org/astrolog.htm).
# Permission is granted to freely use, modify, and distribute these
# routines provided these credits and notices remain unmodified with any
# altered or distributed versions of the program.
#
# First created 11/21/1991.
#
# This Makefile is included only for convenience. One could easily compile
# Astrolog on a Unix system by hand with the command:
# % cc -c -O -Wno-write-strings *.cpp; cc -o astrolog *.o -lm -lX11
# Generally, all that needs to be done to compile once astrolog.h has been
# edited, is compile each source file, and link them together with the math
# library, and if applicable, the main X library.
#
NAME = astrolog
OBJ = astrolog.o data.o data2.o general.o io.o\
 calc.o matrix.o placalc.o placalc2.o\
 charts0.o charts1.o charts2.o charts3.o intrpret.o\
 xdata.o xgeneral.o xdevice.o xcharts0.o xcharts1.o xcharts2.o xscreen.o\
 swedate.o swehouse.o swejpl.o swemmoon.o swemplan.o swemptab.o sweph.o\
 swephlib.o

# If you don't have X windows, delete the "-lX11" part from the line below:
LIBS = -lm -lX11
CFLAGS = -O

astrolog:: $(OBJ)
	cc -o $(NAME) $(OBJ) $(LIBS)
	strip $(NAME)
#
