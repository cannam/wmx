
#include "Border.h"
#include "Client.h"
#include "Manager.h"
#include "Rotated.h"

// Some shaping mods due to Jacques Garrigue, garrigue@kurims.kyoto-u.ac.jp

#if CONFIG_USE_PIXMAPS != False
#include <X11/xpm.h>
#include "background.xpm"
#endif

// These are degenerate initialisations, don't change them
int *Border::m_tabWidth = 0;
XRotFontStruct **Border::m_tabFont = 0;
GC  *Border::m_drawGC = 0;

unsigned long *Border::m_foregroundPixel;
unsigned long *Border::m_backgroundPixel;
unsigned long *Border::m_frameBackgroundPixel;
unsigned long *Border::m_buttonBackgroundPixel;
unsigned long *Border::m_borderPixel;
Pixmap Border::m_backgroundPixmap = None;


class BorderRectangle // must resemble XRectangle in storage
{
public:
    BorderRectangle() { }
    BorderRectangle(int xx, int yy, int ww, int hh) {
	x = xx; y = yy; width = ww; height = hh;
    }
    BorderRectangle(const BorderRectangle &b) {
	x = b.x; y = b.y; width = b.width; height = b.height;
    }

    XRectangle *xrectangle() { return (XRectangle *)this; }

    short x, y;
    unsigned short width, height;
};

declareList(RectangleList, BorderRectangle);
implementList(RectangleList, BorderRectangle);

class BorderRectangleList : public RectangleList
{
public:
    BorderRectangleList() { }
    virtual ~BorderRectangleList() { }

    void append(const BorderRectangle &r) { RectangleList::append(r); }
    void append(int x, int y, int w, int h);
    XRectangle *xrectangles() { return (XRectangle *)array(0, count()); }
};

void BorderRectangleList::append(int x, int y, int w, int h)
{
    append(BorderRectangle(x, y, w, h));
}


Border::Border(Client *const c, Window child) :
    m_client(c), m_parent(0), m_tab(0),
    m_child(child), m_button(0), m_resize(0), m_label(0),
    m_prevW(-1), m_prevH(-1), m_tabHeight(-1)
{
    m_parent = root();
    if (m_tabFont == 0) initialiseStatics(c->windowManager());

    shaped = c->isShaped();
	
//#if CONFIG_MAD_FEEDBACK != 0
    m_feedback = 0;
    m_fedback = False;
//#endif
}


Border::~Border()
{
    if (m_parent != root()) {
	if (!m_parent) fprintf(stderr,"wmx: zero parent in Border::~Border\n");
	else {
	    XDestroyWindow(display(), m_tab);
	    XDestroyWindow(display(), m_button);
	    XDestroyWindow(display(), m_parent);
	    XDestroyWindow(display(), m_resize);

	    if (m_feedback) {
	        XDestroyWindow(display(), m_feedback);
	    }
	}
    }

    if (m_label) free(m_label);
}


