/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#ifndef UNITY_BUILD
#include "Mouse.h"

#include <windows.h>
//#include <windowsx.h>

#include "MouseController.h"
#include "2d.h"
#include "OsApi.h"
#include "GameSequence.h"
#include "vdinput.h"
#endif

#define MOUSE_MODE_DI	0
#define MOUSE_MODE_WIN	1

#ifdef NDEBUG
static int Mouse_mode = MOUSE_MODE_DI;
#else
static int Mouse_mode = MOUSE_MODE_WIN;
#endif

static int mouse_inited = 0;
static int Di_mouse_inited = 0;
static int Mouse_x;
static int Mouse_y;

CRITICAL_SECTION mouse_lock;

// #define USE_DIRECTINPUT

int mouse_flags;
int mouse_left_pressed = 0;
int mouse_right_pressed = 0;
int mouse_middle_pressed = 0;
int mouse_left_up = 0;
int mouse_right_up = 0;
int mouse_middle_up = 0;
int Mouse_dx = 0;
int Mouse_dy = 0;
int Mouse_dz = 0;

int di_init();
void di_cleanup();
void mouse_force_pos(int x, int y);
void mouse_eval_deltas_di();

void mouse_close()
{
	if (!mouse_inited)
		return;

#ifdef USE_DIRECTINPUT
	di_cleanup();
#endif
	mouse_inited = 0;
	DeleteCriticalSection( &mouse_lock );
}

void mouse_init()
{
	// Initialize queue
	if ( mouse_inited ) return;
	mouse_inited = 1;

	InitializeCriticalSection( &mouse_lock );

	ENTER_CRITICAL_SECTION(&mouse_lock);

	mouse_flags = 0;
	Mouse_x = gr_screen.max_w / 2;
	Mouse_y = gr_screen.max_h / 2;

#ifdef USE_DIRECTINPUT
	if (!di_init())
		Mouse_mode = MOUSE_MODE_WIN;
#else
	Mouse_mode = MOUSE_MODE_WIN;
#endif

	LEAVE_CRITICAL_SECTION(&mouse_lock);	

	atexit( mouse_close );
}


// ----------------------------------------------------------------------------
// mouse_mark_button() is called asynchronously by the OS when a mouse button
// goes up or down.  The mouse button that is affected is passed via the 
// flags parameter.  
//
// parameters:   flags ==> mouse button pressed/released
//               set   ==> 1 - button is pressed
//                         0 - button is released

void mouse_mark_button( uint flags, int set)
{
	if ( !mouse_inited ) return;

	ENTER_CRITICAL_SECTION(&mouse_lock);

	if ( !(mouse_flags & MOUSE_LEFT_BUTTON) )	{

		if ( (flags & MOUSE_LEFT_BUTTON) && (set == 1) ) {
			mouse_left_pressed++;

////////////////////////////
/// SOMETHING TERRIBLE IS ABOUT TO HAPPEN.  I FEEL THIS IS NECESSARY FOR THE DEMO, SINCE
/// I DON'T WANT TO CALL CRITICAL SECTION CODE EACH FRAME TO CHECK THE LEFT MOUSE BUTTON.
/// PLEASE SEE ALAN FOR MORE INFORMATION.
////////////////////////////
#ifdef FS2_DEMO
					{
					extern void demo_reset_trailer_timer();
					demo_reset_trailer_timer();
					}
#endif
////////////////////////////
/// IT'S OVER.  SEE, IT WASN'T SO BAD RIGHT?  IT'S IS VERY UGLY LOOKING, I KNOW.
////////////////////////////

		}
	}
	else {
		if ( (flags & MOUSE_LEFT_BUTTON) && (set == 0) ){
			mouse_left_up++;
		}
	}

	if ( !(mouse_flags & MOUSE_RIGHT_BUTTON) )	{

		if ( (flags & MOUSE_RIGHT_BUTTON) && (set == 1) ){
			mouse_right_pressed++;
		}
	}
	else {
		if ( (flags & MOUSE_RIGHT_BUTTON) && (set == 0) ){
			mouse_right_up++;
		}
	}

	if ( !(mouse_flags & MOUSE_MIDDLE_BUTTON) )	{

		if ( (flags & MOUSE_MIDDLE_BUTTON) && (set == 1) ){
			mouse_middle_pressed++;
		}
	}
	else {
		if ( (flags & MOUSE_MIDDLE_BUTTON) && (set == 0) ){
			mouse_middle_up++;
		}
	}

	if ( set ){
		mouse_flags |= flags;
	} else {
		mouse_flags &= ~flags;
	}

	LEAVE_CRITICAL_SECTION(&mouse_lock);	
}

