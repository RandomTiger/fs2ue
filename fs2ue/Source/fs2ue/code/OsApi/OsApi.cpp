/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#ifndef UNITY_BUILD
#include "OsApi.h"

#include <windows.h>
//#include <windowsx.h>
//#include <commctrl.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
//#include <winsock.h>
//#include <stdarg.h>
//#include <direct.h>

//#include "AntTweakBar.h"

#include "key.h"
#include "palman.h"
#include "MouseController.h"
#include "OutWnd.h"
#include "2d.h"
#include "cfile.h"
#include "sound.h"
#include "FreespaceResource.h"
#include "ManagePilot.h"
#include "joy.h"
#include "ForceFeedback.h"
#include "gamesequence.h"
#include "freespace.h"
#include "OsRegistry.h"
#include "cmdline.h"

#include "VoiceRecognition.h"
#endif

#if defined(FS2_UE)
#include "FS2UETestLib.h"
#else
#include "FreespaceResource.h"
#endif

// ----------------------------------------------------------------------------------------------------
// OSAPI DEFINES/VARS
//

const int WM_RECOEVENT = WM_USER+190;             // Arbitrary user defined message for reco callback

// os-wide globals
static HINSTANCE	hInstApp;
static HWND			hwndApp = NULL;
static int			fAppActive = 0;
static int			main_window_inited = 0;
static char			szWinTitle[128];
static char			szWinClass[128];
static HANDLE		hThread=NULL;
static DWORD		ThreadID;
static int			WinX, WinY, WinW, WinH;
static int			Os_inited = 0;

static CRITICAL_SECTION Os_lock;

int Os_debugger_running = 0;

// ----------------------------------------------------------------------------------------------------
// OSAPI FORWARD DECLARATIONS
//

#ifdef THREADED
	// thread handler for the main message thread
	DWORD win32_process(DWORD lparam);
#else
	DWORD win32_process1(DWORD lparam);
	DWORD win32_process1(DWORD lparam);
#endif

// Fills in the Os_debugger_running with non-zero if debugger detected.
void os_check_debugger();

// called at shutdown. Makes sure all thread processing terminates.
void os_deinit();

// go through all windows and try and find the one that matches the search string
BOOL __stdcall os_enum_windows( HWND hwnd, char * search_string );

// message handler for the main thread
LRESULT CALLBACK win32_message_handler(HWND hwnd,UINT msg,WPARAM wParam, LPARAM lParam);

// create the main window
BOOL win32_create_window();


// ----------------------------------------------------------------------------------------------------
// OSAPI FUNCTIONS
//

// initialization/shutdown functions -----------------------------------------------

// If app_name is NULL or ommited, then TITLE is used
// for the app name, which is where registry keys are stored.
void os_init(char * wclass, char * title, char *app_name, char *version_string )
{
	os_init_registry_stuff(Osreg_company_name, title, version_string);

	strcpy( szWinTitle, title );
	strcpy( szWinClass, wclass );	

	InitializeCriticalSection( &Os_lock );

	#ifdef THREADED
		// Create an even to signal that the window is created, 
		// so that we don't return from this function until 
		// the window is all properly created.
		HANDLE Window_created = CreateEvent( NULL, FALSE, FALSE, NULL );
		hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)win32_process, Window_created, 0, &ThreadID );
		if ( WaitForSingleObject( Window_created, 5000 )==WAIT_TIMEOUT)	{			//INFINITE );
			mprintf(( "Wait timeout!\n" ));
		}
		CloseHandle(Window_created);
		Window_created = NULL;
	#else
		win32_process1(0);
	#endif // THREADED

	// initialized
	Os_inited = 1;

	// check to see if we're running under msdev
	os_check_debugger();

	atexit(os_deinit);
}

// set the main window title
void os_set_title( char * title )
{
	strcpy( szWinTitle, title );
	SetWindowTextA( hwndApp, szWinTitle );
}

