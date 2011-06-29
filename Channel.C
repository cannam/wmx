/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#include "Manager.h"
#include "Client.h"
#include <sys/time.h>

static const char *numerals[10][7] = {
    { " ### ", "#   #", "#  ##", "# # #", "##  #", "#   #", " ### " },
    { "  #  ", " ##  ", "  #  ", "  #  ", "  #  ", "  #  ", " ### " },
    { " ### ", "#   #", "    #", "  ## ", " #   ", "#    ", "#####" },
    { "#####", "   # ", "  #  ", " ### ", "    #", "    #", "#### " },
    { "   # ", "  ## ", " # # ", "#  # ", "#####", "   # ", "   # " },
    { "#####", "#    ", "#### ", "    #", "    #", "    #", "#### " },
    { "  ## ", " #   ", "#    ", "#### ", "#   #", "#   #", " ### " },
    { "#####", "    #", "   # ", "  #  ", " #   ", " #   ", " #   " },
    { " ### ", "#   #", "#   #", " ### ", "#   #", "#   #", " ### " },
    { " ### ", "#   #", "#   #", " ####", "    #", "   # ", " ##  " }
};


Window WindowManager::createNumberWindow(int screen, const char *colour)
{
    XColor nearest, ideal;
            
    if (!XAllocNamedColor(display(), DefaultColormap(display(), screen),
                          colour, &nearest, &ideal)) {
	    
        if (!XAllocNamedColor(display(), DefaultColormap(display(), screen),
                              "black", &nearest, &ideal)) {
		
            fatal("Couldn't allocate requested numeral colour or black");
        }
    }
            
    XSetWindowAttributes wa;
    wa.background_pixel = nearest.pixel;
    wa.override_redirect = True;
	
    Window w = XCreateWindow
        (display(), mroot(screen),
         0, 0, 1, 1, 0, CopyFromParent, CopyFromParent,
         CopyFromParent, CWOverrideRedirect | CWBackPixel, &wa);

    return w;
}


int WindowManager::shapeNumberWindow(Window w, int n, int minDigits)
{
    int i, x, y;
    XRectangle r;
    Boolean first = True;
    char number[7];
    sprintf(number, "%0*d", minDigits, n);

    for (i = 0; i < (int)strlen(number); ++i) {
	for (y = 0; y < 7; ++y) {
	    for (x = 0; x < 5; ++x) {
		if (numerals[number[i]-'0'][y][x] != ' ') {

                    r.x = 10 + (i * 6 + x) * CONFIG_CHANNEL_NUMBER_SIZE;
                    r.y = y * CONFIG_CHANNEL_NUMBER_SIZE;
                    r.width = r.height = CONFIG_CHANNEL_NUMBER_SIZE;

		    XShapeCombineRectangles
                          (display(), w, ShapeBounding,
                           0, 0, &r, 1, first ? ShapeSet : ShapeUnion,
                           YXBanded);

		    first = False;
		}
	    }
	}
    }

    return (5 * CONFIG_CHANNEL_NUMBER_SIZE + 10) * strlen(number);
}
    
    
void WindowManager::flipChannel(Boolean statusOnly, Boolean flipDown,
				Boolean quickFlip, Client *push)
{
    int wid, i, sc;
    if (!CONFIG_CHANNEL_SURF) return;

    for (sc = 0; sc < screensTotal(); sc++) {
	if (!m_channelWindow[sc]) {
            m_channelWindow[sc] = createNumberWindow(sc, CONFIG_CHANNEL_NUMBER);
	}
    }

    int nextChannel;

    if (statusOnly) nextChannel = m_currentChannel;
    else {
	if (!flipDown) {
	    nextChannel = m_currentChannel + 1;
	    if (nextChannel > m_channels) nextChannel = 1;
	} else {
	    nextChannel = m_currentChannel - 1;
	    if (nextChannel < 1) nextChannel = m_channels;
	}
    }

    for (sc = 0; sc < screensTotal(); sc++) {

        wid = shapeNumberWindow(m_channelWindow[sc], nextChannel, 1);

        XMoveResizeWindow(display(), m_channelWindow[sc],
                          DisplayWidth(display(), sc) - 30 - wid,
                          30, 500, 160);

	XMapRaised(display(), m_channelWindow[sc]);
    }

    if (!statusOnly) {

	ClientList considering;

        for (int layer = 0; layer < MAX_LAYER; ++layer) {
            fprintf(stderr, "WindowManager::flipChannel: considering layer %d\n", layer);
	    for (i = (int)m_orderedClients[layer].count()-1; i >= 0; --i) {
                fprintf(stderr, "WindowManager::flipChannel: considering client %p\n", m_orderedClients[layer].item(i));
		considering.append(m_orderedClients[layer].item(i));
	    }
	}

	for (i = 0; i < (int)considering.count(); ++i) {
	    if (considering.item(i) == push || considering.item(i)->isSticky()
#if CONFIG_USE_WINDOW_GROUPS
		|| (push &&
		    push->hasWindow(considering.item(i)->groupParent()))
#endif
		) {
		considering.item(i)->setChannel(nextChannel);
	    } else {
		considering.item(i)->flipChannel(True, nextChannel);
	    }
	}

	considering.remove_all();
    }

    m_currentChannel = nextChannel;
    m_channelChangeTime = timestamp(True) +
	(quickFlip ? CONFIG_QUICK_FLIP_DELAY : CONFIG_FLIP_DELAY);

    netwmUpdateChannelList();
}


