
#include "Config.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

DynamicConfig DynamicConfig::dynamicConfig;

struct DynamicConfigImpl
{
    char options[1000];	// Old options-string
    char path[1000];	// Path to Options-Link
    char focus; 	// 1 = Click , 2 = Raise, 4 = Autoraise
    int  delay;
    char kbd;		// 1 = Keyboard on
    char menu;		// 0 = no unmapped, 1 = everything
    char feedback;	// 0 = no , 1 = yes
    char disable;	// 0 = New Window option, 1 = no New
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
    m_impl->delay = 400;
    m_impl->kbd = 1;		// 1 = Keyboard on
    m_impl->menu = 1;	// 0 = no unmapped, 1 = everything
    m_impl->feedback = 1;	// 0 = no , 1 = yes
    m_impl->disable = 0;        // 0 = allow New window, 1 = don't

    scan(1);
}

DynamicConfig::~DynamicConfig()
{
    delete m_impl;
}

char DynamicConfig::clickFocus() { return m_impl->focus & 1; }
char DynamicConfig::raiseFocus() { return m_impl->focus & 2; }
char DynamicConfig::autoRaiseFocus() { return m_impl->focus & 4; }
int  DynamicConfig::raiseDelay() { return m_impl->delay; }
char DynamicConfig::useKeyboard() { return m_impl->kbd & 1; }
char DynamicConfig::fullMenu() { return m_impl->menu & 1; }
char DynamicConfig::useFeedback() { return m_impl->feedback & 1; }
char DynamicConfig::disableNew() { return m_impl->disable & 1; }

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
	    if (OPTION("on")) m_impl->feedback = 1;
	    else if (OPTION("off")) m_impl->feedback = 0;

	if (OPTION("focus:"))
	    if (OPTION("click")) m_impl->focus = 3;
	    else if (OPTION("raise")) m_impl->focus = 2;
	    else if (OPTION("delay-raise")) {
		m_impl->focus = 2;
		if (OPTION(",")) m_impl->delay = strtol(s, &s, 10);
	    } else if (OPTION("follow")) m_impl->focus = 0;
	
	if (*s != '\0') {
	    fprintf(stderr, "\nwmx: Dynamic configuration error: "
		    "`%s' @ position %d", s, string - s);
	}

    } while (s = strtok(NULL, "/"));

    fprintf(stderr, "\n");

#undef OPTION
}