void Border::initialiseStatics(WindowManager *wm)
{
    if (m_drawGC) return;

    XGCValues *values;
    
    m_drawGC = (GC *) malloc(wm->screensTotal() * sizeof(GC));
    m_tabFont = (XRotFontStruct **) malloc(wm->screensTotal() * 
					   sizeof(XRotFontStruct *));
    m_tabWidth = (int *) malloc(wm->screensTotal() * sizeof(int));
    m_foregroundPixel = (unsigned long *) malloc(wm->screensTotal() * 
						 sizeof(unsigned long));
    m_backgroundPixel = (unsigned long *) malloc(wm->screensTotal() * 
						 sizeof(unsigned long));
    m_frameBackgroundPixel = (unsigned long *) malloc(wm->screensTotal() * 
						      sizeof(unsigned long));
    m_buttonBackgroundPixel = (unsigned long *) malloc(wm->screensTotal() * 
						       sizeof(unsigned long));
    m_borderPixel = (unsigned long *) malloc(wm->screensTotal() * 
					     sizeof(unsigned long));
    
    values = (XGCValues *) malloc(wm->screensTotal() * sizeof(XGCValues));

    if (sizeof(BorderRectangle) != sizeof(XRectangle)) {
	wm->fatal("internal error: border rectangle and X rectangle\n"
		  "have different storage requirements -- bailing out");
    }

    for (int i = 0; i < wm->screensTotal(); i++)
    {
	m_tabWidth[i] = -1;
	
	m_foregroundPixel[i] = wm->allocateColour
	  (i, CONFIG_TAB_FOREGROUND, "tab foreground");
	m_backgroundPixel[i] = wm->allocateColour
	  (i, CONFIG_TAB_BACKGROUND, "tab background");
	m_frameBackgroundPixel[i] = wm->allocateColour
	  (i, CONFIG_FRAME_BACKGROUND, "frame background");
	m_buttonBackgroundPixel[i] = wm->allocateColour
	  (i, CONFIG_BUTTON_BACKGROUND, "button background");
	m_borderPixel[i] = wm->allocateColour
	  (i, CONFIG_BORDERS, "border");

	if (!(m_tabFont[i] = XRotLoadFont(wm->display(), i,
					  CONFIG_NICE_FONT, 90.0)))
	{
	    if(!(m_tabFont[i] = XRotLoadFont(wm->display(), i,
					     CONFIG_NASTY_FONT, 90.0)))
	    {
		wm->fatal("couldn't load default rotated font, bailing out");
	    }
	}

	m_tabWidth[i] = m_tabFont[i]->height + 4;
	if (m_tabWidth[i] < TAB_TOP_HEIGHT * 2 + 8) {
	    m_tabWidth[i] = TAB_TOP_HEIGHT * 2 + 8;
	}

	values[i].foreground = m_foregroundPixel[i];
	values[i].background = m_backgroundPixel[i];
	values[i].function = GXcopy;
	values[i].line_width = 0;
	values[i].subwindow_mode = IncludeInferiors;
	
	m_drawGC[i] = XCreateGC
          (wm->display(), wm->mroot(i),
           GCForeground | GCBackground | GCFunction |
           GCLineWidth | GCSubwindowMode,
           &values[i]);
    
	if (!m_drawGC[i]) {
	    wm->fatal("couldn't allocate border GC");
	}

#if CONFIG_USE_PIXMAPS != False
        int use_dynamic = True;
        XpmAttributes attrs;

        //	attrs.valuemask = 0L;

        attrs.valuemask =
          XpmCloseness | XpmVisual | XpmColormap | XpmDepth;
        attrs.closeness = 40000;
	attrs.visual = XDefaultVisual(wm->display(), i);
	attrs.colormap = XDefaultColormap(wm->display(), i);
	attrs.depth = XDefaultDepth(wm->display(), i);
			
        char *home = getenv ("HOME");
        if (home) {
            char *pixMapPath = (char *) malloc (sizeof (char) * (strlen (home)
                                                                 + strlen (CONFIG_COMMAND_MENU) + 20));
            if (pixMapPath) {
                strcpy (pixMapPath, home);
                strcat (pixMapPath, "/");
                strcat (pixMapPath, CONFIG_COMMAND_MENU);
                strcat (pixMapPath, "/border.xpm");
                
                // look for background pixmap file in
                // the users .wmx directory. It is *not*
                // an error, if none is found. (joze)
                // 
                // (todo: omit compiling in default pixmap, and look for
                //  it in systemwide directory)
                if (XpmReadFileToPixmap (wm->display(), wm->root(), pixMapPath,
                                         &m_backgroundPixmap, NULL, &attrs)
                    != XpmSuccess) {
                    use_dynamic = False;
                }
                free (pixMapPath);
            }
        }
        
        if (use_dynamic == False) {
            switch (XpmCreatePixmapFromData
                    (wm->display(), wm->root(), background,
                     &m_backgroundPixmap, NULL, &attrs)) {
            case XpmSuccess:
                break;
            case XpmColorError:
                fprintf(stderr, "wmx: Pixmap colour error\n");
                m_backgroundPixmap = None;
                break;
            case XpmOpenFailed:
                fprintf(stderr, "wmx: Pixmap open failed\n");
                m_backgroundPixmap = None;
                break;
            case XpmFileInvalid:
                fprintf(stderr, "wmx: Pixmap file invalid\n");
                m_backgroundPixmap = None;
                break;
            case XpmNoMemory:
                fprintf(stderr, "wmx: Ran out of memory when loading pixamp\n");
                m_backgroundPixmap = None;
                break;
            case XpmColorFailed:
                fprintf(stderr, "wmx: Pixmap colour failed\n");
                m_backgroundPixmap = None;
                break;
            default:
                fprintf(stderr, "wmx: Unknown error loading pixmap\n");
                m_backgroundPixmap = None;
                break;
            }
	} else
#endif  
            m_backgroundPixmap = None;
    }
}


Boolean Border::hasWindow(Window w)
{
    return (w != root() && (w == m_parent || w == m_tab ||
			    w == m_button || w == m_resize));
}


Pixmap Border::backgroundPixmap(WindowManager *wm)
{
    if (!m_drawGC) Border::initialiseStatics(wm);
    return m_backgroundPixmap;
}


