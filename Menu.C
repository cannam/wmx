
#include "Menu.h"
#include "Manager.h"
#include "Client.h"

#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

Boolean       Menu::m_initialised = False;
GC            Menu::m_menuGC;
XFontStruct  *Menu::m_font;
unsigned long Menu::m_foreground;
unsigned long Menu::m_background;
unsigned long Menu::m_border;
Window        Menu::m_window;


Menu::Menu(WindowManager *manager, XButtonEvent *e)
    : m_windowManager(manager), m_event(e), m_items(0), m_nItems(0)
{
    if (!m_initialised)
    {
	m_foreground = m_windowManager->allocateColour
	    (CONFIG_MENU_FOREGROUND, "menu foreground");
	m_background = m_windowManager->allocateColour
	    (CONFIG_MENU_BACKGROUND, "menu background");
	m_border = m_windowManager->allocateColour
	    (CONFIG_MENU_BORDERS, "menu border");

	m_font = XLoadQueryFont(display(), CONFIG_NICE_MENU_FONT);
	if (!m_font) m_font = XLoadQueryFont(display(), CONFIG_NASTY_FONT);
	if (!m_font) m_windowManager->fatal("couldn't load menu font\n");

	XGCValues values;
	values.background = m_background;
	values.foreground = m_foreground ^ m_background;
	values.function = GXxor;
	values.line_width = 0;
	values.subwindow_mode = IncludeInferiors;
	values.font = m_font->fid;
	
	m_menuGC = XCreateGC
	    (display(), root(),
	     GCForeground | GCBackground | GCFunction |
	     GCLineWidth | GCSubwindowMode, &values);

	XChangeGC(display(), Border::drawGC(m_windowManager), GCFont, &values);

	m_window = XCreateSimpleWindow
	    (display(), root(), 0, 0, 1, 1, 1,
	     m_border, m_background);

	XSetWindowAttributes attr;

#if ( CONFIG_USE_PIXMAPS != False ) && ( CONFIG_USE_PIXMAP_MENUS != False )
	attr.background_pixmap = Border::backgroundPixmap(manager);
#endif
	attr.save_under =
	    (DoesSaveUnders(ScreenOfDisplay(display(), screen())) ?
	     True : False);

#if ( CONFIG_USE_PIXMAPS != False ) && ( CONFIG_USE_PIXMAP_MENUS != False ) 
	XChangeWindowAttributes
	    (display(), m_window, CWBackPixmap, &attr);
#endif
	XChangeWindowAttributes
	    (display(), m_window, CWSaveUnder, &attr);
    }
}

Menu::~Menu()
{
    XUnmapWindow(display(), m_window);
}

void Menu::cleanup(WindowManager *const wm)
{
    XFreeFont(wm->display(), m_font);
    XFreeGC(wm->display(), m_menuGC);
}

int nobuttons(XButtonEvent *e) // straight outta 9wm
{
    int state;
    state = (e->state & AllButtonMask);
    return (e->type == ButtonRelease) && (state & (state - 1)) == 0;
}

