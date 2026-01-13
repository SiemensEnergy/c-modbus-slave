CC := gcc
LD := gcc

LIB_SRC := \
	mbadu_tcp.c \
	mbcoil.c \
	mbcrc.c \
	mbfn_coils.c \
	mbfn_dags.c \
	mbfn_regs.c \
	mbfn_serial.c \
	mbinst.c \
	mbpdu.c \
	mbreg.c \
	endian.c

SRC := ${wildcard *.c}
OBJ := ${SRC:.c=.o} ${LIB_SRC:.c=.o}

CFLAGS := -std=c11 -I../../src -Wall -Wextra -Wpedantic

LDFLAGS :=

.PHONY: all clean

all: server

clean:
	@rm -rf *.o server

server: ${OBJ}
	${LD} -o $@ ${LDFLAGS} $^

%.o: %.c
	${CC} -o $@ ${CFLAGS} -c $<

%.o: ../../src/%.c
	${CC} -o $@ ${CFLAGS} -c $<