GC Border::drawGC(WindowManager *wm,int screen)
{
    if (!m_drawGC) Border::initialiseStatics(wm);
    return m_drawGC[screen];
}


void Border::fatal(char *s)
{
    windowManager()->fatal(s);
}


Display *Border::display()
{
    return m_client->display();
}


int Border::screen()
{
    return m_client->screen();
}


WindowManager *Border::windowManager()
{
    return m_client->windowManager();
}


Window Border::root()
{
    return m_client->root();
}


void Border::expose(XExposeEvent *e)
{
    if (e->window != m_tab) return;
    drawLabel();
}


void Border::drawLabel()
{
    if (m_label) {
	XClearWindow(display(), m_tab);
	XRotDrawString(display(), screen(), m_tabFont[screen()], m_tab,
		       m_drawGC[screen()], 2 +m_tabFont[screen()]->max_ascent,
		       m_tabHeight - 1, m_label, strlen(m_label));
    }
}


Boolean Border::isTransient(void)
{
    return m_client->isTransient();
}


Boolean Border::isFixedSize(void)
{
    return m_client->isFixedSize();
}


void Border::fixTabHeight(int maxHeight)
{
    m_tabHeight = 0x7fff;
    maxHeight -= m_tabWidth[screen()];	// for diagonal

    // At least we need the button and its box.
    if (maxHeight < m_tabWidth[screen()] + 2) maxHeight = 
      m_tabWidth[screen()] + 2;

    if (m_label) free(m_label);
    m_label = NewString(m_client->label());
    
    if (m_label) {
	m_tabHeight =
	    XRotTextWidth(m_tabFont[screen()], m_label, strlen(m_label)) + 6 + m_tabWidth[screen()];
    }

    if (m_tabHeight <= maxHeight) return;

//!!!
//    if (m_label) free(m_label);
//    m_label = m_client->iconName() ?
//	NewString(m_client->iconName()) : NewString("incognito");
    if (!m_label) {
	m_label = m_client->iconName() ?
	    NewString(m_client->iconName()) : NewString("incognito");
    }

    int len = strlen(m_label);
    m_tabHeight = XRotTextWidth(m_tabFont[screen()], m_label, len) + 6 + m_tabWidth[screen()];
    if (m_tabHeight <= maxHeight) return;

    char *newLabel = (char *)malloc(len + 3);

    do {
	strncpy(newLabel, m_label, len - 1);
	strcpy(newLabel + len - 1, "...");
	m_tabHeight = XRotTextWidth(m_tabFont[screen()], newLabel,
				    strlen(newLabel)) + 6 + m_tabWidth[screen()];
	--len;

    } while (m_tabHeight > maxHeight && len > 2);

    free(m_label);
    m_label = newLabel;

    if (m_tabHeight > maxHeight) {
	m_tabHeight = maxHeight;
	free(m_label);
	m_label = NewString("");
    }
}


void Border::shapeTransientParent(int w, int h)
{
    if (m_client->isShaped()) {
	// dude is shaped. just grab the list of rectangles (jeez, why
	// didn't they just keep the pixmap too?)  and apply it w/ the
	// xIndent and yIndent (otherwise you get the outline)

	int count = 0;
	int ordering = 0;
	
	XRectangle* recs =  XShapeGetRectangles(display(), m_child, ShapeBounding, &count, &ordering);
	XShapeCombineRectangles
	    (display(), m_parent, ShapeBounding, xIndent(), yIndent(),
	     recs, count, ShapeSet, ordering);

	XFree(recs);

//      count me among the clueless... i have no idea why
//      this code is not needed.... (henri, 1.9.99)

/*	recs =  XShapeGetRectangles(display(), m_child, ShapeClip, &count, &ordering);
	XShapeCombineRectangles
	    (display(), m_parent, ShapeClip, xIndent(), yIndent(),
	     recs, count, ShapeSet, ordering);

	XFree(recs);
 */

    } else {
	BorderRectangle r(xIndent() - 1, yIndent() - 1, w + 2, h + 2);
	
	XShapeCombineRectangles
	    (display(), m_parent, ShapeBounding, 0, 0,
	     r.xrectangle(), 1, ShapeSet, YXBanded);
	
	r = BorderRectangle(xIndent(), yIndent(), w, h);
	
	XShapeCombineRectangles
	    (display(), m_parent, ShapeClip, 0, 0,
	     r.xrectangle(), 1, ShapeSet, YXBanded);
    }
}


