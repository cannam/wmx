/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#include "Manager.h"
#include "Menu.h"
#include "Client.h"

#include <X11/Xlocale.h>

#include <string.h>
#include <X11/Xproto.h>
#include <X11/keysym.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "Cursors.h"
#include <X11/cursorfont.h>

Atom    Atoms::wm_state;
Atom    Atoms::wm_changeState;
Atom    Atoms::wm_protocols;
Atom    Atoms::wm_delete;
Atom    Atoms::wm_takeFocus;
Atom    Atoms::wm_colormaps;
Atom    Atoms::wmx_running;

Atom    Atoms::netwm_supportingWmCheck;
Atom    Atoms::netwm_wmName;
Atom    Atoms::netwm_supported;
Atom    Atoms::netwm_clientList;
Atom    Atoms::netwm_clientListStacking;
Atom    Atoms::netwm_desktop;
Atom    Atoms::netwm_desktopCount;
Atom    Atoms::netwm_desktopNames;
Atom    Atoms::netwm_activeWindow;
Atom    Atoms::netwm_winLayer;
Atom    Atoms::netwm_winDesktopButtonProxy;
Atom    Atoms::netwm_winState;
Atom    Atoms::netwm_winDesktop;
Atom    Atoms::netwm_winHints;
Atom    Atoms::netwm_winType;

Atom    Atoms::netwm_winType_desktop;
Atom    Atoms::netwm_winType_dock;   
Atom    Atoms::netwm_winType_toolbar;
Atom    Atoms::netwm_winType_menu;   
Atom    Atoms::netwm_winType_utility;
Atom    Atoms::netwm_winType_splash; 
Atom    Atoms::netwm_winType_dialog; 
Atom    Atoms::netwm_winType_dropdown;
Atom    Atoms::netwm_winType_popup;  
Atom    Atoms::netwm_winType_tooltip;
Atom    Atoms::netwm_winType_notify; 
Atom    Atoms::netwm_winType_combo;  
Atom    Atoms::netwm_winType_dnd;    
Atom    Atoms::netwm_winType_normal; 

int     WindowManager::m_signalled = False;
int     WindowManager::m_restart   = False;
Boolean WindowManager::m_initialising = False;
Boolean ignoreBadWindowErrors;

implementPList(ClientList, Client);

#if CONFIG_GROUPS != False
implementPList(ListList, ClientList);
#endif

implementList(AtomList, Atom);

WindowManager::WindowManager(int argc, char **argv) :
    m_focusChanging(False),
#ifdef CONFIG_USE_SESSION_MANAGER
    m_smFD(-1),
