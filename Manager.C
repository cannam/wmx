
#include "Manager.h"
#include "Menu.h"
#include "Client.h"
#include <string.h>
#include <X11/Xproto.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Cursors.h"

Atom    Atoms::wm_state;
Atom    Atoms::wm_changeState;
Atom    Atoms::wm_protocols;
Atom    Atoms::wm_delete;
Atom    Atoms::wm_takeFocus;
Atom    Atoms::wm_colormaps;
Atom    Atoms::wmx_running;

int     WindowManager::m_signalled = False;
Boolean WindowManager::m_initialising = False;
Boolean ignoreBadWindowErrors;

implementPList(ClientList, Client);


WindowManager::WindowManager() :
    m_focusChanging(False),
    m_altPressed(False)
{
    fprintf(stderr, "\nwmx: Copyright (c) 1996-7 Chris Cannam."
	    "  Second release, May 1997\n"
	    "     Parts derived from 9wm Copyright (c) 1994-96 David Hogan\n"
	    "     Command menu code Copyright (c) 1997 Jeremy Fitzhardinge\n"
	    "     %s\n     Copying and redistribution encouraged.  "
	    "No warranty.\n\n", XV_COPYRIGHT);

    if (CONFIG_AUTO_RAISE) {
	if (CONFIG_CLICK_TO_FOCUS) {
	    fatal("can't have auto-raise-with-delay with click-to-focus");
	} else if (CONFIG_RAISE_ON_FOCUS) {
	    fatal("can't have raise-on-focus AND auto-raise-with-delay");
	} else {
	    fprintf(stderr, "     Focus follows, auto-raise with delay.  ");
	}

    } else {
	if (CONFIG_CLICK_TO_FOCUS) {
	    if (CONFIG_RAISE_ON_FOCUS) {
		fprintf(stderr, "     Click to focus.  ");
	    } else {
		fatal("can't have click-to-focus without raise-on-focus");
	    }
	} else {
	    if (CONFIG_RAISE_ON_FOCUS) {
		fprintf(stderr, "     Focus follows, auto-raise.  ");
	    } else {
		fprintf(stderr, "     Focus follows pointer.  ");
	    }
	}
    }

    if (CONFIG_EVERYTHING_ON_ROOT_MENU) {
	fprintf(stderr, "All clients on menu.\n");
    } else {
	fprintf(stderr, "Hidden clients only on menu.\n");
    }

    if (CONFIG_PROD_SHAPE) {
	fprintf(stderr, "     Shape prodding on.  ");
    } else {
	fprintf(stderr, "     Shape prodding off.  ");
    }

    if (CONFIG_USE_PIXMAPS) {
	fprintf(stderr, "Fancy borders.  ");
    } else {
	fprintf(stderr, "Plain borders.  ");
    }

    if (CONFIG_MAD_FEEDBACK) {
	fprintf(stderr, "Skeletal feedback on.  ");
    } else {
	fprintf(stderr, "Skeletal feedback off.  ");
    }

    if (CONFIG_USE_KEYBOARD) {
	fprintf(stderr, "\n     Keyboard controls available.  ");
    } else {
	fprintf(stderr, "\n     No keyboard controls.  ");
    }

    fprintf(stderr, "Command menu taken from $HOME/"
	    CONFIG_COMMAND_MENU ".");

    fprintf(stderr, "\n     (To reconfigure, simply edit and recompile.)\n\n");

    m_display = XOpenDisplay(NULL);
    if (!m_display) fatal("can't open display");

    m_shell = (char *)getenv("SHELL");
    if (!m_shell) m_shell = NewString("/bin/sh");

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

    int dummy;
    if (!XShapeQueryExtension(m_display, &m_shapeEvent, &dummy))
	fatal("no shape extension, can't run without it");

    // we only cope with one screen!
    initialiseScreen();

    XSetSelectionOwner(m_display, Atoms::wmx_running,
		       None, timestamp(True)); // used to have m_menuWindow
    XSync(m_display, False);
    m_initialising = False;
    m_returnCode = 0;
    
    clearFocus();
    scanInitialWindows();
//    flipChannel(True);
    loop();
}


WindowManager::~WindowManager()
{
    // empty
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
	fprintf(stderr, "wmx: another window manager running?\n");
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
			 XColor *fg, XColor *bg)
{
    Pixmap pixmap =
 	XCreateBitmapFromData(d, w, (const char *)bits, width, height);

    Pixmap mask =
 	XCreateBitmapFromData(d, w, (const char *)mask_bits, width, height);

    Cursor cursor = XCreatePixmapCursor(d, pixmap, mask, fg, bg, xhot, yhot);
    XFreePixmap(d, pixmap);
    XFreePixmap(d, mask);

    return cursor;
}


