
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


void WindowManager::flipChannel(Boolean statusOnly, Boolean flipDown,
				Boolean quickFlip, Client *push)
{
    int x, y, i, sc;
    if (!CONFIG_CHANNEL_SURF) return;

    for(sc = 0; sc < screensTotal(); sc++)
    {
	if (!m_channelWindow[sc]) {

	XColor nearest, ideal;

	    if (!XAllocNamedColor(display(), DefaultColormap(display(), sc),
			      CONFIG_CHANNEL_NUMBER, &nearest, &ideal)) {
	    
		if (!XAllocNamedColor(display(), DefaultColormap(display(), sc),
				  "black", &nearest, &ideal)) {
		
		fatal("Couldn't allocate green or black");
	    }
	}

	XSetWindowAttributes wa;
	wa.background_pixel = nearest.pixel;
	wa.override_redirect = True;
	
	    m_channelWindow[sc] = XCreateWindow
	      (display(), mroot(sc), 0, 0, 1, 1, 0, CopyFromParent, CopyFromParent,
	     CopyFromParent, CWOverrideRedirect | CWBackPixel, &wa);
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

    XRectangle r;
    Boolean first = True;
    char number[7];
    sprintf(number, "%d", nextChannel);

    for (i = 0; i < strlen(number); ++i) {
	for (y = 0; y < 7; ++y) {
	    for (x = 0; x < 5; ++x) {
		if (numerals[number[i]-'0'][y][x] != ' ') {
/*		    
		    r.x = i * 110 + x * 20; r.y = y * 20;
		    r.width = r.height = 20;
 */      
                    r.x = 10 + (i * 6 + x) * CONFIG_CHANNEL_NUMBER_SIZE;
                    r.y = y * CONFIG_CHANNEL_NUMBER_SIZE;
                    r.width = r.height = CONFIG_CHANNEL_NUMBER_SIZE;
                    for(sc = 0; sc < screensTotal(); sc++)
                    {
		    XShapeCombineRectangles
                          (display(), m_channelWindow[sc], ShapeBounding,
                           0, 0, &r, 1, first ? ShapeSet : ShapeUnion,
                           YXBanded);
                    }
		    first = False;
		}
	    }
	}
    }

    for(sc = 0; sc < screensTotal(); sc++)
    {
/*
        XMoveResizeWindow(display(), m_channelWindow[sc],
			  DisplayWidth(display(), sc) - 30 -
		      110 * strlen(number), 30, 500, 160);
 */

        XMoveResizeWindow(display(), m_channelWindow[sc],
                          DisplayWidth(display(), sc) - 30 -
                          (5 * CONFIG_CHANNEL_NUMBER_SIZE + 10) *
                          strlen(number), 30, 500, 160);
	XMapRaised(display(), m_channelWindow[sc]);
    }

    if (!statusOnly) {

	ClientList considering;

	for (i = m_orderedClients.count()-1; i >= 0; --i) {
	    considering.append(m_orderedClients.item(i));
	}

	for (i = 0; i < considering.count(); ++i) {
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

#if CONFIG_GNOME_COMPLIANCE != False
    gnomeUpdateChannelList();
#endif
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

    for (i = m_orderedClients.count()-1; i >= 0; --i) {
	considering.append(m_orderedClients.item(i));
    }

    for (i = 0; i < considering.count(); ++i) {
	considering.item(i)->flipChannel(False, m_currentChannel);
	if (considering.item(i)->channel() == m_channels &&
	    !considering.item(i)->isSticky()) createNewChannel();
    }

    if (m_activeClient && m_activeClient->channel() != channel()) {
	m_activeClient = 0;
    }

    checkChannel(m_channels-1);
}    


void WindowManager::checkChannel(int ch)
{
    if (m_channels <= 2 || ch < m_channels - 1) return;

    for (int i = m_orderedClients.count()-1; i >= 0; --i) {
	if (m_orderedClients.item(i)->channel() == ch) return;
    }

    --m_channels;

#if CONFIG_GNOME_COMPLIANCE != False
    gnomeUpdateChannelList();
#endif

    if (m_currentChannel > m_channels) m_currentChannel = m_channels;

    checkChannel(ch - 1);
}


void WindowManager::createNewChannel()
{
    ++m_channels;
#if CONFIG_GNOME_COMPLIANCE != False
    gnomeUpdateChannelList();
#endif

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