void mouse_flush()
{
	if (!mouse_inited)
		return;

	mouse_eval_deltas();
	Mouse_dx = Mouse_dy = Mouse_dz = 0;
	ENTER_CRITICAL_SECTION(&mouse_lock);
	mouse_left_pressed = 0;
	mouse_right_pressed = 0;
	mouse_middle_pressed = 0;
	mouse_flags = 0;
	LEAVE_CRITICAL_SECTION(&mouse_lock);	
}

int mouse_down_count(int n, int reset_count)
{
	int tmp = 0;
	if ( !mouse_inited ) return 0;

	if ( (n < LOWEST_MOUSE_BUTTON) || (n > HIGHEST_MOUSE_BUTTON)) return 0;

	ENTER_CRITICAL_SECTION(&mouse_lock);

	switch (n) {
		case MOUSE_LEFT_BUTTON:
			tmp = mouse_left_pressed;
			if ( reset_count ) {
				mouse_left_pressed = 0;
			}
			break;

		case MOUSE_RIGHT_BUTTON:
			tmp = mouse_right_pressed;
			if ( reset_count ) {
				mouse_right_pressed = 0;
			}
			break;

		case MOUSE_MIDDLE_BUTTON:
			tmp = mouse_middle_pressed;
			if ( reset_count ) {
				mouse_middle_pressed = 0;
			}
			break;
	} // end switch

	LEAVE_CRITICAL_SECTION(&mouse_lock);	

	return tmp;
}

// mouse_up_count() returns the number of times button n has gone from down to up
// since the last call
//
// parameters:  n - button of mouse (see #define's in mouse.h)
//
int mouse_up_count(int n)
{
	int tmp = 0;
	if ( !mouse_inited ) return 0;

	if ( (n < LOWEST_MOUSE_BUTTON) || (n > HIGHEST_MOUSE_BUTTON)) return 0;

	ENTER_CRITICAL_SECTION(&mouse_lock);

	switch (n) {
		case MOUSE_LEFT_BUTTON:
			tmp = mouse_left_up;
			mouse_left_up = 0;
			break;

		case MOUSE_RIGHT_BUTTON:
			tmp = mouse_right_up;
			mouse_right_up = 0;
			break;

		case MOUSE_MIDDLE_BUTTON:
			tmp = mouse_middle_up;
			mouse_middle_up = 0;
			break;

		default:
			Assert(0);	// can't happen
			break;
	} // end switch

	LEAVE_CRITICAL_SECTION(&mouse_lock);	

	return tmp;
}

// returns 1 if mouse button btn is down, 0 otherwise

int mouse_down(int btn)
{
	int tmp;
	if ( !mouse_inited ) return 0;

	if ( (btn < LOWEST_MOUSE_BUTTON) || (btn > HIGHEST_MOUSE_BUTTON)) return 0;


	ENTER_CRITICAL_SECTION(&mouse_lock);


	if ( mouse_flags & btn )
		tmp = 1;
	else
		tmp = 0;

	LEAVE_CRITICAL_SECTION(&mouse_lock);	

	return tmp;
}

// returns the fraction of time btn has been down since last call 
// (currently returns 1 if buttons is down, 0 otherwise)
//
float mouse_down_time(int btn)
{
	float tmp;
	if ( !mouse_inited ) return 0.0f;

	if ( (btn < LOWEST_MOUSE_BUTTON) || (btn > HIGHEST_MOUSE_BUTTON)) return 0.0f;

	ENTER_CRITICAL_SECTION(&mouse_lock);

	if ( mouse_flags & btn )
		tmp = 1.0f;
	else
		tmp = 0.0f;

	LEAVE_CRITICAL_SECTION(&mouse_lock);

	return tmp;
}

void mouse_get_delta(int *dx, int *dy, int *dz)
{
	if (dx)
		*dx = Mouse_dx;
	if (dy)
		*dy = Mouse_dy;
	if (dz)
		*dz = Mouse_dz;
}

