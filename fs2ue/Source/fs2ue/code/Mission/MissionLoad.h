/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _MISSIONLOAD_H
#define _MISSIONLOAD_H

#ifndef UNITY_BUILD
#include "GlobalIncs/PsTypes.h"
#endif

// -----------------------------------------------
// For recording most recent missions played
// -----------------------------------------------
#define			MAX_RECENT_MISSIONS	10
extern	char	Recent_missions[MAX_RECENT_MISSIONS][MAX_FILENAME_LEN];
extern	int	Num_recent_missions;

// Mission_load takes no parameters.
// It expects the following variables to be set correctly:
// Game_current_mission_filename

int mission_load();
void mission_init();

// Functions for mission load menu
void mission_load_menu_init();
void mission_load_menu_close();
void mission_load_menu_do();

#endif
