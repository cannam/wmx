
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
// #defined to dConfig.something() will take its default from Config.C

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
    char disableNew();

    static DynamicConfig dynamicConfig;

private:
    DynamicConfigImpl *m_impl;
    void update(char *);
};

static DynamicConfig &dConfig = DynamicConfig::dynamicConfig;


// =================================================
// Section I. Straightforward operational parameters
// =================================================

// List visible as well as hidden clients on the root menu?  (Visible
// ones will be towards the bottom of the menu, flush-right.)
#define CONFIG_EVERYTHING_ON_ROOT_MENU (dConfig.fullMenu())

// Spawn a temporary new shell between the wm and each new process?
#define CONFIG_EXEC_USING_SHELL   False

// What to run to get a new window (from the "New" menu option)
#define CONFIG_NEW_WINDOW_LABEL "New"
#define CONFIG_NEW_WINDOW_COMMAND "xterm"
#define CONFIG_NEW_WINDOW_COMMAND_OPTIONS 0
// or, for example,
//#define CONFIG_NEW_WINDOW_COMMAND_OPTIONS "-ls","-sb","-sl","1024",0
// alternatively,
#define CONFIG_DISABLE_NEW_WINDOW_COMMAND (dConfig.disableNew())

// Area where [exit wmx] is added (0 -> everywhere -# -> px from other side)
#define CONFIG_EXIT_CLICK_SIZE_X 0
#define CONFIG_EXIT_CLICK_SIZE_Y -3
 
// Directory under $HOME in which to look for commands for the
// middle-button menu
#define CONFIG_COMMAND_MENU       ".wmx"
// only used if COMMAND_MENU is not found; ignored if invalid directory:
#define CONFIG_SYSTEM_COMMAND_MENU	"/usr/local/lib/wmx/menu"

// Focus possibilities.
// 
// You can't have CLICK_TO_FOCUS without RAISE_ON_FOCUS, but the other
// combinations should be okay.  If you set AUTO_RAISE you must leave
// the other two False; you'll then get focus-follows, auto-raise, and
// a delay on auto-raise as configured in the DELAY settings below.

#define CONFIG_CLICK_TO_FOCUS     (dConfig.clickFocus())
#define CONFIG_RAISE_ON_FOCUS     (dConfig.raiseFocus())
#define CONFIG_AUTO_RAISE         (dConfig.autoRaiseFocus())

// Delays when using AUTO_RAISE focus method
// 
// In theory these only apply when using AUTO_RAISE, not when just
// using RAISE_ON_FOCUS without CLICK_TO_FOCUS.  First of these is the
// usual delay before raising; second is the delay after the pointer
// has stopped moving (only when over simple X windows such as xvt).

#define CONFIG_AUTO_RAISE_DELAY       (dConfig.raiseDelay())
#define CONFIG_POINTER_STOPPED_DELAY  80
#define CONFIG_DESTROY_WINDOW_DELAY   1000

// Number of pixels off the screen you have to push a window
// before the manager notices the window is off-screen (the higher
// the value, the easier it is to place windows at the screen edges)

#define CONFIG_BUMP_DISTANCE      16

// If CONFIG_PROD_SHAPE is True, all frame element shapes will be
// recalculated afresh every time their focus changes.  This will
// probably slow things down hideously, but has been reported as
// necessary on some systems (possibly SunOS 4.x with OpenWindows).

#define CONFIG_PROD_SHAPE         False

// If RESIZE_UPDATE is True, windows will opaque-resize "correctly";
// if False, behaviour will be as in wm2 (stretching the background
// image only).

#define CONFIG_RESIZE_UPDATE      True

// If RAISELOWER_ON_CLICK is True, clicking on the title of the
// topmost window will lower instead of raising it (patch due to
// Kazushi (Jam) Marukawa)

#define CONFIG_RAISELOWER_ON_CLICK	False

// If USE_WINDOW_GROUPS is True, then if an application marks a mapped
// window as a group leader, when iconified, moved between desktops,
// killed etc, the leader will take all the rest of its window group
// with it.  Very few applications make any use of this -- indeed I
// haven't found any to test it on yet, so it probably doesn't work

#define CONFIG_USE_WINDOW_GROUPS	True

// If USE_SESSION_MANAGER is True, and you have an Xsmc session
// manager running, some relevant information on which client lives on
// which desktop, etc., should be saved between sessions.  This is not
// yet implemented, so in fact wmx should just print out a startup
// message and then ignore it.  Want to finish the implementation?
// You're welcome.

#define CONFIG_USE_SESSION_MANAGER	False


// ========================
// Section II. Key bindings
// ========================

// Allow keyboard control?
#define CONFIG_USE_KEYBOARD       (dConfig.useKeyboard())

// This is a keyboard modifier mask as defined in <X11/X.h>.  It's the
// modifier required for wm controls: e.g. Alt/Left and Alt/Right to
// flip channels, and Alt/Tab to switch windows.  (The default value
// of Mod1Mask corresponds to Alt on many keyboards.  On my 105-key
// PC keyboard, Mod3Mask corresponds to the windows-95 key.)
// N.B.: if you have NumLock switched on, wmx might not get key events.

#define CONFIG_ALT_KEY_MASK       Mod1Mask
//#define CONFIG_ALT_KEY_MASK       Mod3Mask

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

