
#include "Manager.h"
#include "Client.h"

#include <X11/Xutil.h>
#include <X11/keysym.h>

#if I18N
#include <X11/Xmu/Atoms.h>
#endif

#if CONFIG_GNOME_COMPLIANCE != False
// needed this to be able to use CARD32
#include <X11/Xmd.h>
#endif

const char *const Client::m_defaultLabel = "incognito";



Client::Client(WindowManager *const wm, Window w, Boolean shaped) :
    m_window(w),
    m_transient(None),
    m_revert(0),
    m_sticky(False),
    m_fixedSize(False),
    m_state(WithdrawnState),
    m_managed(False),
    m_reparenting(False),
    m_stubborn(False),
    m_lastPopTime(0L),
    m_isFullHeight(False),
    m_isFullWidth(False),
    m_colormap(None),
    m_colormapWinCount(0),
    m_colormapWindows(NULL),
    m_windowColormaps(NULL),
    m_windowManager(wm),
    m_shaped(shaped)
{

    XWindowAttributes attr;
    XGetWindowAttributes(display(), m_window, &attr);

    m_x = attr.x;
    m_y = attr.y;
    m_w = attr.width;
    m_h = attr.height;
    m_bw = attr.border_width;
    m_wroot = attr.root;
    m_name = m_iconName = 0;
    m_sizeHints.flags = 0L;

    wm->setScreenFromRoot(m_wroot);
    m_screen = wm->screen();
    
    m_label = NewString(m_defaultLabel);
    m_border = new Border(this, w);

    m_channel = wm->channel();
    m_unmappedForChannel = False;

#if CONFIG_GNOME_COMPLIANCE != False
    gnomeSetChannel();
#endif


//#if CONFIG_MAD_FEEDBACK != 0
    m_speculating = m_levelRaised = False;
//#endif

    if (attr.map_state == IsViewable) manage(True);
}


Client::~Client()
{
    // empty
}    


Boolean Client::hasWindow(Window w)
{
    return ((m_window == w) || m_border->hasWindow(w));
}

Window Client::root()
{
    return m_wroot;
}


int Client::screen()
{
    return m_screen;
}

void Client::release()
{
    // assume wm called for this, and will remove me from its list itself

    if (m_window == None) {
	fprintf(stderr,
		"wmx: invalid parent in Client::release (released twice?)\n");
    }

    windowManager()->skipInRevert(this, m_revert);

    if (isHidden()) unhide(False);
    windowManager()->removeFromOrderedList(this);

    delete m_border;
    m_window = None;

    if (isActive()) {
	if (CONFIG_CLICK_TO_FOCUS) {
	    if (m_revert) {
		windowManager()->setActiveClient(m_revert);
		m_revert->activate();
	    } else windowManager()->setActiveClient(0);
	} else {
	    windowManager()->setActiveClient(0);
	}
    }

    if (m_colormapWinCount > 0) {
	XFree((char *)m_colormapWindows);
	free((char *)m_windowColormaps); // not allocated through X
    }

    if (m_iconName) XFree(m_iconName);
    if (m_name)     XFree(m_name);
    if (m_label) free((void *)m_label);
    
    delete this;
}


void Client::unreparent()
{
    XWindowChanges wc;

    if (!isWithdrawn()) {
	gravitate(True);
	XReparentWindow(display(), m_window, root(), m_x, m_y);
    }

    wc.border_width = m_bw;
    XConfigureWindow(display(), m_window, CWBorderWidth, &wc);

    XSync(display(), True);
}


void Client::installColormap()
{
    Client *cc = 0;
    int i, found;

    if (m_colormapWinCount != 0) {

	found = 0;

	for (i = m_colormapWinCount - 1; i >= 0; --i) {
	    windowManager()->installColormap(m_windowColormaps[i]);
	    if (m_colormapWindows[i] == m_window) ++found;
	}

	if (found == 0) {
	    windowManager()->installColormap(m_colormap);
	}

    } else if (m_transient != None &&
 	       (cc = windowManager()->windowToClient(m_transient))) {
	if (cc)
	    cc->installColormap();
    } else {
	windowManager()->installColormap(m_colormap);
    }
}