// Forces the actual windows cursor to be at (x,y).  This may be independent of our tracked (x,y) mouse pos.
void mouse_force_pos(int x, int y)
{
	if (os_foreground()) {  // only mess with windows's mouse if we are in control of it
		POINT pnt;

		pnt.x = x;
		pnt.y = y;
		ClientToScreen((HWND) os_get_window(), &pnt);
		SetCursorPos(pnt.x, pnt.y);
	}
}

// change in mouse position since last call
void mouse_eval_deltas()
{
	static int old_x = 0;
	static int old_y = 0;
	int tmp_x, tmp_y, cx, cy;

	Mouse_dx = Mouse_dy = Mouse_dz = 0;
	if (!mouse_inited)
		return;

	if (Mouse_mode == MOUSE_MODE_DI) {
		mouse_eval_deltas_di();
		return;
	}

	cx = gr_screen.max_w / 2;
	cy = gr_screen.max_h / 2;

	ENTER_CRITICAL_SECTION(&mouse_lock);

	POINT pnt;
	GetCursorPos(&pnt);
	ScreenToClient((HWND)os_get_window(), &pnt);
	tmp_x = pnt.x;
	tmp_y = pnt.y;

	Mouse_dx = tmp_x - old_x;
	Mouse_dy = tmp_y - old_y;
	Mouse_dz = 0;

	if (g_MouseController.GetMouseState(MouseController::MOUSE_STATE_KEEP_CENTERED) && g_MouseController.GetMouseHidden()) {
		if (Mouse_dx || Mouse_dy)
			mouse_force_pos(cx, cy);

		old_x = cx;
		old_y = cy;

	} else {
		old_x = tmp_x;
		old_y = tmp_y;
	}

	LEAVE_CRITICAL_SECTION(&mouse_lock);
}

static LPDIRECTINPUT			Di_mouse_obj = NULL;
static LPDIRECTINPUTDEVICE	Di_mouse = NULL;

void mouse_eval_deltas_di()
{
	int repeat = 1;
	HRESULT hr = 0;
	DIMOUSESTATE mouse_state;

	Mouse_dx = Mouse_dy = Mouse_dz = 0;
	if (!Di_mouse_inited)
		return;

	repeat = 1;
	memset(&mouse_state, 0, sizeof(mouse_state));
	while (repeat) {
		repeat = 0;

		hr = Di_mouse->GetDeviceState(sizeof(mouse_state), &mouse_state);
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED)) {
			// DirectInput is telling us that the input stream has
			// been interrupted.  We aren't tracking any state
			// between polls, so we don't have any special reset
			// that needs to be done.  We just re-acquire and
			// try again.
			Sleep(500);		// Pause a half second...
			hr = Di_mouse->Acquire();
			if (SUCCEEDED(hr))
				repeat = 1;
		}
	}

	if (SUCCEEDED(hr)) {
		Mouse_dx = (int) mouse_state.lX;
		Mouse_dy = (int) mouse_state.lY;
		Mouse_dz = (int) mouse_state.lZ;

	} else {
		Mouse_dx = Mouse_dy = Mouse_dz = 0;
	}

	Mouse_x += Mouse_dx;
	Mouse_y += Mouse_dy;

	if (Mouse_x < 0)
		Mouse_x = 0;

	if (Mouse_y < 0)
		Mouse_y = 0;

	if (Mouse_x >= gr_screen.max_w)
		Mouse_x = gr_screen.max_w - 1;

	if (Mouse_y >= gr_screen.max_h)
		Mouse_y = gr_screen.max_h - 1;

	// keep the mouse inside our window so we don't switch applications or anything (debug bug people reported?)
	// JH: Dang!  This makes the mouse readings in DirectInput act screwy!
//	mouse_force_pos(gr_screen.max_w / 2, gr_screen.max_h / 2);
}

