
#ifndef _CONFIG_H_
#define _CONFIG_H_

// ============================
// Configuration header for wmx
// ============================
// 
// To configure: change the relevant bit of the file, and rebuild the
// manager.  Make sure all necessary source files are built (by
// running "make depend" before you start).
// 
// This file is in four sections.  (This might imply that it's getting
// too long.)  The sections are:
// 
// I.   Straightforward operational parameters
// II.  Key bindings
// III. Colours and fonts
// IV.  Flashy stuff: channels, pixmaps, skeletal feedback &c.
// 
// All timing values are in milliseconds, but accuracy depends on the
// minimum timer value available to select, so they should be taken
// with a pinch of salt.  On the machine I'm using now, I mentally
// double all the given values.
// 
// -- Chris Cannam, January 1998


// This isn't one of the sections.  This is the structure for Stefan
// `Sec' Zehl's runtime configuration hack -- see Config.C.  It's a
// bit incomplete, but works, mostly.  Anything in this file that's
// #defined to DynamicConfig::config.something() will take its default
// from Config.C

struct DynamicConfigImpl;
class DynamicConfig
{
public:
    DynamicConfig();
    ~DynamicConfig();
    void scan(char startup = 0);

    char clickFocus();
    char raiseFocus();
    char autoRaiseFocus();
    int  raiseDelay();
    char useKeyboard();
    char fullMenu();
    char useFeedback();
    int  feedbackDelay();
    char disableNew();
    char rightCirculate();
    char rightLower();
    char rightToggleHeight();
    char passFocusClick();
    int  tabMargin();
    char *tabForeground();
    char *tabBackground();
    char *frameBackground();

    static DynamicConfig config;

private:
    DynamicConfigImpl *m_impl;
    void update(char *);
};


// =================================================
// Section I. Straightforward operational parameters
// =================================================

// List visible as well as hidden clients on the root menu?  (Visible
// ones will be towards the bottom of the menu, flush-right.)
#define CONFIG_EVERYTHING_ON_ROOT_MENU (DynamicConfig::config.fullMenu())

// Spawn a temporary new shell between the wm and each new process?
#define CONFIG_EXEC_USING_SHELL   False

// What to run to get a new window (from the "New" menu option)
#define CONFIG_NEW_WINDOW_LABEL "New"
#define CONFIG_NEW_WINDOW_COMMAND "x-terminal-emulator"
#define CONFIG_NEW_WINDOW_COMMAND_OPTIONS 0
// or, for example,
//#define CONFIG_NEW_WINDOW_COMMAND_OPTIONS "-ls","-sb","-sl","1024",0
// alternatively,
#define CONFIG_DISABLE_NEW_WINDOW_COMMAND (DynamicConfig::config.disableNew())

// Area where [exit wmx] is added (0 -> everywhere -# -> px from other side)
#define CONFIG_EXIT_CLICK_SIZE_X 0
#define CONFIG_EXIT_CLICK_SIZE_Y -3
 
// Directory under $HOME in which to look for commands for the
// middle-button menu
#define CONFIG_COMMAND_MENU       ".wmx"
// only used if COMMAND_MENU is not found; ignored if invalid directory:
#define CONFIG_SYSTEM_COMMAND_MENU	"/usr/local/lib/wmx/menu"
// append screennumber to COMMAND_MENU directory;
// use non screen style as fallback
#define CONFIG_ADD_SCREEN_TO_COMMAND_MENU False
 
// Focus possibilities.
// 
// You can't have CLICK_TO_FOCUS without RAISE_ON_FOCUS, but the other
// combinations should be okay.  If you set AUTO_RAISE you must leave
// the other two False; you'll then get focus-follows, auto-raise, and
// a delay on auto-raise as configured in the DELAY settings below.

#define CONFIG_CLICK_TO_FOCUS     (DynamicConfig::config.clickFocus())
#define CONFIG_RAISE_ON_FOCUS     (DynamicConfig::config.raiseFocus())
#define CONFIG_AUTO_RAISE         (DynamicConfig::config.autoRaiseFocus())

#define CONFIG_PASS_FOCUS_CLICK   (DynamicConfig::config.passFocusClick())

// Delays when using AUTO_RAISE focus method
// 
// In theory these only apply when using AUTO_RAISE, not when just
// using RAISE_ON_FOCUS without CLICK_TO_FOCUS.  First of these is the
// usual delay before raising; second is the delay after the pointer
// has stopped moving (only when over simple X windows such as xvt).

#define CONFIG_AUTO_RAISE_DELAY       (DynamicConfig::config.raiseDelay())
#define CONFIG_POINTER_STOPPED_DELAY  80
#define CONFIG_DESTROY_WINDOW_DELAY   600