void Client::manage(Boolean mapped)
{
    static int lastX = 0, lastY = 0;
    Boolean shouldHide, reshape;
    XWMHints *hints;
    Display *d = display();
    long mSize;
    int state;

    XSelectInput(d, m_window, ColormapChangeMask | EnterWindowMask |
		 PropertyChangeMask | FocusChangeMask | KeyPressMask |
		 KeyReleaseMask); //!!!

    if (CONFIG_USE_KEYBOARD) {

	int i;
	int keycode;

	static KeySym keys[] = {
	    CONFIG_FLIP_UP_KEY, CONFIG_FLIP_DOWN_KEY, CONFIG_CIRCULATE_KEY,
	    CONFIG_HIDE_KEY, CONFIG_DESTROY_KEY, CONFIG_RAISE_KEY,
	    CONFIG_LOWER_KEY, CONFIG_FULLHEIGHT_KEY, CONFIG_NORMALHEIGHT_KEY,
            CONFIG_FULLWIDTH_KEY, CONFIG_NORMALWIDTH_KEY,
	    CONFIG_MAXIMISE_KEY, CONFIG_UNMAXIMISE_KEY,
            CONFIG_STICKY_KEY

#if CONFIG_WANT_KEYBOARD_MENU
	    , CONFIG_CLIENT_MENU_KEY, CONFIG_COMMAND_MENU_KEY
#endif
	};

        XGrabKey(display(), XKeysymToKeycode(display(), CONFIG_ALT_KEY),
                 0, m_window, True, GrabModeAsync, GrabModeAsync);

	for (i = 0; i < sizeof(keys)/sizeof(keys[0]); ++i) {
	    keycode = XKeysymToKeycode(display(), keys[i]);
	    if (keycode) {
		XGrabKey(display(), keycode,
			 m_windowManager->altModMask()|LockMask|Mod2Mask,
			 m_window, True,
			 GrabModeAsync, GrabModeAsync);
		XGrabKey(display(), keycode,
			 m_windowManager->altModMask()|LockMask,
			 m_window, True,
			 GrabModeAsync, GrabModeAsync);
		XGrabKey(display(), keycode,
			 m_windowManager->altModMask()|Mod2Mask,
			 m_window, True,
			 GrabModeAsync, GrabModeAsync);
		XGrabKey(display(), keycode,
			 m_windowManager->altModMask(),
			 m_window, True,
			 GrabModeAsync, GrabModeAsync);
	    }
	}

#if CONFIG_GROUPS != False
	static KeySym numbers[] = {
	    XK_0, XK_1, XK_2, XK_3, XK_4, XK_5, XK_5, 
	    XK_6, XK_7, XK_8, XK_9 };

	for (i = 0; i < sizeof(numbers)/sizeof(numbers[0]); ++i) {
	    keycode = XKeysymToKeycode(display(), numbers[i]);
	    if (keycode) {
		// someone please tell me there is a better way of 
		// doing this....

		// both caps-lock and num-lock
		XGrabKey(display(), keycode,
			 m_windowManager->altModMask()|CONFIG_GROUP_REMOVE_ALL|
			 LockMask|Mod2Mask,
			 m_window, True,
			 GrabModeAsync, GrabModeAsync);
		XGrabKey(display(), keycode,
			 m_windowManager->altModMask()|CONFIG_GROUP_ADD|
			 LockMask|Mod2Mask,
			 m_window, True,
			 GrabModeAsync, GrabModeAsync);
		XGrabKey(display(), keycode,
			 m_windowManager->altModMask()|LockMask|Mod2Mask,
			 m_window, True,
			 GrabModeAsync, GrabModeAsync);

		// only caps-lock
		XGrabKey(display(), keycode,
			 m_windowManager->altModMask()|CONFIG_GROUP_REMOVE_ALL|
			 LockMask,
			 m_window, True,
			 GrabModeAsync, GrabModeAsync);
		XGrabKey(display(), keycode,
			 m_windowManager->altModMask()|CONFIG_GROUP_ADD|LockMask,
			 m_window, True,
			 GrabModeAsync, GrabModeAsync);
		XGrabKey(display(), keycode,
			 m_windowManager->altModMask()|LockMask,
			 m_window, True,
			 GrabModeAsync, GrabModeAsync);
		// only num-lock
		XGrabKey(display(), keycode,
			 m_windowManager->altModMask()|CONFIG_GROUP_REMOVE_ALL|
			 Mod2Mask,
			 m_window, True,
			 GrabModeAsync, GrabModeAsync);
		XGrabKey(display(), keycode,
			 m_windowManager->altModMask()|CONFIG_GROUP_ADD|Mod2Mask,
			 m_window, True,
			 GrabModeAsync, GrabModeAsync);
		XGrabKey(display(), keycode,
			 m_windowManager->altModMask()|Mod2Mask,
			 m_window, True,
			 GrabModeAsync, GrabModeAsync);
		// no locks
		XGrabKey(display(), keycode,
			 m_windowManager->altModMask()|CONFIG_GROUP_REMOVE_ALL,
			 m_window, True,
			 GrabModeAsync, GrabModeAsync);
		XGrabKey(display(), keycode,
			 m_windowManager->altModMask()|CONFIG_GROUP_ADD,
			 m_window, True,
			 GrabModeAsync, GrabModeAsync);
		XGrabKey(display(), keycode,
			 m_windowManager->altModMask(),
			 m_window, True,
			 GrabModeAsync, GrabModeAsync);

	    }
	}
#endif
	keycode = XKeysymToKeycode(display(), CONFIG_QUICKRAISE_KEY);
	if (keycode) {
	    XGrabKey(display(), keycode, AnyModifier, m_window, True,
		     GrabModeAsync, GrabModeAsync);
	}

	keycode = XKeysymToKeycode(display(), CONFIG_QUICKHIDE_KEY);
	if (keycode) {
	    XGrabKey(display(), keycode, AnyModifier, m_window, True,
		     GrabModeAsync, GrabModeAsync);
	}

	keycode = XKeysymToKeycode(display(), CONFIG_QUICKHEIGHT_KEY);
	if (keycode) {
	    XGrabKey(display(), keycode, AnyModifier, m_window, True,
		     GrabModeAsync, GrabModeAsync);
	}

	if (CONFIG_USE_CHANNEL_KEYS) {
	    for (i = 0; i < 12; ++i) {
		keycode = XKeysymToKeycode(display(), XK_F1 + i);
		if (keycode) {
		    XGrabKey(display(), keycode,
			     m_windowManager->altModMask(), m_window, True,
			     GrabModeAsync, GrabModeAsync);
		}
	    }
	}
    }

    m_iconName = getProperty(XA_WM_ICON_NAME);
    m_name = getProperty(XA_WM_NAME);
    setLabel();

    getColormaps();
    getProtocols();
    getTransient();

    hints = XGetWMHints(d, m_window);

#if CONFIG_USE_WINDOW_GROUPS != False
    m_groupParent = hints ? hints->window_group : None;
    if (m_groupParent == None) m_groupParent = m_window;
//    fprintf(stderr, "Client %p (%s) has window %ld and groupParent %ld\n",
//	    this, m_name, m_window, m_groupParent);
#endif

    if (!getState(&state)) {
	state = hints ? hints->initial_state : NormalState;
    }

    shouldHide = (state == IconicState);
    if (hints) XFree(hints);

    if (XGetWMNormalHints(d, m_window, &m_sizeHints, &mSize) == 0 ||
	m_sizeHints.flags == 0) {
	m_sizeHints.flags = PSize;
    }

    m_fixedSize = False;
    if ((m_sizeHints.flags & (PMinSize | PMaxSize)) == (PMinSize | PMaxSize) &&
	(m_sizeHints.min_width  == m_sizeHints.max_width &&
	 m_sizeHints.min_height == m_sizeHints.max_height)) m_fixedSize = True;

    reshape = !mapped;

    if (m_fixedSize) {
	if ((m_sizeHints.flags & USPosition)) reshape = False;
	if ((m_sizeHints.flags & PPosition) && shouldHide) reshape = False;
	if ((m_transient != None)) reshape = False;
    }

    if ((m_sizeHints.flags & PBaseSize)) {
	m_minWidth  = m_sizeHints.base_width;
	m_minHeight = m_sizeHints.base_height;
    } else if ((m_sizeHints.flags & PMinSize)) {
	m_minWidth  = m_sizeHints.min_width;
	m_minHeight = m_sizeHints.min_height;
    } else {
	m_minWidth = m_minHeight = 50;
    }

    // act

    gravitate(False);

    // zeros are iffy, should be calling some Manager method
    int dw = DisplayWidth(display(), 0), dh = DisplayHeight(display(), 0);

    if (m_w < m_minWidth) {
	m_w = m_minWidth; m_fixedSize = False; reshape = True;
    }
    if (m_h < m_minHeight) {
	m_h = m_minHeight; m_fixedSize = False; reshape = True;
    }

    if (m_w > dw - 8) m_w = dw - 8;
    if (m_h > dh - 8) m_h = dh - 8;

    if (!mapped && m_transient == None &&
	!(m_sizeHints.flags & (PPosition | USPosition))) {

	lastX += 60; lastY += 40;

	if (lastX + m_w + m_border->xIndent() > dw) {
	    lastX = 0;
	}
	if (lastY + m_h + m_border->yIndent() > dh) {
	    lastY = 0;
	}
	m_x = lastX; m_y = lastY;
    }

    if (m_x > dw - m_border->xIndent()) {
	m_x = dw - m_border->xIndent();
    }

    if (m_y > dh - m_border->yIndent()) {
	m_y = dh - m_border->yIndent();
    }

    if (m_x < m_border->xIndent()) m_x = m_border->xIndent();
    if (m_y < m_border->yIndent()) m_y = m_border->yIndent();
    
    m_border->configure(m_x, m_y, m_w, m_h, 0L, Above);

    if (mapped) m_reparenting = True;
    if (reshape && !m_fixedSize) XResizeWindow(d, m_window, m_w, m_h);
    XSetWindowBorderWidth(d, m_window, 0);

    m_border->reparent();

    // (support for shaped windows absent)

    XAddToSaveSet(d, m_window);
    m_managed = True;

    if (shouldHide) hide();
    else {
	XMapWindow(d, m_window);
	m_border->map();
	setState(NormalState);

	if (CONFIG_CLICK_TO_FOCUS ||
	    (m_transient != None && activeClient() &&
	    activeClient()->m_window == m_transient)) {
	    activate();
	    mapRaised();
	} else {
	    deactivate();
	}
    }
    
    if (activeClient() && !isActive()) {
	activeClient()->installColormap();
    }

    if (CONFIG_AUTO_RAISE) {
	m_windowManager->stopConsideringFocus();
	focusIfAppropriate(False);
    }

    m_windowManager->hoistToTop(this);
    sendConfigureNotify(); // due to Martin Andrews
}