int mouse_get_pos(int *xpos, int *ypos)
{
	int flags;

	if (Mouse_mode == MOUSE_MODE_DI) {
		if (xpos)
			*xpos = Mouse_x;

		if (ypos)
			*ypos = Mouse_y;

		return mouse_flags;
	}

	if (!mouse_inited) {
		*xpos = *ypos = 0;
		return 0;
	}

	POINT pnt;
	GetCursorPos(&pnt);
	ScreenToClient((HWND)os_get_window(), &pnt);

//	EnterCriticalSection(&mouse_lock);

	flags = mouse_flags;
	Mouse_x = pnt.x;
	Mouse_y = pnt.y;

//	LeaveCriticalSection(&mouse_lock);

	if (Mouse_x < 0){
		Mouse_x = 0;
	}

	if (Mouse_y < 0){
		Mouse_y = 0;
	}

	if (Mouse_x >= gr_screen.max_w){
		Mouse_x = gr_screen.max_w - 1;
	}

	if (Mouse_y >= gr_screen.max_h){
		Mouse_y = gr_screen.max_h - 1;
	}
	
	if (xpos){
		*xpos = Mouse_x;
	}

	if (ypos){
		*ypos = Mouse_y;
	}

	return flags;
}

void mouse_get_real_pos(int *mx, int *my)
{
	if (Mouse_mode == MOUSE_MODE_DI) {
		*mx = Mouse_x;
		*my = Mouse_y;
		return;
	}

	POINT pnt;
	GetCursorPos(&pnt);
	ScreenToClient((HWND)os_get_window(), &pnt);
	
	*mx = pnt.x;
	*my = pnt.y;
}

void mouse_set_pos(int xpos, int ypos)
{
	if (Mouse_mode == MOUSE_MODE_DI) {
		Mouse_x = xpos;
		Mouse_y = ypos;
		return;
	}

	if ((xpos != Mouse_x) || (ypos != Mouse_y)){
		mouse_force_pos(xpos, ypos);
	}
}

int di_init()
{
#if defined(PREPROC_ENABLED_DI)
	HRESULT hr;

	if (Mouse_mode == MOUSE_MODE_WIN){
		return 0;
	}

	Di_mouse_inited = 0;
	hr = DirectInputCreate(GetModuleHandle(NULL), DIRECTINPUT_VERSION, &Di_mouse_obj, NULL);
	if (FAILED(hr)) {
		hr = DirectInputCreate(GetModuleHandle(NULL), 0x300, &Di_mouse_obj, NULL);
		if (FAILED(hr)) {
			mprintf(( "DirectInputCreate() failed!\n" ));
			return FALSE;
		}
	}

	hr = Di_mouse_obj->CreateDevice(GUID_SysMouse, &Di_mouse, NULL);
	if (FAILED(hr)) {
		mprintf(( "CreateDevice() failed!\n" ));
		return FALSE;
	}

	hr = Di_mouse->SetDataFormat(&c_dfDIMouse);
	if (FAILED(hr)) {
		mprintf(( "SetDataFormat() failed!\n" ));
		return FALSE;
	}

	hr = Di_mouse->SetCooperativeLevel((HWND)os_get_window(), DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	if (FAILED(hr)) {
		mprintf(( "SetCooperativeLevel() failed!\n" ));
		return FALSE;
	}
/*
	DIPROPDWORD hdr;

	// Turn on buffering
	hdr.diph.dwSize = sizeof(DIPROPDWORD); 
	hdr.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	hdr.diph.dwObj = 0;		
	hdr.diph.dwHow = DIPH_DEVICE;	// Apply to entire device
	hdr.dwData = 16;	//MAX_BUFFERED_KEYBOARD_EVENTS;

	hr = Di_mouse->SetProperty( DIPROP_BUFFERSIZE, &hdr.diph );
	if (FAILED(hr)) {
		mprintf(( "SetProperty DIPROP_BUFFERSIZE failed\n" ));
		return FALSE;
	}

	Di_event = CreateEvent( NULL, FALSE, FALSE, NULL );
	Assert(Di_event != NULL);

	hr = Di_mouse->SetEventNotification(Di_event);
	if (FAILED(hr)) {
		mprintf(( "SetEventNotification failed\n" ));
		return FALSE;
	}
*/
	Di_mouse->Acquire();

	Di_mouse_inited = 1;
#endif
	return TRUE;
}

void di_cleanup()
{
#if defined(PREPROC_ENABLED_DI)
	// Destroy any lingering IDirectInputDevice object.
	if (Di_mouse) {
		// Unacquire the device one last time just in case we got really confused
		// and tried to exit while the device is still acquired.
		Di_mouse->Unacquire();

		Di_mouse->Release();
		Di_mouse = NULL;
	}

	// Destroy any lingering IDirectInput object.
	if (Di_mouse_obj) {
		Di_mouse_obj->Release();
		Di_mouse_obj = NULL;
	}
#endif
	Di_mouse_inited = 0;
}
