include config.mk

PROG = kmx_doom
OBJECTS = src/kmx_doom.o src/bridge.o src/engine.o src/textures.o \
	src/hud.o

all: ${PROG}

${PROG}: ${OBJECTS}
	${CC} ${CFLAGS} -o ${PROG} ${OBJECTS} ${LDFLAGS} ${LIBS}

src/kmx_doom.o: src/kmx_doom.c src/bridge.h src/engine.h src/hud.h config.mk
	${CC} ${CPPFLAGS} ${CFLAGS} -Isrc -c src/kmx_doom.c -o src/kmx_doom.o

src/bridge.o: src/bridge.c src/bridge.h src/engine.h src/hud.h config.mk
	${CC} ${CPPFLAGS} ${CFLAGS} -Isrc -c src/bridge.c -o src/bridge.o

src/engine.o: src/engine.c src/engine.h config.mk
	${CC} ${CPPFLAGS} ${CFLAGS} -Isrc -c src/engine.c -o src/engine.o

src/textures.o: src/textures.c src/engine.h config.mk
	${CC} ${CPPFLAGS} ${CFLAGS} -Isrc -c src/textures.c -o src/textures.o

src/hud.o: src/hud.c src/hud.h src/engine.h config.mk
	${CC} ${CPPFLAGS} ${CFLAGS} -Isrc -c src/hud.c -o src/hud.o

run: ${PROG}
	./${PROG}

clean:
	rm -f ${PROG} ${OBJECTS}

distclean: clean
	rm -f config.mk

.PHONY: all clean distclean run