void Client::selectOnMotion(Window w, Boolean select)
{
    if (!CONFIG_AUTO_RAISE) return;
    if (!w || w == root()) return;

    if (w == m_window || m_border->hasWindow(w)) {
	XSelectInput(display(), m_window, // not "w"
		     ColormapChangeMask | EnterWindowMask |
		     PropertyChangeMask | FocusChangeMask |
		     (select ? PointerMotionMask : 0L));
    } else {
	XSelectInput(display(), w, select ? PointerMotionMask : 0L);
    }
}


void Client::decorate(Boolean active)
{
    m_border->decorate(active, m_w, m_h);
}


void Client::activate()
{
//    fprintf(stderr, "Client::activate (this = %p, window = %x, parent = %x)\n",
//	    this, m_window, parent());

    if (parent() == root()) {
	fprintf(stderr, "wmx: warning: bad parent in Client::activate\n");
	return;
    }

    if (!m_managed || isHidden() || isWithdrawn() ||
	(m_channel != windowManager()->channel())) return;

    if (isActive()) {
	decorate(True);
	if (CONFIG_AUTO_RAISE || CONFIG_RAISE_ON_FOCUS) mapRaised();
	return;
    }

    if (activeClient()) {
	activeClient()->deactivate();
	// & some other-screen business
    }

    XUngrabButton(display(), AnyButton, AnyModifier, parent());

    XSetInputFocus(display(), m_window, RevertToPointerRoot,
		   windowManager()->timestamp(False));

    if (m_protocol & PtakeFocus) {
	sendMessage(Atoms::wm_protocols, Atoms::wm_takeFocus);
    }

    // now set revert of window that reverts to this one so as to
    // revert to the window this one used to revert to (huh?)

    windowManager()->skipInRevert(this, m_revert);

    m_revert = activeClient();
    while (m_revert && !m_revert->isNormal()) m_revert = m_revert->revertTo();

    windowManager()->setActiveClient(this);
    decorate(True);

    installColormap();		// new!
}


