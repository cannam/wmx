
#include "Manager.h"
#include "Menu.h"
#include "Client.h"

#if I18N
#include <X11/Xlocale.h>
#endif

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

#if CONFIG_GNOME_COMPLIANCE != False
Atom    Atoms::gnome_supportingWmCheck;
Atom    Atoms::gnome_protocols;
Atom    Atoms::gnome_clienList;
Atom    Atoms::gnome_workspace;
Atom    Atoms::gnome_workspaceCount;
Atom    Atoms::gnome_workspaceNames;
Atom    Atoms::gnome_winLayer;
Atom    Atoms::gnome_winDesktopButtonProxy;
#endif

int     WindowManager::m_signalled = False;
int     WindowManager::m_restart   = False;
Boolean WindowManager::m_initialising = False;
Boolean ignoreBadWindowErrors;

implementPList(ClientList, Client);

#if CONFIG_GROUPS != False
implementPList(ListList, ClientList);
#endif

WindowManager::WindowManager(int argc, char **argv) :
    m_focusChanging(False),
    m_altPressed(False),
    m_altStateRetained(False),
    m_altModMask(0) // later
{
    char *home = getenv("HOME");
    char *wmxdir = getenv("WMXDIR");
    
    fprintf(stderr, "\nwmx: Copyright (c) 1996-2003 Chris Cannam."
	    "  Not a release\n"
	    "     Parts derived from 9wm Copyright (c) 1994-96 David Hogan\n"
	    "     Command menu code Copyright (c) 1997 Jeremy Fitzhardinge\n"
 	    "     Japanize code Copyright (c) 1998 Kazushi (Jam) Marukawa\n"
 	    "     Original keyboard-menu code Copyright (c) 1998 Nakayama Shintaro\n"
	    "     Dynamic configuration code Copyright (c) 1998 Stefan `Sec' Zehl\n"
	    "     Multihead display code Copyright (c) 2000 Sven Oliver `SvOlli' Moll\n"
	    "     See source distribution for other patch contributors\n"
	    "     %s\n     Copying and redistribution encouraged.  "
	    "No warranty.\n\n", XV_COPYRIGHT);

    int i;
#if CONFIG_USE_SESSION_MANAGER != False
    char *oldSessionId = 0;
#endif
    
    if (argc > 1) {

#if CONFIG_USE_SESSION_MANAGER != False
	// Damn!  This means we have to support a command-line argument
	if (argc == 3 && !strcmp(argv[1], "-clientId")) {
	    oldSessionId = argv[2];
	} else {
#endif

	for (i = strlen(argv[0])-1; i > 0 && argv[0][i] != '/'; --i);
	fprintf(stderr, "\nwmx: Usage: %s [-clientId id]\n",
		argv[0] + (i > 0) + i);
	exit(2);

#if CONFIG_USE_SESSION_MANAGER != False
	}
#endif
    }

#if I18N
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
#endif
    
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
		fprintf(stderr, "     Click to focus.");
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
	fprintf(stderr, "  All clients on menu.");
    } else {
	fprintf(stderr, "  Hidden clients only on menu.");
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

#if I18N
    fprintf(stderr, "\n     Operating system locale is \"%s\".",
	    ret_setlocale ? ret_setlocale : "(NULL)");
#endif

    if (CONFIG_GNOME_COMPLIANCE) {
        fprintf(stderr, "\n     Partial GNOME compliance.");
    } else {
        fprintf(stderr, "\n     Not GNOME compliant.");
    }

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
    KeyCode alt = XKeysymToKeycode(m_display, CONFIG_ALT_KEY);
    XModifierKeymap *modmap = XGetModifierMapping(m_display);
    for (i = 0; i < (8 * modmap->max_keypermod); ++i) {
        if (modmap->modifiermap[i] == alt) {
            m_altModMask = 1 << (i / modmap->max_keypermod);
        }
    }
    if (!m_altModMask)
        fatal("no modifier corresponds to the configured Alt keysym");
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

#if CONFIG_GNOME_COMPLIANCE != False
    Atoms::gnome_supportingWmCheck = XInternAtom(m_display, "_WIN_SUPPORTING_WM_CHECK", False);
    Atoms::gnome_protocols         = XInternAtom(m_display, "_WIN_PROTOCOLS",           False);
    Atoms::gnome_clienList         = XInternAtom(m_display, "_WIN_CLIENT_LIST",         False);
    Atoms::gnome_workspace         = XInternAtom(m_display, "_WIN_WORKSPACE",           False);
    Atoms::gnome_workspaceCount    = XInternAtom(m_display, "_WIN_WORKSPACE_COUNT",     False);
    Atoms::gnome_workspaceNames    = XInternAtom(m_display, "_WIN_WORKSPACE_NAMES",     False);
    Atoms::gnome_winLayer          = XInternAtom(m_display, "_WIN_LAYER",               False);
    Atoms::gnome_winDesktopButtonProxy 
	                           = XInternAtom(m_display, "_WIN_DESKTOP_BUTTON_PROXY",False);
#endif

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

#if CONFIG_GNOME_COMPLIANCE != False
    gnomeInitialiseCompliance();
#endif

#if CONFIG_GROUPS != False
    for (int i = 0; i < 10; i++) {
	grouping.append(new ClientList());
    }
#endif

    clearFocus();
    scanInitialWindows();
    loop();
    if(m_restart == True){
	fprintf(stderr,"restarting wmx from SIGHUP\n");
	execv(argv[0],argv);
    }
}