// call at program end
void os_cleanup()
{
	// Tell the app to quit
	PostMessage( hwndApp, WM_DESTROY, 0, 0 );
	
	#ifndef NDEBUG
		outwnd_close();
	#endif
}


// window management -----------------------------------------------------------------

// Returns 1 if app is not the foreground app.
int os_foreground()
{
	return fAppActive;
}

// Returns the handle to the main window
void *os_get_window()
{
#ifdef FS2_UE
	return UFS2UETestLib::GetWindowHandle();
#else
	return hwndApp;
#endif
}

uint os_get_version()
{
	OSVERSIONINFO osInfo;
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&osInfo);

//	if(osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT2)
	{
		if(osInfo.dwMajorVersion < 5)
		{
			return OS_WIN_2000;
		}

		if(osInfo.dwMajorVersion == 5)
		{
			switch(osInfo.dwMinorVersion)
			{
			case 0: return OS_WIN_2000;
			case 1: return OS_WIN_XP;
			case 2: return OS_WIN_2003_SERVER;
			}
		}

		if(osInfo.dwMajorVersion == 6)
		{
			switch(osInfo.dwMinorVersion)
			{
			case 0: return OS_WIN_VISTA;
			case 1: return OS_WIN_WIN7;
			default: return OS_POST_WIN7;
			}
		}

		if(osInfo.dwMajorVersion > 6)
		{
			return OS_POST_WIN7;
		}
	}

	return OS_UNKNOWN;
}


// process management -----------------------------------------------------------------

// Sleeps for n milliseconds or until app becomes active.
void os_sleep(int ms)
{
#if !defined(FS2_UE)
	Sleep(ms);
#endif
}

// Used to stop message processing
void os_suspend()
{
	ENTER_CRITICAL_SECTION(&Os_lock);	
}

// resume message processing
void os_resume()
{
	LEAVE_CRITICAL_SECTION(&Os_lock);	
}


// ----------------------------------------------------------------------------------------------------
// OSAPI FORWARD DECLARATIONS
//

#ifdef THREADED