void Client::deactivate()	// called from wm?
{
    if (parent() == root()) {
	fprintf(stderr, "wmx: warning: bad parent in Client::deactivate\n");
	return;
    }

    XGrabButton(display(), AnyButton, AnyModifier, parent(), False,
		ButtonPressMask | ButtonReleaseMask,
		GrabModeAsync, GrabModeSync, None, None);

    decorate(False);
}


void Client::sendMessage(Atom a, long l)
{
    XEvent ev;
    int status;
    long mask;

    memset(&ev, 0, sizeof(ev));
    ev.xclient.type = ClientMessage;
    ev.xclient.window = m_window;
    ev.xclient.message_type = a;
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = l;
    ev.xclient.data.l[1] = windowManager()->timestamp(False);
    mask = 0L;
    status = XSendEvent(display(), m_window, False, mask, &ev);

    if (status == 0) {
	fprintf(stderr, "wmx: warning: Client::sendMessage failed\n");
    }
}


static int getProperty_aux(Display *d, Window w, Atom a, Atom type, long len,
			   unsigned char **p)
{
    Atom realType;
    int format;
    unsigned long n, extra;
    int status;
    Boolean retried = False;

tryagain:
    status = XGetWindowProperty(d, w, a, 0L, len, False, type, &realType,
				&format, &n, &extra, p);

    if (status != Success || *p == 0) return -1;
    if (n == 0) XFree((void *) *p);
    if (type == XA_STRING || type == AnyPropertyType || retried) {
	if (realType != XA_STRING || format != 8) {
#if I18N
	    // XA_STRING is needed by a caller.  But XGetWindowProperty()
	    // returns other typed data.  So try to convert them.
	    XTextProperty textprop;
	    textprop.value = *p;
	    textprop.encoding = realType;
	    textprop.format = format;
	    textprop.nitems = n;

	    char **list;
	    int num, cnt;
	    cnt = XmbTextPropertyToTextList(d, &textprop, &list, &num);
	    unsigned char *string;
	    if (cnt == Success && num > 0 && *list) {
		string = (unsigned char*)NewString(*list);
		XFreeStringList(list);
		n = num;
	    } else if (cnt > Success) {
		fprintf(stderr, "Something wrong, cannot conver "
			"text property\n"
			"  original type %ld, string %s\n"
			"  converted string %s\n"
			"  decide to use original value as STRING\n",
			textprop.encoding, textprop.value, *list);
		string = (unsigned char*)NewString(*(char**)p);
		XFreeStringList(list);
		n = n;
	    } else if (!retried) {
		retried = True;
		type = AnyPropertyType;
		goto tryagain;
	    } else {
		string = NULL;
		n = 0;
	    }
	    XFree((void *) *p);
	    *p = string;
#else
	    XFree((void *) *p);
	    *p = NULL;
#endif
	}
    }

    return n;
}


