
#ifndef _MANAGER_H_
#define _MANAGER_H_

#include "General.h"
#include "listmacro.h"

class Client;
declarePList(ClientList, Client);

#if CONFIG_GROUPS != False
declarePList(ListList, ClientList);
#endif

class WindowManager {
public:
    WindowManager(int argc, char **argv);
    ~WindowManager();

    void fatal(const char *);

    // for call from Client and within:

    Client *windowToClient(Window, Boolean create = False);
    Client *activeClient() { return m_activeClient; }
    Boolean raiseTransients(Client *); // true if raised any
    Time timestamp(Boolean reset);
    void clearFocus();

    void setActiveClient(Client *const c) { m_activeClient = c; }

    void addToHiddenList(Client *);
    void removeFromHiddenList(Client *);
    void skipInRevert(Client *, Client *);

    Display *display() { return m_display; }
    Window root() { return m_root[screen()]; }
    Window mroot(int i) { return m_root[i]; }
    int screen() { return m_screenNumber; }
    int screensTotal() { return m_screensTotal; }
    void setScreen(int i) { m_screenNumber = i; }
    void setScreenFromRoot(Window);
    void setScreenFromPointer();

    int altModMask() { return m_altModMask; }

    enum RootCursor {
	NormalCursor, DeleteCursor, DownCursor, RightCursor, DownrightCursor
    };

#ifdef CONFIG_USE_WINDOW_GROUPS
    void withdrawGroup(Window groupParent, Client *omit,
		       Boolean changeState = True);
    void hideGroup(Window groupParent, Client *omit);
    void unhideGroup(Window groupParent, Client *omit, Boolean map);
    void killGroup(Window groupParent, Client *omit);
#endif

    void installCursor(RootCursor);
    void installCursorOnWindow(RootCursor, Window);
    void installColormap(Colormap);
    unsigned long allocateColour(int, char *, char *);

    void considerFocusChange(Client *, Window, Time timestamp);
    void stopConsideringFocus();

    // shouldn't really be public
    int attemptGrab(Window, Window, int, int);
    int attemptGrabKey(Window, int);
    void releaseGrab(XButtonEvent *);
    void releaseGrabKeyMode(XButtonEvent *);
    void releaseGrabKeyMode(XKeyEvent *);
    void spawn(char *, char *);

    int channel() { return m_currentChannel; }
    int channels() { return m_channels; }
    void setSignalled() { m_looping = False; } // ...
    void gotoChannel(int channel, Client *push);
    
    ClientList &clients() { return m_clients; }
    ClientList &hiddenClients() { return m_hiddenClients; }

    void hoistToTop(Client *);
    void hoistToBottom(Client *);
    void removeFromOrderedList(Client *);
    Boolean isTop(Client *);

    // for exposures during client grab, and window map/unmap/destroy
    // during menu display:
    void dispatchEvent(XEvent *);

#if CONFIG_GROUPS != False
    void windowGrouping(XKeyEvent *ev, KeySym key, Client *c);
#endif

#if CONFIG_GNOME_COMPLIANCE != False
    void gnomeUpdateWindowList();
    void gnomeUpdateChannelList();
    void gnomeUpdateCurrentChannel();
#endif

private:
    int loop();
    void release();

    Display *m_display;
    int m_screenNumber;
    int m_screensTotal;
  
    Window *m_root;

    Colormap *m_defaultColormap;
//    int *m_minimumColormaps;

    Cursor *m_cursor;
    Cursor *m_xCursor;
    Cursor *m_vCursor;
    Cursor *m_hCursor;
    Cursor *m_vhCursor;
    
    char *m_terminal;
    char *m_shell;

    ClientList m_clients;
    ClientList m_hiddenClients;
    ClientList m_orderedClients;
    Client *m_activeClient;

#if CONFIG_GROUPS != False
    ListList grouping;
#endif

    int m_shapeEvent;
    int m_currentTime;

    int m_channels;
    int m_currentChannel;	// from 1 to ...
    void flipChannel(Boolean statusOnly, Boolean flipDown,
		     Boolean quickFlip, Client *push); // bleah!
    void instateChannel();
    void createNewChannel();
    void checkChannel(int);
    Time m_channelChangeTime;
    Window m_channelWindow;

    Boolean m_looping;
    int m_returnCode;

    static Boolean m_initialising;
    static int errorHandler(Display *, XErrorEvent *);
    static void sigHandler(int);
    static int m_signalled;
    static int m_restart;

    void initialiseScreen();
    void scanInitialWindows();

    void circulate(Boolean activeFirst);

    Boolean m_focusChanging;	// checking times for focus change
    Client *m_focusCandidate;
    Window  m_focusCandidateWindow;
    Time    m_focusTimestamp;	// time of last crossing event
    Boolean m_focusPointerMoved;
    Boolean m_focusPointerNowStill;
    void checkDelaysForFocus();

#if CONFIG_USE_SESSION_MANAGER != False
    int m_smFD;
    IceConn m_smIceConnection;
    SmcConn m_smConnection;
    char *m_oldSessionId;
    char *m_newSessionId;
    char *m_sessionProgram;

    static void smWatchFD(IceConn c, IcePointer, Bool, IcePointer *);
    static void smSaveYourself(SmcConn, SmPointer, int, Bool, int, Bool);
    static void smSaveYourself2(SmcConn, SmPointer);
    static void smShutdownCancelled(SmcConn, SmPointer);
    static void smSaveComplete(SmcConn, SmPointer);
    static void smDie(SmcConn, SmPointer);

    void initialiseSession(char *sessionProgram, char *oldSessionId);
    void setSessionProperties();
#endif

    void nextEvent(XEvent *);	// return

    void eventButton(XButtonEvent *, XEvent *);
    void eventKeyRelease(XKeyEvent *);
    void eventMapRequest(XMapRequestEvent *);
    void eventConfigureRequest(XConfigureRequestEvent *);
    void eventUnmap(XUnmapEvent *);
    void eventCreate(XCreateWindowEvent *);
    void eventDestroy(XDestroyWindowEvent *);
    void eventClient(XClientMessageEvent *);
    void eventColormap(XColormapEvent *);
    void eventProperty(XPropertyEvent *);
    void eventEnter(XCrossingEvent *);
    void eventReparent(XReparentEvent *);
    void eventFocusIn(XFocusInEvent *);
    void eventExposure(XExposeEvent *);

    Boolean m_altPressed;
    Boolean m_altStateRetained;
    void eventKeyPress(XKeyEvent *);

#if CONFIG_GNOME_COMPLIANCE != False
    void gnomeInitialiseCompliance();
    Window gnome_win;

#endif

    int m_altModMask;
};

#endif

