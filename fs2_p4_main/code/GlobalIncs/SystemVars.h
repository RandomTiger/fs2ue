/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _SYSTEMVARS_H
#define _SYSTEMVARS_H

#ifndef UNITY_BUILD
#include "math.h"
#endif

enum ePlayerGameState 
{
	GM_MULTIPLAYER	=				(1 << 0),
	GM_NORMAL		=				(1 << 1),
	GM_DEAD_DIED 	=				(1 << 2),				//	Died, waiting to blow up.
	GM_DEAD_BLEW_UP	=				(1 << 4),				//	Blew up.
	GM_IN_MISSION	=				(1 << 5),				// Player is actually in the mission -- not at a pre-mission menu

	GM_DEAD			=				(GM_DEAD_DIED | GM_DEAD_BLEW_UP)
};
#define  GM_STANDALONE_SERVER			(1 << 8)
#define	GM_STATS_TRANSFER				(1 << 9)				// in the process of stats transfer
#define	GM_CAMPAIGN_MODE				(1 << 10)			// are we currently in a campaign.

#define	GM_DEMO_RECORD					(1 << 11)			// recording a demo
#define	GM_DEMO_PLAYBACK				(1 << 12)			// playing a demo back
#define	GM_DEMO							(GM_DEMO_RECORD | GM_DEMO_PLAYBACK)			// true whenever a demo is being recorded or played back

#define	VM_EXTERNAL						(1 << 0)				//	Set if not viewing from player position.
#define	VM_SLEWED						(1 << 1)				//	Set if viewer orientation is slewed.
#define	VM_DEAD_VIEW					(1 << 2)				//	Set if viewer is watching from dead view.
#define	VM_CHASE							(1 << 3)				//	Chase view.
#define	VM_OTHER_SHIP					(1 << 4)				//	View from another ship.
#define	VM_EXTERNAL_CAMERA_LOCKED	(1 << 5)				// External camera is locked in place (ie controls move ship not camera)
#define	VM_WARP_CHASE					(1	<< 6)				// View while warping out (form normal view mode)
#define	VM_PADLOCK_UP					(1 << 7)
#define	VM_PADLOCK_REAR				(1 << 8)
#define	VM_PADLOCK_LEFT				(1 << 9)
#define	VM_PADLOCK_RIGHT				(1 << 10)
#define	VM_WARPIN_ANCHOR				(1 << 11)			// special warpin camera mode

#define	VM_PADLOCK_ANY (VM_PADLOCK_UP|VM_PADLOCK_REAR|VM_PADLOCK_LEFT|VM_PADLOCK_RIGHT)


typedef struct vei {
	angles	angles;			//	Angles defining viewer location.
	float		distance;		//	Distance from which to view, plus 2x radius.
} vei;

typedef struct vci {	
	angles	angles;			
	float		distance;		// Distance from which to view, plus 3x radius
} vci;

extern fix Missiontime;
extern fix Frametime;
extern int Framecount;

extern int Game_mode;

extern int Viewer_mode;
extern int Rand_count;

extern int Game_restoring;		// If set, this means we are restoring data from disk

// The detail level.  Anything below zero draws simple models earlier than it
// should.   Anything above zero draws higher detail models longer than it should.
// -2=lowest
// -1=low
// 0=normal (medium)	
// 1=high
// 2=extra high
extern int Game_detail_level;

#define DETAIL_DEFAULT (0xFFFFFFFF)

#define DETAIL_FLAG_STARS			(1<<0)	// draw the stars
#define DETAIL_FLAG_NEBULAS		(1<<1)	// draw the motion debris
#define DETAIL_FLAG_MOTION			(1<<2)	// draw the motion debris
#define DETAIL_FLAG_PLANETS		(1<<3)	// draw planets
#define DETAIL_FLAG_MODELS			(1<<4)	// draw models not as blobs
#define DETAIL_FLAG_LASERS			(1<<5)	// draw lasers not as pixels
#define DETAIL_FLAG_CLEAR			(1<<6)	// clear screen background after each frame
#define DETAIL_FLAG_HUD				(1<<7)	// draw hud stuff
#define DETAIL_FLAG_FIREBALLS		(1<<8)	// draw fireballs
#define DETAIL_FLAG_COLLISION		(1<<9)	// use good collision detection


extern uint Game_detail_flags;

extern angles	Viewer_slew_angles;
extern vei		Viewer_external_info;
extern vci		Viewer_chase_info;

extern int Is_standalone;
extern int Interface_framerate;				// show interface framerate during flips
extern int Interface_last_tick;				// last timer tick on flip

// for notifying players of unknown ship types
extern int Fred_found_unknown_ship_during_parsing;

#define NOISE_NUM_FRAMES 15

// Noise numbers go from 0 to 1.0
extern float Noise[NOISE_NUM_FRAMES];


// If true, then we are using Direct3D hardware.  This is used for game type stuff
// that changes when you're using hardware.
extern int D3D_enabled;

// game skill levels 
#define	NUM_SKILL_LEVELS	5

//====================================================================================
// DETAIL LEVEL STUFF
// If you change any of this, be sure to increment the player file version 
// in Freespace\ManagePilot.cpp and change Detail_defaults in SystemVars.cpp 
// or bad things will happen, I promise.
//====================================================================================

#define MAX_DETAIL_LEVEL 4			// The highest valid value for the "analog" detail level settings

// If you change this, update player file in ManagePilot.cpp 
typedef struct detail_levels {

	int		setting;						// Which default setting this was created from.   0=lowest... NUM_DEFAULT_DETAIL_LEVELS-1, -1=Custom

	// "Analogs"
	int		nebula_detail;				// 0=lowest detail, MAX_DETAIL_LEVEL=highest detail
	int		detail_distance;			// 0=lowest MAX_DETAIL_LEVEL=highest	
	int		hardware_textures;		// 0=max culling, MAX_DETAIL_LEVEL=no culling
	int		num_small_debris;			// 0=min number, MAX_DETAIL_LEVEL=max number
	int		num_particles;				// 0=min number, MAX_DETAIL_LEVEL=max number
	int		num_stars;					// 0=min number, MAX_DETAIL_LEVEL=max number
	int		shield_effects;			// 0=min, MAX_DETAIL_LEVEL=max
	int		lighting;					// 0=min, MAX_DETAIL_LEVEL=max	

	// Booleans
	int		targetview_model;			// 0=off, 1=on	
	int		planets_suns;				// 0=off, 1=on			
	int		weapon_extras;				// extra weapon details. trails, glows
} detail_levels;

// Global values used to access detail levels in game and libs
extern detail_levels Detail;

#define NUM_DEFAULT_DETAIL_LEVELS	4	// How many "predefined" detail levels there are

// Call this with:
// 0 - lowest
// NUM_DEFAULT_DETAIL_LEVELS - highest
// To set the parameters in Detail to some set of defaults
void detail_level_set(int level);

// Returns the current detail level or -1 if custom.
int current_detail_level();

//====================================================================================
// Memory stuff from WinDebug.cpp
extern int TotalRam;
void windebug_memwatch_init();

void game_busy();

#endif