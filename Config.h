
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


// =================================================
// Section I. Straightforward operational parameters
// =================================================

// List visible as well as hidden clients on the root menu?  (Visible
// ones will be towards the bottom of the menu, flush-right.)
#define CONFIG_EVERYTHING_ON_ROOT_MENU True

// Spawn a temporary new shell between the wm and each new process?
#define CONFIG_EXEC_USING_SHELL   False

// What to run to get a new window (from the "New" menu option)
#define CONFIG_NEW_WINDOW_COMMAND "xterm"

// Directory under $HOME in which to look for commands for the
// middle-button menu
#define CONFIG_COMMAND_MENU       ".wmx"

// Focus possibilities.
// 
// You can't have CLICK_TO_FOCUS without RAISE_ON_FOCUS, but the other
// combinations should be okay.  If you set AUTO_RAISE you must leave
// the other two False; you'll then get focus-follows, auto-raise, and
// a delay on auto-raise as configured in the DELAY settings below.

#define CONFIG_CLICK_TO_FOCUS     False
#define CONFIG_RAISE_ON_FOCUS     False
#define CONFIG_AUTO_RAISE         False

// Delays when using AUTO_RAISE focus method
// 
// In theory these only apply when using AUTO_RAISE, not when just
// using RAISE_ON_FOCUS without CLICK_TO_FOCUS.  First of these is the
// usual delay before raising; second is the delay after the pointer
// has stopped moving (only when over simple X windows such as xvt).

#define CONFIG_AUTO_RAISE_DELAY       400
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


// ========================
// Section II. Key bindings
// ========================

// Allow keyboard control?
#define CONFIG_USE_KEYBOARD       True

// This is a keyboard modifier mask as defined in <X11/X.h>.  It's the
// modifier required for wm controls: e.g. Alt/Left and Alt/Right to
// flip channels, and Alt/Tab to switch windows.  (The default value
// of Mod1Mask corresponds to Alt on many keyboards.  On my MS Natural
// keyboard, Mod4Mask corresponds to the windows-95 key.)
// N.B.: if you have NumLock switched on, wmx might not get key events.

#define CONFIG_ALT_KEY_MASK       Mod1Mask
//#define CONFIG_ALT_KEY_MASK       Mod4Mask

// And these define the rest of the keyboard controls, when the above
// modifier is pressed; they're keysyms as defined in <X11/keysym.h>
// and <X11/keysymdef.h>

#define CONFIG_FLIP_UP_KEY        XK_Right
#define CONFIG_FLIP_DOWN_KEY      XK_Left
#define CONFIG_HIDE_KEY           XK_Return
#define CONFIG_RAISE_KEY          XK_Up
#define CONFIG_LOWER_KEY          XK_Down
#define CONFIG_FULLHEIGHT_KEY     XK_Page_Up
#define CONFIG_NORMALHEIGHT_KEY   XK_Page_Down
// The next two are keys that risk clashing with Emacs, if you use Alt
// as the modifier.  The commented variants might work better for some.
#define CONFIG_CIRCULATE_KEY      XK_Tab
//#define CONFIG_CIRCULATE_KEY      XK_grave
//#define CONFIG_CIRCULATE_KEY      XK_section
#define CONFIG_DESTROY_KEY        XK_Delete
//#define CONFIG_DESTROY_KEY        XK_Insert


// ==============================
// Section III. Colours and fonts
// ==============================

// Fonts used all over the place.  NICE_FONT is for the frames, and
// NICE_MENU_FONT for the menus.  NASTY_FONT is what you'll get if it
// can't find one of the NICE ones.

#define CONFIG_NICE_FONT	  "-*-lucida-bold-r-*-*-14-*-75-75-*-*-*-*"
#define CONFIG_NICE_MENU_FONT	  "-*-lucida-medium-r-*-*-14-*-75-75-*-*-*-*"
#define CONFIG_NASTY_FONT	  "fixed"

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
// sum of the two bits at the top.  I like 7 for plain frames, or 8 if
// using pixmaps (see later)

#define CONFIG_FRAME_THICKNESS    8


// ========================
// Section IV. Flashy stuff
// ========================

// If USE_PIXMAPS is True the frames will be decorated with the pixmap
// in ./background.xpm; if USE_PIXMAP_MENUS is also True, the menus
// will be too.  The latter screws up in palette-based visuals, but
// should be okay in true-colour.

#define CONFIG_USE_PIXMAPS        False
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

#define CONFIG_MAD_FEEDBACK       True
#define CONFIG_FEEDBACK_DELAY     300


#endif

