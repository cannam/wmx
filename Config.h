
#ifndef _CONFIG_H_
#define _CONFIG_H_

// All timing values are in milliseconds, but accuracy depends on the
// minimum timer value available to select, so they should be taken
// with a pinch of salt.  On the machine I'm using now, I mentally
// double all the given values.  --cc


// NICE_FONT is for the frames, and NICE_MENU_FONT for the menus.
// NASTY_FONT is what you'll get if it can't find one of the NICE
// ones.

#define CONFIG_NICE_FONT	  "-*-lucida-bold-r-*-*-14-*-75-75-*-*-*-*"
#define CONFIG_NICE_MENU_FONT	  "-*-lucida-medium-r-*-*-14-*-75-75-*-*-*-*"
#define CONFIG_NASTY_FONT	  "fixed"

#define CONFIG_EXEC_USING_SHELL   False
#define CONFIG_NEW_WINDOW_COMMAND "xterm"
#define CONFIG_EVERYTHING_ON_ROOT_MENU True

// Include keyboard control?
#define CONFIG_USE_KEYBOARD       True

// This is a keyboard modifier mask as defined in <X11/X.h>.  It's the
// modifier required for wm controls: e.g. Alt/Left and Alt/Right to
// flip channels, and Alt/Tab to switch windows.  (The default value
// of Mod1Mask corresponds to Alt on many keyboards.)

#define CONFIG_ALT_KEY_MASK       Mod1Mask

// And these define the rest of the keyboard controls, when the above
// modifier is pressed; they're keysyms as defined in <X11/keysym.h>
// and <X11/keysymdef.h>

#define CONFIG_FLIP_UP_KEY        XK_Right
#define CONFIG_FLIP_DOWN_KEY      XK_Left
#define CONFIG_CIRCULATE_KEY      XK_Tab
#define CONFIG_HIDE_KEY           XK_Return
#define CONFIG_DESTROY_KEY        XK_Delete
#define CONFIG_RAISE_KEY          XK_Up
#define CONFIG_LOWER_KEY          XK_Down
#define CONFIG_FULLHEIGHT_KEY     XK_Page_Up
#define CONFIG_NORMALHEIGHT_KEY   XK_Page_Down

// Directory under $HOME which is used for commands

#define CONFIG_COMMAND_MENU       ".wmx"

// You can't have CLICK_TO_FOCUS without RAISE_ON_FOCUS but the other
// combinations should be okay.  If you set AUTO_RAISE you must leave
// the other two False; you'll then get focus-follows, auto-raise, and
// a delay on auto-raise as configured in the DELAY settings below.

#define CONFIG_CLICK_TO_FOCUS     False
#define CONFIG_RAISE_ON_FOCUS     False
#define CONFIG_AUTO_RAISE         False

// In theory these only apply when using AUTO_RAISE, not when just
// using RAISE_ON_FOCUS without CLICK_TO_FOCUS.  First of these is the
// usual delay before raising; second is the delay after the pointer
// has stopped moving (only when over simple X windows such as xvt).

#define CONFIG_AUTO_RAISE_DELAY       400
#define CONFIG_POINTER_STOPPED_DELAY  80
#define CONFIG_DESTROY_WINDOW_DELAY   1000L

#define CONFIG_TAB_FOREGROUND	  "black"
#define CONFIG_TAB_BACKGROUND     "gray80"
#define CONFIG_FRAME_BACKGROUND   "gray95"
#define CONFIG_BUTTON_BACKGROUND  "gray95"
#define CONFIG_BORDERS            "black"

#define CONFIG_MENU_FOREGROUND    "black"
#define CONFIG_MENU_BACKGROUND    "gray80"
#define CONFIG_MENU_BORDERS       "black"

// I like 7 for plain frames, or 8 if using pixmaps:

#define CONFIG_FRAME_THICKNESS    8

// If CONFIG_PROD_SHAPE is True, all frame element shapes will be
// recalculated afresh every time their focus changes.  This will
// probably slow things down hideously, but has been reported as
// necessary on some systems (possibly SunOS 4.x with OpenWindows).

#define CONFIG_PROD_SHAPE         False


// Flashier stuff:

// If USE_PIXMAPS is True the frames will be decorated with the pixmap
// in ./background.xpm; if USE_PIXMAP_MENUS is also True, the menus
// will be too.  The latter screws up in palette-based visuals, but
// should be okay in true-colour.

#define CONFIG_USE_PIXMAPS        True
#define CONFIG_USE_PIXMAP_MENUS   False

// Set CHANNEL_SURF for multi-channel switching; CHANNEL_CLICK_SIZE is
// how close you have to middle-button-click to the top-right corner
// of the root window before the channel change happens.

#define CONFIG_CHANNEL_SURF       True
#define CONFIG_CHANNEL_CLICK_SIZE 120

// Set MAD_FEEDBACK for skeletal representations of windows when
// flicking through the client menu and changing channels.  The DELAY
// is how long skeletal feedback has to persist before wmx decides to
// post the entire window contents instead; if it's negative, the
// contents will never be posted.  Experiment with these -- if your
// machine is fast and you're really hyper, you might even like a
// delay of 0ms.

#define CONFIG_MAD_FEEDBACK       True
#define CONFIG_FEEDBACK_DELAY     300

// If RESIZE_UPDATE is True, windows will opaque-resize "correctly";
// if False, behaviour will be as in wm2 (stretching the background
// image only).

#define CONFIG_RESIZE_UPDATE      True


#endif