#endif
    m_altPressed(False),
    m_altStateRetained(False),
    m_netwmCheckWin(0),
    m_altModMask(0) // later
{
    char *home = getenv("HOME");
    char *wmxdir = getenv("WMXDIR");
    
    fprintf(stderr, "\nwmx: Copyright (c) 1996-2008 Chris Cannam."
	    "  Not a release\n"
	    "     Parts derived from 9wm Copyright (c) 1994-96 David Hogan\n"
	    "     Command menu code Copyright (c) 1997 Jeremy Fitzhardinge\n"
 	    "     Japanize code Copyright (c) 1998 Kazushi (Jam) Marukawa\n"
 	    "     Original keyboard-menu code Copyright (c) 1998 Nakayama Shintaro\n"
	    "     Dynamic configuration code Copyright (c) 1998 Stefan `Sec' Zehl\n"
	    "     Multihead display code Copyright (c) 2000 Sven Oliver `SvOlli' Moll\n"
	    "     Original NETWM code Copyright (c) 2000 Jamie Montgomery\n"
	    "     See source distribution for other patch contributors\n"
#ifdef CONFIG_USE_XFT
	    "     Copying and redistribution encouraged.  "
	    "No warranty.\n\n"
#else
	    "     %s\n     Copying and redistribution encouraged.  "
	    "No warranty.\n\n", XV_COPYRIGHT
#endif
	    );

    int i, j;
#if CONFIG_USE_SESSION_MANAGER != False
    char *oldSessionId = 0;
#endif
    
    if (argc > 1) {

#if CONFIG_USE_SESSION_MANAGER != False
	// Damn!  This means we have to support a command-line argument
	if (argc == 3 && !strcmp(argv[1], "--sm-client-id")) {
	    oldSessionId = argv[2];
	} else {
#endif

	for (i = strlen(argv[0])-1; i > 0 && argv[0][i] != '/'; --i);
	fprintf(stderr, "\nwmx: Usage: %s [--sm-client-id id]\n",
		argv[0] + (i > 0) + i);
	exit(2);

#if CONFIG_USE_SESSION_MANAGER != False
	}
#endif
    }

    if (!setlocale(LC_ALL, ""))
 	fprintf(stderr,
		"Warning: locale not supported by C library, locale unchanged\n");
    if (!XSupportsLocale()) { 
 	fprintf(stderr,
		"Warning: locale not supported by Xlib, locale set to C\n");  
 	setlocale(LC_ALL, "C");
    }
    if (!XSetLocaleModifiers(""))
 	fprintf(stderr,
		"Warning: X locale modifiers not supported, using default\n");
    char* ret_setlocale = setlocale(LC_ALL, NULL);
 					// re-query in case overwritten
    
    if (CONFIG_AUTO_RAISE) {
	if (CONFIG_CLICK_TO_FOCUS) {
	    fatal("can't have auto-raise-with-delay with click-to-focus");
	} else if (CONFIG_RAISE_ON_FOCUS) {
	    fatal("can't have raise-on-focus AND auto-raise-with-delay");
	} else {
	    fprintf(stderr, "     Focus follows, auto-raise with delay.");
	}

    } else {
	if (CONFIG_CLICK_TO_FOCUS) {
	    if (CONFIG_RAISE_ON_FOCUS) {
                if (CONFIG_PASS_FOCUS_CLICK) {
		    fprintf(stderr, "     Click to focus, focus clicks passed on to client.");
                } else {
                    fprintf(stderr, "     Click to focus, focus clicks not passed on to clients.");
                }
	    } else {
		fatal("can't have click-to-focus without raise-on-focus");
	    }
	} else {
	    if (CONFIG_RAISE_ON_FOCUS) {
		fprintf(stderr, "     Focus follows, auto-raise.");
	    } else {
		fprintf(stderr, "     Focus follows pointer.");
	    }
	}
    }

    if (CONFIG_EVERYTHING_ON_ROOT_MENU) {
	fprintf(stderr, "\n     All clients on menu.");
    } else {
	fprintf(stderr, "\n     Hidden clients only on menu.");
    }

    if (CONFIG_USE_SESSION_MANAGER) {
	fprintf(stderr, "  Using session manager.");
    } else {
	fprintf(stderr, "  No session manager.");
    }

    if (CONFIG_PROD_SHAPE) {
	fprintf(stderr, "\n     Shape prodding on.");
    } else {
	fprintf(stderr, "\n     Shape prodding off.");
    }

    if (CONFIG_USE_PIXMAPS) {
	fprintf(stderr, "  Fancy borders.");
    } else {
	fprintf(stderr, "  Plain borders.");
    }

    if (CONFIG_MAD_FEEDBACK) {
	fprintf(stderr, "  Skeletal feedback on.");
    } else {
	fprintf(stderr, "  Skeletal feedback off.");
    }

    if (CONFIG_USE_KEYBOARD) {
	fprintf(stderr, "\n     Keyboard controls available.");
    } else {
	fprintf(stderr, "\n     No keyboard controls.");
    }

    if (CONFIG_WANT_KEYBOARD_MENU) {
	fprintf(stderr, "  Keyboard menu available.");
    } else {
	fprintf(stderr, "  No keyboard menu available.");
    }

    if (CONFIG_CHANNEL_SURF) {
	fprintf(stderr, "\n     Channels on.");
    } else {
	fprintf(stderr, "\n     Channels off.");
    }	

    if (CONFIG_USE_CHANNEL_KEYS) {
	fprintf(stderr, "  Quick keyboard channel-surf available.");
    } else {
	fprintf(stderr, "  No quick keyboard channel-surf.");
    }

    fprintf(stderr, "\n     Operating system locale is \"%s\".",
	    ret_setlocale ? ret_setlocale : "(NULL)");

    fprintf(stderr, "\n     NETWM compliant.");

    fprintf(stderr, "\n     Command menu taken from ");
    if (wmxdir == NULL) {
	fprintf(stderr, "%s/%s.\n", home, CONFIG_COMMAND_MENU);
    } else {
	if (*wmxdir == '/') {
	    fprintf(stderr, "%s.\n", wmxdir);
	} else {
	    fprintf(stderr, "%s/%s.\n", home, wmxdir);
	}
    }

//    fprintf(stderr, "     (To reconfigure, simply edit and recompile.)\n\n");

    m_display = XOpenDisplay(NULL);
    if (!m_display) fatal("can't open display");

    m_shell = (char *)getenv("SHELL");
    if (!m_shell) m_shell = NewString("/bin/sh");

    // find out what the Alt keycode and thus modifier mask are
    int kpk = 0;
    int kmin = 0;
    int kmax = 0;
    XDisplayKeycodes(m_display, &kmin, &kmax);
    KeySym *keymap = XGetKeyboardMapping(m_display, kmin, kmax - kmin, &kpk);
    XModifierKeymap *modmap = XGetModifierMapping(m_display);
    KeyCode alt = 0;
    for (i = 0; i < (kmax - kmin) * kpk; ++i) {
	if (keymap[i] == CONFIG_ALT_KEY) {
	    alt = kmin + (i / kpk);
	    for (j = 0; j < (8 * modmap->max_keypermod); ++j) {
		if (modmap->modifiermap[j] == alt) {
		    m_altModMask = 1 << (j / modmap->max_keypermod);
		}
	    }
	    if (m_altModMask) break;
	}
    }

    if (!m_altModMask) {
	fprintf(stderr, "configured Alt keysym: 0x%x (keycode %d, 0x%x)\n",
                CONFIG_ALT_KEY, alt, alt);
        fatal("no modifier corresponds to the configured Alt keysym");
    }
    XFreeModifiermap(modmap);

    m_initialising = True;
    XSetErrorHandler(errorHandler);
    ignoreBadWindowErrors = False;

    // 9wm does more, I think for nohup
    signal(SIGTERM, sigHandler);
    signal(SIGINT,  sigHandler);
    signal(SIGHUP,  sigHandler);

    m_currentTime = -1;
    m_activeClient = 0;

    m_channels = 2;
    m_currentChannel = 1;
    m_channelChangeTime = 0;
    m_channelWindow = 0;

    Atoms::wm_state      = XInternAtom(m_display, "WM_STATE",            False);
    Atoms::wm_changeState= XInternAtom(m_display, "WM_CHANGE_STATE",     False);
    Atoms::wm_protocols  = XInternAtom(m_display, "WM_PROTOCOLS",        False);
    Atoms::wm_delete     = XInternAtom(m_display, "WM_DELETE_WINDOW",    False);
    Atoms::wm_takeFocus  = XInternAtom(m_display, "WM_TAKE_FOCUS",       False);
    Atoms::wm_colormaps  = XInternAtom(m_display, "WM_COLORMAP_WINDOWS", False);
    Atoms::wmx_running   = XInternAtom(m_display, "_WMX_RUNNING",        False);

    Atoms::netwm_supportingWmCheck = XInternAtom(m_display, "_NET_SUPPORTING_WM_CHECK", False);
    Atoms::netwm_wmName            = XInternAtom(m_display, "_NET_WM_NAME", False);
    Atoms::netwm_supported         = XInternAtom(m_display, "_NET_SUPPORTED",           False);
    Atoms::netwm_clientList        = XInternAtom(m_display, "_NET_CLIENT_LIST",         False);
    Atoms::netwm_clientListStacking= XInternAtom(m_display, "_NET_CLIENT_LIST_STACKING",False);
    Atoms::netwm_desktop           = XInternAtom(m_display, "_NET_CURRENT_DESKTOP",     False);
    Atoms::netwm_desktopCount      = XInternAtom(m_display, "_NET_NUMBER_OF_DESKTOPS",  False);
    Atoms::netwm_desktopNames      = XInternAtom(m_display, "_NET_DESKTOP_NAMES",       False);
    Atoms::netwm_activeWindow      = XInternAtom(m_display, "_NET_ACTIVE_WINDOW",       False);
    //!!! "replaced with _NET_WM_WINDOW_TYPE functional hint":
    Atoms::netwm_winLayer          = XInternAtom(m_display, "_WIN_LAYER",               False);
    //!!! what is this??
    Atoms::netwm_winDesktopButtonProxy 
	                           = XInternAtom(m_display, "_WIN_DESKTOP_BUTTON_PROXY",False);
    //!!! "replaced with _NET_WM_WINDOW_TYPE functional hint":
    Atoms::netwm_winHints          = XInternAtom(m_display, "_WIN_HINTS",               False);
    Atoms::netwm_winState          = XInternAtom(m_display, "_NET_WM_STATE",            False);    
    Atoms::netwm_winDesktop        = XInternAtom(m_display, "_NET_WM_DESKTOP",          False);    

    Atoms::netwm_winType           = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE",      False);

    Atoms::netwm_winType_desktop   = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE_DESKTOP", False);
    Atoms::netwm_winType_dock      = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE_DOCK", False);
    Atoms::netwm_winType_toolbar   = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE_TOOLBAR", False);
    Atoms::netwm_winType_menu      = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE_MENU", False);
    Atoms::netwm_winType_utility   = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE_UTILITY", False);
    Atoms::netwm_winType_splash    = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE_SPLASH", False);
    Atoms::netwm_winType_dialog    = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    Atoms::netwm_winType_dropdown  = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU", False);
    Atoms::netwm_winType_popup     = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE_POPUP_MENU", False);
    Atoms::netwm_winType_tooltip   = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE_TOOLTIP", False);
    Atoms::netwm_winType_notify    = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE_NOTIFICATION", False);
    Atoms::netwm_winType_combo     = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE_COMBO", False);
    Atoms::netwm_winType_dnd       = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE_DND", False);
    Atoms::netwm_winType_normal    = XInternAtom(m_display, "_NET_WM_WINDOW_TYPE_NORMAL", False);
    
    int dummy;
    if (!XShapeQueryExtension(m_display, &m_shapeEvent, &dummy))
	fatal("no shape extension, can't run without it");

    initialiseScreen();
    if (m_screensTotal > 1) {
        fprintf(stderr, "\n     Detected %d screens.", m_screensTotal);
    }
    
    XSetSelectionOwner(m_display, Atoms::wmx_running,
		       None, timestamp(True)); // used to have m_menuWindow
    XSync(m_display, False);
    m_initialising = False;
    m_returnCode = 0;

#if CONFIG_USE_SESSION_MANAGER != False
    initialiseSession(argv[0], oldSessionId);
#endif

    netwmInitialiseCompliance();

#if CONFIG_GROUPS != False
    for (int i = 0; i < 10; i++) {
	grouping.append(new ClientList());
    }
#endif

    clearFocus();
    scanInitialWindows();
    updateStackingOrder();
    loop();
    if (m_restart == True){
	fprintf(stderr,"restarting wmx from SIGHUP\n");
	execv(argv[0],argv);
    }
}


