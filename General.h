
#ifndef _GENERAL_H_
#define _GENERAL_H_
#define CHECKPOINT fprintf(stderr,"%s:%d\n",__FILE__,__LINE__);

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#ifdef _POSIX_SOURCE
#undef _POSIX_SOURCE
#endif

#ifndef __FreeBSD__
#define _POSIX_SOURCE 1
#endif

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <X11/extensions/shape.h>

// True and False are defined in Xlib.h
typedef char Boolean;

#define NewString(x) (strcpy((char *)malloc(strlen(x)+1),(x)))

#ifndef SIGNAL_CALLBACK_TYPE
//#define SIGNAL_CALLBACK_TYPE (void (*)(...))
#define SIGNAL_CALLBACK_TYPE (void (*)(int))
#endif

#define signal(x,y)     \
  do { \
    struct sigaction sAct; \
    (void)sigemptyset(&sAct.sa_mask); \
    sAct.sa_flags = 0; \
    sAct.sa_handler = (SIGNAL_CALLBACK_TYPE(y)); \
    (void)sigaction((x), &sAct, NULL); \
  } while (0)

#include "Config.h"

#if CONFIG_USE_SESSION_MANAGER != False
#include <X11/SM/SMlib.h>
#endif

class Atoms {
public:
    static Atom wm_state;
    static Atom wm_changeState;
    static Atom wm_protocols;
    static Atom wm_delete;
    static Atom wm_takeFocus;
    static Atom wm_colormaps;
    static Atom wmx_running;

#if CONFIG_GNOME_COMPLIANCE != False
    static Atom gnome_supportingWmCheck;
    static Atom gnome_protocols;
    static Atom gnome_clienList;
    static Atom gnome_workspace;
    static Atom gnome_workspaceCount;
    static Atom gnome_workspaceNames;
    static Atom gnome_winLayer;
    static Atom gnome_winDesktopButtonProxy;
#endif

};

extern Boolean ignoreBadWindowErrors; // tidiness hack

#define AllButtonMask	( Button1Mask | Button2Mask | Button3Mask \
			| Button4Mask | Button5Mask )
#define ButtonMask	( ButtonPressMask | ButtonReleaseMask )
#define DragMask        ( ButtonMask | ButtonMotionMask )
#define MenuMask	( ButtonMask | ButtonMotionMask | ExposureMask )
#define MenuGrabMask	( ButtonMask | ButtonMotionMask | StructureNotifyMask \
			  | SubstructureNotifyMask )

#endif
