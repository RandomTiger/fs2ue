/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _FREESPACE_H
#define _FREESPACE_H
#ifndef STAMPER_PROGRAM							// because of all the dependancies, I have to do this...yuck!!!  MWA 7/21/97

#ifndef UNITY_BUILD
#include "GlobalIncs/pstypes.h"
#include "GlobalIncs/SystemVars.h"
#include "Graphics/2d.h"
#endif

// --------------------------------------------------------------------------------------------------------
// FREESPACE DEFINES/VARS
//
														
// filename extensions
#define FS_MISSION_FILE_EXT				NOX(".fs2")
#define FS_CAMPAIGN_FILE_EXT				NOX(".fc2")

// CDROM volume names
#ifdef MULTIPLAYER_BETA_BUILD
	#define FS_CDROM_VOLUME_1					NOX("FS2_BETA")
	#define FS_CDROM_VOLUME_2					NOX("FS2_BETA")
#elif defined(E3_BUILD)
	#define FS_CDROM_VOLUME_1					NOX("FS2_E3DEMO")
	#define FS_CDROM_VOLUME_2					NOX("FS2_E3DEMO")
#elif defined(OEM_BUILD)
	#define FS_CDROM_VOLUME_1					NOX("FS2_OEM")
	#define FS_CDROM_VOLUME_2					NOX("FS2_OEM")
	#define FS_CDROM_VOLUME_3					NOX("FS2_OEM")
#else
	#define FS_CDROM_VOLUME_1					NOX("FREESPACE2_1")
	#define FS_CDROM_VOLUME_2					NOX("FREESPACE2_2")
	#define FS_CDROM_VOLUME_3					NOX("FREESPACE2_3")

	// old volume names
	// #define FS_CDROM_VOLUME_1					NOX("FREESPACE_1")
	// #define FS_CDROM_VOLUME_2					NOX("FREESPACE_2")
	// #define FS_CDROM_VOLUME_3					NOX("FREESPACE_3")
#endif

// frametime/missiontime variables
extern fix Frametime;
extern float flFrametime;
extern fix Missiontime;

// 0 - 4
extern int Game_skill_level;

// see GM_* defines in systemvars.h
extern int Game_mode;

// if this value is set anywhere within the game_do_state_common() function, the normal do_state() will not be called
// for this frame. Useful for getting out of sticky sequencing situations.
extern int Game_do_state_should_skip;

// time compression
extern fix Game_time_compression;

// Set if subspace is active this level
extern int Game_subspace_effect;		

// The current mission being played.
extern char Game_current_mission_filename[MAX_FILENAME_LEN];

// game's CDROM directory
extern char Game_CDROM_dir[MAX_PATH_LEN];

// if the ships.tbl the player has is valid
extern int Game_ships_tbl_valid;

// if the weapons.tbl the player has is valid
extern int Game_weapons_tbl_valid;


// this is a mission actually designed at Volition
#define MAX_BUILTIN_MISSIONS					100
#define FSB_FROM_VOLITION						(1<<0)			// we made it in-house
#define FSB_MULTI									(1<<1)			// is a multiplayer mission
#define FSB_TRAINING								(1<<2)			// is a training mission
#define FSB_CAMPAIGN								(1<<3)			// is a campaign mission
#define FSB_CAMPAIGN_FILE						(1<<4)			// is actually a campaign file

typedef struct fs_builtin_mission {
	char filename[MAX_FILENAME_LEN];
	int flags;															// see FSB_* defines above
	char cd_volume[MAX_FILENAME_LEN];							// cd volume which this needs
} fs_builtin_mission;


// --------------------------------------------------------------------------------------------------------
// FREESPACE FUNCTIONS
//

// mission management -------------------------------------------------

// loads in the currently selected mission
int game_start_mission();		

// shutdown a mission
void game_level_close();


// gameplay stuff -----------------------------------------------------

// stop the game (mission) timer
void game_stop_time();

// start the game (mission) timer
void game_start_time();

// call whenever in a loop or if you need to get a keypress
int game_check_key();

// poll for keypresses
int game_poll();

// function to read keyboard stuff
void game_process_keys();

// call this to set frametime properly (once per frame)
void game_set_frametime(int state);

// Used to halt all looping game sounds
void game_stop_looped_sounds();

// do stuff that may need to be done regardless of state
void game_do_state_common(int state,int no_networking = 0);


// skill level --------------------------------------------------------

// increase the skill level (will wrap around to min skill level)
void game_increase_skill_level();

// get the default game skill level
int game_get_default_skill_level();

// a keypress.  See CPP file for more info.
void game_flush();

// misc ---------------------------------------------------------------

// lookup the specified filename. return an fs_builtin_mission* if found, NULL otherwise
fs_builtin_mission *game_find_builtin_mission(char *filename);



//================================================================
// GAME FLASH STUFF  - code in FreeSpace.cpp

// Adds a flash for Big Ship explosions
// cap range from 0 to 1
void big_explosion_flash(float flash);

// Loads the best palette for this level, based
// on nebula color and hud color.  You could just call palette_load_table with
// the appropriate filename, but who wants to do that.
void game_load_palette();

//================================================================

// Call at the beginning of each frame
void game_whack_reset();

// Call to apply a whack to a the ship. Used for force feedback
void game_whack_apply( float x, float y );

//===================================================================

// make sure a CD is in the drive before continuing (returns 1 to continue, otherwise 0).
int game_do_cd_check(char *volume_name=NULL);
int game_do_cd_check_specific(char *volume_name, int cdnum);
int find_freespace_cd(char *volume_name=NULL);
int set_cdrom_path(int drive_num);
int game_do_cd_mission_check(char *filename);

// Used to tell the player that a feature isn't available in the demo version of FreeSpace
void game_feature_not_in_demo_popup();

//	Return version string for demo or full version, depending on build.
void get_version_string(char *str);

// format the specified time (fixed point) into a nice string
void game_format_time(fix m_time,char *time_str);

// if the game is running using hacked data
int game_hacked_data();

// show the oem upsell screens (end of campaign, or close of game
void oem_upsell_show_screens();

#endif			// endif of #ifndef STAMPER_PROGRAM


bool FREESPACE_Init(
#if !defined(FS2_UE)
	HINSTANCE hInst,
#endif
	LPSTR szCmdLine);

bool FREESPACE_Update(const float DeltaTime);
void FREESPACE_Shutdown();

#endif 