WindowManager::~WindowManager()
{
    // empty
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
//    fprintf(stderr, "client %d is %p\n", i, c);

	if (c->isNormal() || c->isNormalButElsewhere()) normalList.append(c);
	else unparentList.append(c);
    }

    for (i = normalList.count()-1; i >= 0; --i) {
	unparentList.append(normalList.item(i));
    }

    m_clients.remove_all();
    
    for (i = 0; i < unparentList.count(); ++i) {
//	fprintf(stderr, "unparenting client %p\n",unparentList.item(i));
	unparentList.item(i)->unreparent();
	unparentList.item(i)->release();
	unparentList.item(i) = 0;
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
    perror(message);
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
  
    m_root     = (Window *) malloc(m_screensTotal * sizeof(Window));
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


unsigned long WindowManager::allocateColour(int screen, char *name, char *desc)
{
    XColor nearest, ideal;

    if (!XAllocNamedColor
	(display(), DefaultColormap(display(), screen), name,
	 &nearest, &ideal)) {

	char error[100];
	sprintf(error, "couldn't load %s colour", desc);
	fatal(error);

    } else return nearest.pixel;
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
    m_signalled = True;
    if (signal == SIGHUP)
	m_restart = True;
}

void WindowManager::scanInitialWindows()
{
    unsigned int i, n, s;
    Window w1, w2, *wins;
    XWindowAttributes attr;
    
    for(s=0;s<m_screensTotal;s++)
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

#if CONFIG_GNOME_COMPLIANCE != False
        
        // i would like to catch layer info here.
        // if it's layer 0, then don't map it....
        Atom returnType;
        int returnFormat;
        unsigned long count;
        unsigned long bytes_remain;
        unsigned char *prop;

        int m_layer = 256;

        if (XGetWindowProperty(m_display, w, Atoms::gnome_winLayer, 0, 1,
                               False, XA_CARDINAL, &returnType, &returnFormat,
                               &count, &bytes_remain, &prop) == Success) {
        
            if (returnType == XA_CARDINAL && returnFormat == 32 && count == 1) {
                m_layer = ((long *)prop)[0];
                //fprintf(stderr, "0x%X:layer == %d\n", w, m_layer);
                XFree(prop);
                XMapWindow(m_display, w);
                return 0;
            }
        
            XFree(prop);
        }
#endif
    
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

        int shapeq = XShapeQueryExtents(m_display, w, &bounding_shape, 
                                        &x_bounding, &y_bounding, 
                                        &w_bounding, &h_bounding, &clip_shape,
                                        &x_clip, &y_clip, &w_clip, &h_clip);

        if (bounding_shape == 1) {
            //	fprintf(stderr, "0x%X: shapeq = %d, bounding_shape = %d x=%d y=%d w=%d h=%d clip_shape=%d x=%d y=%d w=%d h=%d\n",w, shapeq, bounding_shape, x_bounding, y_bounding, w_bounding, h_bounding,clip_shape,  x_clip, y_clip, w_clip, h_clip);
	
            //            int count = 0;
            //            int ordering = 0;
            
            //            XRectangle* recs =
            //                XShapeGetRectangles(m_display, w, ShapeBounding,
            //                                    &count, &ordering);
            //	fprintf(stderr, "rectangles: count= %d\n", count);

            //            XFree(recs);

            XMapWindow(m_display, w);

            newC = new Client(this, w, true);
        } else {
            newC = new Client(this, w, false);
        }

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

#if CONFIG_GNOME_COMPLIANCE != False
    gnomeUpdateWindowList(); 
#endif
}


void WindowManager::removeFromHiddenList(Client *c)
{
    for (int i = 0; i < m_hiddenClients.count(); ++i) {
	if (m_hiddenClients.item(i) == c) {
	    m_hiddenClients.remove(i);

#if CONFIG_GNOME_COMPLIANCE != False
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
		gnomeUpdateWindowList(); 
	    }
#endif
	    return;
	}
    }
}