void Border::setTransientFrameVisibility(Boolean visible, int w, int h)
{


    if (m_client->isShaped()) {
	// don't do anything. these guys are shaped, 
	//they should take care of showing that they have focus
	return;

    } else {

	int i;
	BorderRectangleList rl;
	
	rl.append(0, 0, w + 1, yIndent() - 1);
	for (i = 1; i < yIndent(); ++i) {
	    rl.append(w + 1, i - 1, i + 1, 1);
	}
	rl.append(0, yIndent() - 1, xIndent() - 1, h - yIndent() + 2);
	for (i = 1; i < yIndent(); ++i) {
	    rl.append(i - 1, h, 1, i + 2);
	}
	
	XShapeCombineRectangles
	    (display(), m_parent, ShapeBounding,
	     0, 0, rl.xrectangles(), rl.count(),
	     visible ? ShapeUnion : ShapeSubtract, YXSorted);
	
	rl.remove_all();
	
	rl.append(1, 1, w, yIndent() - 2);
	for (i = 2; i < yIndent(); ++i) {
	    rl.append(w + 1, i - 1, i, 1);
	}
	rl.append(1, yIndent() - 1, xIndent() - 2, h - yIndent() + 1);
	for (i = 2; i < yIndent(); ++i) {
	    rl.append(i - 1, h, 1, i + 1);
	}
	
	XShapeCombineRectangles
	    (display(), m_parent, ShapeClip,
	     0, 0, rl.xrectangles(), rl.count(),
	     visible ? ShapeUnion : ShapeSubtract, YXSorted);
    }
}   


void Border::shapeParent(int w, int h)
{
    int i;
    int mainRect;
    BorderRectangleList rl;

    if (isTransient()) {
	shapeTransientParent(w, h);
	return;
    }

    // Bounding rectangles -- clipping will be the same except for
    // child window border

    // top of tab
    rl.append(0, 0, w + m_tabWidth[screen()] + 1, TAB_TOP_HEIGHT + 2);

    // struts in tab, left...
    rl.append(0, TAB_TOP_HEIGHT + 1,
	      TAB_TOP_HEIGHT + 2, m_tabWidth[screen()] - TAB_TOP_HEIGHT*2 - 1);

    // ...and right
    rl.append(m_tabWidth[screen()] - TAB_TOP_HEIGHT, TAB_TOP_HEIGHT + 1,
	      TAB_TOP_HEIGHT + 2, m_tabWidth[screen()] - TAB_TOP_HEIGHT*2 - 1);

    mainRect = rl.count();
    rl.append(xIndent() - 1, yIndent() - 1, w + 2, h + 2);

    // main tab
    rl.append(0, m_tabWidth[screen()] - TAB_TOP_HEIGHT, m_tabWidth[screen()] + 2,
	      m_tabHeight - m_tabWidth[screen()] + TAB_TOP_HEIGHT);

    // diagonal
    for (i = 1; i < m_tabWidth[screen()] - 1; ++i) {
	int y = m_tabHeight + i - 1;
	/* JG: Check position */
	if (y < h) rl.append(i, y, m_tabWidth[screen()] - i + 2, 1);
//	rl.append(i, m_tabHeight + i - 1, m_tabWidth - i + 2, 1);
    }

    XShapeCombineRectangles
	(display(), m_parent, ShapeBounding,
	 0, 0, rl.xrectangles(), rl.count(), ShapeSet, YXSorted);

    rl.item(mainRect).x ++;
    rl.item(mainRect).y ++;
    rl.item(mainRect).width -= 2;
    rl.item(mainRect).height -= 2;

    XShapeCombineRectangles
	(display(), m_parent, ShapeClip,
	 0, 0, rl.xrectangles(), rl.count(), ShapeSet, YXSorted);
}


