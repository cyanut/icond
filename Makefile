#GLIB_CFLAGS=$(shell pkg-config --cflags glib-2.0)
#GLIB_LIBS=$(shell pkg-config --libs   glib-2.0)

EX_CFLAGS=$(shell pkg-config --cflags gtk+-2.0 gdk-pixbuf-2.0 librsvg-2.0)
EX_LIBS=$(shell pkg-config --libs gtk+-2.0 gdk-pixbuf-2.0 librsvg-2.0)
XLIB_CFLAGS=-I /usr/X11R6/include

XLIB_LIBS=-L /usr/X11R6/lib -lX11 -lXmu

LIBS=${EX_LIBS} ${XLIB_LIBS} 
CFLAGS= ${EX_CFLAGS} ${XLIB_CFLAGS}
DEBUG_CFLAGS = -g -Wall -pedantic ${CFLAGS}
PREFIX=/usr/
BINDER=$(PREFIX)/bin

all: xicond

debug: xicond-dbg

xicond-dbg: xicond-dbg.o
	gcc ${LIBS} $^ -o $@

xicond-dbg.o: xicond.c
	gcc ${DEBUG_CFLAGS} -c $^ -o $@

xicond.o: xicond.c
	gcc ${CFLAGS} -c $^ -o $@

xicond: xicond.o
	gcc ${LIBS} $^ -o $@

.PHONY: clean
clean:
	rm -f xicond.o xicond xicond-dbg xicond-dbg.o

.PHONY: install
install: xseticon
	install -d $(BINDIR)
	install -m 755 xseticon $(BINDIR)
