
#include "Manager.h"
#include "Client.h"
#include "Menu.h"
#include <X11/keysym.h>
#include <sys/time.h>


void WindowManager::eventButton(XButtonEvent *e)
{
    Client *c = windowToClient(e->window);

    if (e->button == Button3 && m_channelChangeTime == 0) {
	circulate(e->window == e->root);
	return;
    }

    if (e->window == e->root) {

	if (e->button == Button1 && m_channelChangeTime == 0) {
	    ClientMenu menu(this, e);

	} else if (e->x > DisplayWidth(display(), screen()) -
		   CONFIG_CHANNEL_CLICK_SIZE &&
		   e->y < CONFIG_CHANNEL_CLICK_SIZE) {

	    if (e->button == Button2) {

		if (m_channelChangeTime == 0) flipChannel(False, True);
		else flipChannel(False, False);

	    } else if (e->button == Button1 && m_channelChangeTime != 0) {

		flipChannel(False, False, NULL, True);
	    }

	} else if (e->button == Button2 && m_channelChangeTime == 0) {
	    CommandMenu menu(this, e);
	}

    } else if (c) {

	if (e->button == Button2 && CONFIG_CHANNEL_SURF) {
	    if (m_channelChangeTime == 0) flipChannel(False, True);
	    else flipChannel(False, False, c);
	    return;
	}

	c->eventButton(e);
	return;
    }
}


void WindowManager::circulate(Boolean activeFirst)
{
    Client *c = 0;
    if (activeFirst) c = m_activeClient;

    if (!c) {

	int i, j;

	if (!m_activeClient) i = -1;
	else {
	    for (i = 0; i < m_clients.count(); ++i) {
		if (m_clients.item(i)->channel() != channel()) continue;
		if (m_clients.item(i) == m_activeClient) break;
	    }

	    if (i >= m_clients.count()-1) i = -1;
	}

	for (j = i + 1;
	     (!m_clients.item(j)->isNormal() ||
	       m_clients.item(j)->isTransient() ||
	       m_clients.item(j)->channel() != channel()); ++j) {

	    if (j >= m_clients.count() - 1) j = -1;
	    if (j == i) return; // no suitable clients
	}

	c = m_clients.item(j);
    }

    c->activateAndWarp();
}

void WindowManager::eventKeyPress(XKeyEvent *ev)
{
    KeySym key = XKeycodeToKeysym(display(), ev->keycode, 0);

    if (CONFIG_USE_KEYBOARD && (ev->state & CONFIG_ALT_KEY_MASK)) {

	Client *c = windowToClient(ev->window);

	switch (key) {

	case CONFIG_FLIP_DOWN_KEY:
	    flipChannel(False, False, NULL, True);
	    break;

	case CONFIG_FLIP_UP_KEY:
	    flipChannel(False, False);
	    break;
	
	case CONFIG_CIRCULATE_KEY:
	    circulate(False);
	    break;

	case CONFIG_HIDE_KEY:
	    c->hide();
	    break;

	case CONFIG_DESTROY_KEY:
	    c->kill();
	    break;

	case CONFIG_RAISE_KEY:
	    c->mapRaised();
	    break;

	case CONFIG_LOWER_KEY:
	    c->lower();
	    break;

	case CONFIG_FULLHEIGHT_KEY:
	    c->fullHeight();
	    break;

	case CONFIG_NORMALHEIGHT_KEY:
	    c->normalHeight();
	    break;

	default:
	    return;
	}

	XSync(display(), False);
	XUngrabKeyboard(display(), CurrentTime);
    }

    return;
}

void WindowManager::eventKeyRelease(XKeyEvent *ev)
{
    return;
}

void Client::activateAndWarp()
{
    mapRaised();
    ensureVisible();
    XWarpPointer(display(), None, parent(), 0, 0, 0, 0,
		 m_border->xIndent() / 2, m_border->xIndent() + 8);
    activate();
}