void Border::shapeTab(int w, int h)
{
    int i;
    BorderRectangleList rl;

    if (isTransient()) {
	return;
    }

    // Bounding rectangles

    rl.append(0, 0, w + m_tabWidth[screen()] + 1, TAB_TOP_HEIGHT + 2);
    rl.append(0, TAB_TOP_HEIGHT + 1, TAB_TOP_HEIGHT + 2,
	      m_tabWidth[screen()] - TAB_TOP_HEIGHT*2 - 1);
    
    rl.append(m_tabWidth[screen()] - TAB_TOP_HEIGHT, TAB_TOP_HEIGHT + 1,
	      TAB_TOP_HEIGHT + 2, m_tabWidth[screen()] - TAB_TOP_HEIGHT*2 - 1);

    rl.append(0, m_tabWidth[screen()] - TAB_TOP_HEIGHT, m_tabWidth[screen()] + 2,
	      m_tabHeight - m_tabWidth[screen()] + TAB_TOP_HEIGHT);

    for (i = 1; i < m_tabWidth[screen()] - 1; ++i) {
	int y = m_tabHeight + i - 1;
	/* JG: Check position */
	if (y <= h) rl.append(i, y, m_tabWidth[screen()] - i + 2, 1);
//	rl.append(i, m_tabHeight + i - 1, m_tabWidth[screen()] - i + 2, 1);
    }

    XShapeCombineRectangles
	(display(), m_tab, ShapeBounding,
	 0, 0, rl.xrectangles(), rl.count(), ShapeSet, YXSorted);

    rl.remove_all();

    // Clipping rectangles

    rl.append(1, 1, w + m_tabWidth[screen()] - 1, TAB_TOP_HEIGHT);

    rl.append(1, TAB_TOP_HEIGHT + 1, TAB_TOP_HEIGHT,
	      m_tabWidth[screen()] + TAB_TOP_HEIGHT*2 - 1);

    rl.append(m_tabWidth[screen()] - TAB_TOP_HEIGHT + 1, TAB_TOP_HEIGHT + 1,
	      TAB_TOP_HEIGHT, m_tabWidth[screen()] + TAB_TOP_HEIGHT*2 - 1);

    rl.append(1, m_tabWidth[screen()] - TAB_TOP_HEIGHT + 1, m_tabWidth[screen()],
	      m_tabHeight - m_tabWidth[screen()] + TAB_TOP_HEIGHT - 1);

    for (i = 1; i < m_tabWidth[screen()] - 2; ++i) {
	int y = m_tabHeight + i - 1;
	/* JG: Check position */
	if (y < h) rl.append(i + 1, y, m_tabWidth[screen()] - i, 1);
//	rl.append(i + 1, m_tabHeight + i - 1, m_tabWidth[screen()] - i, 1);
    }

    XShapeCombineRectangles
	(display(), m_tab, ShapeClip,
	 0, 0, rl.xrectangles(), rl.count(), ShapeSet, YXSorted);
}


void Border::resizeTab(int h)
{
    int i;
    int shorter, longer, operation;

    if (isTransient()) {
	return;
    }

    int prevTabHeight = m_tabHeight;
    fixTabHeight(h);
    // If resize is not needed, title might be needed redraw.
    // Because this is called from rename() sometimes.
    // So do it independently.
    if (m_tabHeight == prevTabHeight)
    {
	drawLabel();
	return;
    }

    XWindowChanges wc;
    wc.height = m_tabHeight + 2 + m_tabWidth[screen()];
    XConfigureWindow(display(), m_tab, CWHeight, &wc);

    if (m_tabHeight > prevTabHeight) {

	shorter = prevTabHeight;
	longer = m_tabHeight;
	operation = ShapeUnion;

    } else {

	shorter = m_tabHeight;
	longer = prevTabHeight + m_tabWidth[screen()];
	operation = ShapeSubtract;
    }

    BorderRectangle r(0, shorter, m_tabWidth[screen()] + 2, longer - shorter);

    XShapeCombineRectangles(display(), m_parent, ShapeBounding,
			    0, 0, r.xrectangle(), 1, operation, YXBanded);

    XShapeCombineRectangles(display(), m_parent, ShapeClip,
			    0, 0, r.xrectangle(), 1, operation, YXBanded);

    XShapeCombineRectangles(display(), m_tab, ShapeBounding,
			    0, 0, r.xrectangle(), 1, operation, YXBanded);

    r.x ++; r.width -= 2;

    XShapeCombineRectangles(display(), m_tab, ShapeClip,
			    0, 0, r.xrectangle(), 1, operation, YXBanded);

    if (m_client->isActive()) {
	// restore a bit of the frame edge
	r.x = m_tabWidth[screen()] + 1; r.y = shorter;
	r.width = FRAME_WIDTH - 1; r.height = longer - shorter;
	XShapeCombineRectangles(display(), m_parent, ShapeBounding,
				0, 0, r.xrectangle(), 1, ShapeUnion, YXBanded);
    }

    BorderRectangleList rl;
    for (i = 1; i < m_tabWidth[screen()] - 1; ++i) {
	rl.append(i, m_tabHeight + i - 1, m_tabWidth[screen()] - i + 2, 1);
    }
	
    XShapeCombineRectangles
	(display(), m_parent, ShapeBounding,
	 0, 0, rl.xrectangles(), rl.count(), ShapeUnion, YXBanded);

    XShapeCombineRectangles
	(display(), m_parent, ShapeClip,
	 0, 0, rl.xrectangles(), rl.count(), ShapeUnion, YXBanded);

    XShapeCombineRectangles
	(display(), m_tab, ShapeBounding,
	 0, 0, rl.xrectangles(), rl.count(), ShapeUnion, YXBanded);

    if (rl.count() < 2) return;

    for (i = 0; i < rl.count() - 1; ++i) {
	rl.item(i).x ++; rl.item(i).width -= 2;
    }

    XShapeCombineRectangles
	(display(), m_tab, ShapeClip,
	 0, 0, rl.xrectangles(), rl.count() - 1, ShapeUnion, YXBanded);
}