// Number of pixels off the screen you have to push a window
// before the manager notices the window is off-screen (the higher
// the value, the easier it is to place windows at the screen edges)

#define CONFIG_BUMP_DISTANCE      16

// If CONFIG_BUMP_EVERYWHERE is defined, windows will "bump" against
// other window edges as well as the edges of the screen

#define CONFIG_BUMP_EVERYWHERE    True

// If CONFIG_PROD_SHAPE is True, all frame element shapes will be
// recalculated afresh every time their focus changes.  This will
// probably slow things down hideously, but has been reported as
// necessary on some systems (possibly SunOS 4.x with OpenWindows).

#define CONFIG_PROD_SHAPE         False

// If RESIZE_UPDATE is True, windows will opaque-resize "correctly";
// if False, behaviour will be as in wm2 (stretching the background
// image only).

#define CONFIG_RESIZE_UPDATE      True

// If USE_COMPOSITE is true, wmx will enable composite redirects for
// all windows if the Composite extension is present.  This should
// make no difference at all to the appearance or behaviour of wmx,
// but it may make it substantially faster with modern video cards
// that optimise rendering more than old-fashioned window operations.

#define CONFIG_USE_COMPOSITE      True

// If RAISELOWER_ON_CLICK is True, clicking on the title of the
// topmost window will lower instead of raising it (patch due to
// Kazushi (Jam) Marukawa)

#define CONFIG_RAISELOWER_ON_CLICK	False

// If USE_WINDOW_GROUPS is True, then if an application marks a mapped
// window as a group leader, when iconified, moved between desktops,
// killed etc, the leader will take all the rest of its window group
// with it.  Very few applications make any use of this -- indeed I
// haven't found any to test it on yet, so it probably doesn't work.
// This is different from the group stuff from Henri Naccache to be
// found further down this file

#define CONFIG_USE_WINDOW_GROUPS	True

// If USE_SESSION_MANAGER is True, and you have an Xsmc session
// manager running, wmx will respond to session manager callbacks and
// notify the session manager of its restart command.

#define CONFIG_USE_SESSION_MANAGER	True

// Specify the maximum length of an entry in the client menu or the command
// menu. Set this to zero if you want no limitation

#define MENU_ENTRY_MAXLENGTH		80


// ========================
// Section II. Key bindings
// ========================

// Allow keyboard control?
#define CONFIG_USE_KEYBOARD       (DynamicConfig::config.useKeyboard())

// This is the key for wm controls: e.g. Alt/Left and Alt/Right to
// flip channels, and Alt/Tab to switch windows.  (On my 105-key
// PC keyboard, Meta_L corresponds to the left Windows key.)

#define CONFIG_ALT_KEY            XK_Super_L

// And these define the rest of the keyboard controls, when the above
// modifier is pressed; they're keysyms as defined in <X11/keysym.h>
// and <X11/keysymdef.h>

#define CONFIG_FLIP_UP_KEY        XK_Right
#define CONFIG_FLIP_DOWN_KEY      XK_Left
#define CONFIG_HIDE_KEY           XK_Return
#define CONFIG_STICKY_KEY         XK_Pause
#define CONFIG_RAISE_KEY          XK_Up
#define CONFIG_LOWER_KEY          XK_Down
// Prior and Next should be the same as Page_Up and Page_Down in R6
#define CONFIG_FULLHEIGHT_KEY     XK_Prior
#define CONFIG_NORMALHEIGHT_KEY   XK_Next
#define CONFIG_FULLWIDTH_KEY      XK_KP_Add
#define CONFIG_NORMALWIDTH_KEY    XK_KP_Subtract
#define CONFIG_MAXIMISE_KEY       XK_Home
#define CONFIG_UNMAXIMISE_KEY     XK_End
#define CONFIG_SAME_KEY_MAX_UNMAX False

// With modifier, print a list of client data to stdout
#define CONFIG_DEBUG_KEY          XK_Print

// The next two may clash badly with Emacs, if you use Alt as the
// modifier.  The commented variants might work better for some.
#define CONFIG_CIRCULATE_KEY	XK_Tab
//#define CONFIG_CIRCULATE_KEY	XK_grave
//#define CONFIG_CIRCULATE_KEY	XK_section
#define CONFIG_DESTROY_KEY	XK_BackSpace
//#define CONFIG_DESTROY_KEY	XK_Delete
//#define CONFIG_DESTROY_KEY	XK_Insert