// The next two may clash badly with Emacs, if you use Alt as the
// modifier.  The commented variants might work better for some.
#define CONFIG_CIRCULATE_KEY      XK_Tab
//#define CONFIG_CIRCULATE_KEY      XK_grave
//#define CONFIG_CIRCULATE_KEY      XK_section
#define CONFIG_DESTROY_KEY		XK_BackSpace
//#define CONFIG_DESTROY_KEY        XK_Delete
//#define CONFIG_DESTROY_KEY        XK_Insert

// If WANT_KEYBOARD_MENU is True, then the MENU_KEY, when pressed with
// the modifier, will call up a client menu with keyboard navigation
#define CONFIG_WANT_KEYBOARD_MENU	True
#define CONFIG_MENU_KEY		XK_Menu
// these are for navigating on the menu; they don't require a modifier
#define CONFIG_MENU_UP_KEY	XK_Up
#define CONFIG_MENU_DOWN_KEY	XK_Down
#define CONFIG_MENU_SELECT_KEY	XK_Return
#define CONFIG_MENU_CANCEL_KEY	XK_Escape

// Useful for fortunate people with Sun Type-5 keyboards.  These don't
// require the modifier to be pressed.
#define CONFIG_QUICKRAISE_KEY	XK_F15
#define CONFIG_QUICKHIDE_KEY	XK_F17
#define CONFIG_QUICKHEIGHT_KEY	XK_F13
#define CONFIG_QUICKRAISE_ALSO_LOWERS True 


// ==============================
// Section III. Colours and fonts
// ==============================

// Define I18N to enable I18N patch by Kazushi (Jam) Marukawa
// 
// Ability to display I18N characters.  Defines I18N as "1" to display
// them and execute wmx with LANG environment variable set "ja_JP".
// You need I18N ability of OS and X11 library also to do.  If you
// don't know about them, read the message wmx with I18N displays
// about result of setlocale().  It must be "ja" or your locale but
// not "C".  If you get "C", there are problems.  Try to add
// "-DX_LOCALE" as CFLAGS.  It might be help.  If you got "C" after
// all, probably you should compile your X11 library with -DX_LOCALE.
// I did it for my NetBSD box. :-)

#define I18N 1

// Fonts used all over the place.  NICE_FONT is for the frames, and
// NICE_MENU_FONT for the menus.  NASTY_FONT is what you'll get if it
// can't find one of the NICE ones.  If you have I18N, these are font
// lists rather than just simple fonts

#if I18N
#define CONFIG_NICE_FONT	  "-*-lucida-bold-r-*-*-14-*-75-75-*-*-*-*,-*-*-medium-r-*-*-14-*-75-75-*-*-*-*"
#define CONFIG_NICE_MENU_FONT	  "-*-lucida-medium-r-*-*-14-*-75-75-*-*-*-*,-*-*-medium-r-*-*-14-*-75-75-*-*-*-*"
#define CONFIG_NASTY_FONT	  "fixed,-*-*-*-*-*-*-14-*-75-75-*-*-*-*"
#else
#define CONFIG_NICE_FONT	  "-*-lucida-bold-r-*-*-14-*-75-75-*-*-*-*"
#define CONFIG_NICE_MENU_FONT	  "-*-lucida-medium-r-*-*-14-*-75-75-*-*-*-*"
#define CONFIG_NASTY_FONT	  "fixed"
#endif

// If USE_PLAIN_X_CURSORS is True, wmx will use cursors from the
// standard X cursor font; otherwise it will install its own.  You may
// wish to set this if your X client and server are on different
// endian machines, or if you've aliased the cursor font to something
// else you like better, or if you just prefer the plain X cursors

#define CONFIG_USE_PLAIN_X_CURSORS	False

// Colours for window decorations.  The BORDERS one is for the
// one-pixel border around the edge of each piece of decoration, not
// for the whole decoration

#define CONFIG_TAB_FOREGROUND	  "black"
#define CONFIG_TAB_BACKGROUND     "gray80"
#define CONFIG_FRAME_BACKGROUND   "gray95"
#define CONFIG_BUTTON_BACKGROUND  "gray95"
#define CONFIG_BORDERS            "black"

#define CONFIG_MENU_FOREGROUND    "black"
#define CONFIG_MENU_BACKGROUND    "gray80"
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
#define CONFIG_USE_PIXMAP_MENUS   False

// Set CHANNEL_SURF for multi-channel switching; CHANNEL_CLICK_SIZE is
// how close you have to middle-button-click to the top-right corner
// of the root window before the channel change happens.  Set
// USE_CHANNEL_KEYS if you want Alt-F1, Alt-F2 etc for quick channel
// changes, provided USE_KEYBOARD is also True.

#define CONFIG_CHANNEL_SURF       True
#define CONFIG_CHANNEL_CLICK_SIZE 120
#define CONFIG_USE_CHANNEL_KEYS   True

// FLIP_DELAY is the length of time the big green number stays in the
// top-right when flipping channels, before the windows reappear.
// QUICK_FLIP_DELAY is the equivalent figure used when flipping with
// the Alt-Fn keys.  Milliseconds.

#define CONFIG_FLIP_DELAY         1000
#define CONFIG_QUICK_FLIP_DELAY   200

// Set MAD_FEEDBACK for skeletal representations of windows when
// flicking through the client menu and changing channels.  The DELAY
// is how long skeletal feedback has to persist before wmx decides to
// post the entire window contents instead; if it's negative, the
// contents will never be posted.  Experiment with these -- if your
// machine is fast and you're really hyper, you might even like a
// delay of 0ms.

#define CONFIG_MAD_FEEDBACK       (dConfig.useFeedback())
#define CONFIG_FEEDBACK_DELAY     300


#endif