void WindowManager::initialiseScreen()
{
    int i = 0;
    m_screenNumber = i;

    m_root = RootWindow(m_display, i);
    m_defaultColormap = DefaultColormap(m_display, i);
    m_minimumColormaps = MinCmapsOfScreen(ScreenOfDisplay(m_display, i));

    XColor black, white, temp;

    if (!XAllocNamedColor(m_display, m_defaultColormap, "black", &black, &temp))
	fatal("couldn't load colour \"black\"!");
    if (!XAllocNamedColor(m_display, m_defaultColormap, "white", &white, &temp))
	fatal("couldn't load colour \"white\"!");

    m_cursor = makeCursor
	(m_display, m_root, cursor_bits, cursor_mask_bits,
	 cursor_width, cursor_height, cursor_x_hot,
	 cursor_y_hot, &black, &white);

    m_xCursor = makeCursor
	(m_display, m_root, ninja_cross_bits, ninja_cross_mask_bits,
	 ninja_cross_width, ninja_cross_height, ninja_cross_x_hot,
	 ninja_cross_y_hot, &black, &white);

    m_hCursor = makeCursor
	(m_display, m_root, cursor_right_bits, cursor_right_mask_bits,
	 cursor_right_width, cursor_right_height, cursor_right_x_hot,
	 cursor_right_y_hot, &black, &white);

    m_vCursor = makeCursor
	(m_display, m_root, cursor_down_bits, cursor_down_mask_bits,
	 cursor_down_width, cursor_down_height, cursor_down_x_hot,
	 cursor_down_y_hot, &black, &white);

    m_vhCursor = makeCursor
	(m_display, m_root, cursor_down_right_bits, cursor_down_right_mask_bits,
	 cursor_down_right_width, cursor_down_right_height,
	 cursor_down_right_x_hot, cursor_down_right_y_hot, &black, &white);

    XSetWindowAttributes attr;
    attr.cursor = m_cursor;
    attr.event_mask = SubstructureRedirectMask | SubstructureNotifyMask |
	ColormapChangeMask | ButtonPressMask | ButtonReleaseMask | 
	PropertyChangeMask | LeaveWindowMask | KeyPressMask | KeyReleaseMask;
    XChangeWindowAttributes(m_display, m_root, CWCursor | CWEventMask, &attr);
    XSync(m_display, False);
}


unsigned long WindowManager::allocateColour(char *name, char *desc)
{
    XColor nearest, ideal;

    if (!XAllocNamedColor
	(display(), DefaultColormap(display(), m_screenNumber), name,
	 &nearest, &ideal)) {

	char error[100];
	sprintf(error, "couldn't load %s colour", desc);
	fatal(error);

    } else return nearest.pixel;
}


void WindowManager::installCursor(RootCursor c)
{
    installCursorOnWindow(c, m_root);
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
	XChangeProperty(m_display, m_root, Atoms::wmx_running,
			Atoms::wmx_running, 8, PropModeAppend,
			(unsigned char *)"", 0);
	XMaskEvent(m_display, PropertyChangeMask, &event);

	m_currentTime = event.xproperty.time;
    }

    return m_currentTime;
}

void WindowManager::sigHandler()
{
    m_signalled = True;
}

void WindowManager::scanInitialWindows()
{
    unsigned int i, n;
    Window w1, w2, *wins;
    XWindowAttributes attr;

    XQueryTree(m_display, m_root, &w1, &w2, &wins, &n);

    for (i = 0; i < n; ++i) {

	XGetWindowAttributes(m_display, wins[i], &attr);
//	if (attr.override_redirect || wins[i] == m_menuWindow) continue;
	if (attr.override_redirect) continue;

	(void)windowToClient(wins[i], True);
    }

    XFree((void *)wins);
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
	Client *newC = new Client(this, w);
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
	XInstallColormap(m_display, m_defaultColormap);
    } else {
	XInstallColormap(m_display, cmap);
    }
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
}


void WindowManager::removeFromHiddenList(Client *c)
{
    for (int i = 0; i < m_hiddenClients.count(); ++i) {
	if (m_hiddenClients.item(i) == c) {
	    m_hiddenClients.remove(i);
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
    }
}


void WindowManager::removeFromOrderedList(Client *c)
{
    for (int i = 0; i < m_orderedClients.count(); ++i) {
	if (m_orderedClients.item(i) == c) {
	    m_orderedClients.remove(i);
	    return;
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

		char *pstring = (char *)malloc(strlen(displayName) + 10);
		sprintf(pstring, "DISPLAY=%s", displayName);
		putenv(pstring);
	    }

	    if (CONFIG_EXEC_USING_SHELL) {
		if (file) execl(m_shell, m_shell, "-c", file, 0);
		else execl(m_shell, m_shell, "-c", name, 0);
		fprintf(stderr, "wmx: exec %s", m_shell);
		perror(" failed");
	    }

	    if (file) execl(file, name, 0);
	    else execlp(name, name, 0);
	    XBell(display(), 70);
	    fprintf(stderr, "wmx: exec %s:%s failed (errno %d)\n",
		    name, file, errno);

//	    execlp(CONFIG_NEW_WINDOW_COMMAND, CONFIG_NEW_WINDOW_COMMAND, 0);
//	    fprintf(stderr, "wmx: exec %s", CONFIG_NEW_WINDOW_COMMAND);
//	    perror(" failed");

//	    execlp("xterm", "xterm", "-ut", 0);
//	    perror("wmx: exec xterm failed");
	    exit(1);
	}
	exit(0);
    }
    wait((int *) 0);
}