void WindowManager::hoistToTop(Client *c)
{
    int i;

    for (i = 0; i < m_orderedClients.count(); ++i) {
	if (m_orderedClients.item(i) == c) {
	    m_orderedClients.move_to_start(i);
	    break;
	}
    }

    if (i >= m_orderedClients.count()) {
	m_orderedClients.append(c);
	m_orderedClients.move_to_start(m_orderedClients.count()-1);

#if CONFIG_GNOME_COMPLIANCE != False
        gnomeUpdateWindowList(); 
#endif

    }
}


void WindowManager::hoistToBottom(Client *c)
{
    int i;

    for (i = 0; i < m_orderedClients.count(); ++i) {
	if (m_orderedClients.item(i) == c) {
	    m_orderedClients.move_to_end(i);
	    break;
	}
    }

    if (i >= m_orderedClients.count()) {
	m_orderedClients.append(c);
//	m_orderedClients.move_to_end(m_orderedClients.count()-1);
    }
}


void WindowManager::removeFromOrderedList(Client *c)
{
    for (int i = 0; i < m_orderedClients.count(); ++i) {
	if (m_orderedClients.item(i) == c) {
	    m_orderedClients.remove(i);
#if CONFIG_GNOME_COMPLIANCE != False
	    gnomeUpdateWindowList(); 
#endif
	    return;
	}
    }
}
    

Boolean WindowManager::isTop(Client *c)
{
    return (m_orderedClients.item(0) == c) ? True : False;
}

void WindowManager::withdrawGroup(Window groupParent, Client *omit, Boolean changeState)
{
    for (int i = 0; i < m_orderedClients.count(); ++i) {
	Client *ic = m_orderedClients.item(i);
	if (ic->groupParent() == groupParent && !ic->isGroupParent() &&
	    ic != omit) {
	    ic->withdraw(changeState);
	}
    }
}

void WindowManager::hideGroup(Window groupParent, Client *omit)
{
    for (int i = 0; i < m_orderedClients.count(); ++i) {
	Client *ic = m_orderedClients.item(i);
	if (ic->groupParent() == groupParent && !ic->isGroupParent() &&
	    ic != omit) {
	    ic->hide();
	}
    }
}

void WindowManager::unhideGroup(Window groupParent, Client *omit, Boolean map)
{
    for (int i = 0; i < m_orderedClients.count(); ++i) {
	Client *ic = m_orderedClients.item(i);
	if (ic->groupParent() == groupParent && !ic->isGroupParent() &&
	    ic != omit) {
	    ic->unhide(map);
	}
    }
}

