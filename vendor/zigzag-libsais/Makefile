CC ?= gcc
OPT ?= -O3 -march=native -fomit-frame-pointer -funroll-loops
OMP ?= -fopenmp -DLIBSAIS_OPENMP
CFLAGS = $(OPT) $(OMP)

.PHONY: all clean

all: libsais.o libsais64.o

libsais.o: libsais/libsais.c libsais/libsais.h
	$(CC) $(CFLAGS) -c -o $@ libsais/libsais.c

libsais64.o: libsais/libsais64.c libsais/libsais64.h libsais/libsais.h
	$(CC) $(CFLAGS) -c -o $@ libsais/libsais64.c

clean:
	rm -f libsais.o libsais64.o
