
# if you're not using background pixmaps, remove -lXpm from the next line
LIBS	=  -L/usr/X11/lib -lXpm -lXext -lX11 -lXmu -lm

CC	= gcc
CCC	= gcc
CFLAGS	= -O2
OBJECTS	= Border.o Buttons.o Channel.o Client.o Events.o Main.o Manager.o Menu.o Rotated.o

.c.o:
	$(CC) -c $(CFLAGS) $<

.C.o:
	$(CCC) -c $(CFLAGS) $<

wmx:	$(OBJECTS)
	mv -f wmx wmx.old >& /dev/null || true
	$(CCC) -o wmx $(OBJECTS) $(LIBS)

depend:
	makedepend -- $(CFLAGS) -- *.C

clean:
	rm -f *.o core

distclean: clean
	rm -f wmx wmx.old *~

