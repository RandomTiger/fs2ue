/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef _OSAPI_H
#define _OSAPI_H

#ifndef FS2_UE
#include "TChar.h"
#endif

#ifndef UNITY_BUILD
#include "GlobalIncs/PsTypes.h"
#endif


// --------------------------------------------------------------------------------------------------
// OSAPI DEFINES/VARS
//

// set if running under MsDev - done after os_init(...) has returned
extern int Os_debugger_running;

// game-wide
// #define THREADED

#ifdef THREADED
	#define ENTER_CRITICAL_SECTION(csc)		do { EnterCriticalSection(csc); } while(0);
	#define LEAVE_CRITICAL_SECTION(csc)		do { LeaveCriticalSection(csc); } while(0);
#else
	#define ENTER_CRITICAL_SECTION(csc)		do { } while(0);
	#define LEAVE_CRITICAL_SECTION(csc)		do { } while(0);
#endif

// --------------------------------------------------------------------------------------------------
// OSAPI FUNCTIONS
//

// initialization/shutdown functions -----------------------------------------------

// If app_name is NULL or ommited, then TITLE is used
// for the app name, which is where registry keys are stored.
void os_init(char * wclass, char * title, char *app_name=NULL, char *version_string=NULL );

// set the main window title
void os_set_title( char * title );

// call at program end
void os_cleanup();


// window management ---------------------------------------------------------------

// toggle window size between full screen and windowed
void os_toggle_fullscreen();

// Returns 1 if app is not the foreground app.
int os_foreground();

// Returns the handle to the main window
void *os_get_window();

enum
{
	OS_PRE_WIN7,
	OS_WIN_95,
	OS_WIN_98,
	OS_WIN_2000,
	OS_WIN_XP,
	OS_WIN_2003_SERVER,
	OS_WIN_VISTA,
	OS_WIN_WIN7,
	OS_UNKNOWN,
	OS_POST_WIN7,
};

uint os_get_version();

// process management --------------------------------------------------------------

// call to process windows messages. only does something in non THREADED mode
void os_poll();

// Sleeps for n milliseconds or until app becomes active.
void os_sleep(int ms);

// Used to stop message processing
void os_suspend();

// resume message processing
void os_resume();

int os_get_voice_recognition_event_id();

void osapi_GetCurrentDirectory(TCHAR *CurrentDir);
void osapi_SetCurrentDirectory(TCHAR *NewDir);

#endif