// If WANT_KEYBOARD_MENU is True, then the MENU_KEY, when pressed with
// the modifier, will call up a client menu with keyboard navigation
#define CONFIG_WANT_KEYBOARD_MENU	True
#define CONFIG_CLIENT_MENU_KEY		XK_Menu
#define CONFIG_COMMAND_MENU_KEY		XK_Multi_key
#define CONFIG_EXIT_ON_KBD_MENU		True
// these are for navigating on the menu; they don't require a modifier
#define CONFIG_MENU_UP_KEY	XK_Up
#define CONFIG_MENU_DOWN_KEY	XK_Down
#define CONFIG_MENU_SELECT_KEY	XK_Return
#define CONFIG_MENU_CANCEL_KEY	XK_Escape

// Useful for fortunate people with Sun Type-5 keyboards.  These don't
// require the modifier to be pressed.
#define CONFIG_WANT_SUNKEYS	True
#define CONFIG_WANT_SUNPOWERKEY	True
#define CONFIG_QUICKRAISE_KEY	XK_F15
#define CONFIG_QUICKHIDE_KEY	XK_F17
#define CONFIG_QUICKHEIGHT_KEY	XK_F13
#define CONFIG_QUICKCLOSE_KEY	XK_F11
#define CONFIG_QUICKRAISE_ALSO_LOWERS True 
#define CONFIG_SUNPOWER_EXEC	"/usr/openwin/bin/sys-suspend"
#define CONFIG_SUNPOWER_OPTIONS	"-x","-h",0
#define CONFIG_SUNPOWER_SHIFTOPTIONS	"-x","-n",0

// Mouse Configuration
// Use this section to remap your mouse button actions.
//   Button1 = LMB, Button2 = MMB, Button3 = RMB 
//   Button4 = WheelUp, Button5 = WheelDown 
// To prevent one or more of these from being supported
// at all, define it to CONFIG_NO_BUTTON.
#define CONFIG_NO_BUTTON 999
#define CONFIG_CLIENTMENU_BUTTON  Button1
#define CONFIG_COMMANDMENU_BUTTON Button2
#define CONFIG_CIRCULATE_BUTTON   Button3 // switch window, when over frame
#define CONFIG_PREVCHANNEL_BUTTON Button5 // flip channel, when over frame
#define CONFIG_NEXTCHANNEL_BUTTON Button4 // flip channel, when over frame


// ==============================
// Section III. Colours and fonts
// ==============================

// Define CONFIG_USE_XFT to enable Xft support for drawing fonts.
// Otherwise plain X calls will be used.  The rest of the font
// configuration depends on whether or not this is set.

#define CONFIG_USE_XFT 1

#ifdef CONFIG_USE_XFT

// Fonts used all over the place.  FRAME_FONT is a comma-separated
// list of font names to use for the frame text (the first available
// font in the list is used).  MENU_FONT is likewise for menu text.
// The SIZE values are in pixels.

//!!! no proper way to handle italic/bold yet

#define CONFIG_FRAME_FONT "Bitstream Vera Sans,Lucida Sans Unicode"
#define CONFIG_FRAME_FONT_SIZE 12

#define CONFIG_MENU_FONT "Bitstream Vera Sans,Lucida Sans Unicode"
#define CONFIG_MENU_FONT_SIZE 12

#else

// Fonts used all over the place.  NICE_FONT is for the frames, and
// NICE_MENU_FONT for the menus.  NASTY_FONT is what you'll get if it
// can't find one of the NICE ones.  These are font lists, rather than
// single fonts

#define CONFIG_NICE_FONT	  "-*-lucida-bold-r-*-*-14-*-75-75-*-*-*-*,-*-*-medium-r-*-*-14-*-75-75-*-*-*-*"
#define CONFIG_NICE_MENU_FONT	  "-*-lucida-medium-r-*-*-14-*-75-75-*-*-*-*,-*-*-medium-r-*-*-14-*-75-75-*-*-*-*"
#define CONFIG_NASTY_FONT	  "fixed,-*-*-*-*-*-*-14-*-75-75-*-*-*-*"

#endif

// CONFIG_TAB_MARGIN defines the size of the gap on the left and right of the
// taxt in the tab.

#define CONFIG_TAB_MARGIN   (DynamicConfig::config.tabMargin())


// If USE_PLAIN_X_CURSORS is True, wmx will use cursors from the
// standard X cursor font; otherwise it will install its own.  You may
// wish to set this if your X client and server are on different
// endian machines, or if you've aliased the cursor font to something
// else you like better, or if you just prefer the plain X cursors

#define CONFIG_USE_PLAIN_X_CURSORS	True

// Colours for window decorations.  The BORDERS one is for the
// one-pixel border around the edge of each piece of decoration, not
// for the whole decoration

