CC= gcc
CFLAGS= -std=c99 -Wall -O3

LFLAGS= libmcbsp1.2.0.a -pthread -lrt -lm
#For MacOS use instead:
#LFLAGS= libmcbsp1.2.0.a -lm

OBJIP= bspinprod.o bspedupack.o
OBJBEN= bspbench.o bspedupack.o
OBJSORT= bspsort_test.o bspsort.o bspedupack.o

all: inprod bench sort

inprod: $(OBJIP)
	$(CC) $(CFLAGS) -o inprod $(OBJIP) $(LFLAGS)

bench: $(OBJBEN)
	$(CC) $(CFLAGS) -o bench $(OBJBEN) $(LFLAGS)

sort: $(OBJSORT)
	$(CC) $(CFLAGS) -o sort $(OBJSORT) $(LFLAGS)

bspinprod.o:  bspedupack.h
bspbench.o:   bspedupack.h
bspsort.o:    bspedupack.h
bspedupack.o: bspedupack.h

.PHONY: clean
clean:
	rm -f $(OBJIP) $(OBJBEN) $(OBJSORT) inprod  bench sort
