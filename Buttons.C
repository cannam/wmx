
#include "Manager.h"
#include "Client.h"
#include "Menu.h"
#include <X11/keysym.h>
#include <sys/time.h>

#if CONFIG_WANT_SUNKEYS
#if CONFIG_WANT_SUNPOWERKEY
#include <X11/Sunkeysym.h>
#endif
#endif

void WindowManager::eventButton(XButtonEvent *e, XEvent *ev)
{
    enum {Vertical, Maximum, Horizontal};
    setScreenFromPointer();
    Client *c = windowToClient(e->window);

#if CONFIG_GNOME_BUTTON_COMPLIANCE == False
    if (e->button == CONFIG_CIRCULATE_BUTTON && m_channelChangeTime == 0) {
	if (DynamicConfig::config.rightCirculate())
	    circulate(e->window == e->root);
	else if (DynamicConfig::config.rightLower())
 	{
	    if (e->window != e->root && c) c->lower();
 	}
 	else if (DynamicConfig::config.rightToggleHeight())
 	{
  	    if (e->window != e->root && c) {
 		if (c->isFullHeight()) {
 		    c->unmaximise(Vertical);
 		} else {
 		    c->maximise(Vertical);
 		}
 	    }
 	}
	return;
    }
#endif
 
    if (e->button == CONFIG_NEXTCHANNEL_BUTTON && CONFIG_CHANNEL_SURF) {
        // wheel "up" - increase channel
        flipChannel(False, False, True, c);
        return ;
    } else if (e->button == CONFIG_PREVCHANNEL_BUTTON && CONFIG_CHANNEL_SURF) {
        // wheel "down" - decrease channel
        flipChannel(False, True, True, c);
        return ;
    }
    
    if (e->window == e->root) {

#if CONFIG_GNOME_BUTTON_COMPLIANCE != False
        if ((e->button == CONFIG_CLIENTMENU_BUTTON || e->button == CONFIG_CIRCULATE_BUTTON)
            && m_channelChangeTime == 0) {

	    XUngrabPointer(m_display, CurrentTime);
	    XSendEvent(m_display, gnome_win, False, SubstructureNotifyMask, ev);
	    return;
	} 
        
	if (e->button == CONFIG_COMMANDMENU_BUTTON && m_channelChangeTime == 0) {
	    ClientMenu menu(this, (XEvent *)e);
	    return;
	}
#endif

	if (e->button == CONFIG_CLIENTMENU_BUTTON && m_channelChangeTime == 0) {
	    ClientMenu menu(this, (XEvent *)e);

	} else if (e->x > DisplayWidth(display(), screen()) -
		   CONFIG_CHANNEL_CLICK_SIZE &&
		   e->y < CONFIG_CHANNEL_CLICK_SIZE) {

	    if (e->button == CONFIG_COMMANDMENU_BUTTON) {
#if CONFIG_USE_CHANNEL_MENU	
                ChannelMenu(this, (XEvent *)e);
#else
                if (m_channelChangeTime == 0) flipChannel(True, False, False, 0);
                else flipChannel(False, False, False, 0);
                
            } else if (e->button == CONFIG_CLIENTMENU_BUTTON && m_channelChangeTime != 0) {
                
                flipChannel(False, True, False, 0);
#endif
	    }

	} else if (e->button == CONFIG_COMMANDMENU_BUTTON && m_channelChangeTime == 0) {
	    DynamicConfig::config.scan();
	    CommandMenu menu(this, (XEvent *)e);
	}
        
    } else if (c) {

	if (e->button == CONFIG_COMMANDMENU_BUTTON && CONFIG_CHANNEL_SURF) {
#if CONFIG_USE_CHANNEL_MENU	
	    ChannelMenu menu(this, (XEvent *)e);
#else
            if (m_channelChangeTime == 0) flipChannel(True, False, False, 0);
            else flipChannel(False, False, False, c);
#endif
	    return;
	} else if (e->button == CONFIG_CLIENTMENU_BUTTON && m_channelChangeTime != 0) {
	    // allow left-button to push down a channel --cc 19991001
	    flipChannel(False, True, False, c);
	    return;
 	}

	c->eventButton(e);
	return;
    }
}


