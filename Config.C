
#include "Config.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

// 32 is enough to fit even "light goldenrod yellow" comfortably
#define COLOR_LEN 32

DynamicConfig DynamicConfig::config;

struct DynamicConfigImpl
{
    char options[1000];	// Old options-string
    char path[1000];	// Path to Options-Link
    char focus; 	// 1 = Click , 2 = Raise, 4 = Autoraise
    int  raisedelay;
    char kbd;		// 1 = Keyboard on
    char menu;		// 0 = no unmapped, 1 = everything
    char feedback;	// 0 = no , 1 = yes
    int  feeddelay;
    char disable;	// 0 = New Window option, 1 = no New
    char rightBt;	// 0 = disable, 1 = circulate, 2 = lower, 4 = height
    char tabfg[COLOR_LEN];     // black
    char tabbg[COLOR_LEN];     // gray80
    char framebg[COLOR_LEN];   // gray95
};  

DynamicConfig::DynamicConfig() : m_impl(new DynamicConfigImpl)
{
    strcpy(m_impl->options, "");
    
    char *home = getenv("HOME");
    char *wmxdir = getenv("WMXDIR");
    
    if (!wmxdir) {

	if (!home) m_impl->path[0] = 0;
	else {
	    strcpy(m_impl->path, getenv("HOME"));  // Path to Options-Link
	    strcat(m_impl->path, "/" CONFIG_COMMAND_MENU "/options");
	}

    } else {

	if (*wmxdir == '/') strcpy(m_impl->path, wmxdir);
	else {
	    strcpy(m_impl->path, home);
	    strcat(m_impl->path, "/");
	    strcat(m_impl->path, wmxdir);
	}
	strcat(m_impl->path, "/options");
    }

    m_impl->focus = 0;	// 1 = Click , 2 = Raise, 4 = Autoraise
    m_impl->raisedelay = 400;
    m_impl->kbd = 1;		// 1 = Keyboard on
    m_impl->menu = 1;	// 0 = no unmapped, 1 = everything
    m_impl->feedback = 1;	// 0 = no , 1 = yes
    m_impl->feeddelay = 300;
    m_impl->disable = 0;        // 0 = allow New window, 1 = don't
    m_impl->rightBt = 1;	// 0 = disable, 1 = circulate, 2 = lower
    strcpy(m_impl->tabfg, "black");
    strcpy(m_impl->tabbg, "gray80");
    strcpy(m_impl->framebg, "gray95");

    scan(1);
}

DynamicConfig::~DynamicConfig()
{
    delete m_impl;
}

char DynamicConfig::clickFocus() { return m_impl->focus & 1; }
char DynamicConfig::raiseFocus() { return m_impl->focus & 2; }
char DynamicConfig::autoRaiseFocus() { return m_impl->focus & 4; }
int  DynamicConfig::raiseDelay() { return m_impl->raisedelay; }
char DynamicConfig::useKeyboard() { return m_impl->kbd & 1; }
char DynamicConfig::fullMenu() { return m_impl->menu & 1; }
char DynamicConfig::useFeedback() { return m_impl->feedback & 1; }
int  DynamicConfig::feedbackDelay() { return m_impl->feeddelay; }
char DynamicConfig::disableNew() { return m_impl->disable & 1; }
char DynamicConfig::rightCirculate() { return m_impl->rightBt & 1; }
char DynamicConfig::rightLower() { return m_impl->rightBt & 2; }
char DynamicConfig::rightToggleHeight() { return m_impl->rightBt & 4; }
char *DynamicConfig::tabForeground() { return m_impl->tabfg; }
char *DynamicConfig::tabBackground() { return m_impl->tabbg; }
char *DynamicConfig::frameBackground() { return m_impl->framebg; }

void DynamicConfig::scan(char startup)
{
    char temp[1000];
    memset(temp, 0, 1000);

    if (m_impl->path[0] && readlink(m_impl->path, temp, 999) > 0) {
	if (strcmp(temp, m_impl->options) != 0) { // Did it change ?
	    strcpy(m_impl->options, temp);
	    update(temp);
	}
    } else if (startup) {
	fprintf(stderr, "\nwmx: No dynamic configuration found\n");
    }
}


void DynamicConfig::update(char *string)
{
    char *s;

#define OPTION(x) ( (!strncasecmp(s, x, strlen(x))) && (s += strlen(x)) )

    fprintf(stderr, "\nwmx: Reading dynamic configuration... ");

    s = strtok(string, "/");
    do {
	fprintf(stderr, ">%s< ",s);

	if (OPTION("menu:"))
	    if (OPTION("full")) m_impl->menu = 1;
	    else if (OPTION("part")) m_impl->menu = 0;

	if (OPTION("new:"))
	    if (OPTION("on")) m_impl->disable = 0;
	    else if (OPTION("off")) m_impl->disable = 1;
	
	if (OPTION("keyboard:"))
	    if (OPTION("on")) m_impl->kbd = 1;
	    else if (OPTION("off")) m_impl->kbd = 0;

	if (OPTION("feedback:"))
	    if (OPTION("on")) {
		m_impl->feedback = 1;
		if (OPTION(",")) m_impl->feeddelay = strtol(s, &s, 10);
	    } else if (OPTION("off")) m_impl->feedback = 0;

	if (OPTION("focus:"))
	    if (OPTION("click")) m_impl->focus = 3;
	    else if (OPTION("raise")) m_impl->focus = 2;
	    else if (OPTION("delay-raise")) {
		m_impl->focus = 4;
		if (OPTION(",")) m_impl->raisedelay = strtol(s, &s, 10);
	    } else if (OPTION("follow")) m_impl->focus = 0;

	if (OPTION("right:"))
	    if (OPTION("off")) m_impl->rightBt = 0;
	    else if (OPTION("circulate")) m_impl->rightBt = 1;
	    else if (OPTION("lower")) m_impl->rightBt = 2;
	    else if (OPTION("toggleheight")) m_impl->rightBt = 4;
	
	if (OPTION("tabfg:")) {
	    strncpy(m_impl->tabfg, s, COLOR_LEN);
	    m_impl->tabfg[COLOR_LEN-1] = '\0';	// prevent unterminated string
	    s += strlen(m_impl->tabfg);		// avoid error message below
	}
	if (OPTION("tabbg:")) {
	    strncpy(m_impl->tabbg, s, COLOR_LEN);
	    m_impl->tabbg[COLOR_LEN-1] = '\0';
	    s += strlen(m_impl->tabbg);
	}
	if (OPTION("framebg:")) {
	    strncpy(m_impl->framebg, s, COLOR_LEN);
	    m_impl->framebg[COLOR_LEN-1] = '\0';
	    s += strlen(m_impl->framebg);
	}

	if (*s != '\0') {
	    fprintf(stderr, "\nwmx: Dynamic configuration error: "
		    "`%s' @ position %d", s, string - s);
	}

    } while ((s = strtok(NULL, "/")));

    fprintf(stderr, "\n");

#undef OPTION
}

