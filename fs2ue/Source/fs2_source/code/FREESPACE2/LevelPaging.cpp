/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/
#if !defined(FS2_UE)

#include "UnityBuild.h"

#ifndef UNITY_BUILD
#include "Freespace.h"
#include "bmpman/BmpMan.h"
#include "LevelPaging.h"
#endif
#endif

// All the page in functions
extern void ship_page_in();
extern void debris_page_in();
extern void particle_page_in();
extern void stars_page_in();
extern void hud_page_in();
extern void radar_page_in();
extern void weapons_page_in();
extern void fireballs_page_in();
extern void shockwave_page_in();
extern void shield_hit_page_in();
extern void asteroid_page_in();
extern void training_mission_page_in();
extern void neb2_page_in();
extern void message_pagein_mission_messages();

// Pages in all the texutures for the currently
void level_page_in()
{

	mprintf(( "Beginning level bitmap paging...\n" ));

	if(!(Game_mode & GM_STANDALONE_SERVER)){		
		bm_page_in_start();
	}

	// Most important ones first
	ship_page_in();
	weapons_page_in();
	fireballs_page_in();
	particle_page_in();
	debris_page_in();
	hud_page_in();
	radar_page_in();
	training_mission_page_in();
	stars_page_in();
	shockwave_page_in();
	shield_hit_page_in();
	asteroid_page_in();
	neb2_page_in();

	message_pagein_mission_messages();

	if(!(Game_mode & GM_STANDALONE_SERVER)){
		bm_page_in_stop();
	}

	mprintf(( "Ending level bitmap paging...\n" ));

}