void Client::eventButton(XButtonEvent *e)
{
    if (e->type != ButtonPress) return;

    mapRaised();

    if (e->button == Button1) {
	if (m_border->hasWindow(e->window)) {

	    m_border->eventButton(e);
	}
    }

    if (!isNormal() || isActive() || e->send_event) return;
    activate();
}


int WindowManager::attemptGrab(Window w, Window constrain, int mask, int t)
{
    int status;
    if (t == 0) t = timestamp(False);
    status = XGrabPointer(display(), w, False, mask, GrabModeAsync,
			  GrabModeAsync, constrain, None, t);
    return status;
}


void WindowManager::releaseGrab(XButtonEvent *e)
{
    XEvent ev;
    if (!nobuttons(e)) {
	for (;;) {
	    XMaskEvent(display(), ButtonMask | ButtonMotionMask, &ev);
	    if (ev.type == MotionNotify) continue;
	    e = &ev.xbutton;
	    if (nobuttons(e)) break;
	}
    }

    XUngrabPointer(display(), e->time);
    m_currentTime = e->time;
}


void Client::move(XButtonEvent *e)
{
    int x = -1, y = -1, xoff, yoff;
    Boolean done = False;
    ShowGeometry geometry(m_windowManager, e);

    if (m_windowManager->attemptGrab
	(root(), None, DragMask, e->time) != GrabSuccess) {
	return;
    }

    xoff = m_border->xIndent() - e->x;
    yoff = m_border->yIndent() - e->y;

    XEvent event;
    Boolean found;
    Boolean doSomething = False;
    struct timeval sleepval;

    while (!done) {

	found = False;

	while (XCheckMaskEvent(display(), DragMask | ExposureMask, &event)) {
	    found = True;
	    if (event.type != MotionNotify) break;
	}

	if (!found) {
	    sleepval.tv_sec = 0;
	    sleepval.tv_usec = 50000;
	    select(0, 0, 0, 0, &sleepval);
	    continue;
	}
	
	switch (event.type) {

	default:
	    fprintf(stderr, "wmx: unknown event type %d\n", event.type);
	    break;

	case Expose:
	    m_windowManager->eventExposure(&event.xexpose);
	    break;

	case ButtonPress:
	    // don't like this
	    XUngrabPointer(display(), event.xbutton.time);
	    doSomething = False;
	    done = True;
	    break;

	case ButtonRelease:

	    x = event.xbutton.x; y = event.xbutton.y;
	    if (!nobuttons(&event.xbutton)) doSomething = False;

	    m_windowManager->releaseGrab(&event.xbutton);
	    done = True;
	    break;

	case MotionNotify:
	    x = event.xbutton.x; y = event.xbutton.y;
	    if (x + xoff != m_x || y + yoff != m_y) {
		geometry.update(x + xoff, y + yoff);
		m_border->moveTo(x + xoff, y + yoff);
		doSomething = True;
	    }
	    break;
	}
    }

    geometry.remove();

    if (x >= 0 && doSomething) {
	m_x = x + xoff;
	m_y = y + yoff;
    }

    if (CONFIG_CLICK_TO_FOCUS) activate();
    m_border->moveTo(m_x, m_y);
    sendConfigureNotify();
}


void Client::fixResizeDimensions(int &w, int &h, int &dw, int &dh)
{
    if (w < 50) w = 50;
    if (h < 50) h = 50;
    
    if (m_sizeHints.flags & PResizeInc) {
	w = m_minWidth  + (((w - m_minWidth) / m_sizeHints.width_inc) *
			   m_sizeHints.width_inc);
	h = m_minHeight + (((h - m_minHeight) / m_sizeHints.height_inc) *
			   m_sizeHints.height_inc);

	dw = (w - m_minWidth)  / m_sizeHints.width_inc;
	dh = (h - m_minHeight) / m_sizeHints.height_inc;
    } else {
	dw = w; dh = h;
    }

    if (m_sizeHints.flags & PMaxSize) {
	if (w > m_sizeHints.max_width)  w = m_sizeHints.max_width;
	if (h > m_sizeHints.max_height) h = m_sizeHints.max_height;
    }

    if (w < m_minWidth)  w = m_minWidth;
    if (h < m_minHeight) h = m_minHeight;
}


