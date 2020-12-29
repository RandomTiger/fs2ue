/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _MOUSE_H
#define _MOUSE_H

#ifndef UNITY_BUILD
#include "GlobalIncs/PsTypes.h"
#endif

// call once to init the mouse
void mouse_init();

extern void mouse_mark_button( uint flags, int set );

// Fills in xpos & ypos if not NULL.
// Returns Button states
// Always returns coordinates clipped to screen coordinates.
extern int mouse_get_pos( int *xpos, int *ypos );

// get_real_pos could be negative.
extern void mouse_get_real_pos(int *mx, int *my);

extern void mouse_set_pos(int xpos,int ypos);

enum
{
	MOUSE_LEFT_BUTTON	= (1<<0),
	MOUSE_RIGHT_BUTTON	= (1<<1),
	MOUSE_MIDDLE_BUTTON	= (1<<2),

	MOUSE_NUM_BUTTONS	=	3,

// keep the following two #defines up to date with the #defines above
	LOWEST_MOUSE_BUTTON	 =(1<<0),
	HIGHEST_MOUSE_BUTTON =	(1<<2),
};

// Returns the number of times button n went from up to down since last call
int mouse_down_count(int n, int reset_count = 1);
// Returns the number of times button n went from down to up since last call
int mouse_up_count(int n);

extern void mouse_flush();

int mouse_down(int btn);			// returns 1 if mouse button btn is down, 0 otherwise
float mouse_down_time(int btn);	// returns the fraction of time btn has been down since last call

void mouse_eval_deltas();
void mouse_get_delta(int *dx = NULL, int *dy = NULL, int *dz = NULL);

#endif
