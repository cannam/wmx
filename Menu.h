
#ifndef _MENU_H_
#define _MENU_H_

#include "General.h"
#include "Manager.h"

class Menu 
{
public:
    Menu(WindowManager *manager, XEvent *e);
    virtual ~Menu();

    virtual int getSelection();
    static void cleanup(WindowManager *const);

protected:
    static Window m_window;
    
    static Boolean m_initialised;
    static GC m_menuGC;
#if I18N
    static XFontSet m_fontset;
#endif
    static XFontStruct *m_font;
    static unsigned long m_foreground;
    static unsigned long m_background;
    static unsigned long m_border;

    char **m_items;
    int m_nItems;
    int m_nHidden;

    WindowManager *m_windowManager;
    XEvent *m_event;

    virtual char **getItems(int *nitems, int *nhidden) = 0;

    Display *display() { return m_windowManager->display(); }
    Window root()      { return m_windowManager->root();    }
    int screen()       { return m_windowManager->screen();  }

    virtual void showFeedback(int) { }
    virtual void removeFeedback(int, Boolean) { }
    virtual void raiseFeedbackLevel(int) { }
};

class ClientMenu : public Menu
{
public:
    ClientMenu(WindowManager *, XEvent *e);
    virtual ~ClientMenu();

private:
    virtual char **getItems(int *, int *);
    ClientList m_clients;
    Boolean m_allowExit;

    Client *checkFeedback(int);
    virtual void showFeedback(int);
    virtual void removeFeedback(int, Boolean);
    virtual void raiseFeedbackLevel(int);
};

class CommandMenu : public Menu
{
public:
    CommandMenu(WindowManager *, XEvent *e);
    virtual ~CommandMenu();

private:
    virtual char **getItems(int *, int *);
    char *m_commandDir;
};

class ShowGeometry : public Menu
{
public:
    ShowGeometry(WindowManager *, XEvent *);
    virtual ~ShowGeometry();
    
    void update(int x, int y);
    void remove();

private:
    virtual char **getItems(int *, int *);
};

int nobuttons(XButtonEvent *e);

#endif /* _MENU_H_ */
