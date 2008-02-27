/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

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

#include "Config.h"

#include "listmacro.h"

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

#ifdef CONFIG_USE_XFT
#include <ft2build.h>
#include FT_FREETYPE_H 
#include FT_OUTLINE_H
#include FT_GLYPH_H
#include <X11/Xft/Xft.h>
#endif

#if CONFIG_USE_SESSION_MANAGER != False
#include <X11/SM/SMlib.h>
#endif

//!!! these replaced by atoms
#define WIN_STATE_STICKY          (1<<0) /*everyone knows sticky*/
#define WIN_STATE_MINIMIZED       (1<<1) /*Reserved - definition is unclear*/
#define WIN_STATE_MAXIMIZED_VERT  (1<<2) /*window in maximized V state*/
#define WIN_STATE_MAXIMIZED_HORIZ (1<<3) /*window in maximized H state*/
#define WIN_STATE_HIDDEN          (1<<4) /*not on taskbar but window visible*/
#define WIN_STATE_SHADED          (1<<5) /*shaded (MacOS / Afterstep style)*/
#define WIN_STATE_HID_WORKSPACE   (1<<6) /*not on current desktop*/
#define WIN_STATE_HID_TRANSIENT   (1<<7) /*owner of transient is hidden*/
#define WIN_STATE_FIXED_POSITION  (1<<8) /*window is fixed in position even*/
#define WIN_STATE_ARRANGE_IGNORE  (1<<9) /*ignore for auto arranging*/

#define WIN_HINTS_SKIP_FOCUS      (1<<0) /*"alt-tab" skips this win*/
#define WIN_HINTS_SKIP_WINLIST    (1<<1) /*do not show in window list*/
#define WIN_HINTS_SKIP_TASKBAR    (1<<2) /*do not show on taskbar*/
#define WIN_HINTS_GROUP_TRANSIENT (1<<3) /*Reserved - definition is unclear*/
#define WIN_HINTS_FOCUS_ON_CLICK  (1<<4) /*app only accepts focus if clicked*/
  
class Atoms {
public:
    static Atom wm_state;
    static Atom wm_changeState;
    static Atom wm_protocols;
    static Atom wm_delete;
    static Atom wm_takeFocus;
    static Atom wm_colormaps;
    static Atom wmx_running;

    static Atom netwm_supportingWmCheck;
    static Atom netwm_supported;
    static Atom netwm_clientList;
    static Atom netwm_clientListStacking;
    static Atom netwm_desktop;
    static Atom netwm_desktopCount;
    static Atom netwm_desktopNames;
    static Atom netwm_activeWindow;

    static Atom netwm_winLayer; //!!! obsolete
    static Atom netwm_winDesktopButtonProxy; //!!! what the hell?

    static Atom netwm_winHints; //!!! obsolete
    static Atom netwm_winState; //!!! meaning has changed (was int, now atoms)

    static Atom netwm_winDesktop;

    static Atom netwm_winType;

    static Atom netwm_winType_desktop; // desktop active background window
    static Atom netwm_winType_dock;    // dock or panel to remain on top
    static Atom netwm_winType_toolbar; // managed torn-off toolbar window
    static Atom netwm_winType_menu;    // managed torn-off menu window
    static Atom netwm_winType_utility; // small persistent palette or similar
    static Atom netwm_winType_splash;  // splash screen
    static Atom netwm_winType_dialog;  // dialog; default for managed transient
    static Atom netwm_winType_dropdown;// menu window (override-redirect)
    static Atom netwm_winType_popup;   // menu window (override-redirect)
    static Atom netwm_winType_tooltip; // tooltip (override-redirect)
    static Atom netwm_winType_notify;  // e.g. battery low (override-redirect)
    static Atom netwm_winType_combo;   // combobox menu (override-redirect)
    static Atom netwm_winType_dnd;     // dragged object (override-redirect)
    static Atom netwm_winType_normal;  // normal top-level window
};

/* These are the netwm window types that we actually care about. */

enum ClientType 
{

    /* netwm_winType_normal,
       or no hint and either override-redirect or no transient: */
    NormalClient,

    /* netwm_winType_dialog,
       or no hint and transient: */
    DialogClient,

    DesktopClient,  // netwm_winType_desktop
    DockClient,     // netwm_winType_dock
    ToolbarClient,  // netwm_winType_toolbar
    MenuClient,     // netwm_winType_menu
    UtilityClient,  // netwm_winType_utility
    SplashClient    // netwm_winType_splash
};

declareList(AtomList, Atom);

#define MAX_LAYER 13
#define DESKTOP_LAYER 0
// -- above this line, windows do not receive focus
// -- above this line, windows are borderless
#define NORMAL_LAYER 4
#define DIALOG_LAYER 6
// -- below this line, windows do not receive focus
#define TOOLBAR_LAYER 7
// -- below this line, windows are borderless
#define DOCK_LAYER 8
#define FULLSCREEN_LAYER 11

extern Boolean ignoreBadWindowErrors; // tidiness hack

#define AllButtonMask	( Button1Mask | Button2Mask | Button3Mask \
			| Button4Mask | Button5Mask )
#define ButtonMask	( ButtonPressMask | ButtonReleaseMask )
#define DragMask        ( ButtonMask | ButtonMotionMask )
#define MenuMask	( ButtonMask | ButtonMotionMask | ExposureMask )
#define MenuGrabMask	( ButtonMask | ButtonMotionMask | StructureNotifyMask \
			  | SubstructureNotifyMask )

#endif
