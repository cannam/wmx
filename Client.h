
#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "General.h"
#include "Manager.h"
#include "Border.h"


class Client {
public:
    Client(WindowManager *const, Window, Boolean);
    void release();

    /* for call from WindowManager: */

    void activate();		/* active() */
    void deactivate();		/* setactive(0) */
    void gravitate(Boolean invert);
    void installColormap();
    void unreparent();
    void withdraw(Boolean changeState = True);
    void hide();
    void unhide(Boolean map);
    void rename();
    void kill();
    void mapRaised();		// without activating
    void lower();
    void raiseOrLower();	// without activating, raise if it isn't top.
				// otherwise lower.

    void move(XButtonEvent *);		// event for grab timestamp & coords
    void resize(XButtonEvent *, Boolean, Boolean);
    void moveOrResize(XButtonEvent *);
    void ensureVisible();	// make sure x, y are on-screen

    // These are the only accepted calls for feedback: the Manager
    // should *not* call directly into the Border
    void showFeedback();
    void raiseFeedbackLevel();
    void removeFeedback(Boolean mapped);

    void manage(Boolean mapped);
    Boolean hasWindow(Window);

    Client *revertTo() { return m_revert; }
    void setRevertTo(Client *c) { m_revert = c; }

    Boolean isHidden()     { return (m_state == IconicState);    }
    Boolean isWithdrawn()  { return (m_state == WithdrawnState); }
    Boolean isNormal()     { return (m_state == NormalState);    }
    Boolean isKilled()     { return (m_window == None);          }
    Boolean isTransient()  { return ((m_transient != None)||(m_shaped)); }
    Boolean isSticky()    { return m_sticky; }
    Window  transientFor() { return m_transient; }
#ifdef CONFIG_USE_WINDOW_GROUPS
    Window  groupParent() { return m_groupParent; }
    Boolean isGroupParent() { return m_window == m_groupParent; }
#endif
    Boolean isFixedSize()  { return m_fixedSize; }
    Boolean isShaped()     { return m_shaped; }

    const char *label()    { return m_label;    }
    const char *name()     { return m_name;     }
    const char *iconName() { return m_iconName; }

    int channel() { return m_channel;  }
    void flipChannel(Boolean leaving, int newChannel);
    Boolean isNormalButElsewhere() { return isNormal()||m_unmappedForChannel; }
    void setChannel(int channel) { m_channel = channel;
#if CONFIG_GNOME_COMPLIANCE != False
                                   gnomeSetChannel();
#endif
}
    void setSticky(Boolean sticky) { m_sticky = sticky; }

    void sendMessage(Atom, long);
    void sendConfigureNotify();

    void warpPointer();
    void activateAndWarp();
    void focusIfAppropriate(Boolean);
    void selectOnMotion(Window, Boolean);

    void fullHeight();
    void normalHeight();
    Boolean isFullHeight() { return m_isFullHeight; }
    void makeThisNormalHeight() { m_isFullHeight = False; }

    /* for call from within: */

    void fatal(char *m)    { m_windowManager->fatal(m);              }
    Display *display()     { return m_windowManager->display();      }
    int screen();
    Window parent()        { return m_border->parent();              }
    Window root();
    Client *activeClient() { return m_windowManager->activeClient(); }
    Boolean isActive()     { return (activeClient() == this);        }

    WindowManager *windowManager() { return m_windowManager; }

    // for call from equivalent wm functions in Events.C:

    void eventButton(XButtonEvent *);
    void eventMapRequest(XMapRequestEvent *);
    void eventConfigureRequest(XConfigureRequestEvent *);
    void eventUnmap(XUnmapEvent *);
    void eventColormap(XColormapEvent *);
    void eventProperty(XPropertyEvent *);
    void eventEnter(XCrossingEvent *);
    void eventFocusIn(XFocusInEvent *);
    void eventExposure(XExposeEvent *);

#if CONFIG_GNOME_COMPLIANCE != False
    void gnomeSetChannel();
    Window window()        { return m_window; }
#endif

protected:      // cravenly submitting to gcc's warnings
    ~Client();

private:

    Window m_window;
    Window m_transient;
    Window m_groupParent;
    Border *m_border;

    Boolean m_shaped;

    Client *m_revert;

    int m_x;
    int m_y;
    int m_w;
    int m_h;
    int m_bw;
    Window m_wroot;
    int m_screen;
    Boolean doSomething;	// Become true if move() or resize() made
				// effect to this client.

    int m_channel;
    Boolean m_unmappedForChannel;
    Boolean m_sticky;

//#if CONFIG_MAD_FEEDBACK != 0
    Boolean m_levelRaised;
    Boolean m_speculating;
//#endif

    XSizeHints m_sizeHints;
    Boolean m_fixedSize;
    int m_minWidth;
    int m_minHeight;
    void fixResizeDimensions(int &, int &, int &, int &);
    Boolean coordsInHole(int, int);

    int m_state;
    int m_protocol;
    Boolean m_managed;
    Boolean m_reparenting;
    Boolean m_stubborn;		// keeps popping itself to the front
    Time m_lastPopTime;

    Boolean m_isFullHeight;
    int m_normalH;
    int m_normalY;

    char *m_name;
    char *m_iconName;
    const char *m_label;	// alias: one of (instance,class,name,iconName)
    static const char *const m_defaultLabel;

    Colormap m_colormap;
    int m_colormapWinCount;
    Window *m_colormapWindows;
    Colormap *m_windowColormaps;

    WindowManager *const m_windowManager;

    char *getProperty(Atom);
    int getAtomProperty(Atom, Atom);
    int getIntegerProperty(Atom);

    // accessors 
    Boolean getState(int *);
    void setState(int);

    // internal instantiation requests
    Boolean setLabel(void);	// returns True if changed
    void getColormaps(void);
    void getProtocols(void);
    void getTransient(void);

    void decorate(Boolean active);
};

#define Pdelete    1
#define PtakeFocus 2

#endif