void Client::resize(XButtonEvent *e, Boolean horizontal, Boolean vertical)
{
    if (isFixedSize()) return;
    ShowGeometry geometry(m_windowManager, e);

    if (m_windowManager->attemptGrab
	(root(), None, DragMask, e->time) != GrabSuccess) {
	return;
    }

    if (vertical && horizontal)
	m_windowManager->installCursor(WindowManager::DownrightCursor);
    else if (vertical)
	m_windowManager->installCursor(WindowManager::DownCursor);
    else
	m_windowManager->installCursor(WindowManager::RightCursor);

    Window dummy;
    XTranslateCoordinates(display(), e->window, parent(),
			  e->x, e->y, &e->x, &e->y, &dummy);

    int xorig = e->x;
    int yorig = e->y;
    int x = xorig;
    int y = yorig;
    int w = m_w, h = m_h;
    int prevW, prevH;
    int dw, dh;

    XEvent event;
    Boolean found;
    Boolean doSomething = False;
    Boolean done = False;
    struct timeval sleepval;

    while (!done) {

	found = False;

	while (XCheckMaskEvent(display(), DragMask | ExposureMask, &event)) {
	    found = True;
	    if (event.type != MotionNotify) break;
	}

	if (!found) {
	    sleepval.tv_sec = 0;
	    sleepval.tv_usec = 50000;
	    select(0, 0, 0, 0, &sleepval);
	    continue;
	}
	
	switch (event.type) {

	default:
	    fprintf(stderr, "wmx: unknown event type %d\n", event.type);
	    break;

	case Expose:
	    m_windowManager->eventExposure(&event.xexpose);
	    break;

	case ButtonPress:
	    // don't like this
	    XUngrabPointer(display(), event.xbutton.time);
	    done = True;
	    break;

	case ButtonRelease:

	    x = event.xbutton.x; y = event.xbutton.y;

	    if (!nobuttons(&event.xbutton)) x = -1;
	    m_windowManager->releaseGrab(&event.xbutton);
	    
	    done = True;
	    break;

	case MotionNotify:
	    x = event.xbutton.x; y = event.xbutton.y;

	    if (vertical && horizontal) {
		prevH = h; h = y - m_y;
		prevW = w; w = x - m_x;
		fixResizeDimensions(w, h, dw, dh);
		if (h == prevH && w == prevW) break;
		m_border->configure(m_x, m_y, w, h, CWWidth | CWHeight, 0);
		if (CONFIG_RESIZE_UPDATE)
		    XResizeWindow(display(), m_window, w, h);
		geometry.update(dw, dh);
		doSomething = True;

	    } else if (vertical) {
		prevH = h; h = y - m_y;
		fixResizeDimensions(w, h, dw, dh);
		if (h == prevH) break;
		m_border->configure(m_x, m_y, w, h, CWHeight, 0);
		if (CONFIG_RESIZE_UPDATE)
		    XResizeWindow(display(), m_window, w, h);
		geometry.update(dw, dh);
		doSomething = True;

	    } else {
		prevW = w; w = x - m_x;
		fixResizeDimensions(w, h, dw, dh);
		if (w == prevW) break;
		m_border->configure(m_x, m_y, w, h, CWWidth, 0);
		if (CONFIG_RESIZE_UPDATE)
		    XResizeWindow(display(), m_window, w, h);
		geometry.update(dw, dh);
		doSomething = True;
	    }

	    break;
	}
    }

    if (doSomething) {

	geometry.remove();

	if (vertical && horizontal) {
	    m_w = x - m_x;
	    m_h = y - m_y;
	    fixResizeDimensions(m_w, m_h, dw, dh);
	    m_border->configure(m_x, m_y, m_w, m_h, CWWidth|CWHeight, 0, True);
	} else if (vertical) {
	    m_h = y - m_y;
	    fixResizeDimensions(m_w, m_h, dw, dh);
	    m_border->configure(m_x, m_y, m_w, m_h, CWHeight, 0, True);
	} else {
	    m_w = x - m_x;
	    fixResizeDimensions(m_w, m_h, dw, dh);
	    m_border->configure(m_x, m_y, m_w, m_h, CWWidth, 0, True);
	}

	XMoveResizeWindow(display(), m_window,
			  m_border->xIndent(), m_border->yIndent(), m_w, m_h);

	if (vertical) makeThisNormalHeight(); // in case it was full-height
	sendConfigureNotify();
    }

    m_windowManager->installCursor(WindowManager::NormalCursor);
}