void WindowManager::instateChannel()
{
    int i;
    m_channelChangeTime = 0;
    for(i = 0; i < screensTotal(); i++)
    {
	XUnmapWindow(display(), m_channelWindow[i]);
    }

    ClientList considering;

    for (int layer = 0; layer < MAX_LAYER; ++layer) {
        for (i = m_orderedClients[layer].count()-1; i >= 0; --i) {
	    considering.append(m_orderedClients[layer].item(i));
        }
    }

    for (i = 0; i < considering.count(); ++i) {
	considering.item(i)->flipChannel(False, m_currentChannel);
	if ((considering.item(i)->channel() == m_channels) &&
	    !considering.item(i)->isSticky()) createNewChannel();
    }

    if (m_activeClient && (m_activeClient->channel() != channel())) {
        if (m_activeClient) m_activeClient->deactivate();
	m_activeClient = 0;
        XSetInputFocus(m_display, PointerRoot, None, timestamp(False));
    }

    checkChannel(m_channels-1);
}    


void WindowManager::checkChannel(int ch)
{
    if (m_channels <= 2 || ch < m_channels - 1) return;

    for(int layer = 0; layer < MAX_LAYER; ++layer) {
        for (int i = m_orderedClients[layer].count()-1; i >= 0; --i) {
	    if (m_orderedClients[layer].item(i)->channel() == ch) return;
        }
    }

    --m_channels;

    netwmUpdateChannelList();

    if (m_currentChannel > m_channels) m_currentChannel = m_channels;

    checkChannel(ch - 1);
}


void WindowManager::createNewChannel()
{
    ++m_channels;
    netwmUpdateChannelList();
}

void WindowManager::gotoChannel(int channel, Client *push)
{
    if (channel == m_currentChannel) {
	flipChannel(True, False, False, 0);
	return;
    }

    if (channel > 0 && channel <= m_channels) {

	while (m_currentChannel != channel) {
	    if (m_channels < channel) {
		flipChannel(False, False, True, push);
	    } else {
		flipChannel(False, True, True, push);
	    }
	    XSync(display(), False);
	}
    }
}

void
WindowManager::ensureChannelExists(int channel)
{
    if (m_channels <= channel) {
        m_channels = channel + 1;
        netwmUpdateChannelList();
    }
}

void
WindowManager::updateClock()
{
    time_t t;
    struct tm *lt;

    fprintf(stderr, "updateClock\n");

    time(&t);
    lt = localtime(&t);
    
    if (!m_clockWindow[0]) {
        m_clockWindow[0] = createNumberWindow(0, CONFIG_CLOCK_NUMBER);
        m_clockWindow[1] = createNumberWindow(0, CONFIG_CLOCK_NUMBER);
    }

    shapeNumberWindow(m_clockWindow[0], lt->tm_hour, 2);
    shapeNumberWindow(m_clockWindow[1], lt->tm_min, 2);
    
    XMoveResizeWindow(display(), m_clockWindow[0],
                      30, 30, 500, 160);
    
    XMoveResizeWindow(display(), m_clockWindow[1],
                      30, 9 * CONFIG_CHANNEL_NUMBER_SIZE + 30, 500, 160);

    XMapWindow(display(), m_clockWindow[0]);
    XMapWindow(display(), m_clockWindow[1]);
}

