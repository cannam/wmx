
#include "Manager.h"
#include "Client.h"
#include <sys/time.h>

static char *numerals[10][7] = {
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


void WindowManager::flipChannel(Boolean firstTime, Boolean statusOnly,
				Client *push)
{
    if (!CONFIG_CHANNEL_SURF) return;

    if (!m_channelWindow) {

	int screen = m_screenNumber;
	XColor nearest, ideal;

	if (!XAllocNamedColor(display(), DefaultColormap(display(), screen),
			      "green", &nearest, &ideal)) {
	    
	    if (!XAllocNamedColor(display(), DefaultColormap(display(), screen),
				  "black", &nearest, &ideal)) {
		
		fatal("Couldn't allocate green or black");
	    }
	}

	XSetWindowAttributes wa;
	wa.background_pixel = nearest.pixel;
	wa.override_redirect = True;
	
	m_channelWindow = XCreateWindow
	    (m_display, m_root, 0, 0, 1, 1, 0, CopyFromParent, CopyFromParent,
	     CopyFromParent, CWOverrideRedirect | CWBackPixel, &wa);
    }

    int nextChannel =
	(statusOnly || firstTime) ? m_currentChannel : m_currentChannel + 1;
    if (nextChannel > m_channels) nextChannel = 1;

    int x, y, i;
    XRectangle r;
    Boolean first = True;
    char number[7];
    sprintf(number, "%d", nextChannel);

    for (i = 0; i < strlen(number); ++i) {
	for (y = 0; y < 7; ++y) {
	    for (x = 0; x < 5; ++x) {
		if (numerals[number[i]-'0'][y][x] != ' ') {
		    
		    r.x = i * 110 + x * 20; r.y = y * 20;
		    r.width = r.height = 20;
		    
		    XShapeCombineRectangles
			(display(), m_channelWindow, ShapeBounding,
			 0, 0, &r, 1, first ? ShapeSet : ShapeUnion, YXBanded);

		    first = False;
		}
	    }
	}
    }

    XMoveResizeWindow(display(), m_channelWindow,
		      DisplayWidth(display(), m_screenNumber) - 30 -
		      110 * strlen(number), 30, 500, 160);
    XMapRaised(display(), m_channelWindow);

    if (!statusOnly) {

	ClientList considering;

	for (i = m_orderedClients.count()-1; i >= 0; --i) {
	    considering.append(m_orderedClients.item(i));
	}

	for (i = 0; i < considering.count(); ++i) {
	    if (considering.item(i) == push) {
		considering.item(i)->setChannel(nextChannel);
	    } else {
		considering.item(i)->flipChannel(True);
	    }
	}

	considering.remove_all();
    }

    m_currentChannel = nextChannel;
    m_channelChangeTime = timestamp(True);
}


void WindowManager::instateChannel()
{
    int i;
    m_channelChangeTime = 0;
    XUnmapWindow(display(), m_channelWindow);

    ClientList considering;

    for (i = m_orderedClients.count()-1; i >= 0; --i) {
	considering.append(m_orderedClients.item(i));
    }

    for (i = 0; i < considering.count(); ++i) {
	considering.item(i)->flipChannel(False);
	if (considering.item(i)->channel() == m_channels) createNewChannel();
    }

    if (m_activeClient && m_activeClient->channel() != channel()) {
	m_activeClient = 0;
    }

    checkChannel(m_channels-1);
}    


void WindowManager::checkChannel(int ch)
{
    if (m_channels < 2 || ch < m_channels - 1) return;

    for (int i = m_orderedClients.count()-1; i >= 0; --i) {
	if (m_orderedClients.item(i)->channel() == ch) return;
    }

    --m_channels;
    if (m_currentChannel > m_channels) m_currentChannel = m_channels;

    checkChannel(ch - 1);
}


void WindowManager::createNewChannel()
{
    ++m_channels;
}