char *Client::getProperty(Atom a)
{
    unsigned char *p;

    // no -- allow any type, not just string -- SGI has "compound
    // text", and part-garbage is possibly better than "incognito" --
    // thanks to Bill Spitzak

//    if (getProperty_aux(display(), m_window, a, XA_STRING, 100L, &p) <= 0) {
    if (getProperty_aux(display(), m_window, a, AnyPropertyType, 100L, &p) <= 0) {
	return NULL;
    }
    return (char *)p;
}


int Client::getAtomProperty(Atom a, Atom type)
{
    char **p, *x;
    if (getProperty_aux(display(), m_window, a, type, 1L,
			(unsigned char **)&p) <= 0) {
	return 0;
    }

    x = *p;
    XFree((void *)p);
    return (int)x;
}


int Client::getIntegerProperty(Atom a)
{
    return getAtomProperty(a, XA_INTEGER);
}


void Client::setState(int state)
{
    m_state = state;

    long data[2];
    data[0] = (long)state;
    data[1] = (long)None;

    XChangeProperty(display(), m_window, Atoms::wm_state, Atoms::wm_state,
		    32, PropModeReplace, (unsigned char *)data, 2);
}


Boolean Client::getState(int *state)
{
    long *p = 0;

    if (getProperty_aux(display(), m_window, Atoms::wm_state, Atoms::wm_state,
			2L, (unsigned char **)&p) <= 0) {
	return False;
    }

    *state = (int) *p;
    XFree((char *)p);
    return True;
}


void Client::getProtocols()
{
    long n;
    Atom *p;

    m_protocol = 0;
    if ((n = getProperty_aux(display(), m_window, Atoms::wm_protocols, XA_ATOM,
			     20L, (unsigned char **)&p)) <= 0) {
	return;
    }

    for (int i = 0; i < n; ++i) {
	if (p[i] == Atoms::wm_delete) {
	    m_protocol |= Pdelete;
	} else if (p[i] == Atoms::wm_takeFocus) {
	    m_protocol |= PtakeFocus;
	}
    }

    XFree((char *) p);
}