WindowManager::~WindowManager()
{
    if (m_netwmCheckWin) {
        XDestroyWindow(m_display, m_netwmCheckWin);
    }
}


int WindowManager::numdigits(int number)
{
    int n = 0;
    do { ++n; number /= 10; } while (number);
    return n;
}


void WindowManager::release()
{
    if (m_returnCode != 0) return; // hasty exit

    ClientList normalList, unparentList;
    Client *c;
    int i;

    for (i = 0; i < m_clients.count(); ++i) {
	c = m_clients.item(i);
        fprintf(stderr, "release: client %d is %p\n", i, c);

	if (c->isNormal() || c->isNormalButElsewhere()) normalList.append(c);
	else unparentList.append(c);
    }

    for (i = normalList.count()-1; i >= 0; --i) {
	unparentList.append(normalList.item(i));
    }

    m_clients.remove_all();
    m_hiddenClients.remove_all();

    for (i = 0; i < unparentList.count(); ++i) {
	fprintf(stderr, "release: unparenting client %p\n",unparentList.item(i));
	unparentList.item(i)->unreparent();
        fprintf(stderr, "release: releasing client %p\n", unparentList.item(i));
	unparentList.item(i)->release();
    }

    XSetInputFocus(m_display, PointerRoot, RevertToPointerRoot,
		   timestamp(False));
    installColormap(None);

    XFreeCursor(m_display, m_cursor);
    XFreeCursor(m_display, m_xCursor);
    XFreeCursor(m_display, m_vCursor);
    XFreeCursor(m_display, m_hCursor);
    XFreeCursor(m_display, m_vhCursor);

    Menu::cleanup(this);

    XCloseDisplay(m_display);
}