void Border::shapeResize()
{
    int i;
    BorderRectangleList rl;

    for (i = 0; i < FRAME_WIDTH*2; ++i) {
	rl.append(FRAME_WIDTH*2 - i - 1, i, i + 1, 1);
    }

    XShapeCombineRectangles
	(display(), m_resize, ShapeBounding, 0, 0,
	 rl.xrectangles(), rl.count(), ShapeSet, YXBanded);

    rl.remove_all();

    for (i = 1; i < FRAME_WIDTH*2; ++i) {
	rl.append(FRAME_WIDTH*2 - i, i, i, 1);
    }

    XShapeCombineRectangles
	(display(), m_resize, ShapeClip, 0, 0,
	 rl.xrectangles(), rl.count(), ShapeSet, YXBanded);

    rl.remove_all();

    for (i = 0; i < FRAME_WIDTH*2 - 3; ++i) {
	rl.append(FRAME_WIDTH*2 - i - 1, i + 3, 1, 1);
    }

    XShapeCombineRectangles
	(display(), m_resize, ShapeClip, 0, 0,
	 rl.xrectangles(), rl.count(), ShapeSubtract, YXBanded);

    windowManager()->installCursorOnWindow
	(WindowManager::DownrightCursor, m_resize);
}


void Border::setFrameVisibility(Boolean visible, int w, int h)
{
    BorderRectangleList rl;

    if (CONFIG_PROD_SHAPE) {
	shapeParent(w, h);
	shapeTab(w, h);
    }

    if (isTransient()) {
	setTransientFrameVisibility(visible, w, h);
	return;
    }

    // Bounding rectangles

    rl.append(m_tabWidth[screen()] + w + 1, 0, FRAME_WIDTH + 1, FRAME_WIDTH);

    rl.append(m_tabWidth[screen()] + 2, TAB_TOP_HEIGHT + 2, w,
	      FRAME_WIDTH - TAB_TOP_HEIGHT - 2);

    // for button
    int ww = m_tabWidth[screen()] - TAB_TOP_HEIGHT*2 - 4;
    rl.append((m_tabWidth[screen()] + 2 - ww) / 2, (m_tabWidth[screen()]+2 - ww) / 2, ww, ww);

    rl.append(m_tabWidth[screen()] + 2, FRAME_WIDTH,
	      FRAME_WIDTH - 2, m_tabHeight + m_tabWidth[screen()] - FRAME_WIDTH - 2);

    // swap last two if sorted wrong
    if (rl.item(rl.count()-2).y > rl.item(rl.count()-1).y) {
	rl.append(rl.item(rl.count()-2));
	rl.remove(rl.count()-3);
    }

    int final = rl.count();
    rl.append(rl.item(final-1));
    rl.item(final).x -= 1;
    rl.item(final).y += rl.item(final).height;
    rl.item(final).width += 1;
    rl.item(final).height = h - rl.item(final).height + 2;

    XShapeCombineRectangles(display(), m_parent, ShapeBounding,
			    0, 0, rl.xrectangles(), rl.count(),
			    visible ? ShapeUnion : ShapeSubtract, YXSorted);
    rl.remove_all();

    // Clip rectangles

    rl.append(m_tabWidth[screen()] + w + 1, 1, FRAME_WIDTH, FRAME_WIDTH - 1);

    rl.append(m_tabWidth[screen()] + 2, TAB_TOP_HEIGHT + 2, w,
	      FRAME_WIDTH - TAB_TOP_HEIGHT - 2);

    // for button
    ww = m_tabWidth[screen()] - TAB_TOP_HEIGHT*2 - 6;
    rl.append((m_tabWidth[screen()] + 2 - ww) / 2, (m_tabWidth[screen()]+2 - ww) / 2, ww, ww);

    rl.append(m_tabWidth[screen()] + 2, FRAME_WIDTH,
	      FRAME_WIDTH - 2, h - FRAME_WIDTH);

    // swap last two if sorted wrong
    if (rl.item(rl.count()-2).y > rl.item(rl.count()-1).y) {
	rl.append(rl.item(rl.count()-2));
	rl.remove(rl.count()-3);
    }

    rl.append(m_tabWidth[screen()] + 2, h, FRAME_WIDTH - 2, FRAME_WIDTH + 1);

    XShapeCombineRectangles(display(), m_parent, ShapeClip,
			    0, 0, rl.xrectangles(), rl.count(),
			    visible ? ShapeUnion : ShapeSubtract, YXSorted);
    rl.remove_all();

    if (visible && !isFixedSize()) {
	XMapRaised(display(), m_resize);
    } else {
	XUnmapWindow(display(), m_resize);
    }
}