void WindowManager::circulate(Boolean activeFirst)
{
    Client *c = 0;
    if (m_clients.count() == 0) return;
    if (activeFirst) c = m_activeClient;

    // cc 2000/05/11: change direction each time we release Alt

    static int direction = 1;
    if (!m_altStateRetained) direction = -direction;
    m_altStateRetained = True;

    if (!c) {

	int i, j;

	if (!m_activeClient) {
            i = (direction > 0 ? -1 : m_clients.count());
        } else {
	    for (i = 0; i < m_clients.count(); ++i) {
		if (m_clients.item(i)->channel() != channel()) continue;
		if (m_clients.item(i) == m_activeClient) break;
	    }

	    if (i >= m_clients.count() - 1 && direction > 0) i = -1;
            else if (i == 0 && direction < 0) i = m_clients.count();
	}

	for (j = i + direction;
             (!m_clients.item(j)->isNormal() ||
               m_clients.item(j)->isTransient() ||
               m_clients.item(j)->channel() != channel());
             j += direction) {

	    if (direction > 0 && j >= m_clients.count() - 1) j = -1;
            if (direction < 0 && j <= 0) j = m_clients.count();

	    if (j == i) return; // no suitable clients
	}

	c = m_clients.item(j);
    }

    c->activateAndWarp();
}

