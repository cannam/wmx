
#include "Manager.h"

// This is largely thieved from icewm, by Marko Macek.  (If you want a
// window manager that looks more conventional but works very smoothly
// and isn't too heavy on resources, icewm is the best I've seen.)

// Haven't actually got around to making this save anything worthwhile
// yet, of course...

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
            wm->m_smFD = -1;
	}
    }
}

void WindowManager::smSaveYourself(SmcConn c, SmPointer, int, Bool, int, Bool)
{
    SmcRequestSaveYourselfPhase2(c, &smSaveYourself2, NULL);
}

void WindowManager::smSaveYourself2(SmcConn c, SmPointer)
{
    //...

    // Here we'd like to save information about which client is on
    // which channel and at which coordinates -- that's easy enough,
    // it's applying the data when we restart that's the tricky bit.

    // To associate location data with particular clients, we need to
    // have retrieved the necessary session-management identifier
    // from each client (presumably on client initialisation); see
    // the R6 ICCCM for details of the relevant atoms

    // Do nothing, for now

    SmcSaveYourselfDone(c, True);
}

void WindowManager::smShutdownCancelled(SmcConn c, SmPointer)
{
    //...
}

void WindowManager::smSaveComplete(SmcConn c, SmPointer)
{
    //...
}

void WindowManager::smDie(SmcConn c, SmPointer wmp)
{
    WindowManager *wm = (WindowManager *)wmp;

    SmcCloseConnection(c, 0, NULL);
    if (c == wm->m_smConnection) {
	wm->m_smConnection = NULL;
	wm->m_smIceConnection = NULL; //???
    }
}

void WindowManager::initialiseSession(char *sessionProg, char *oldSessionId)
{
    // Largely thieved from icewm -- shout to the Marko Macek Massive!

    if (getenv("SESSION_MANAGER") == 0) {
	fprintf(stderr, "wmx: no SESSION_MANAGER in environment, ignoring\n");
        return;
    }

    if (IceAddConnectionWatch(&smWatchFD, (IcePointer)this) == 0) {
        fprintf(stderr, "wmx: IceAddConnectionWatch failed\n");
        return;
    }

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
    { (char *)SmCloneCommand, (char *)SmLISTofARRAY8, 2,
      (SmPropValue *)&restartVal };

    SmProp *props[] = {
        &programProp,
        &userIDProp,
        &restartProp,
        &cloneProp
    };

    char *user = getenv("USER");
    const char *clientId = "-clientId";

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

