
#ifndef _MENU_H_
#define _MENU_H_

#include "General.h"
#include "Manager.h"

class Menu 
{
public:
    Menu(WindowManager *manager, XButtonEvent *e);
    virtual ~Menu();

    virtual int getSelection();
    static void cleanup(WindowManager *const);

protected:
    static Window m_window;
    
    static Boolean m_initialised;
    static GC m_menuGC;
    static XFontStruct *m_font;
    static unsigned long m_foreground;
    static unsigned long m_background;
    static unsigned long m_border;

    char **m_items;
    int m_nItems;
    int m_nHidden;

    WindowManager *m_windowManager;
    XButtonEvent *m_event;

    virtual char **getItems(int *nitems, int *nhidden) = 0;

    Display *display() { return m_windowManager->display(); }
    Window root()      { return m_windowManager->root();    }
    int screen()       { return m_windowManager->screen();  }
};

class ClientMenu : public Menu
{
public:
    ClientMenu(WindowManager *, XButtonEvent *e);
    virtual ~ClientMenu();

private:
    virtual char **getItems(int *, int *);
    ClientList m_clients;
    Boolean m_allowExit;
};

class CommandMenu : public Menu
{
public:
    CommandMenu(WindowManager *, XButtonEvent *e);
    virtual ~CommandMenu();

private:
    virtual char **getItems(int *, int *);
    char *m_commandDir;
};

class ShowGeometry : public Menu
{
public:
    ShowGeometry(WindowManager *, XButtonEvent *);
    virtual ~ShowGeometry();
    
    void update(int x, int y);
    void remove();

private:
    virtual char **getItems(int *, int *);
};

int nobuttons(XButtonEvent *e);

#endif /* _MENU_H_ */