void Border::configure(int x, int y, int w, int h,
		       unsigned long mask, int detail,
		       Boolean force) // must reshape everything
{
    if (!m_parent || m_parent == root()) {

	// create windows, then shape them afterwards

	m_parent = XCreateSimpleWindow
	    (display(), root(), 1, 1, 1, 1, 0,
	     m_borderPixel[screen()], m_frameBackgroundPixel[screen()]);

	m_tab = XCreateSimpleWindow
	    (display(), m_parent, 1, 1, 1, 1, 0,
	     m_borderPixel[screen()], m_backgroundPixel[screen()]);

	m_button = XCreateSimpleWindow
	    (display(), m_parent, 1, 1, 1, 1, 0,
	     m_borderPixel[screen()], m_buttonBackgroundPixel[screen()]);

	m_resize = XCreateWindow
	    (display(), m_parent, 1, 1, FRAME_WIDTH*2, FRAME_WIDTH*2, 0,
	     CopyFromParent, InputOutput, CopyFromParent, 0L, 0);

//#if CONFIG_MAD_FEEDBACK != 0
	if (CONFIG_MAD_FEEDBACK) {
	    m_feedback = XCreateSimpleWindow
		(display(), root(), 0, 0, 1, 1, 1,
		 m_borderPixel[screen()], m_backgroundPixel[screen()]);
	}
//#endif

	shapeResize();

	XSelectInput(display(), m_parent,
		     SubstructureRedirectMask | SubstructureNotifyMask |
		     ButtonPressMask | ButtonReleaseMask);

	if (!isTransient()) {
	    XSelectInput(display(), m_tab,
			 ExposureMask | ButtonPressMask | ButtonReleaseMask |
			 EnterWindowMask);
	}

	XSelectInput(display(), m_button, ButtonPressMask | ButtonReleaseMask);
	XSelectInput(display(), m_resize, ButtonPressMask | ButtonReleaseMask);
	mask |= CWX | CWY | CWWidth | CWHeight | CWBorderWidth;

	XSetWindowAttributes wa;
	wa.background_pixmap = m_backgroundPixmap;

	if (m_backgroundPixmap) {
	    XChangeWindowAttributes(display(), m_parent, CWBackPixmap, &wa);
	    XChangeWindowAttributes(display(), m_tab,    CWBackPixmap, &wa);
	    XChangeWindowAttributes(display(), m_button, CWBackPixmap, &wa);
	}

	if (CONFIG_MAD_FEEDBACK) {
	    wa.save_under =
		(DoesSaveUnders(ScreenOfDisplay(display(), 0)) ? True : False);
	    if (m_backgroundPixmap) {
		XChangeWindowAttributes
		    (display(), m_feedback, CWSaveUnder | CWBackPixmap, &wa);
	    } else {
		XChangeWindowAttributes(display(), m_feedback, CWSaveUnder, &wa);
	    }
	}
    }

    XWindowChanges wc;
    wc.x = x - xIndent();
    wc.y = y - yIndent();
    wc.width  = w + xIndent() + 1;
    wc.height = h + yIndent() + 1;
    wc.border_width = 0;
    wc.sibling = None;
    wc.stack_mode = detail;
    XConfigureWindow(display(), m_parent, mask, &wc);

    unsigned long rmask = 0L;
    if (mask & CWWidth)  rmask |= CWX;
    if (mask & CWHeight) rmask |= CWY;
    wc.x = w - FRAME_WIDTH*2 + xIndent();
    wc.y = h - FRAME_WIDTH*2 + yIndent();
    XConfigureWindow(display(), m_resize, rmask, &wc);
    
    if (force ||
	(m_prevW < 0 || m_prevH < 0) ||
	((mask & (CWWidth | CWHeight)) && (w != m_prevW || h != m_prevH))) {

	int prevTabHeight = m_tabHeight;
	if (isTransient()) m_tabHeight = 10; // arbitrary
	else fixTabHeight(h);

	shapeParent(w, h);
	setFrameVisibility(m_client->isActive(), w, h);

	if (force || w != m_prevW ||
	    prevTabHeight != m_tabHeight || m_prevW < 0 || m_prevH < 0) {

	    wc.x = 0;
	    wc.y = 0;
	    wc.width = w + xIndent();
	    wc.height = m_tabHeight + 2 + m_tabWidth[screen()];
	    XConfigureWindow(display(), m_tab, mask, &wc);
	    shapeTab(w, h);
	}

	m_prevW = w;
	m_prevH = h;

    } else {

	resizeTab(h);
    }

    wc.x = TAB_TOP_HEIGHT + 2;
    wc.y = wc.x;
    wc.width = wc.height = m_tabWidth[screen()] - TAB_TOP_HEIGHT*2 - 4;
    XConfigureWindow(display(), m_button, mask, &wc);
}


