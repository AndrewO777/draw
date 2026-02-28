include config.mk

OBJS=draw.c
CC=gcc
COMPILER_FLAGS=-Wall -Wextra
LINKER_FLAGS=-lSDL3
all: draw
draw: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o draw -Ofast
debug: $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) -g $(LINKER_FLAGS) -o draw
clean:
	rm -f draw
install: draw
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	rm -f $(DESTDIR)$(PREFIX)/bin/draw
	cp -f draw $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/draw
