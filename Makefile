
CC	= gcc
CCC	= g++

# If you're not using background pixmaps, remove -lXpm from the LIBS.
# If your X libraries are somewhere other than /usr/X11/lib, give their
# location here.
LIBS	= -L/usr/X11/lib -lXpm -lXext -lX11 -lXmu -lSM -lICE -lm

# If your X includes are not in /usr/include/X11, add their location
# as a -I option here (excluding the X11 bit).  If you're using I18N
# and Xlocale, please add -DX_LOCALE.
CFLAGS = -g -O2 -I/usr/include -I/usr/openwin/include

OBJECTS	= Border.o Buttons.o Channel.o Client.o Config.o Events.o Main.o Manager.o Menu.o Rotated.o Session.o

.c.o:
	$(CC) -c $(CFLAGS) $<

.C.o:
	$(CCC) -c $(CFLAGS) $<

all:	depend wmx

wmx:	$(OBJECTS)
	test -f wmx && mv -f wmx wmx.old || true
	$(CCC) -o wmx $(OBJECTS) $(LIBS)

depend:
	-makedepend -- $(CFLAGS) -- *.C

clean:
	rm -f *.o core

distclean: clean
	rm -f wmx wmx.old *~ *.bak

# DO NOT DELETE

Border.o: Border.h General.h /usr/include/unistd.h /usr/include/features.h
Border.o: /usr/include/sys/cdefs.h /usr/include/posix_opt.h
Border.o: /usr/include/gnu/types.h
Border.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
Border.o: /usr/include/confname.h /usr/include/sys/types.h
Border.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
Border.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
Border.o: /usr/include/sys/bitypes.h /usr/include/sys/time.h
Border.o: /usr/include/linux/time.h /usr/include/sys/time.h
Border.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
Border.o: /usr/include/signal.h /usr/include/linux/signal.h
Border.o: /usr/include/asm/signal.h /usr/include/errno.h
Border.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
Border.o: /usr/include/stdlib.h /usr/include/alloca.h /usr/include/X11/X.h
Border.o: /usr/include/X11/Xlib.h /usr/include/X11/Xfuncproto.h
Border.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xos.h
Border.o: /usr/include/string.h /usr/include/fcntl.h
Border.o: /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h
Border.o: /usr/include/X11/Xutil.h /usr/include/X11/Xatom.h
Border.o: /usr/include/X11/extensions/shape.h Config.h
Border.o: /usr/include/X11/SM/SMlib.h /usr/include/X11/SM/SM.h
Border.o: /usr/include/X11/ICE/ICElib.h /usr/include/X11/ICE/ICE.h Rotated.h
Border.o: Client.h Manager.h listmacro.h /usr/include/assert.h
Border.o: /usr/include/X11/xpm.h background.xpm
Buttons.o: Manager.h General.h /usr/include/unistd.h /usr/include/features.h
Buttons.o: /usr/include/sys/cdefs.h /usr/include/posix_opt.h
Buttons.o: /usr/include/gnu/types.h
Buttons.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
Buttons.o: /usr/include/confname.h /usr/include/sys/types.h
Buttons.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
Buttons.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
Buttons.o: /usr/include/sys/bitypes.h /usr/include/sys/time.h
Buttons.o: /usr/include/linux/time.h /usr/include/sys/time.h
Buttons.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
Buttons.o: /usr/include/signal.h /usr/include/linux/signal.h
Buttons.o: /usr/include/asm/signal.h /usr/include/errno.h
Buttons.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
Buttons.o: /usr/include/stdlib.h /usr/include/alloca.h /usr/include/X11/X.h
Buttons.o: /usr/include/X11/Xlib.h /usr/include/X11/Xfuncproto.h
Buttons.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xos.h
Buttons.o: /usr/include/string.h /usr/include/fcntl.h
Buttons.o: /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h
Buttons.o: /usr/include/X11/Xutil.h /usr/include/X11/Xatom.h
Buttons.o: /usr/include/X11/extensions/shape.h Config.h
Buttons.o: /usr/include/X11/SM/SMlib.h /usr/include/X11/SM/SM.h
Buttons.o: /usr/include/X11/ICE/ICElib.h /usr/include/X11/ICE/ICE.h
Buttons.o: listmacro.h /usr/include/assert.h Client.h Border.h Rotated.h
Buttons.o: Menu.h /usr/include/X11/keysym.h /usr/include/X11/keysymdef.h
Channel.o: Manager.h General.h /usr/include/unistd.h /usr/include/features.h
Channel.o: /usr/include/sys/cdefs.h /usr/include/posix_opt.h
Channel.o: /usr/include/gnu/types.h
Channel.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
Channel.o: /usr/include/confname.h /usr/include/sys/types.h
Channel.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
Channel.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
Channel.o: /usr/include/sys/bitypes.h /usr/include/sys/time.h
Channel.o: /usr/include/linux/time.h /usr/include/sys/time.h
Channel.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
Channel.o: /usr/include/signal.h /usr/include/linux/signal.h
Channel.o: /usr/include/asm/signal.h /usr/include/errno.h
Channel.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
Channel.o: /usr/include/stdlib.h /usr/include/alloca.h /usr/include/X11/X.h
Channel.o: /usr/include/X11/Xlib.h /usr/include/X11/Xfuncproto.h
Channel.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xos.h
Channel.o: /usr/include/string.h /usr/include/fcntl.h
Channel.o: /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h
Channel.o: /usr/include/X11/Xutil.h /usr/include/X11/Xatom.h
Channel.o: /usr/include/X11/extensions/shape.h Config.h
Channel.o: /usr/include/X11/SM/SMlib.h /usr/include/X11/SM/SM.h
Channel.o: /usr/include/X11/ICE/ICElib.h /usr/include/X11/ICE/ICE.h
Channel.o: listmacro.h /usr/include/assert.h Client.h Border.h Rotated.h
Client.o: Manager.h General.h /usr/include/unistd.h /usr/include/features.h
Client.o: /usr/include/sys/cdefs.h /usr/include/posix_opt.h
Client.o: /usr/include/gnu/types.h
Client.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
Client.o: /usr/include/confname.h /usr/include/sys/types.h
Client.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
Client.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
Client.o: /usr/include/sys/bitypes.h /usr/include/sys/time.h
Client.o: /usr/include/linux/time.h /usr/include/sys/time.h
Client.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
Client.o: /usr/include/signal.h /usr/include/linux/signal.h
Client.o: /usr/include/asm/signal.h /usr/include/errno.h
Client.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
Client.o: /usr/include/stdlib.h /usr/include/alloca.h /usr/include/X11/X.h
Client.o: /usr/include/X11/Xlib.h /usr/include/X11/Xfuncproto.h
Client.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xos.h
Client.o: /usr/include/string.h /usr/include/fcntl.h
Client.o: /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h
Client.o: /usr/include/X11/Xutil.h /usr/include/X11/Xatom.h
Client.o: /usr/include/X11/extensions/shape.h Config.h
Client.o: /usr/include/X11/SM/SMlib.h /usr/include/X11/SM/SM.h
Client.o: /usr/include/X11/ICE/ICElib.h /usr/include/X11/ICE/ICE.h
Client.o: listmacro.h /usr/include/assert.h Client.h Border.h Rotated.h
Client.o: /usr/include/X11/keysym.h /usr/include/X11/keysymdef.h
Client.o: /usr/include/X11/Xmu/Atoms.h /usr/include/X11/Intrinsic.h
Client.o: /usr/include/X11/Xresource.h /usr/include/X11/Core.h
Client.o: /usr/include/X11/Composite.h /usr/include/X11/Constraint.h
Client.o: /usr/include/X11/Object.h /usr/include/X11/RectObj.h
Config.o: Config.h /usr/include/string.h /usr/include/features.h
Config.o: /usr/include/sys/cdefs.h
Config.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
Config.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
Config.o: /usr/include/ctype.h /usr/include/endian.h /usr/include/bytesex.h
Config.o: /usr/include/stdlib.h /usr/include/errno.h
Config.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
Config.o: /usr/include/alloca.h /usr/include/unistd.h
Config.o: /usr/include/posix_opt.h /usr/include/gnu/types.h
Config.o: /usr/include/confname.h /usr/include/sys/types.h
Config.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
Config.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
Config.o: /usr/include/sys/bitypes.h
Events.o: Manager.h General.h /usr/include/unistd.h /usr/include/features.h
Events.o: /usr/include/sys/cdefs.h /usr/include/posix_opt.h
Events.o: /usr/include/gnu/types.h
Events.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
Events.o: /usr/include/confname.h /usr/include/sys/types.h
Events.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
Events.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
Events.o: /usr/include/sys/bitypes.h /usr/include/sys/time.h
Events.o: /usr/include/linux/time.h /usr/include/sys/time.h
Events.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
Events.o: /usr/include/signal.h /usr/include/linux/signal.h
Events.o: /usr/include/asm/signal.h /usr/include/errno.h
Events.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
Events.o: /usr/include/stdlib.h /usr/include/alloca.h /usr/include/X11/X.h
Events.o: /usr/include/X11/Xlib.h /usr/include/X11/Xfuncproto.h
Events.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xos.h
Events.o: /usr/include/string.h /usr/include/fcntl.h
Events.o: /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h
Events.o: /usr/include/X11/Xutil.h /usr/include/X11/Xatom.h
Events.o: /usr/include/X11/extensions/shape.h Config.h
Events.o: /usr/include/X11/SM/SMlib.h /usr/include/X11/SM/SM.h
Events.o: /usr/include/X11/ICE/ICElib.h /usr/include/X11/ICE/ICE.h
Events.o: listmacro.h /usr/include/assert.h Client.h Border.h Rotated.h
Main.o: Manager.h General.h /usr/include/unistd.h /usr/include/features.h
Main.o: /usr/include/sys/cdefs.h /usr/include/posix_opt.h
Main.o: /usr/include/gnu/types.h
Main.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
Main.o: /usr/include/confname.h /usr/include/sys/types.h
Main.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
Main.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
Main.o: /usr/include/sys/bitypes.h /usr/include/sys/time.h
Main.o: /usr/include/linux/time.h /usr/include/sys/time.h
Main.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
Main.o: /usr/include/signal.h /usr/include/linux/signal.h
Main.o: /usr/include/asm/signal.h /usr/include/errno.h
Main.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
Main.o: /usr/include/stdlib.h /usr/include/alloca.h /usr/include/X11/X.h
Main.o: /usr/include/X11/Xlib.h /usr/include/X11/Xfuncproto.h
Main.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xos.h
Main.o: /usr/include/string.h /usr/include/fcntl.h /usr/include/linux/fcntl.h
Main.o: /usr/include/asm/fcntl.h /usr/include/X11/Xutil.h
Main.o: /usr/include/X11/Xatom.h /usr/include/X11/extensions/shape.h Config.h
Main.o: /usr/include/X11/SM/SMlib.h /usr/include/X11/SM/SM.h
Main.o: /usr/include/X11/ICE/ICElib.h /usr/include/X11/ICE/ICE.h listmacro.h
Main.o: /usr/include/assert.h Client.h Border.h Rotated.h
Manager.o: Manager.h General.h /usr/include/unistd.h /usr/include/features.h
Manager.o: /usr/include/sys/cdefs.h /usr/include/posix_opt.h
Manager.o: /usr/include/gnu/types.h
Manager.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
Manager.o: /usr/include/confname.h /usr/include/sys/types.h
Manager.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
Manager.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
Manager.o: /usr/include/sys/bitypes.h /usr/include/sys/time.h
Manager.o: /usr/include/linux/time.h /usr/include/sys/time.h
Manager.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
Manager.o: /usr/include/signal.h /usr/include/linux/signal.h
Manager.o: /usr/include/asm/signal.h /usr/include/errno.h
Manager.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
Manager.o: /usr/include/stdlib.h /usr/include/alloca.h /usr/include/X11/X.h
Manager.o: /usr/include/X11/Xlib.h /usr/include/X11/Xfuncproto.h
Manager.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xos.h
Manager.o: /usr/include/string.h /usr/include/fcntl.h
Manager.o: /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h
Manager.o: /usr/include/X11/Xutil.h /usr/include/X11/Xatom.h
Manager.o: /usr/include/X11/extensions/shape.h Config.h
Manager.o: /usr/include/X11/SM/SMlib.h /usr/include/X11/SM/SM.h
Manager.o: /usr/include/X11/ICE/ICElib.h /usr/include/X11/ICE/ICE.h
Manager.o: listmacro.h /usr/include/assert.h Menu.h Client.h Border.h
Manager.o: Rotated.h /usr/include/X11/Xlocale.h /usr/include/locale.h
Manager.o: /usr/include/X11/Xproto.h /usr/include/X11/Xmd.h
Manager.o: /usr/include/X11/Xprotostr.h /usr/include/sys/wait.h
Manager.o: /usr/include/waitflags.h /usr/include/waitstatus.h
Manager.o: /usr/include/endian.h /usr/include/bytesex.h Cursors.h
Manager.o: /usr/include/X11/cursorfont.h
Menu.o: Menu.h General.h /usr/include/unistd.h /usr/include/features.h
Menu.o: /usr/include/sys/cdefs.h /usr/include/posix_opt.h
Menu.o: /usr/include/gnu/types.h
Menu.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
Menu.o: /usr/include/confname.h /usr/include/sys/types.h
Menu.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
Menu.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
Menu.o: /usr/include/sys/bitypes.h /usr/include/sys/time.h
Menu.o: /usr/include/linux/time.h /usr/include/sys/time.h
Menu.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
Menu.o: /usr/include/signal.h /usr/include/linux/signal.h
Menu.o: /usr/include/asm/signal.h /usr/include/errno.h
Menu.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
Menu.o: /usr/include/stdlib.h /usr/include/alloca.h /usr/include/X11/X.h
Menu.o: /usr/include/X11/Xlib.h /usr/include/X11/Xfuncproto.h
Menu.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xos.h
Menu.o: /usr/include/string.h /usr/include/fcntl.h /usr/include/linux/fcntl.h
Menu.o: /usr/include/asm/fcntl.h /usr/include/X11/Xutil.h
Menu.o: /usr/include/X11/Xatom.h /usr/include/X11/extensions/shape.h Config.h
Menu.o: /usr/include/X11/SM/SMlib.h /usr/include/X11/SM/SM.h
Menu.o: /usr/include/X11/ICE/ICElib.h /usr/include/X11/ICE/ICE.h Manager.h
Menu.o: listmacro.h /usr/include/assert.h Client.h Border.h Rotated.h
Menu.o: /usr/include/dirent.h /usr/include/linux/limits.h
Menu.o: /usr/include/linux/dirent.h /usr/include/posix1_lim.h
Menu.o: /usr/include/sys/stat.h /usr/include/linux/stat.h
Menu.o: /usr/include/X11/keysym.h /usr/include/X11/keysymdef.h
Rotated.o: /usr/include/X11/Xlib.h /usr/include/sys/types.h
Rotated.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
Rotated.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
Rotated.o: /usr/include/sys/bitypes.h /usr/include/X11/X.h
Rotated.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
Rotated.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
Rotated.o: /usr/include/X11/Xutil.h /usr/include/stdlib.h
Rotated.o: /usr/include/features.h /usr/include/sys/cdefs.h
Rotated.o: /usr/include/errno.h /usr/include/linux/errno.h
Rotated.o: /usr/include/asm/errno.h /usr/include/alloca.h
Rotated.o: /usr/include/string.h /usr/include/stdio.h /usr/include/libio.h
Rotated.o: /usr/include/_G_config.h Config.h Rotated.h
Session.o: Manager.h General.h /usr/include/unistd.h /usr/include/features.h
Session.o: /usr/include/sys/cdefs.h /usr/include/posix_opt.h
Session.o: /usr/include/gnu/types.h
Session.o: /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h
Session.o: /usr/include/confname.h /usr/include/sys/types.h
Session.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
Session.o: /usr/include/asm/posix_types.h /usr/include/asm/types.h
Session.o: /usr/include/sys/bitypes.h /usr/include/sys/time.h
Session.o: /usr/include/linux/time.h /usr/include/sys/time.h
Session.o: /usr/include/stdio.h /usr/include/libio.h /usr/include/_G_config.h
Session.o: /usr/include/signal.h /usr/include/linux/signal.h
Session.o: /usr/include/asm/signal.h /usr/include/errno.h
Session.o: /usr/include/linux/errno.h /usr/include/asm/errno.h
Session.o: /usr/include/stdlib.h /usr/include/alloca.h /usr/include/X11/X.h
Session.o: /usr/include/X11/Xlib.h /usr/include/X11/Xfuncproto.h
Session.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xos.h
Session.o: /usr/include/string.h /usr/include/fcntl.h
Session.o: /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h
Session.o: /usr/include/X11/Xutil.h /usr/include/X11/Xatom.h
Session.o: /usr/include/X11/extensions/shape.h Config.h
Session.o: /usr/include/X11/SM/SMlib.h /usr/include/X11/SM/SM.h
Session.o: /usr/include/X11/ICE/ICElib.h /usr/include/X11/ICE/ICE.h
Session.o: listmacro.h /usr/include/assert.h