void WindowManager::eventKeyPress(XKeyEvent *ev)
{
    enum {Vertical, Maximum, Horizontal};
    KeySym key = XKeycodeToKeysym(display(), ev->keycode, 0);
    
    if (CONFIG_USE_KEYBOARD) {

	Client *c = windowToClient(ev->window);

        if (key == CONFIG_ALT_KEY) {
            m_altPressed = True;
            m_altStateRetained = False;
        }

#if CONFIG_WANT_SUNKEYS
#ifdef CONFIG_QUICKRAISE_KEY
	if (key == CONFIG_QUICKRAISE_KEY && c) {

	    if (isTop(c) && (CONFIG_QUICKRAISE_ALSO_LOWERS == True)) {
		c->lower();
	    } else {
		c->mapRaised();
	    }
	    
	} else
#endif // CONFIG_QUICKRAISE_KEY
#ifdef CONFIG_QUICKHIDE_KEY
	if (key == CONFIG_QUICKHIDE_KEY && c) {
	    c->hide();
	      
	} else 
#endif // CONFIG_QUICKHIDE_KEY
#ifdef CONFIG_QUICKCLOSE_KEY
	if (key == CONFIG_QUICKCLOSE_KEY && c) {
	    c->kill();

	} else
#endif // CONFIG_QUICKCLOSE_KEY
#ifdef CONFIG_QUICKHEIGHT_KEY
	if (key == CONFIG_QUICKHEIGHT_KEY && c) {

	    if (c->isFullHeight()) {
		c->unmaximise(Vertical);
	    } else {
		c->maximise(Vertical);
	    }

	} else
#endif // CONFIG_QUICKHEIGHT_KEY
#if CONFIG_WANT_SUNPOWERKEY
	  if (key == SunXK_PowerSwitch) {
           pid_t pid = fork();
           if(pid == 0)
           {
               execl(CONFIG_SUNPOWER_EXEC,CONFIG_SUNPOWER_EXEC,
                     CONFIG_SUNPOWER_OPTIONS);
           }

       } else if (key == SunXK_PowerSwitchShift) {
           pid_t pid = fork();
           if(pid == 0)
           {
               execl(CONFIG_SUNPOWER_EXEC,CONFIG_SUNPOWER_EXEC,
                     CONFIG_SUNPOWER_SHIFTOPTIONS);
           }
       } else
#endif // CONFIG_WANT_SUNPOWERKEY
#endif // CONFIG_WANT_SUNKEYS

       if (ev->state & m_altModMask) {

           if (!m_altPressed) {
               // oops! bug
//               fprintf(stderr, "wmx: Alt key record in inconsistent state\n");
               m_altPressed = True;
               m_altStateRetained = False;
//        fprintf(stderr, "state is %ld, mask is %ld\n",
//                (long)ev->state, (long)m_altModMask);
           }

           if (key >= XK_F1 && key <= XK_F12 &&
		CONFIG_CHANNEL_SURF && CONFIG_USE_CHANNEL_KEYS) {

		int channel = key - XK_F1 + 1;

		gotoChannel(channel, 0);
		
	    } else {

		// These key names also appear in Client::manage(), so
		// when adding a key it must be added in both places

		switch (key) {

		case CONFIG_FLIP_DOWN_KEY:
		    flipChannel(False, True, True, 0);
		    break;

		case CONFIG_FLIP_UP_KEY:
		    flipChannel(False, False, True, 0);
		    break;
	
		case CONFIG_CIRCULATE_KEY:
		    circulate(False);
		    break;

		case CONFIG_HIDE_KEY:
		    if (c) c->hide();
		    break;

		case CONFIG_DESTROY_KEY:
		    if (c) c->kill();
		    break;

		case CONFIG_RAISE_KEY:
		    if (c) c->mapRaised();
		    break;

		case CONFIG_LOWER_KEY:
		    if (c) c->lower();
		    break;

		case CONFIG_FULLHEIGHT_KEY:
		    if (c) c->maximise(Vertical);
		    break;
		
		case CONFIG_NORMALHEIGHT_KEY:
		    if (c && !CONFIG_SAME_KEY_MAX_UNMAX)
                        c->unmaximise(Vertical);
		    break;
		
		case CONFIG_FULLWIDTH_KEY:
		    if (c) c->maximise(Horizontal);
		    break;
		
		case CONFIG_NORMALWIDTH_KEY:
		    if (c && !CONFIG_SAME_KEY_MAX_UNMAX)
                        c->unmaximise(Horizontal);
		    break;
		
		case CONFIG_MAXIMISE_KEY:
		    if (c) c->maximise(Maximum);
		    break;
		
		case CONFIG_UNMAXIMISE_KEY:
		    if (c && !CONFIG_SAME_KEY_MAX_UNMAX)
                        c->unmaximise(Maximum);
		    break;
		
		case CONFIG_STICKY_KEY:
		    if (c) c->setSticky(!(c->isSticky()));
		    break;

		case CONFIG_CLIENT_MENU_KEY:
		    if (CONFIG_WANT_KEYBOARD_MENU) {
			ClientMenu menu(this, (XEvent *)ev);
			break;
		    }
		case CONFIG_COMMAND_MENU_KEY:
		    if (CONFIG_WANT_KEYBOARD_MENU) {
			CommandMenu menu(this, (XEvent *)ev);
			break;
		    }

#if CONFIG_GROUPS != False
		case XK_0:
		case XK_1:
		case XK_2:
		case XK_3:
		case XK_4:
		case XK_5:
		case XK_6:
		case XK_7:
		case XK_8:
		case XK_9: {
		    windowGrouping(ev, key, c);
		    break;
		}
#endif		
		default:
		    return;
		}
	    }
	}

	XSync(display(), False);
	XUngrabKeyboard(display(), CurrentTime);
    }

    return;
}

void WindowManager::eventKeyRelease(XKeyEvent *ev)
{
    KeySym key = XKeycodeToKeysym(display(), ev->keycode, 0);
    
    if (key == CONFIG_ALT_KEY) {
        m_altPressed = False;
        m_altStateRetained = False;

//        fprintf(stderr, "state is %ld, mask is %ld\n",
//                (long)ev->state, (long)m_altModMask);
    }
    return;
}

