/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#ifndef UNITY_BUILD
#include "UiDefs.h"
#include "Ui.h"
#include "Timer.h"
#endif

UI_MOUSE ui_mouse;

int ui_mouse_inited = 0;

void ui_mouse_process()
{
	int buttons;

	if (!ui_mouse_inited) {
		ui_mouse_inited = 1;
		ui_mouse.x = 0;
		ui_mouse.y = 0;
		ui_mouse.dx = 0;
		ui_mouse.dy = 0;
		ui_mouse.b1_status = 0;
		ui_mouse.b1_last_status = 0;
		ui_mouse.b1_time_lastpressed=0;
		ui_mouse.b2_status = 0;
		ui_mouse.b2_last_status = 0;
		ui_mouse.b2_time_lastpressed = 0;
		ui_mouse.timestamp = timer_get_milliseconds();
	}

	buttons = g_MouseController.GetPos( &ui_mouse.x, &ui_mouse.y );

	// check if mouse pressed
	if (buttons & MouseController::MOUSE_LEFT_BUTTON)
		ui_mouse.b1_status = BUTTON_PRESSED;
	else
		ui_mouse.b1_status = BUTTON_RELEASED;

	if (buttons & MouseController::MOUSE_RIGHT_BUTTON)
		ui_mouse.b2_status = BUTTON_PRESSED;
	else
		ui_mouse.b2_status = BUTTON_RELEASED;

	// now check if we missed something between checks, just in case
	if (g_MouseController.DownCount(MouseController::MOUSE_LEFT_BUTTON))
		ui_mouse.b1_status = BUTTON_PRESSED;

	if (g_MouseController.UpCount(MouseController::MOUSE_LEFT_BUTTON))
		ui_mouse.b1_status = BUTTON_RELEASED;

	if (g_MouseController.DownCount(MouseController::MOUSE_RIGHT_BUTTON))
		ui_mouse.b2_status = BUTTON_PRESSED;

	if (g_MouseController.UpCount(MouseController::MOUSE_RIGHT_BUTTON))
		ui_mouse.b2_status = BUTTON_RELEASED;

	// check for double clicks
	if ((ui_mouse.b1_status & BUTTON_PRESSED) && (ui_mouse.b1_last_status & BUTTON_RELEASED) ) {
		if ( timer_get_milliseconds() <= ui_mouse.b1_time_lastpressed + 250 )  //&& (ui_mouse.moved==0)
			ui_mouse.b1_status |= BUTTON_DOUBLE_CLICKED;

		ui_mouse.b1_time_lastpressed = timer_get_milliseconds();
		ui_mouse.b1_status |= BUTTON_JUST_PRESSED;

	} else if ((ui_mouse.b1_status & BUTTON_RELEASED) && (ui_mouse.b1_last_status & BUTTON_PRESSED) )
		ui_mouse.b1_status |= BUTTON_JUST_RELEASED;

	if ((ui_mouse.b2_status & BUTTON_PRESSED) && (ui_mouse.b2_last_status & BUTTON_RELEASED) ) {
		if ( timer_get_milliseconds() <= ui_mouse.b2_time_lastpressed + 250 )  //&& (ui_mouse.moved==0)
			ui_mouse.b2_status |= BUTTON_DOUBLE_CLICKED;

		ui_mouse.b2_time_lastpressed = timer_get_milliseconds();
		ui_mouse.b2_status |= BUTTON_JUST_PRESSED;

	} else if ((ui_mouse.b2_status & BUTTON_RELEASED) && (ui_mouse.b2_last_status & BUTTON_PRESSED) )
		ui_mouse.b2_status |= BUTTON_JUST_RELEASED;

	ui_mouse.b1_last_status = ui_mouse.b1_status;
	ui_mouse.b2_last_status = ui_mouse.b2_status;
}

void ui_mouse_reset()
{
	ui_mouse.b1_last_status = ui_mouse.b1_status = 0;
	ui_mouse.b2_last_status = ui_mouse.b2_status = 0;
}