int Menu::getSelection()
{
    m_items = getItems(&m_nItems, &m_nHidden);
	
    if (m_event->window == m_window || m_nItems == 0) return -1;
    
    int width, maxWidth = 10;
    for(int i = 0; i < m_nItems; i++) {
	width = XTextWidth(m_font, m_items[i], strlen(m_items[i]));
	if (width > maxWidth) maxWidth = width;
    }
    maxWidth += 32;

    int selecting = -1, prev = -1;
    int entryHeight = m_font->ascent + m_font->descent + 4;
    int totalHeight = entryHeight * m_nItems + 13;
    int x = m_event->x - maxWidth/2;
    int y = m_event->y - 2;
    int mx = DisplayWidth (display(), screen()) - 1;
    int my = DisplayHeight(display(), screen()) - 1;
    Boolean warp = False;

    if (x < 0) {
	m_event->x -= x;
	x = 0;
	warp = True;
    } else if (x + maxWidth >= mx) {
	m_event->x -= x + maxWidth - mx;
	x = mx - maxWidth;
	warp = True;
    }
    
    if (y < 0) {
	m_event->y -= y;
	y = 0;
	warp = True;
    } else if (y + totalHeight >= my) {
	m_event->y -= y + totalHeight - my;
	y = my - totalHeight;
	warp = True;
    }

    if (warp) XWarpPointer(display(), None, root(),
			   None, None, None, None, m_event->x, m_event->y);

    XMoveResizeWindow(display(), m_window, x, y, maxWidth, totalHeight);
    XSelectInput(display(), m_window, MenuMask);
    XMapRaised(display(), m_window);

    if (m_windowManager->attemptGrab(m_window, None,
				     MenuGrabMask, m_event->time)
	!= GrabSuccess) {
	XUnmapWindow(display(), m_window);
	return -1;
    }

    Boolean done = False;
    Boolean drawn = False;
    XEvent event;
    struct timeval sleepval;
    unsigned long tdiff = 0L;
    Boolean speculating = False;
    Boolean foundEvent;

    while (!done)
    {
	foundEvent = False;
	if (CONFIG_FEEDBACK_DELAY >= 0 &&
	    tdiff > CONFIG_FEEDBACK_DELAY && !speculating) {

	    if (selecting >= 0 && selecting < m_nItems) {
		raiseFeedbackLevel(selecting);
		XRaiseWindow(display(), m_window);
	    }

	    speculating = True;
	}

	while (XCheckMaskEvent(display(), MenuMask, &event)) {
	    foundEvent = True;
	    if (event.type != MotionNotify) break;
	}

	if (!foundEvent) {
	    sleepval.tv_sec = 0;
	    sleepval.tv_usec = 10000;
	    select(0, 0, 0, 0, &sleepval);
	    tdiff += 10;
	    continue;
	}
	
	switch (event.type)
	{
	default:
	    fprintf(stderr, "wmx: unknown event type %d\n", event.type);
	    break;
	    
	case ButtonPress:
	    break;
	    
	case ButtonRelease:
	{
	    int i;
	    
	    if (drawn) {

		if (event.xbutton.button != m_event->button) break;
		x = event.xbutton.x;
		y = event.xbutton.y - 11;
		i = y / entryHeight;
		
		if (selecting >= 0 && y >= selecting * entryHeight - 3 &&
		    y <= (selecting + 1) * entryHeight - 3) i = selecting;

		if (x < 0 || x > maxWidth || y < -3) i = -1;
		else if (i < 0 || i >= m_nItems) i = -1;

	    } else {
		selecting = -1;
	    }
	    
	    if (!nobuttons(&event.xbutton)) i = -1;
	    m_windowManager->releaseGrab(&event.xbutton);
	    XUnmapWindow(display(), m_window);
	    selecting = i;
	    done = True;
	    break;
	}

	case MotionNotify:
	    if (!drawn) break;
	    x = event.xbutton.x;
	    y = event.xbutton.y - 11;
	    prev = selecting;
	    selecting = y / entryHeight;

	    if (prev >= 0 && y >= prev * entryHeight - 3 &&
		y <= (prev+1) * entryHeight - 3) selecting = prev;

	    if (x < 0 || x > maxWidth || y < -3) selecting = -1;
	    else if (selecting < 0 || selecting > m_nItems) selecting = -1;

	    if (selecting == prev) break;
	    tdiff = 0; speculating = False;

	    if (prev >= 0 && prev < m_nItems) {
		removeFeedback(prev, speculating);
		XFillRectangle(display(), m_window, m_menuGC,
			       4, prev * entryHeight + 9,
			       maxWidth - 8, entryHeight);
	    }

	    if (selecting >= 0 && selecting < m_nItems) {
		showFeedback(selecting);
		XRaiseWindow(display(), m_window);
		XFillRectangle(display(), m_window, m_menuGC,
			       4, selecting * entryHeight + 9,
			       maxWidth - 8, entryHeight);
	    }
	    
	    break;
			
	case Expose:

	    if (CONFIG_MAD_FEEDBACK && event.xexpose.window != m_window) {
		m_windowManager->eventExposure((XExposeEvent *)&event);
		break;
	    }

	    XClearWindow(display(), m_window);
			
	    XDrawRectangle(display(), m_window, m_menuGC, 2, 7,
			   maxWidth - 5, totalHeight - 10);

	    for (int i = 0; i < m_nItems; i++) {

		int dx = XTextWidth(m_font, m_items[i], strlen(m_items[i]));
		int dy = i * entryHeight + m_font->ascent + 10;

		if (i >= m_nHidden) {
		    XDrawString(display(), m_window,
				Border::drawGC(m_windowManager),
				maxWidth - 8 - dx, dy,
				m_items[i], strlen(m_items[i]));
		} else {
		    XDrawString(display(), m_window,
				Border::drawGC(m_windowManager),
				8, dy, m_items[i], strlen(m_items[i]));
		}
	    }

	    if (selecting >= 0 && selecting < m_nItems) {
		XFillRectangle(display(), m_window, m_menuGC,
			       4, selecting * entryHeight + 9,
			       maxWidth - 8, entryHeight);
	    }

	    drawn = True;
	    break;
	}
    }

    if (selecting >= 0) removeFeedback(selecting, speculating);
    return selecting;
}



