CC = arm-none-linux-gnueabi-gcc
LD = ${CC}

CFLAGS = -Wall -static -O2 -march=armv7-a -mcpu=cortex-a8 -mfpu=neon-fp16 -ftree-vectorize
LDFLAGS = ${CFLAGS}
LIBS = -ltomcrypt

SYMBOLS = -DUSE_SYSEXITS

SOURCES = flashgc.c gopt.c gcgen.c
OBJECTS = $(addsuffix .o, $(basename ${SOURCES}))
TARGET_BIN = $(word 1, $(basename ${SOURCES}))

all: ${TARGET_BIN}

%.o: %.c
	${CC} -c ${CFLAGS} ${SYMBOLS} -o $@ $<

${TARGET_BIN}: ${OBJECTS}
	${LD} ${LDFLAGS} -o $@ $(OBJECTS) ${LIBS}

clean:
	rm -f *.o *~ *.out ${TARGET_BIN}