void Client::gravitate(Boolean invert)
{
    int gravity;
    int w = 0, h = 0, xdelta, ydelta;

    // possibly shouldn't work if we haven't been managed yet?

    gravity = NorthWestGravity;
    if (m_sizeHints.flags & PWinGravity) gravity = m_sizeHints.win_gravity;

    xdelta = m_bw - m_border->xIndent();
    ydelta = m_bw - m_border->yIndent();

    // note that right and bottom borders have indents of 1

    switch (gravity) {

    case NorthWestGravity:
	break;

    case NorthGravity:
	w = xdelta;
	break;

    case NorthEastGravity:
	w = xdelta + m_bw-1;
	break;

    case WestGravity:
	h = ydelta;
	break;

    case CenterGravity:
    case StaticGravity:
	w = xdelta;
	h = ydelta;
	break;

    case EastGravity:
	w = xdelta + m_bw-1;
	h = ydelta;
	break;

    case SouthWestGravity:
	h = ydelta + m_bw-1;
	break;

    case SouthGravity:
	w = xdelta;
	h = ydelta + m_bw-1;
	break;

    case SouthEastGravity:
	w = xdelta + m_bw-1;
	h = ydelta + m_bw-1;
	break;

    default:
	fprintf(stderr, "wmx: bad window gravity %d for window 0x%lx\n",
		gravity, m_window);
	return;
    }

    w += m_border->xIndent();
    h += m_border->yIndent();

    if (invert) { w = -w; h = -h; }

    m_x += w;
    m_y += h;
}


Boolean Client::setLabel(void)
{
    const char *newLabel;

    if (m_name) newLabel = m_name;
    else if (m_iconName) newLabel = m_iconName;
    else newLabel = m_defaultLabel;

    if (!m_label) {

	m_label = NewString(newLabel);
	return True;

    } else if (strcmp(m_label, newLabel)) {

	free((void *)m_label);
	m_label = NewString(newLabel);
	return True;

    } else return True;//False;// dammit!
}


void Client::getColormaps(void)
{
    int i, n;
    Window *cw;
    XWindowAttributes attr;

    if (!m_managed) {
	XGetWindowAttributes(display(), m_window, &attr);
	m_colormap = attr.colormap;
    }

    n = getProperty_aux(display(), m_window, Atoms::wm_colormaps, XA_WINDOW,
			100L, (unsigned char **)&cw);

    if (m_colormapWinCount != 0) {
	XFree((char *)m_colormapWindows);
	free((char *)m_windowColormaps);
    }

    if (n <= 0) {
	m_colormapWinCount = 0;
	return;
    }
    
    m_colormapWinCount = n;
    m_colormapWindows = cw;

    m_windowColormaps = (Colormap *)malloc(n * sizeof(Colormap));

    for (i = 0; i < n; ++i) {
	if (cw[i] == m_window) {
	    m_windowColormaps[i] = m_colormap;
	} else {
	    XSelectInput(display(), cw[i], ColormapChangeMask);
	    XGetWindowAttributes(display(), cw[i], &attr);
	    m_windowColormaps[i] = attr.colormap;
	}
    }
}


void Client::getTransient()
{
    Window t = None;

    if (XGetTransientForHint(display(), m_window, &t) != 0) {

	if (windowManager()->windowToClient(t) == this) {
	    fprintf(stderr,
		    "wmx: warning: client \"%s\" thinks it's a transient "
		    "for\nitself -- ignoring WM_TRANSIENT_FOR property...\n",
		    m_label ? m_label : "(no name)");
	    m_transient = None;
	} else {		
	    m_transient = t;
	}
    } else {
	m_transient = None;
    }
}


void Client::hide()
{
    if (isHidden()) {
	fprintf(stderr, "wmx: Client already hidden in Client::hide\n");
	return;
    }

    m_border->unmap();
    XUnmapWindow(display(), m_window);

    if (isActive()) windowManager()->clearFocus();

    setState(IconicState);
    windowManager()->addToHiddenList(this);

#if CONFIG_USE_WINDOW_GROUPS != False
    if (isGroupParent()) {
	windowManager()->hideGroup(groupParent(), this);
    }
#endif
}


void Client::unhide(Boolean map)
{
    if (CONFIG_MAD_FEEDBACK) {
	m_speculating = False;
	if (!isHidden()) return;
    }

    if (!isHidden()) {
	fprintf(stderr, "wmx: Client not hidden in Client::unhide\n");
	return;
    }

    windowManager()->removeFromHiddenList(this);

    if (map) {
	setState(NormalState);

	if (m_channel == windowManager()->channel()) {
	    XMapWindow(display(), m_window);
	}
	mapRaised();

	if (CONFIG_AUTO_RAISE) focusIfAppropriate(False);
	else if (CONFIG_CLICK_TO_FOCUS) activate();
    }

#if CONFIG_USE_WINDOW_GROUPS != False
    if (isGroupParent()) {
	windowManager()->unhideGroup(groupParent(), this, map);
    }
#endif
}