void WindowManager::fatal(const char *message)
{
    fprintf(stderr, "wmx: ");
    if (errno != 0) perror(message);
    else fprintf(stderr, "%s", message);
    fprintf(stderr, "\n");
    exit(1);
}


int WindowManager::errorHandler(Display *d, XErrorEvent *e)
{
    if (m_initialising && (e->request_code == X_ChangeWindowAttributes) &&
	e->error_code == BadAccess) {
	fprintf(stderr, "\nwmx: another window manager running?\n");
	exit(1);
    }

    // ugh
    if (ignoreBadWindowErrors == True && e->error_code == BadWindow) return 0;

    char msg[100], number[30], request[100];
    XGetErrorText(d, e->error_code, msg, 100);
    sprintf(number, "%d", e->request_code);
    XGetErrorDatabaseText(d, "XRequest", number, "", request, 100);

    if (request[0] == '\0') sprintf(request, "<request-code-%d>",
				    e->request_code);

    fprintf(stderr, "wmx: %s (0x%lx): %s\n", request, e->resourceid, msg);

    if (m_initialising) {
	fprintf(stderr, "wmx: failure during initialisation, abandoning\n");
	exit(1);
    }

    return 0;
}    


static Cursor makeCursor(Display *d, Window w,
			 unsigned char *bits, unsigned char *mask_bits,
			 int width, int height, int xhot, int yhot,
			 XColor *fg, XColor *bg, int fontIndex)
{
    Cursor cursor;

    if (CONFIG_USE_PLAIN_X_CURSORS) {

	cursor = XCreateFontCursor(d, fontIndex);

    } else {

	Pixmap pixmap =
	    XCreateBitmapFromData(d, w, (const char *)bits, width, height);
	
	Pixmap mask =
	    XCreateBitmapFromData(d, w, (const char *)mask_bits, width,height);

	cursor = XCreatePixmapCursor(d, pixmap, mask, fg, bg, xhot, yhot);

	XFreePixmap(d, pixmap);
	XFreePixmap(d, mask);
    }

    return cursor;
}

void WindowManager::setScreenFromRoot(Window root)
{
    int s;
    m_screenNumber = 0;
  
    for (s = 0; s < m_screensTotal; s++)
        if (m_root[s] == root)
            m_screenNumber = s;
}

void WindowManager::initialiseScreen()
{
    int i;
    m_screensTotal = ScreenCount(m_display);
  
    m_root = (Window *) malloc(m_screensTotal * sizeof(Window));
    m_defaultColormap = (Colormap *) malloc(m_screensTotal * sizeof(Colormap));
//    m_minimumColormaps = (int *) malloc(m_screensTotal * sizeof(int));
    m_channelWindow = (Window *) malloc(m_screensTotal * sizeof(Window));
    
    for (i = 0 ; i < m_screensTotal ; i++) {

        m_screenNumber = i;
	m_channelWindow[i] = 0;

        m_root[i] = RootWindow(m_display, i);
        m_defaultColormap[i] = DefaultColormap(m_display, i);
//        m_minimumColormaps[i] = MinCmapsOfScreen(ScreenOfDisplay(m_display, i));

        XColor black, white, temp;

        if (!XAllocNamedColor(m_display, m_defaultColormap[i], "black", &black, &temp))
	fatal("couldn't load colour \"black\"!");
        if (!XAllocNamedColor(m_display, m_defaultColormap[i], "white", &white, &temp))
	fatal("couldn't load colour \"white\"!");

        m_cursor = makeCursor
	(m_display, m_root[i], cursor_bits, cursor_mask_bits,
	 cursor_width, cursor_height, cursor_x_hot,
	 cursor_y_hot, &black, &white, XC_top_left_arrow);

        m_xCursor = makeCursor
	(m_display, m_root[i], ninja_cross_bits, ninja_cross_mask_bits,
	 ninja_cross_width, ninja_cross_height, ninja_cross_x_hot,
	 ninja_cross_y_hot, &black, &white, XC_X_cursor);

        m_hCursor = makeCursor
	(m_display, m_root[i], cursor_right_bits, cursor_right_mask_bits,
	 cursor_right_width, cursor_right_height, cursor_right_x_hot,
	 cursor_right_y_hot, &black, &white, XC_right_side);

        m_vCursor = makeCursor
	(m_display, m_root[i], cursor_down_bits, cursor_down_mask_bits,
	 cursor_down_width, cursor_down_height, cursor_down_x_hot,
	 cursor_down_y_hot, &black, &white, XC_bottom_side);

        m_vhCursor = makeCursor
	(m_display, m_root[i], cursor_down_right_bits, cursor_down_right_mask_bits,
	 cursor_down_right_width, cursor_down_right_height,
	 cursor_down_right_x_hot, cursor_down_right_y_hot, &black, &white,
	 XC_bottom_right_corner);

        XSetWindowAttributes attr;
        attr.cursor = m_cursor;
        attr.event_mask = SubstructureRedirectMask | SubstructureNotifyMask |
	ColormapChangeMask | ButtonPressMask | ButtonReleaseMask | 
	PropertyChangeMask | LeaveWindowMask | KeyPressMask | KeyReleaseMask;

        XChangeWindowAttributes(m_display, m_root[i], CWCursor | CWEventMask, &attr);
        XSync(m_display, False);
    }

    m_screenNumber = 0;
}


