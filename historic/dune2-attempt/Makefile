TARGET=dune2

OBJS= \
	dune2.o \


CFLAGS= -Wall -I. -g -O0 $(shell sdl-config --cflags)
LIBS=$(shell sdl-config --static-libs) -lSDL_image

%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

all: ${TARGET}

.PHONY: clean ${TARGET} remake

${TARGET}: ${OBJS}
	${CC} ${CFLAGS} ${LIBS} ${OBJS} -o ${TARGET}


clean:
	rm -f ${TARGET}
	rm -f *.o

remake: clean ${TARGET}