// thread handler for the main message thread
DWORD win32_process(DWORD lparam)
{
	MSG msg;
	HANDLE Window_created = (HANDLE)lparam;

	if ( !win32_create_window() )
		return 0;

	// Let the app continue once the window is created
	SetEvent(Window_created);

	while (1)	{	
		if (WaitMessage() == TRUE)	{
			EnterCriticalSection(&Os_lock);			
			while(PeekMessage(&msg,0,0,0,PM_REMOVE))	{
				if ( msg.message == WM_DESTROY )	{
					LeaveCriticalSection(&Os_lock);

					// cleanup and exit this thread!!
					DeleteCriticalSection(&Os_lock);
					return 0;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			LeaveCriticalSection(&Os_lock);
		} 
	}
	return 0;
}

#else
DWORD win32_process1(DWORD lparam)
{
	if ( !win32_create_window() )
		return 0;

	return 0;
}


DWORD win32_process2(DWORD lparam)
{
	MSG msg;

	while(PeekMessage(&msg,0,0,0,PM_REMOVE))	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
#endif // THREADED

// Fills in the Os_debugger_running with non-zero if debugger detected.
void os_check_debugger()
{
#if !defined(_WIN64)

	HMODULE hMod;
	char search_string[256];
	char myname[128];
	int namelen;
	char * p;

	Os_debugger_running = 0;		// Assume its not

	// Find my EXE file name
	hMod = GetModuleHandle(NULL);
	if ( !hMod ) return;
	namelen = GetModuleFileName( hMod, myname, 127 );	
	if ( namelen < 1 ) return;
	
	// Strip off the .EXE
	p = strstr( myname, ".exe" );
	if (!p) return;
	*p = '\0';

	// Move p to point to first letter of EXE filename
	while( (*p!='\\') && (*p!='/') && (*p!=':') )
		p--;
	p++;	
	if ( strlen(p) < 1 ) return;

	// Build what the debugger's window title would be if the debugger is running...
	sprintf( search_string, "[run] - %s -", p );

	// ... and then search for it.
	EnumWindows( (int (__stdcall *)(struct HWND__ *,long))os_enum_windows, (long)&search_string );
#endif
}

// called at shutdown. Makes sure all thread processing terminates.
void os_deinit()
{
	if (hThread)	{
		CloseHandle(hThread);
		hThread = NULL;
	}
}

// go through all windows and try and find the one that matches the search string
BOOL __stdcall os_enum_windows( HWND hwnd, char * search_string )
{
	char tmp[128];
	int len;

	len = GetWindowTextA( hwnd, tmp, 127 );
	 
	if ( len )	{
		if ( strstr( tmp, search_string ))	{
			Os_debugger_running = 1;		// found the search string!
			return FALSE;	// stop enumerating windows
		}
	}

	return TRUE;	// continue enumeration
}


int Got_message = 0;
// message handler for the main thread
LRESULT CALLBACK win32_message_handler(HWND hwnd,UINT msg,WPARAM wParam, LPARAM lParam)
{
	// Got_message++;

	switch(msg)	{

	case WM_QUERYNEWPALETTE:
		// mprintf(( "WM: QueryNewPalette\n" ));
		return TRUE;	// Say that I've realized my own palette
		break;
	case WM_PALETTECHANGED:
		// mprintf(( "WM: PaletteChanged\n" ));
		break;
	case WM_PALETTEISCHANGING:
		// mprintf(( "WM: PaletteIsChanging\n" ));
		break;

	case WM_DISPLAYCHANGE:
		// mprintf(( "WM: DisplayChange\n" ));
		break;

	case WM_LBUTTONDOWN:
		g_MouseController.MarkButton( MouseController::MOUSE_LEFT_BUTTON, 1 );
		break;

	case WM_LBUTTONUP:
		g_MouseController.MarkButton( MouseController::MOUSE_LEFT_BUTTON, 0 );
		break;

	case WM_RBUTTONDOWN:
		g_MouseController.MarkButton( MouseController::MOUSE_RIGHT_BUTTON, 1 );
		break;

	case WM_RBUTTONUP:
		g_MouseController.MarkButton( MouseController::MOUSE_RIGHT_BUTTON, 0 );
		break;

	case WM_MBUTTONDOWN:
		g_MouseController.MarkButton( MouseController::MOUSE_MIDDLE_BUTTON, 1 );
		break;

	case WM_MBUTTONUP:
		g_MouseController.MarkButton( MouseController::MOUSE_MIDDLE_BUTTON, 0 );
		break;

	case WM_TIMER:
		break;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:	{		
			int nVirtKey;
			uint lKeyData;

			int latency;
			latency = timeGetTime() - GetMessageTime();
			if ( latency < 0 )
				latency=0;

			nVirtKey = (int)wParam;    // virtual-key code 
			lKeyData = (lParam>>16) & 255;          // key data 
			if ( (lParam>>16) & 256 ) lKeyData += 0x80;

			// Fix up print screen, whose OEM code is wrong under 95.
			if ( nVirtKey == VK_SNAPSHOT )	{
				lKeyData = KEY_PRINT_SCRN;	
			}

			if (lKeyData == KEY_RSHIFT)  // either shift is just a shift to us..
				lKeyData = KEY_LSHIFT;

			if (lKeyData == KEY_RALT)  // Same with alt keys..
				lKeyData = KEY_LALT;

//			mprintf(( "Key down = 0x%x|%x\n", lKeyData, nVirtKey ));
			key_mark( lKeyData, 1, latency );
//			mprintf(( "Key down = 0x%x\n", lKeyData ));
			//Warning( LOCATION, "Key = 0x%x", lKeyData );			
		}
		break;

	case WM_SYSKEYUP:
	case WM_KEYUP:	 {		
			int nVirtKey;
			uint lKeyData;

			int latency;
			latency = timeGetTime() - GetMessageTime();
			if ( latency < 0 )
				latency=0;

			nVirtKey = (int) wParam;    // virtual-key code 
			lKeyData = (lParam>>16) & 255;          // key data 
			if ( (lParam>>16) & 256 ) lKeyData += 0x80;

			// Fix up print screen, whose OEM code is wrong under 95.
			if ( nVirtKey == VK_SNAPSHOT )	{
				lKeyData = KEY_PRINT_SCRN;	
			}

			if (lKeyData == KEY_RSHIFT)  // either shift is just a shift to us..
				lKeyData = KEY_LSHIFT;

			if (lKeyData == KEY_RALT)  // Same with alt keys..
				lKeyData = KEY_LALT;

//			mprintf(( "Key up = 0x%x|%x\n", lKeyData, nVirtKey ));
			if ( lKeyData == 0xB7 )	{
				// Hack for PrintScreen which only sends one up message!
				key_mark( lKeyData, 1, latency );		
				key_mark( lKeyData, 0, latency );

			} else {
				key_mark( lKeyData, 0, latency );
			}			
		}		
		break;

	case WM_KILLFOCUS:
		key_lost_focus();
		gr_activate(0);
		break;

	case WM_SETFOCUS:
		key_got_focus();
		gr_activate(1);
		break;

	case WM_ACTIVATE:			//APP:	//
	case WM_ACTIVATEAPP:
		{
			BOOL OldfAppActive = fAppActive;
			
			// The application z-ordering has changed. If we are now the
			// foreground application wParm will be TRUE.
			if ( msg == WM_ACTIVATE )	{
				if ( (LOWORD(wParam) == WA_ACTIVE) || (LOWORD(wParam)==WA_CLICKACTIVE))	{
					fAppActive = TRUE;	//(BOOL)wParam;
				} else {
					fAppActive = FALSE;	//(BOOL)wParam;
				}

			} else {
				fAppActive = (BOOL)wParam;
			}

			//mprintf(( "Activate: %d\n", fAppActive ));

			if ( OldfAppActive != fAppActive )	{

				if ( fAppActive )	{
					// maximize it
					//	mprintf(( "Priority: HIGH\n" ));
					g_ForceFeedback.Reacquire();

#ifdef THREADED
					SetThreadPriority( hThread, THREAD_PRIORITY_HIGHEST );
#endif

					gr_activate(fAppActive);
					//SetThreadPriority( hThread, THREAD_PRIORITY_TIME_CRITICAL );
				} else {
					//mprintf(( "Priority: LOW\n" ));
					g_ForceFeedback.Unacquire();
					if (g_MouseController.GetMouseHidden())	{

					// Disable this mouse unhide, it was enabling the cursor during a window maximize
					// during loading unsafely as the bitmap was potentially being unloaded
					//	g_MouseController.SetMouseHidden(0);
						//ShowCursor(1);
						//mprintf(( "Mouse:Alive\n" ));		
					}
					// minimize it
					// key_outkey( KEY_PAUSE );

#ifdef THREADED
					SetThreadPriority( hThread, THREAD_PRIORITY_NORMAL );
#endif
					gr_activate(fAppActive);
				}
			}
		}
		break;

	case WM_DESTROY:
		// mprintf(( "WM_DESTROY called\n" ));
		PostQuitMessage(0);
		break;

	case WM_CLOSE:
		gameseq_post_event(GS_EVENT_QUIT_GAME);
		break;

	case WM_SYSCOMMAND:
		// mprintf(( "Sys command called '%x'\n", wParam ));
		 if ( wParam != SC_SCREENSAVE ){
			 return DefWindowProc(hwnd, msg, wParam, lParam);
		 }
		 break;

/*
	case MM_WIM_DATA:
		rtvoice_stream_data((uint)hwnd, (uint)wParam, (uint)lParam);
		break;
*/

	 case WM_RECOEVENT:
		if ( Game_mode & GM_IN_MISSION)// && Cmdline_voice_recognition)
		{
			g_VoiceRecognition.ProcessEvent( );
		}
        break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
		break;
	}

	return 0;
}

// create the main window
BOOL win32_create_window()
{
#if !defined(FS2_UE)
	int windowed = Cmdline_window;

	WNDCLASSEX wclass;
	HINSTANCE hInst = GetModuleHandle(NULL);

	wclass.hInstance 		= hInst;
	wclass.lpszClassName = szWinClass;
	wclass.lpfnWndProc	= (WNDPROC)win32_message_handler;		
	if(windowed){
		wclass.style			= CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
	} else {
		wclass.style			= CS_BYTEALIGNCLIENT|CS_VREDRAW | CS_HREDRAW;	// | CS_OWNDC;	//CS_DBLCLKS | CS_PARENTDC| CS_VREDRAW | CS_HREDRAW |;	
	}		
	wclass.cbSize			= sizeof(WNDCLASSEX);
	wclass.hIcon			= LoadIcon(hInst, MAKEINTRESOURCE(IDI_APP_ICON) );
	wclass.hIconSm			= NULL;	//LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1) );
	wclass.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wclass.lpszMenuName	= NULL;	//"FreeSpaceMenu";
	wclass.cbClsExtra		= 0;
	wclass.cbWndExtra		= 0;
	wclass.hbrBackground = (HBRUSH)NULL;

	if (!RegisterClassEx(&wclass)) return FALSE;
	
	int style;
	if(windowed){
		style = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	} else {
		style = WS_POPUP;
	}	

	// make sure we adjust for the actual window border	
	int x_add = GetSystemMetrics(SM_CXFIXEDFRAME) * 2;
	int y_add = GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION);

	// Make a 32x32 window.  It never should get shown, because the graphics code
	// will then size it.
	if(windowed){
		hwndApp = CreateWindowA( szWinClass, szWinTitle,
									style,   
									CW_USEDEFAULT,
									CW_USEDEFAULT,
									1024 + x_add,
									768  + y_add,
									NULL, (HMENU)NULL, hInst, (LPSTR)NULL);	
	} else {
		// Make a 32x32 window.  It never should get shown, because the graphics code
		// will then size it.
		hwndApp = CreateWindowA( szWinClass, szWinTitle,
									style,   
									(GetSystemMetrics( SM_CXSCREEN )-32 )/2,	//x
									(GetSystemMetrics( SM_CYSCREEN )-32 )/2,	//y
									32,32,									// wh
									NULL, (HMENU)NULL, hInst, (LPSTR)NULL);	
	}

	// Hack!! Turn off Window's cursor.
	if(Cmdline_anttweakbar == false)
	{
		ShowCursor(0);
	}
	ClipCursor(NULL);

	main_window_inited = 1;
	#ifndef NDEBUG
		outwnd_init(1);
	#endif	

	if(windowed){
		ShowWindow( hwndApp, SW_SHOWNORMAL );
		UpdateWindow( hwndApp );
	}
#endif
	return TRUE;
}

void os_poll()
{
#ifndef THREADED
	win32_process2(0);
#else
	MSG msg;
	EnterCriticalSection(&Os_lock);
	while(PeekMessage(&msg,0,0,0,PM_NOREMOVE))	{		
		if ( msg.message == WM_DESTROY )	{
			break;
		}
		if (PeekMessage(&msg,0,0,0,PM_REMOVE))	{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}		
		Got_message++;
	}
	LeaveCriticalSection(&Os_lock);
#endif
}

void debug_int3()
{
#if !defined(_WIN64)
	gr_activate(0);
	_asm { int 3 };
	gr_activate(1);
#endif
}


int os_get_voice_recognition_event_id() 
{
	return WM_RECOEVENT;
}