unsigned long WindowManager::allocateColour(int screen, const char *name,
                                            const char *desc)
{
    XColor nearest, ideal;

    if (!XAllocNamedColor
	(display(), DefaultColormap(display(), screen), name,
	 &nearest, &ideal)) {

	char error[100];
	sprintf(error, "couldn't load %s colour", desc);
	fatal(error);
    }

    return nearest.pixel;
}


void WindowManager::installCursor(RootCursor c)
{
    installCursorOnWindow(c, root());
}


void WindowManager::installCursorOnWindow(RootCursor c, Window w)
{
    XSetWindowAttributes attr;

    switch (c) {
    case DeleteCursor:    attr.cursor = m_xCursor;  break;
    case DownCursor:      attr.cursor = m_vCursor;  break;
    case RightCursor:     attr.cursor = m_hCursor;  break;
    case DownrightCursor: attr.cursor = m_vhCursor; break;
    case NormalCursor:    attr.cursor = m_cursor;   break;
    }

    XChangeWindowAttributes(m_display, w, CWCursor, &attr);
}
	

Time WindowManager::timestamp(Boolean reset)
{
    if (reset) m_currentTime = CurrentTime;

    if (m_currentTime == CurrentTime) {

	XEvent event;
	XChangeProperty(m_display, root(), Atoms::wmx_running,
			Atoms::wmx_running, 8, PropModeAppend,
			(unsigned char *)"", 0);
	XMaskEvent(m_display, PropertyChangeMask, &event);

	m_currentTime = event.xproperty.time;
    }

    return m_currentTime;
}

void WindowManager::sigHandler(int signal)
{
    fprintf(stderr, "WindowManager::sigHandler: signal %d\n", signal);
    m_signalled = True;
    if (signal == SIGHUP)
	m_restart = True;
}

void WindowManager::scanInitialWindows()
{
    unsigned int i, n;
    int s;
    Window w1, w2, *wins;
    XWindowAttributes attr;
    
    for (s = 0; s < m_screensTotal; s++)
    {
        XQueryTree(m_display, m_root[s], &w1, &w2, &wins, &n);

        for (i = 0; i < n; ++i) {

	    XGetWindowAttributes(m_display, wins[i], &attr);
//	    if (attr.override_redirect || wins[i] == m_menuWindow) continue;
	    if (attr.override_redirect) continue;
	    (void)windowToClient(wins[i], True);
	}

        XFree((void *)wins);
    }
}


Client *WindowManager::windowToClient(Window w, Boolean create)
{
    if (w == 0) return 0;

    for (int i = m_clients.count()-1; i >= 0; --i) {

	if (m_clients.item(i)->hasWindow(w)) {
	    return m_clients.item(i);
	}
    }

    if (!create) return 0;
    else {
        
        Client *newC = 0;
    
        int bounding_shape = -1;
        int clip_shape = -1;
        int x_bounding = 0;
        int y_bounding = 0;
        unsigned int w_bounding = 0;
        unsigned int h_bounding = 0;
        unsigned int w_clip = 0;
        unsigned int h_clip = 0;
        int x_clip = 0;
        int y_clip = 0;

        (void)XShapeQueryExtents(m_display, w, &bounding_shape, 
				 &x_bounding, &y_bounding, 
				 &w_bounding, &h_bounding, &clip_shape,
				 &x_clip, &y_clip, &w_clip, &h_clip);

	newC = new Client(this, w, bounding_shape==1);
	m_clients.append(newC);
	
	if (m_currentChannel == m_channels) {
	    createNewChannel();
	}

	return newC;
    }
}

void WindowManager::installColormap(Colormap cmap)
{
    if (cmap == None) {
	XInstallColormap(m_display, m_defaultColormap[screen()]);
//	XInstallColormap(m_display, m_defaultColormap);
    } else {
	XInstallColormap(m_display, cmap);
    }
}

void WindowManager::setActiveClient(Client *const c)
{
    if (m_activeClient && m_activeClient != c) {
	m_activeClient->deactivate();
    }
    m_activeClient = c;

    netwmUpdateStackingOrder();
    netwmUpdateActiveClient();
}

void WindowManager::clearFocus()
{
    static Window w = 0;
    Client *active = activeClient();

    if (CONFIG_AUTO_RAISE || !CONFIG_CLICK_TO_FOCUS) {
	setActiveClient(0);
	return;
    }

    if (active) {

	setActiveClient(0);
	active->deactivate();

	for (Client *c = active->revertTo(); c; c = c->revertTo()) {
	    if (c->isNormal()) {
		c->activate();
		return;
	    }
	}

	installColormap(None);
    }

    if (w == 0) {

	XSetWindowAttributes attr;
	int mask = CWOverrideRedirect;
	attr.override_redirect = 1;

	w = XCreateWindow(display(), root(), 0, 0, 1, 1, 0,
			  CopyFromParent, InputOnly, CopyFromParent,
			  mask, &attr);

	XMapWindow(display(), w);
    }

    XSetInputFocus(display(), w, RevertToPointerRoot, timestamp(False));
}


void WindowManager::skipInRevert(Client *c, Client *myRevert)
{
    for (int i = 0; i < m_clients.count(); ++i) {
	if (m_clients.item(i) != c &&
	    m_clients.item(i)->revertTo() == c) {
	    m_clients.item(i)->setRevertTo(myRevert);
	}
    }
}