#if CONFIG_GROUPS != False
void WindowManager::windowGrouping(XKeyEvent *ev, KeySym key, Client *c) {

    int group = key - XK_0;
    Client *x;

    if (ev->state & ShiftMask) {
	grouping.item(group)->remove_all();
	return;
    }

    if (ev->state & ControlMask) {
	if (c) {
	    // remove if it's there
	    int there = -1;

	    for (int i = 0; i < grouping.item(group)->count(); i++) {
		if (grouping.item(group)->item(i) == c) {
		    //		    fprintf(stderr, "removing client from %d at %d\n", group, i);
		    there = i;
		}
	    }
	    
	    if (there != -1) {
		grouping.item(group)->remove(there);
	    } else {
		// add if it's not there
		//fprintf(stderr, "adding client to %d at %d \n", 
		//	group, grouping.item(group)->count());

		grouping.item(group)->append(c);
	    }
	}
    } else {
	
	int count = grouping.item(group)->count();
     	int raise = 1;

	if (count > 0) {
	    x = grouping.item(group)->item(count-1);
	    // it seems like there should be a better way of doing this...
	    while (m_currentChannel != x->channel()) {
		
		raise = 2;

		if (m_currentChannel < x->channel()) {
		    flipChannel(False, False, True, 0);
		} else {
		    flipChannel(False, True, True, 0);
		}
		XSync(display(), False);
	    }

	    int temp = 0;

	    for (int i = 0; i < count; i++) {
		if (m_orderedClients.item(i) == 
		    grouping.item(group)->item(count - 1 - i)) {
		    temp++;
		}
	    }
	    
	    if ((raise != 2) && (temp == count)) {
		raise = 0;
	    }
	    //	    fprintf(stderr, "raise = %d temp =  %d \n", 
	    //	    raise, temp);   
	}

	for (int i = 0; i < count; i++) {
	    x = grouping.item(group)->item(i);
	    if (x) {
		if (raise) {
		    x->mapRaised();
		} else {
		    x->lower();
		}
	    }
	}
    }
}
#endif

void Client::activateAndWarp()
{
    mapRaised();
    ensureVisible();
    warpPointer();
    activate();
}