ClientMenu::ClientMenu(WindowManager *manager, XButtonEvent *e)
    : Menu(manager, e), m_allowExit(False)
{
    int selecting = getSelection();

    if (selecting == m_nItems-1 && m_allowExit) { // getItems sets m_allowExit
	m_windowManager->setSignalled();
	return;
    }
	
    if (selecting == 0) {
	m_windowManager->spawn(CONFIG_NEW_WINDOW_COMMAND, NULL);
	return;
    }

    if (selecting > 0) {

	Client *cl = m_clients.item(selecting - 1);
	
	if (selecting < m_nHidden) cl->unhide(True);
	else if (selecting < m_nItems) {

	    if (CONFIG_CLICK_TO_FOCUS) cl->activate();
	    else cl->mapRaised();
	    cl->ensureVisible();
	}
    }
}

ClientMenu::~ClientMenu()
{
    m_clients.remove_all();
    free(m_items);
}

char **ClientMenu::getItems(int *niR, int *nhR)
{
    int i;

    for (i = 0; i < m_windowManager->hiddenClients().count(); ++i) {
	if (m_windowManager->hiddenClients().item(i)->channel() ==
	    m_windowManager->channel()) {
	    m_clients.append(m_windowManager->hiddenClients().item(i));
	}
    }

    int nh = m_clients.count() + 1;

    if (CONFIG_EVERYTHING_ON_ROOT_MENU) {
	for (i = 0; i < m_windowManager->clients().count(); ++i) {
	    if (m_windowManager->clients().item(i)->isNormal() &&
		m_windowManager->clients().item(i)->channel() ==
		m_windowManager->channel()) {
		m_clients.append(m_windowManager->clients().item(i));
	    }
	}
    }

    int n = m_clients.count() + 1;
	
    int mx = DisplayWidth (display(), screen()) - 1;
    int my = DisplayHeight(display(), screen()) - 1;
    m_allowExit = (m_event->x > mx-3) && (m_event->y > my-3);
    if (m_allowExit) ++n;

    const char **items = (const char **)malloc(n * sizeof(char *));

    for (i = 0; i < n; ++i) {

	if (i == 0) items[i] = "New";
	else if (m_allowExit && i > m_clients.count()) items[i] = "[Exit wmx]";
	else items[i] = m_clients.item(i-1)->label();
    }

    *niR = n;
    *nhR = nh;

    return (char **)items;
}


#if CONFIG_MAD_FEEDBACK != 0

void ClientMenu::showFeedback(int item)
{
    if (item == 0) return;
    Client *c = m_clients.item(item-1);
    c->showFeedback();
}