void WindowManager::addToHiddenList(Client *c)
{
    for (int i = 0; i < m_hiddenClients.count(); ++i) {
	if (m_hiddenClients.item(i) == c) return;
    }

    m_hiddenClients.append(c);

    netwmUpdateWindowList(); 
}


void WindowManager::removeFromHiddenList(Client *c)
{
    fprintf(stderr, "WindowManager::removeFromHiddenList(%p)\n", c);

    for (int i = 0; i < m_hiddenClients.count(); ++i) {
	if (m_hiddenClients.item(i) == c) {
	    m_hiddenClients.remove(i);

	    if (c->channel() != m_currentChannel) {

		while (c->channel() != m_currentChannel) {
		    if (m_currentChannel < c->channel()) {
			flipChannel(False, False, True, 0);
		    } else {
			flipChannel(False, True, True, 0);
		    }
		    XSync(display(), False);
		}
	    } else {
		netwmUpdateWindowList(); 
	    }
	    return;
	}
    }
}


void WindowManager::hoistToTop(Client *c)
{
    fprintf(stderr, "WindowManager::hoistToTop(%p)\n", c);

    int i;
    int layer = c->layer();

    for (i = 0; i < m_orderedClients[layer].count(); ++i) {
	if (m_orderedClients[layer].item(i) == c) {
	    m_orderedClients[layer].move_to_start(i);
            netwmUpdateWindowList();
            return;
	}
    }

    m_orderedClients[layer].append(c);
    m_orderedClients[layer].move_to_start(m_orderedClients[layer].count()-1);
    netwmUpdateWindowList(); 
}


void WindowManager::hoistToBottom(Client *c)
{
    int i;
    int layer=c->layer();

    for (i = 0; i < m_orderedClients[layer].count(); ++i) {
	if (m_orderedClients[layer].item(i) == c) {
	    m_orderedClients[layer].move_to_end(i);
            netwmUpdateWindowList();
            return;
	}
    }

    m_orderedClients[layer].append(c);
    netwmUpdateWindowList();
}


void WindowManager::removeFromOrderedList(Client *c)
{
    int layer = c->layer();

    fprintf(stderr, "WindowManager::removeFromOrderedList(%p) [layer %d]\n",
            c, layer);

    for (int i = 0; i < m_orderedClients[layer].count(); ++i) {
	if (m_orderedClients[layer].item(i) == c) {
	    m_orderedClients[layer].remove(i);
	    netwmUpdateWindowList(); 
	    return;
	}
    }
}
    

Boolean WindowManager::isTop(Client *c)
{
    int layer = c->layer();

    if (m_orderedClients[layer].count() == 0) {
        fprintf(stderr, "Warning: ordered clients list for layer %d is empty even though client %p thinks it's in this layer\n", layer, c);
        return False;
    }

    return (m_orderedClients[layer].item(0) == c) ? True : False;
}

void WindowManager::withdrawGroup(Window groupParent, Client *omit, Boolean changeState)
{
    for (int layer = MAX_LAYER; layer >= 0; --layer)
    for (int i = 0; i < m_orderedClients[layer].count(); ++i) {
	Client *ic = m_orderedClients[layer].item(i);
	if (ic->groupParent() == groupParent && !ic->isGroupParent() &&
	    ic != omit) {
	    ic->withdraw(changeState);
	}
    }
}

void WindowManager::hideGroup(Window groupParent, Client *omit)
{    
    for (int layer = MAX_LAYER; layer >= 0; --layer)
    for (int i = 0; i < m_orderedClients[layer].count(); ++i) {
	Client *ic = m_orderedClients[layer].item(i);
	if (ic->groupParent() == groupParent && !ic->isGroupParent() &&
	    ic != omit) {
	    ic->hide();
	}
    }
}

void WindowManager::unhideGroup(Window groupParent, Client *omit, Boolean map)
{
    for (int layer = MAX_LAYER; layer >= 0; --layer)
    for (int i = 0; i < m_orderedClients[layer].count(); ++i) {
	Client *ic = m_orderedClients[layer].item(i);
	if (ic->groupParent() == groupParent && !ic->isGroupParent() &&
	    ic != omit) {
	    ic->unhide(map);
	}
    }
}

void WindowManager::killGroup(Window groupParent, Client *omit)
{
    for (int layer = MAX_LAYER; layer >= 0; --layer)
    for (int i = 0; i < m_orderedClients[layer].count(); ++i) {
	Client *ic = m_orderedClients[layer].item(i);
	if (ic->groupParent() == groupParent && !ic->isGroupParent() &&
	    ic != omit) {
	    ic->kill();
	}
    }
}


Boolean WindowManager::raiseTransients(Client *c)
{
    Client *first = 0;

    if (!c->isNormal()) return False;

    for (int i = 0; i < m_clients.count(); ++i) {

	if (m_clients.item(i)->isNormal() &&
	    m_clients.item(i)->isTransient()) {

	    if (c->hasWindow(m_clients.item(i)->transientFor())) {

		if (!first) first = m_clients.item(i);
		else m_clients.item(i)->mapRaised();
	    }
	}
    }

    if (first) {
	first->mapRaised();
	return True;
    } else {
	return False;
    }
}

#ifdef sgi
extern "C" {
extern int putenv(char *);	/* not POSIX */
}
#endif