void Client::sendConfigureNotify()
{
    XConfigureEvent ce;

    ce.type   = ConfigureNotify;
    ce.event  = m_window;
    ce.window = m_window;

    ce.x = m_x;
    ce.y = m_y;
    ce.width  = m_w;
    ce.height = m_h;
    ce.border_width = m_bw;
    ce.above = None;
    ce.override_redirect = 0;

    XSendEvent(display(), m_window, False, StructureNotifyMask, (XEvent*)&ce);
}


void Client::withdraw(Boolean changeState)
{
    
    

    m_border->unmap();

    gravitate(True);
    XReparentWindow(display(), m_window, root(), m_x, m_y);

    gravitate(False);

    if (changeState) {
	XRemoveFromSaveSet(display(), m_window);
	setState(WithdrawnState);
    }

#if CONFIG_USE_WINDOW_GROUPS != False
    if (isGroupParent()) {
	windowManager()->withdrawGroup(groupParent(), this, changeState);
    }
#endif

    ignoreBadWindowErrors = True;
    XSync(display(), False);
    ignoreBadWindowErrors = False;
}


void Client::rename()
{
    m_border->configure(0, 0, m_w, m_h, CWWidth | CWHeight, Above);
}


void Client::mapRaised()
{
    if (m_channel == windowManager()->channel()) m_border->mapRaised();
    windowManager()->hoistToTop(this);
    windowManager()->raiseTransients(this);
}


void Client::kill()
{
    if (m_protocol & Pdelete) {
	sendMessage(Atoms::wm_protocols, Atoms::wm_delete);
    } else {
	XKillClient(display(), m_window);
    }

#if CONFIG_USE_WINDOW_GROUPS != False
    if (isGroupParent()) {
	windowManager()->killGroup(groupParent(), this);
    }
#endif
}


void Client::ensureVisible()
{
    int mx = DisplayWidth(display(), 0) - 1; // hack
    int my = DisplayHeight(display(), 0) - 1;
    int px = m_x;
    int py = m_y;
    
    if (m_x + m_w > mx) m_x = mx - m_w;
    if (m_y + m_h > my) m_y = my - m_h;
    if (m_x < 0) m_x = 0;
    if (m_y < 0) m_y = 0;

    if (m_x != px || m_y != py) {
	m_border->moveTo(m_x, m_y);
	sendConfigureNotify();
    }
}


void Client::lower()
{
    m_border->lower();
    windowManager()->hoistToBottom(this);
}


void Client::raiseOrLower()
{
    if (windowManager()->isTop(this)) {
	lower();
    } else {
	mapRaised();
    }
}


void Client::maximise(int max)
{
    enum {Vertical, Maximum, Horizontal};
    
    if (max != Vertical && max != Horizontal && max != Maximum)
        return;
    
    if (m_fixedSize || (m_transient != None))
        return;

    if (CONFIG_SAME_KEY_MAX_UNMAX) {
        if (m_isFullHeight && m_isFullWidth) {
            unmaximise(max);
            return;
        } else if (m_isFullHeight && max == Vertical) {
            unmaximise(max);
            return;
        } else if (m_isFullWidth && max == Horizontal) {
            unmaximise(max);
            return;
        }
    } else if ((m_isFullHeight && max == Vertical)
               || (m_isFullHeight && m_isFullWidth && max == Maximum)
               || (m_isFullWidth && max == Horizontal))
        return;

    int w = (max == Horizontal || (max == Maximum && !m_isFullWidth));
    int h = (max == Vertical || (max == Maximum && !m_isFullHeight));

    if (h) {
	m_normalH = m_h;
	m_normalY = m_y;
	m_h = DisplayHeight(display(), windowManager()->screen())
	      - m_border->yIndent() - 1;

    }
    if (w) {
	m_normalW = m_w;
	m_normalX = m_x;
	m_w = DisplayWidth(display(), windowManager()->screen())
	      - m_border->xIndent() - 1;
    }

    int dw, dh;

    fixResizeDimensions(m_w, m_h, dw, dh);

    if (h) {
	if (m_h > m_normalH) {
	    m_y -= (m_h - m_normalH);
	    if (m_y < m_border->yIndent()) m_y = m_border->yIndent();
	}
	m_isFullHeight = True;
    }
    
    if (w) {
        if (m_w > m_normalW) {
	    m_x -= (m_w - m_normalW);
	    if (m_x < m_border->xIndent()) m_x = m_border->xIndent();
	}
	m_isFullWidth = True;
    }
    
    unsigned long mask;

    if (h & w)
	mask = CWY | CWX | CWHeight | CWWidth;
    else if (h)
	mask = CWY | CWHeight;
    else
	mask = CWX | CWWidth;
    
    m_border->configure(m_x, m_y, m_w, m_h, mask, 0, True);

    XResizeWindow(display(), m_window, m_w, m_h);
    sendConfigureNotify();
}