void Client::moveOrResize(XButtonEvent *e)
{
    if (e->x < m_border->xIndent() && e->y > m_h) {
	resize(e, False, True);
    } else if (e->y < m_border->yIndent() &&
	       e->x > m_w + m_border->xIndent() - m_border->yIndent()) { //hack
	resize(e, True, False);
    } else {
	move(e);
    }
}


void Border::eventButton(XButtonEvent *e)
{
    if (e->window == m_parent) {

	if (!m_client->isActive()) return;
	if (isTransient()) {
	    if (e->x >= xIndent() && e->y >= yIndent()) {
		return;
	    } else {
		m_client->move(e);
		return;
	    }
	}

	m_client->moveOrResize(e);
	return;
	
    } else if (e->window == m_tab) {
	m_client->move(e);
	return;
    }

    if (e->window == m_resize) {
	m_client->resize(e, True, True);
	return;
    }

    if (e->window != m_button || e->type == ButtonRelease) return;

    if (windowManager()->attemptGrab(m_button, None, MenuGrabMask, e->time)
	!= GrabSuccess) {
	return;
    }

    XEvent event;
    Boolean found;
    Boolean done = False;
    struct timeval sleepval;
    unsigned long tdiff = 0L;
    int x = e->x;
    int y = e->y;
    int action = 1;
    int buttonSize = m_tabWidth - TAB_TOP_HEIGHT*2 - 4;

    XFillRectangle(display(), m_button, m_drawGC, 0, 0, buttonSize, buttonSize);

    while (!done) {

	found = False;

	if (tdiff > CONFIG_DESTROY_WINDOW_DELAY && action == 1) {
	    windowManager()->installCursor(WindowManager::DeleteCursor);
	    action = 2;
	}

	while (XCheckMaskEvent(display(), MenuMask, &event)) {
	    found = True;
	    if (event.type != MotionNotify) break;
	}

	if (!found) {
	    sleepval.tv_sec = 0;
	    sleepval.tv_usec = 50000;
	    select(0, 0, 0, 0, &sleepval);
	    tdiff += 50;
	    continue;
	}

	switch (event.type) {

	default:
	    fprintf(stderr, "wmx: unknown event type %d\n", event.type);
	    break;

	case Expose:
	    windowManager()->eventExposure(&event.xexpose);
	    break;

	case ButtonPress:
	    break;

	case ButtonRelease:

	    if (!nobuttons(&event.xbutton)) {
		action = 0;
	    }

	    if (x < 0 || y < 0 || x >= buttonSize || y >= buttonSize) {
		action = 0;
	    }

	    windowManager()->releaseGrab(&event.xbutton);
	    done = True;
	    break;

	case MotionNotify:
	    tdiff = event.xmotion.time - e->time;
	    if (tdiff > 5000L) tdiff = 5001L; // in case of overflow!

	    x = event.xmotion.x;
	    y = event.xmotion.y;

	    if (action == 0 || action == 2) {
		if (x < 0 || y < 0 || x >= buttonSize || y >= buttonSize) {
		    windowManager()->installCursor(WindowManager::NormalCursor);
		    action = 0;
		} else {
		    windowManager()->installCursor(WindowManager::DeleteCursor);
		    action = 2;
		}
	    }

	    break;
	}
    }

    XClearWindow(display(), m_button);
    windowManager()->installCursor(WindowManager::NormalCursor);

    if (tdiff > 5000L) {	// do nothing, they dithered too long
	return;
    }

    if (action == 1) m_client->hide();
    else if (action == 2) m_client->kill();
}