#define CONFIG_TAB_FOREGROUND     (DynamicConfig::config.tabForeground())
#define CONFIG_TAB_BACKGROUND     (DynamicConfig::config.tabBackground())
#define CONFIG_FRAME_BACKGROUND   (DynamicConfig::config.frameBackground())
#define CONFIG_BUTTON_BACKGROUND  (DynamicConfig::config.frameBackground())
#define CONFIG_BORDERS            "black"
#define CONFIG_CHANNEL_NUMBER	  "green"
#define CONFIG_CLOCK_NUMBER       "gray90"

#define CONFIG_MENU_FOREGROUND    (DynamicConfig::config.tabForeground())
#define CONFIG_MENU_BACKGROUND    (DynamicConfig::config.tabBackground())
#define CONFIG_MENU_BORDERS       "black"

// Pixel width for the bit of frame to the left of the window and the
// sum of the two bits at the top

#define CONFIG_FRAME_THICKNESS    8


// ========================
// Section IV. Flashy stuff
// ========================

// If USE_PIXMAPS is True the frames will be decorated with the pixmap
// in ./background.xpm; if USE_PIXMAP_MENUS is also True, the menus
// will be too.  The latter screws up in palette-based visuals, but
// should be okay in true-colour.

#define CONFIG_USE_PIXMAPS        True
#define CONFIG_USE_PIXMAP_MENUS   True

// Set CHANNEL_SURF for multi-channel switching; CHANNEL_CLICK_SIZE is
// how close you have to middle-button-click to the top-right corner
// of the root window before the channel change happens.  Set
// USE_CHANNEL_KEYS if you want Alt-F1, Alt-F2 etc for quick channel
// changes, provided USE_KEYBOARD is also True.  Set USE_CHANNEL_MENU
// if you want to change channels via a keyboard-controlled menu
// instead of linearly up and down one at a time like TV.

#define CONFIG_CHANNEL_SURF       True
#define CONFIG_CHANNEL_CLICK_SIZE 120
#define CONFIG_CHANNEL_NUMBER_SIZE 20
#define CONFIG_USE_CHANNEL_KEYS   True
#define CONFIG_USE_CHANNEL_MENU   False

// FLIP_DELAY is the length of time the big green number stays in the
// top-right when flipping channels, before the windows reappear.
// QUICK_FLIP_DELAY is the equivalent figure used when flipping with
// the Alt-Fn keys or mouse wheel.  Milliseconds.

#define CONFIG_FLIP_DELAY         1000
#define CONFIG_QUICK_FLIP_DELAY   500

// Set MAD_FEEDBACK for skeletal representations of windows when
// flicking through the client menu and changing channels.  The DELAY
// is how long skeletal feedback has to persist before wmx decides to
// post the entire window contents instead; if it's negative, the
// contents will never be posted.  Experiment with these -- if your
// machine is fast and you're really hyper, you might even like a
// delay of 0ms.

#define CONFIG_MAD_FEEDBACK       (DynamicConfig::config.useFeedback())
#define CONFIG_FEEDBACK_DELAY     (DynamicConfig::config.feedbackDelay())

// Position of the geometry window:
// X < 0 left, X > 0 right,  X = 0 center
// Y < 0 top,  Y > 0 bottom, Y = 0 center
#define CONFIG_GEOMETRY_X_POS     0
#define CONFIG_GEOMETRY_Y_POS     0

// Groups are fun. you can bind a bunch of windows to a number key
// and when you press CONFIG_ALT_KEY_MASK and the number key
// all the windows of that group are raised.

// You bind a window to a group by pressing the 
// CONFIG_ALT_KEY_MASK + CONFIG_GROUP_ADD + a number key

// if you press CONFIG_ALT_KEY_MASK + CONFIG_GROUP_REMOVE_ALL
// + a number key that group is cleared of all windows.

#define CONFIG_GROUPS             True
#define CONFIG_GROUP_ADD          ControlMask  
#define CONFIG_GROUP_REMOVE_ALL   ShiftMask

// Set CLOCK if you want wmx to display a clock permanently in the
// background at top-left of screen 0.  Use CONFIG_CLOCK_NUMBER
// to control the colour of the clock digits.

#define CONFIG_CLOCK              False

// This lets you choose whether to keep the regular wmx
// mouse button behaviour, or go w/ the GNOME-described one.
// If this is True, the left mouse button (button1) lets you 
// select 1 or more gmc 'icons' and drag them around etc,
// the right mouse button (button3) pops up the GNOME 
// command menu, and the middle mouse button (button2)
// acts like the left mouse normally does (list of windows)

#define CONFIG_GNOME_BUTTON_COMPLIANCE   False

#endif