void Client::unmaximise(int max)
{
    enum {Vertical, Maximum, Horizontal};
    
    if (max != Vertical && max != Horizontal && max != Maximum)
        return;
    
    if ((!m_isFullHeight && max == Vertical)
        || (!m_isFullWidth && max == Horizontal)
        || (!(m_isFullHeight && m_isFullWidth) && max == Maximum))
        return;
    
    int w = (max == Horizontal || (max == Maximum && m_isFullWidth));
    int h = (max == Vertical || (max == Maximum && m_isFullHeight));

    if (h) {
        m_h = m_normalH;
	m_y = m_normalY;
	m_isFullHeight = False;
    }
    if (w) {
	m_w = m_normalW;
	m_x = m_normalX;
	m_isFullWidth = False;
    }

    unsigned long mask;

    if (h & w)
	mask = CWY | CWX | CWHeight | CWWidth;
    else if (h)
	mask = CWY | CWHeight;
    else
	mask = CWX | CWWidth;
    
    m_border->configure(m_x, m_y, m_w, m_h, mask, 0, True);
    
    XResizeWindow(display(), m_window, m_w, m_h);
    sendConfigureNotify();
}


void Client::warpPointer()
{
    XWarpPointer(display(), None, parent(), 0, 0, 0, 0,
		 m_border->xIndent() / 2, m_border->xIndent() + 8);
}


void Client::flipChannel(Boolean leaving, int newChannel)
{
//    fprintf(stderr, "I could be supposed to pop up now...\n");

    if (m_channel != windowManager()->channel()) {

	if (CONFIG_MAD_FEEDBACK) {
	    if (leaving && m_channel == newChannel &&
		m_unmappedForChannel) {
		showFeedback();
	    }
	}

	return;
    }

    if (leaving) {

	if (CONFIG_MAD_FEEDBACK) {
	    removeFeedback(isNormal()); // mostly it won't be there anyway, but...
	}

	if (!isNormal()) return;
	m_unmappedForChannel = True;
	XUnmapWindow(display(), m_window);
	withdraw(False);
	return;

    } else {

	if (CONFIG_MAD_FEEDBACK) {
	    removeFeedback(isNormal()); // likewise
	}

	if (!m_unmappedForChannel) {
	    if (isNormal()) mapRaised();
	    return;
	}

	m_unmappedForChannel = False;

	setState(WithdrawnState);
	m_border->reparent();
	if (CONFIG_AUTO_RAISE) m_windowManager->stopConsideringFocus();
	XAddToSaveSet(display(), m_window);
	XMapWindow(display(), m_window);
	setState(NormalState);
	mapRaised();
	if (CONFIG_CLICK_TO_FOCUS) activate();
    }
}


void Client::showFeedback()
{
    if (CONFIG_MAD_FEEDBACK) {
	if (m_speculating || m_levelRaised) removeFeedback(False);
	m_border->showFeedback(m_x, m_y, m_w, m_h);
//    XSync(display(), False);
    }
}

void Client::raiseFeedbackLevel()
{
    if (CONFIG_MAD_FEEDBACK) {

	m_border->removeFeedback();
	m_levelRaised = True;

	if (isNormal()) {
	    mapRaised();
	} else if (isHidden()) {
	    unhide(True);
	    m_speculating = True;
//	XSync(display(), False);
	}
    }
}

void Client::removeFeedback(Boolean mapped)
{
    if (CONFIG_MAD_FEEDBACK) {

	m_border->removeFeedback();
	
	if (m_levelRaised) {
	    if (m_speculating) {
		if (!mapped) hide();
		XSync(display(), False);
	    } else {
		// not much we can do
	    }
	}

	m_speculating = m_levelRaised = False;
    }
}

#if CONFIG_GNOME_COMPLIANCE != False
void Client::gnomeSetChannel() {

    Atom                atom_set;
    CARD32              val;
   
    // which is the current channel (or workspace)
    atom_set = XInternAtom(display(), "_WIN_WORKSPACE", False);

    // gnome numbers then 0... not 1...
    val =(CARD32)(channel() - 1);

    XChangeProperty(display(), window(), atom_set, XA_CARDINAL, 32, 
		    PropModeReplace, (unsigned char *)&val, 1);

}
#endif