void WindowManager::spawn(char *name, char *file)
{
    // strange code thieved from 9wm to avoid leaving zombies

    char *displayName = DisplayString(m_display);

    if (fork() == 0) {
	if (fork() == 0) {

	    close(ConnectionNumber(m_display));

	    // if you don't have putenv, miss out this next
	    // conditional and its contents

	    if (displayName && (displayName[0] != '\0')) {
		char *c;
		char *pstring = (char *)malloc(strlen(displayName) + 11 +
					       numdigits(screen()));
		sprintf(pstring, "DISPLAY=%s", displayName);
                c = strrchr(pstring, '.');
                if (c) {
                  sprintf(c + 1, "%d", screen());
                  putenv(pstring);
                }
	    }

	    if (CONFIG_EXEC_USING_SHELL) {
		if (file) execl(m_shell, m_shell, "-c", file, NULL);
		else execl(m_shell, m_shell, "-c", name, NULL);
		fprintf(stderr, "wmx: exec %s", m_shell);
		perror(" failed");
	    }

	    if (file) {
		execl(file, name, NULL);
	    }
	    else {
		if (strcmp(CONFIG_NEW_WINDOW_COMMAND, name)) {
		    execlp(name, name, NULL);
		}
		else {
			execlp(name, name, CONFIG_NEW_WINDOW_COMMAND_OPTIONS, NULL);
		}
	    }

	    XBell(display(), 70);
	    fprintf(stderr, "wmx: exec %s:%s failed (errno %d)\n",
		    name, file, errno);

	    exit(1);
	}
	exit(0);
    }
    wait((int *) 0);
}

void WindowManager::netwmInitialiseCompliance()
{
    // NOTE that this has been altered to coexist with the
    // multihead code; but we're only using screen 0 here
    // as I have no idea how NETWM copes with multiheads --cc

    // most of this is taken verbatim from 
    // http://www.gnome.org/devel/gnomewm
    
    // this part is to tell NETWM we are compliant. The window 
    // is needed to tell NETWM that wmx has exited (i think). 
    m_netwmCheckWin = XCreateSimpleWindow
        (m_display, m_root[0], -200, -200, 5, 5, 0, 0, 0);

    XChangeProperty
        (m_display, m_root[0], Atoms::netwm_supportingWmCheck,
         XA_WINDOW, 32, PropModeReplace,
         (unsigned char*)&m_netwmCheckWin, 1);
    
    XChangeProperty
        (m_display, m_netwmCheckWin, Atoms::netwm_supportingWmCheck,
         XA_WINDOW, 32, PropModeReplace,
         (unsigned char*)&m_netwmCheckWin, 1);
    
    XChangeProperty
        (m_display, m_root[0], Atoms::netwm_wmName,
         XA_STRING, 8, PropModeReplace, (unsigned char *)"wmx", 1);
    
    XChangeProperty
        (m_display, m_netwmCheckWin, Atoms::netwm_wmName,
         XA_STRING, 8, PropModeReplace, (unsigned char *)"wmx", 1);
    
    // Also going to add the desktop button proxy stuff to the same
    // window.  This is an old property, but it's not clear how it has
    // been superseded -- see e.g.
    // http://developer.gnome.org/doc/standards/wm/c71.html

    XChangeProperty
        (m_display, m_root[0], Atoms::netwm_winDesktopButtonProxy,
         XA_CARDINAL, 32, PropModeReplace,
         (unsigned char*)&m_netwmCheckWin, 1);

    XChangeProperty
        (m_display, m_netwmCheckWin, Atoms::netwm_winDesktopButtonProxy,
         XA_CARDINAL, 32, PropModeReplace,
         (unsigned char*)&m_netwmCheckWin, 1);

    // Now we need to tell NETWM how compliant we are

    // some day we will be more compliant
    AtomList supported;
    
    supported.append(Atoms::netwm_clientList);
    supported.append(Atoms::netwm_clientListStacking);
    supported.append(Atoms::netwm_desktop);
    supported.append(Atoms::netwm_desktopCount);
    supported.append(Atoms::netwm_desktopNames);
    supported.append(Atoms::netwm_activeWindow);
    supported.append(Atoms::netwm_winLayer);
    supported.append(Atoms::netwm_winHints);
    supported.append(Atoms::netwm_winState);
    supported.append(Atoms::netwm_winDesktop);
    supported.append(Atoms::netwm_winType);
    supported.append(Atoms::netwm_winDesktopButtonProxy);
    supported.append(Atoms::netwm_supportingWmCheck);
    
    XChangeProperty
        (m_display, m_root[0], Atoms::netwm_supported, XA_ATOM, 32,
         PropModeReplace,
         (unsigned char *)supported.array(0, supported.count()),
         supported.count());

    netwmUpdateChannelList();
}

void WindowManager::updateStackingOrder()
{
    int orderedCount = 0;
    for (int i = MAX_LAYER; i >= 0; --i)
        orderedCount += m_orderedClients[i].count();
    
    Window* windows = new Window[orderedCount];
    Window* windowIter = windows;
    
    for(int layer = MAX_LAYER; layer >= 0; --layer) {
        int top = m_orderedClients[layer].count();
        for (int i = 0; i < top; ++i) {
            Client *c = m_orderedClients[layer].item(i);
	    if (!c->isKilled() && (c->channel()==channel()  || c->isSticky()) && !c->isHidden()) {
                *(windowIter++)=c->parent();
	    }
        }
    }
     
#if 0   
    /******************************/
    /*****  TAKE THIS OUT!!!  *****/
 
    {
        int count = 0;
        for(Window* i=windows; i<windowIter; ++i) {
            for(Window* j=windows; j<windowIter; ++j) {
                if(i!=j && *i==*j) {
                    count++;
                }
            }
        }
        if(count!=0)
            fprintf(stderr, "wmx: %d duplicate windows in stacking list\n", count);
    }
    
    /*****  TAKE THIS OUT!!!  *****/    
    /******************************/
#endif
    
    XRestackWindows(display(), windows, windowIter-windows);
    
    delete[] windows;

    netwmUpdateStackingOrder();
}
  