void Client::eventButton(XButtonEvent *e)
{
    if (e->type != ButtonPress) return;

    Boolean wasTop = windowManager()->isTop(this);

    mapRaised();

    if (e->button == CONFIG_CLIENTMENU_BUTTON) {
	if (m_border->hasWindow(e->window)) {
	    m_border->eventButton(e);
	} else {
	    e->x += m_border->xIndent();
	    e->y += m_border->yIndent();
	    move(e);
	}
    }

    if (CONFIG_RAISELOWER_ON_CLICK && wasTop && !m_doSomething) {
	lower();
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


int WindowManager::attemptGrabKey(Window w, int t)
{
    if (t == 0) t = timestamp(False);
    return XGrabKeyboard(display(), w, False, GrabModeAsync, GrabModeAsync, t);
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


void WindowManager::releaseGrabKeyMode(XButtonEvent *e) 
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

    XUngrabKeyboard(display(), e->time);
    m_currentTime = e->time;
}


void WindowManager::releaseGrabKeyMode(XKeyEvent* e)
{
    XUngrabPointer(display(), e->time);
    XUngrabKeyboard(display(), e->time);
    m_currentTime = e->time;
}


void Client::move(XButtonEvent *e)
{
    int x = -1, y = -1;
    Boolean done = False;
    ShowGeometry geometry(m_windowManager, (XEvent *)e);

    if (m_windowManager->attemptGrab
	(root(), None, DragMask, e->time) != GrabSuccess) {
	return;
    }

    int mx = DisplayWidth (display(), windowManager()->screen()) - 1;
    int my = DisplayHeight(display(), windowManager()->screen()) - 1;
    int xi = m_border->xIndent();
    int yi = m_border->yIndent();

    XEvent event;
    Boolean found;
    struct timeval sleepval;

    m_doSomething = False;
    while (!done) {

	found = False;

	while (XCheckMaskEvent(display(), DragMask | ExposureMask, &event)) {
	    found = True;
	    if (event.type != MotionNotify) break;
	}

	if (!found) {
	    sleepval.tv_sec = 0;
	    sleepval.tv_usec = 1000;
	    select(0, 0, 0, 0, &sleepval);
	    continue;
	}
	
	switch (event.type) {

	default:
	    fprintf(stderr, "wmx: unknown event type %d\n", event.type);
	    break;

	case Expose:
//	    m_windowManager->eventExposure(&event.xexpose);
	    m_windowManager->dispatchEvent(&event);
	    break;

	case ButtonPress:
	    // don't like this
	    XUngrabPointer(display(), event.xbutton.time);
	    m_doSomething = False;
	    done = True;
	    break;

	case ButtonRelease:

	    if (!nobuttons(&event.xbutton)) m_doSomething = False;

	    m_windowManager->releaseGrab(&event.xbutton);
	    done = True;
	    break;

	case MotionNotify:

	    int nx = event.xbutton.x - e->x;
	    int ny = event.xbutton.y - e->y;

	    if (nx != x || ny != y) {

		if (m_doSomething) { // so x,y have sensible values already

		    // bumping!

		    if (nx < x && nx <= 0 && nx > -CONFIG_BUMP_DISTANCE) nx = 0;
		    if (ny < y && ny <= 0 && ny > -CONFIG_BUMP_DISTANCE) ny = 0;

		    if (nx > x && nx >= mx - m_w - xi &&
			nx < mx - m_w - xi + CONFIG_BUMP_DISTANCE)
			nx = mx - m_w - xi;
		    if (ny > y && ny >= my - m_h - yi &&
			ny < my - m_h - yi + CONFIG_BUMP_DISTANCE)
			ny = my - m_h - yi;
		}

		x = nx;
		y = ny;

		geometry.update(x, y);
		m_border->moveTo(x + xi, y + yi);
		m_doSomething = True;
	    }
	    break;
	}
    }

    geometry.remove();

    if (m_doSomething) {
	m_x = x + xi;
	m_y = y + yi;
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
    ShowGeometry geometry(m_windowManager, (XEvent *)e);

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
    Boolean done = False;
    struct timeval sleepval;

    m_doSomething = False;
    while (!done) {

	found = False;

	while (XCheckMaskEvent(display(), DragMask | ExposureMask, &event)) {
	    found = True;
	    if (event.type != MotionNotify) break;
	}

	if (!found) {
	    sleepval.tv_sec = 0;
	    sleepval.tv_usec = 10000;
	    select(0, 0, 0, 0, &sleepval);
	    continue;
	}
	
	switch (event.type) {

	default:
	    fprintf(stderr, "wmx: unknown event type %d\n", event.type);
	    break;

	case Expose:
//	    m_windowManager->eventExposure(&event.xexpose);
	    m_windowManager->dispatchEvent(&event);
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
		m_doSomething = True;

	    } else if (vertical) {
		prevH = h; h = y - m_y;
		fixResizeDimensions(w, h, dw, dh);
		if (h == prevH) break;
		m_border->configure(m_x, m_y, w, h, CWHeight, 0);
		if (CONFIG_RESIZE_UPDATE)
		    XResizeWindow(display(), m_window, w, h);
		geometry.update(dw, dh);
		m_doSomething = True;

	    } else {
		prevW = w; w = x - m_x;
		fixResizeDimensions(w, h, dw, dh);
		if (w == prevW) break;
		m_border->configure(m_x, m_y, w, h, CWWidth, 0);
		if (CONFIG_RESIZE_UPDATE)
		    XResizeWindow(display(), m_window, w, h);
		geometry.update(dw, dh);
		m_doSomething = True;
	    }

	    break;
	}
    }

    if (m_doSomething) {

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

	if (vertical && horizontal) {
	    makeThisNormalHeight();
	    makeThisNormalWidth();
	} else if (vertical)
	    makeThisNormalHeight(); // in case it was full-height
	else if (horizontal)
	    makeThisNormalWidth();
	
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
    int buttonSize = m_tabWidth[screen()] - TAB_TOP_HEIGHT*2 - 4;

    XFillRectangle(display(), m_button, m_drawGC[screen()],
		   0, 0, buttonSize, buttonSize);

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
//	    windowManager()->eventExposure(&event.xexpose);
	    windowManager()->dispatchEvent(&event);
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


