#Francesco Falcon Sm3201408
#
# Makefile per compilare il raytracer in C con OpenMP e mmap

CC ?= gcc
CFLAGS ?= -O2 -std=c99 -Wall -Wextra -Wpedantic
LDFLAGS ?=

CFLAGS += -fopenmp
LDFLAGS += -fopenmp -lm

SRC = main.c scene.c ppm.c
OBJ = $(SRC:.c=.o)
BIN = raytracer

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c scene.h ppm.h
scene.o: scene.c scene.h
ppm.o: ppm.c ppm.h

clean:
	rm -f $(OBJ) $(BIN)

.PHONY: all clean
