include config.mk

PROG = kmx_doom
OBJECTS = src/kmx_doom.o

all: ${PROG}

${PROG}: ${OBJECTS}
	${CC} ${CFLAGS} -o ${PROG} ${OBJECTS} ${LDFLAGS} ${LIBS}

src/kmx_doom.o: src/kmx_doom.c config.mk
	${CC} ${CPPFLAGS} ${CFLAGS} -c src/kmx_doom.c -o src/kmx_doom.o

run: ${PROG}
	KC3_DIR=${KC3} KMX_DOOM_KC3=kc3/doom.kc3 ./${PROG}

clean:
	rm -f ${PROG} ${OBJECTS}

distclean: clean
	rm -f config.mk

.PHONY: all clean distclean run