void ClientMenu::removeFeedback(int item, Boolean mapped)
{
    if (item == 0) return;
    Client *c = m_clients.item(item-1);
    c->removeFeedback(mapped);
}

void ClientMenu::raiseFeedbackLevel(int item)
{
    if (item == 0) return;
    Client *c = m_clients.item(item-1);
    c->raiseFeedbackLevel();
}

#endif


CommandMenu::CommandMenu(WindowManager *manager, XButtonEvent *e)
    : Menu(manager, e)
{
    m_commandDir = NULL;
    const char *home = getenv("HOME");
    if (home == NULL) return;

    m_commandDir =
	(char *)malloc(strlen(home) + strlen(CONFIG_COMMAND_MENU) + 2);
    sprintf(m_commandDir, "%s/" CONFIG_COMMAND_MENU, home);

    int i = getSelection();
	
    if (i >= 0 && i < m_nItems) {
	char *commandFile =
	    (char *)malloc(strlen(m_commandDir) + strlen(m_items[i]) + 2);

	sprintf(commandFile, "%s/%s", m_commandDir, m_items[i]);
	m_windowManager->spawn(m_items[i], commandFile);
	free(commandFile);
    }
}

CommandMenu::~CommandMenu()
{
    free(m_commandDir);
    for (int i = 0; i < m_nItems; i++) {
	free(m_items[i]);
    }
    free(m_items);
}

static int sortstrs(const void *va, const void *vb)
{
    char **a = (char **)va;
    char **b = (char **)vb;    
    return strcmp(*a, *b);
}

char **CommandMenu::getItems(int *niR, int *nhR)
{
    *niR = *nhR = 0;
    const char *home;

    if ((home = getenv("HOME")) == NULL) return NULL;
	
    int dirlen = strlen(m_commandDir);
    char *dirpath = (char *)malloc(dirlen + NAME_MAX + 2);
    strcpy(dirpath, m_commandDir);

    DIR *dir = opendir(m_commandDir);

    if (dir == NULL) {
	free(dirpath);
	return NULL;
    }

    int count = 0;
    struct dirent *ent;

    // We already assume efficient realloc() in Border.C, so we may as
    // well assume it here too.

    char **items = 0;

    while ((ent = readdir(dir)) != NULL) {

	struct stat st;
	sprintf(dirpath + dirlen, "/%s", ent->d_name);
		
	if (stat(dirpath, &st) == -1) continue;
	if (!S_ISREG(st.st_mode) || !(st.st_mode & 0111)) continue;

	items = (!items ? (char **)malloc(sizeof(char *)) :
		 (char **)realloc(items, (count + 1) * sizeof(char *)));
	items[count++] = strdup(ent->d_name);
    }

    free(dirpath);
    closedir(dir);

    qsort(items, count, sizeof(char *), sortstrs);

    *niR = *nhR = count;
    return items;
}


// Fake geometry as a "menu" to save on code

ShowGeometry::ShowGeometry(WindowManager *manager, XButtonEvent *e)
    : Menu(manager, e)
{
    // empty
}

void ShowGeometry::update(int x, int y)
{
    char string[20];

    sprintf(string, "%d %d", x, y);
    int width = XTextWidth(m_font, string, strlen(string)) + 8;
    int height = m_font->ascent + m_font->descent + 8;
    int mx = DisplayWidth (display(), screen()) - 1;
    int my = DisplayHeight(display(), screen()) - 1;
  
    XMoveResizeWindow(display(), m_window,
		      (mx - width) / 2, (my - height) / 2, width, height);
    XClearWindow(display(), m_window);
    XMapRaised(display(), m_window);
    
    XDrawString(display(), m_window, Border::drawGC(m_windowManager),
		4, 4 + m_font->ascent, string, strlen(string));
}

void ShowGeometry::remove()
{
    XUnmapWindow(display(), m_window);
}

ShowGeometry::~ShowGeometry()
{
    // empty
}

char **ShowGeometry::getItems(int *niR, int *nhR)
{
    niR = nhR = 0;
    return NULL;
}


