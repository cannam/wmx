/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#include "Manager.h"

// Thanks to icewm, by Marko Macek.

#if CONFIG_USE_SESSION_MANAGER != False

void WindowManager::smWatchFD(IceConn c, IcePointer wmp,
			      Bool opening, IcePointer *)
{
    WindowManager *wm = (WindowManager *)wmp;

    if (opening) {
        if (wm->m_smFD != -1) { // shouldn't happen
            fprintf(stderr, "wmx: too many ICE connections, ignoring\n");
        } else {
            wm->m_smFD = IceConnectionNumber(c);
        }
    } else {
        if (IceConnectionNumber(c) == wm->m_smFD) {
            fprintf(stderr, "wmx: ICE connection closing\n");
            wm->m_smFD = -1;
	}
    }
}

void WindowManager::smSaveYourself(SmcConn c, SmPointer, int, Bool, int, Bool)
{
    SmcSaveYourselfDone(c, True);
}

void WindowManager::smShutdownCancelled(SmcConn c, SmPointer)
{
    SmcSaveYourselfDone(c, True);
}

void WindowManager::smSaveComplete(SmcConn c, SmPointer)
{
}

void WindowManager::smDie(SmcConn c, SmPointer wmp)
{
    WindowManager *wm = (WindowManager *)wmp;

    SmcCloseConnection(c, 0, NULL);
    if (c == wm->m_smConnection) {
	wm->m_smConnection = NULL;
	wm->m_smIceConnection = NULL;
    }
}

void WindowManager::initialiseSession(char *sessionProg, char *oldSessionId)
{
    if (getenv("SESSION_MANAGER") == 0) {
	fprintf(stderr, "wmx: no SESSION_MANAGER in environment, ignoring\n");
        return;
    }

    if (IceAddConnectionWatch(&smWatchFD, (IcePointer)this) == 0) {
        fprintf(stderr, "wmx: IceAddConnectionWatch failed\n");
        return;
    }

    fprintf(stderr, "wmx: initialising session manager: prog is \"%s\"\n",
            sessionProg ? sessionProg : "(none)");
	
    if (sessionProg) m_sessionProgram = NewString(sessionProg);
    else m_sessionProgram = NewString("wmx");

    if (oldSessionId) m_oldSessionId = NewString(oldSessionId);
    else m_oldSessionId = NULL;

    char errStr[256];
    SmcCallbacks smcall;

    memset(&smcall, 0, sizeof(smcall));
    smcall.save_yourself.callback = &smSaveYourself;
    smcall.save_yourself.client_data = (SmPointer)this;
    smcall.die.callback = &smDie;
    smcall.die.client_data = (SmPointer)this;
    smcall.save_complete.callback = &smSaveComplete;
    smcall.save_complete.client_data = (SmPointer)this;
    smcall.shutdown_cancelled.callback = &smShutdownCancelled;
    smcall.shutdown_cancelled.client_data = (SmPointer)this;
    
    if ((m_smConnection = SmcOpenConnection(NULL, /* network ids */
					    NULL, /* context */
					    1, 0, /* protocol major, minor */
					    SmcSaveYourselfProcMask |
					    SmcSaveCompleteProcMask |
					    SmcShutdownCancelledProcMask |
					    SmcDieProcMask,
					    &smcall,
					    oldSessionId, &m_newSessionId,
					    sizeof(errStr), errStr)) == NULL) {
        fprintf(stderr, "wmx: session manager init error: %s\n", errStr);
        return;
    }
    m_smIceConnection = SmcGetIceConnection(m_smConnection);

    setSessionProperties();
}

void WindowManager::setSessionProperties()
{
    SmPropValue programVal = { 0, NULL };
    SmPropValue userIDVal  = { 0, NULL };
    SmPropValue restartVal[3] = { { 0, NULL }, { 0, NULL }, { 0, NULL } };

    SmProp programProp =
    { (char *)SmProgram, (char *)SmLISTofARRAY8, 1, &programVal };
    SmProp userIDProp =
    { (char *)SmUserID, (char *)SmARRAY8, 1, &userIDVal };
    SmProp restartProp =
    { (char *)SmRestartCommand, (char *)SmLISTofARRAY8, 3,
      (SmPropValue *)&restartVal };
    SmProp cloneProp =
    { (char *)SmCloneCommand, (char *)SmLISTofARRAY8, 1,
      (SmPropValue *)&restartVal };

    SmProp *props[] = {
        &programProp,
        &userIDProp,
        &restartProp,
        &cloneProp
    };

    char *user = getenv("USER");
    const char *clientId = "--sm-client-id";

    programVal.length = strlen(m_sessionProgram);
    programVal.value = m_sessionProgram;

    userIDVal.length = user ? 7 : strlen(user);
    userIDVal.value = user ? (SmPointer)"unknown" : (SmPointer)user;

    restartVal[0].length = strlen(m_sessionProgram);
    restartVal[0].value = m_sessionProgram;
    restartVal[1].length = strlen(clientId);
    restartVal[1].value = (char *)clientId;
    restartVal[2].length = strlen(m_newSessionId);
    restartVal[2].value = m_newSessionId;

    SmcSetProperties(m_smConnection,
                     sizeof(props)/sizeof(props[0]),
                     (SmProp **)&props);
}

#endif