void Border::moveTo(int x, int y)
{
    XWindowChanges wc;
    wc.x = x - xIndent();
    wc.y = y - yIndent();
    XConfigureWindow(display(), m_parent, CWX | CWY, &wc);
}


void Border::map()
{
    if (m_parent == root()) {
	fprintf(stderr, "wmx: bad parent in Border::map()\n");
    } else {
	XMapWindow(display(), m_parent);

	if (!isTransient()) {
	    XMapWindow(display(), m_tab);
	    XMapWindow(display(), m_button);
	}

	if (!isFixedSize()) XMapWindow(display(), m_resize);
    }
}


void Border::mapRaised()
{
    if (m_parent == root()) {
	fprintf(stderr, "wmx: bad parent in Border::mapRaised()\n");
    } else {
	XMapRaised(display(), m_parent);

	if (!isTransient()) {
	    XMapWindow(display(), m_tab);
	    XMapRaised(display(), m_button);
	}

	if (!isFixedSize()) XMapRaised(display(), m_resize);
    }
}


void Border::lower()
{
    XLowerWindow(display(), m_parent);
}


void Border::unmap()
{
    if (m_parent == root()) {
	fprintf(stderr, "wmx: bad parent in Border::unmap()\n");
    } else {
	XUnmapWindow(display(), m_parent);

	if (!isTransient()) {
	    XUnmapWindow(display(), m_tab);
	    XUnmapWindow(display(), m_button);
//	    XUnmapWindow(display(), m_resize); // no, will unmap with parent
	}
    }
}


void Border::decorate(Boolean active, int w, int h)
{
    setFrameVisibility(active, w, h);
}


void Border::reparent()
{
    XReparentWindow(display(), m_child, m_parent, xIndent(), yIndent());
}


void Border::toggleFeedback(int x, int y, int w, int h)
{
    m_fedback = !m_fedback;
    if (!m_feedback || !CONFIG_MAD_FEEDBACK) return;

    if (m_fedback) {

	w += CONFIG_FRAME_THICKNESS + 1;
	h += CONFIG_FRAME_THICKNESS - TAB_TOP_HEIGHT + 1;

	XMoveResizeWindow(display(), m_feedback,
			  x - CONFIG_FRAME_THICKNESS - 1,
			  y - CONFIG_FRAME_THICKNESS + TAB_TOP_HEIGHT - 1,
			  w, h);

	XRectangle r[2];

	r[0].x = 0; r[0].y = 0; r[0].width = w;
	r[0].height = CONFIG_FRAME_THICKNESS - 2;
	r[1].x = 0; r[1].y = r[0].height; r[1].width = r[0].height + 2;
	r[1].height = h - r[0].height;

	XShapeCombineRectangles(display(), m_feedback, ShapeBounding,
				0, 0, r, 2, ShapeSet, YXBanded);

	r[0].x++; r[0].y++; r[0].width -= 2; r[0].height -= 2;
	r[1].x++; r[1].y--; r[1].width -= 2;

	XShapeCombineRectangles(display(), m_feedback, ShapeClip, 0, 0, r, 2,
				ShapeSet, YXBanded);

/* I just can't decide!

	r[0].x = w - 1; r[0].y = 0; r[0].width = 1; r[0].height = h - 1;
	r[1].x = 0; r[1].y = h - 1; r[1].width = w - 1; r[1].height = 1;

	XShapeCombineRectangles(display(), m_feedback, ShapeBounding,
				0, 0, r, 2, ShapeUnion, YXBanded);
				*/
	XMapRaised(display(), m_feedback);

    } else {
	XUnmapWindow(display(), m_feedback);
    }
}

void Border::showFeedback(int x, int y, int w, int h)
{
    if (!m_fedback) toggleFeedback(x, y, w, h);
}

void Border::removeFeedback()
{
    if (m_fedback) toggleFeedback(0, 0, 0, 0);
}