void WindowManager::killGroup(Window groupParent, Client *omit)
{
    for (int i = 0; i < m_orderedClients.count(); ++i) {
	Client *ic = m_orderedClients.item(i);
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
		if (file) execl(m_shell, m_shell, "-c", file, 0);
		else execl(m_shell, m_shell, "-c", name, 0);
		fprintf(stderr, "wmx: exec %s", m_shell);
		perror(" failed");
	    }

	    if (file) {
		execl(file, name, 0);
	    }
	    else {
		if (strcmp(CONFIG_NEW_WINDOW_COMMAND, name)) {
		    execlp(name, name, 0);
		}
		else {
		    execlp(name, name, CONFIG_NEW_WINDOW_COMMAND_OPTIONS);
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


#if CONFIG_GNOME_COMPLIANCE != False

void WindowManager::gnomeInitialiseCompliance()
{
    // NOTE that this has been altered to coexist with the
    // multihead code; but we're only using screen 0 here
    // as I have no idea how GNOME copes with multiheads --cc

    // most of this is taken verbatim from 
    // http://www.gnome.org/devel/gnomewm

    // some day we will be more compliant
    Atom list[5];
    
    // this part is to tell GNOME we are compliant. The window 
    // is needed to tell GNOME that wmx has exited (i think). 
    gnome_win = XCreateSimpleWindow
        (m_display, m_root[0], -200, -200, 5, 5, 0, 0, 0);

    XChangeProperty
        (m_display, m_root[0], Atoms::gnome_supportingWmCheck, XA_CARDINAL, 32, 
         PropModeReplace, (unsigned char*)&gnome_win, 1);
    
    // also going to add the desktop button proxy stuff to the same window

    XChangeProperty
        (m_display, m_root[0], Atoms::gnome_winDesktopButtonProxy, XA_CARDINAL,
         32, PropModeReplace, (unsigned char*)&gnome_win, 1);

    XChangeProperty
        (m_display, gnome_win, Atoms::gnome_supportingWmCheck, XA_CARDINAL,
         32, PropModeReplace, (unsigned char*)&gnome_win, 1);

    // also going to add the desktop button proxy stuff to the same window

    XChangeProperty
        (m_display, gnome_win, Atoms::gnome_winDesktopButtonProxy, XA_CARDINAL,
         32, PropModeReplace, (unsigned char*)&gnome_win, 1);

    // Now we need to tell GNOME how compliant we are
    
    // to be compliant we now need to have a list that is updated 
    // with all the window id's. This is taken care of whenever
    // we call m_clients.append() and .remove()
    list[0] = Atoms::gnome_clienList;
    list[1] = Atoms::gnome_workspace;
    list[2] = Atoms::gnome_workspaceCount;
    list[3] = Atoms::gnome_workspaceNames;
    list[4] = Atoms::gnome_winLayer;
    
    XChangeProperty
        (m_display, m_root[0], Atoms::gnome_protocols, XA_ATOM, 32,
         PropModeReplace, (unsigned char *)list, 5);

    gnomeUpdateChannelList();
}

void WindowManager::gnomeUpdateWindowList()
{
    Atom atom_set;
    long num;
    int  i;
    int  realNum;

    int hiddenCount = m_hiddenClients.count();
    int orderedCount = m_orderedClients.count();

    Window *windows = new Window[hiddenCount + orderedCount];
    int j = 0;

    /* henri@qais.com requested removal 20001128 --cc
    for (i = 0; i < hiddenCount; i++) {
        if (!m_hiddenClients.item(i)->isKilled()) {
            windows[j++] = m_hiddenClients.item(i)->window();
        }
    }
    */

    for (i = 0; i < orderedCount; i++) {
	if (!m_orderedClients.item(i)->isKilled()) {
            windows[j++] = m_orderedClients.item(i)->window();
	}
    }

    XChangeProperty
        (m_display, m_root[0], Atoms::gnome_clienList, XA_CARDINAL, 32, 
         PropModeReplace, (unsigned char*)windows, j);

    delete windows;
}

void WindowManager::gnomeUpdateChannelList()
{
    int     i;
    char  **names, s[1024];
    CARD32  val;
   
    // how many channels are there?
    val = (CARD32) m_channels;
 
    XChangeProperty
        (m_display, m_root[0], Atoms::gnome_workspaceCount, XA_CARDINAL, 32, 
         PropModeReplace, (unsigned char *)&val, 1);

    // set the names of the channels
    names = (char **)malloc(sizeof(char *) * m_channels);

    for (i = 0; i < m_channels; i++) {
        snprintf(s, sizeof(s), "Channel %i", i + 1);
        names[i] = (char *)malloc(strlen(s) + 1);
        strcpy(names[i], s);
    }
    
    XTextProperty textProp;

    if (XStringListToTextProperty(names, m_channels, &textProp)) {
	XSetTextProperty
            (m_display, m_root[0], &textProp, Atoms::gnome_workspaceNames);
	XFree(textProp.value);
    }
    
    for (i = 0; i < m_channels; i++) free(names[i]);
    free(names);

    gnomeUpdateCurrentChannel();
}   

void WindowManager::gnomeUpdateCurrentChannel()
{
    CARD32 val;

    // gnome numbers then 0... not 1...
    val =(CARD32)(m_currentChannel - 1);

    XChangeProperty
        (m_display, m_root[0], Atoms::gnome_workspace, XA_CARDINAL, 32, 
         PropModeReplace, (unsigned char *)&val, 1);

    gnomeUpdateWindowList();
}   

#endif