void WindowManager::netwmUpdateWindowList()
{
    int count = m_clients.count() + m_hiddenClients.count();
    
    Window *byAge = new Window[count];

    count = 0;

    for (int i = 0; i < m_hiddenClients.count(); ++i) {
        Client *c = m_hiddenClients.item(i);
        if (c->isKilled()) continue;
        byAge[count++] = c->window();
//        fprintf(stderr, "[netwm] client %d [%p] [H] window %lx, \"%s\"\n", count, c, c->window(), c->name());
    }

    for (int i = 0; i < m_clients.count(); ++i) {
        Client *c = m_clients.item(i);
        if (!c->isNormal() || c->isKilled()) continue;
        byAge[count++] = c->window();
//        fprintf(stderr, "[netwm] client %d [%p] window %lx, \"%s\"\n", count, c, c->window(), c->name());
    }

//    fprintf(stderr, "[netwm] %d client(s) total, setting to root window %lx\n", count, m_root[0]);
    
    XChangeProperty
        (m_display, m_root[0], Atoms::netwm_clientList, XA_WINDOW, 32, 
         PropModeReplace, (unsigned char*)byAge, count);
  
    delete[] byAge;

    netwmUpdateStackingOrder();
    netwmUpdateActiveClient();
}
  
void WindowManager::netwmUpdateStackingOrder()
{
    int count = m_hiddenClients.count();

    for (int i = MAX_LAYER; i >= 0; --i) {
        count += m_orderedClients[i].count();
    }
    
    Window *byStacking = new Window[count];

    count = 0;

    // Looks like panels and things will test to make sure the client
    // list and stacking list have the same windows in them, before
    // they trust either.  So we'd better include the hidden windows
    // in the stacking list as well as in the client list, otherwise
    // nobody will ever believe anything we say.

    for (int i = 0; i < m_hiddenClients.count(); ++i) {
        Client *c = m_hiddenClients.item(i);
        if (c->isKilled()) continue;
        byStacking[count++] = c->window();
//        fprintf(stderr, "[netwm] stacking order: hidden client %d\n", c);
    }

    for (int layer = 0; layer < MAX_LAYER; ++layer) {
        int top = m_orderedClients[layer].count();
        for (int i = top - 1; i >= 0; --i) {
            Client *c = m_orderedClients[layer].item(i);
	    if (c->isWithdrawn() || c->isKilled() || c->isHidden()) continue;
            byStacking[count++] = c->window();
//            fprintf(stderr, "[netwm] stacking order: item %d is window %lx, client \"%s\"\n", count, c->window(), c->name());
        }
    }
    
//    fprintf(stderr, "[netwm] stacking order: %d client(s) total\n", count);
    
    XChangeProperty
        (m_display, m_root[0], Atoms::netwm_clientListStacking, XA_WINDOW, 32, 
         PropModeReplace, (unsigned char*)byStacking, count);
  
    delete[] byStacking;   
}

void WindowManager::netwmUpdateChannelList()
{
    int     i;
    char  **names, s[1024];
    CARD32  chan;
   
    if (m_channels <= 1) return;

    chan = (CARD32) m_channels;
 
    XChangeProperty
        (m_display, m_root[0], Atoms::netwm_desktopCount, XA_CARDINAL, 32, 
         PropModeReplace, (unsigned char *)&chan, 1);

    // set the names of the channels
    names = new char*[chan];
    
    for (i = 0; i < chan; i++) {
        snprintf(s, sizeof(s), "Channel %i", i + 1);
        names[i] = new char [strlen(s) + 1];
        strcpy(names[i], s);
    }
    
    XTextProperty textProp;

    if (XStringListToTextProperty(names, chan, &textProp)) {
	XSetTextProperty
            (m_display, m_root[0], &textProp, Atoms::netwm_desktopNames);
	XFree(textProp.value);
    }
    
    for (i = 0; i < chan; i++) delete[] names[i];

    delete[] names;

    netwmUpdateCurrentChannel();
}   

void WindowManager::netwmUpdateActiveClient()
{
    if (!m_activeClient) return;

    CARD32 val;
    
    val = m_activeClient->window();
    
    XChangeProperty
        (m_display, m_root[0], Atoms::netwm_activeWindow, XA_WINDOW, 32, 
         PropModeReplace, (unsigned char *)&val, 1);
}    

void WindowManager::netwmUpdateCurrentChannel()
{
    CARD32 val;

    // netwm numbers then 0... not 1...
    val = (CARD32)(m_currentChannel - 1);

    XChangeProperty
        (m_display, m_root[0], Atoms::netwm_desktop, XA_CARDINAL, 32, 
         PropModeReplace, (unsigned char *)&val, 1);

    netwmUpdateWindowList();
}  

void
WindowManager::printClientList()
{
    printf("wmx: %ld client(s)\n", m_clients.count());

    for (int i = 0; i < m_clients.count(); ++i) {

        bool inHidden = false;

        // we don't mind that this is so inefficient
        for (int j = 0; j < m_hiddenClients.count(); ++j) {
            if (m_hiddenClients.item(j) == m_clients.item(i)) {
                inHidden = true;
                break;
            }
        }
        printf("wmx: Client %d (address %p)%s\n", i+1, m_clients.item(i), 
               inHidden ? " [hidden]" : "");

        m_clients.item(i)->printClientData();

        printf("\n");
        fflush(stdout);
    }
}

