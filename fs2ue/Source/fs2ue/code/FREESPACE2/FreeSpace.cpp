/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#include "Freespace.h"

#ifdef FS2_UE
#include <stdlib.h>
#include <process.h>
#include <time.h>
#include <direct.h>

#include "pstypes.h"
#include "SystemVars.h"
#include "key.h"
#include "vecmat.h"
#include "2d.h"
#include "3d.h"
#include "StarField.h"
#include "Lighting.h"
#include "Weapon.h"
#include "Ship.h"
#include "PalMan.h"
#include "OsApi.h"
#include "fireballs.h"
#include "debris.h"
#include "timer.h"
#include "fix.h"
#include "floating.h"
#include "GameSequence.h"
#include "Radar.h"
#include "OptionsMenu.h"
#include "PlayerMenu.h"
#include "TrainingMenu.h"
#include "TechMenu.h"
#include "Ai.h"
#include "Hud.h"
#include "HUDMessage.h"
#include "PsNet.h"
#include "MissionGoals.h"
#include "MissionParse.h"
#include "BmpMan.h"
#include "Joy.h"
#include "Joy_ff.h"
#include "Multi.h"
#include "MultiUtil.h"
#include "MultiMsgs.h"
#include "MultiUI.h"
#include "CFile.h"
#include "Player.h"
#include "FreeSpace.h"
#include "ManagePilot.h"
#include "Sound.h"
#include "ContextHelp.h"
#include "Mouse.h"
#include "joy.h"
#include "MissionBrief.h"
#include "MissionDebrief.h"
#include "ui.h"
#include "MissionShipChoice.h"
#include "Model.h"
#include "HUDconfig.h"
#include "ControlsConfig.h"
#include "MissionMessage.h"
#include "MissionTraining.h"
#include "HUDets.h"
#include "HUDtarget.h"
#include "GameSnd.h"
#include "WinMIDI.h"
#include "EventMusic.h"
#include "AnimPlay.h"
#include "MissionWeaponChoice.h"
#include "MissionLog.h"
#include "AudioStr.h"
#include "HUDlock.h"
#include "MissionCampaign.h"
#include "Credits.h"
#include "MissionHotKey.h"
#include "ObjectSnd.h"
#include "Cmeasure.h"
#include "ai.h"
#include "linklist.h"
#include "Shockwave.h"
#include "Afterburner.h"
#include "Scoring.h"
#include "Stats/FSStats.h"
#include "cmdline.h"
#include "Timer.h"
#include "stand_gui.h"
#include "PcxUtils.h"
#include "HUDtargetbox.h"
#include "multi_xfer.h"
#include "HUDescort.h"
#include "multiutil.h"
#include "Sexp.h"
#include "Medals.h"
#include "MultiTeamSelect.h"
#include "ds3d.h"
#include "ShipFX.h"
#include "ReadyRoom.h"
#include "MainHallMenu.h"
#include "multilag.h"
#include "Trails.h"
#include "Particle.h"
#include "popup.h"
#include "multi_ingame.h"
#include "snazzyui.h"
#include "Asteroid.h"
#include "PopupDead.h"
#include "multi_voice.h"
#include "MissionCmdBrief.h"
#include "RedAlert.h"
#include "GameplayHelp.h"
#include "multilag.h"
#include "StaticRand.h"
#include "multi_pmsg.h"
#include "LevelPaging.h"
#include "observer.h"
#include "multi_pause.h"
#include "multi_endgame.h"
#include "Cutscenes.h"
#include "multi_respawn.h"
// #include "Movie.h"
#include "multi_obj.h"
#include "multi_log.h"
#include "Emp.h"
#include "localize.h"
#include "OsRegistry.h"
#include "Barracks.h"
#include "MissionPause.h"
#include "font.h"
#include "alphacolors.h"
#include "objcollide.h"
#include "flak.h"
#include "Neb.h"
#include "NebLightning.h"
#include "ShipContrails.h"
#include "AWACS.h"
#include "Beam.h"
#include "multi_dogfight.h"
#include "multi_rate.h"
#include "Muzzleflash.h"
#include "Encrypt.h"
#include "Demo.h"
#include "version.h"
#include "MainHalltemp.h"
#include "ExceptionHandler.h"
#include "supernova.h"
#include "HUDshield.h"
#include <io.h>
// #include "names.h"
#include "shiphit.h"
#include "missionloopbrief.h"
#include "IO/PadController.h"
#include "IO/MouseController.h"
#include "IO/InputController.h"
#include "IO/VoiceRecognition.h"
#include "IO/ForceFeedback.h"
#include "Behaviours/ScreenEffects.h"
#include "Game/Game.h"
#include "Cfile/CfileSystem.h"
#include "Stats/FSStats.h"
#include "PackUnpack.h"
#include "MissionBriefCommon.h"

#include <windows.h>
#include <timeapi.h>


#else
// Backwards compatibility for non unity build
#ifdef UNITY_BUILD
#include "UnityBuild.h"
#else
// Pretend we are a unity build
#define UNITY_BUILD
// Include ALL headers
#include "UnityBuild.h"
// Disable Pretend
#undef UNITY_BUILD
#endif
#endif

#include "LevelPaging.h"
#include "FreespaceResource.h"
#include "../Threading/Thread.h"

#ifdef NDEBUG
#ifdef FRED
#error macro FRED is defined when trying to build release Fred.  Please undefine FRED macro in build settings
#endif
#endif

//	Revision history.
//	Full version:
//    1.00.04	5/26/98	MWA -- going final (12 pm)
//    1.00.03	5/26/98	MWA -- going final (3 am)
//    1.00.02	5/25/98	MWA -- going final
//    1.00.01	5/25/98	MWA -- going final
//		0.90		5/21/98	MWA -- getting ready for final.
//		0.10		4/9/98.  Set by MK.
//
//	Demo version: (obsolete since DEMO codebase split from tree)
//		0.03		4/10/98	AL.	Interplay rev
//		0.02		4/8/98	MK.	Increased when this system was modified.
//		0.01		4/7/98?	AL.	First release to Interplay QA.
//
//	OEM version:
//		1.00		5/28/98	AL.	First release to Interplay QA.

void game_level_init(int seed = -1);
void game_post_level_init();
void game_do_frame();
void game_update_missiontime();	// called from game_do_frame() and navmap_do_frame()
void game_reset_time();
void game_show_framerate();			// draws framerate in lower right corner

int Game_no_clear = 0;

int Pofview_running = 0; 
int Nebedit_running = 0;

#define FRAME_FILTER 16

#define DEFAULT_SKILL_LEVEL	2
int	Game_skill_level = DEFAULT_SKILL_LEVEL;

#define	VIEWER_ZOOM_DEFAULT 0.75f			//	Default viewer zoom, 0.625 as per multi-lateral agreement on 3/24/97
float Viewer_zoom = VIEWER_ZOOM_DEFAULT;

#define EXE_FNAME			("fs2.exe")
#define LAUNCHER_FNAME	("freespace2.exe")

// JAS: Code for warphole camera.
// Needs to be cleaned up.
vector Camera_pos;
vector Camera_velocity;
vector Camera_desired_velocity;
matrix Camera_orient = IDENTITY_MATRIX;
float Camera_damping = 1.0f;
float Camera_time = 0.0f;
float Warpout_time = 0.0f;
int Warpout_forced = 0;		// Set if this is a forced warpout that cannot be cancelled.
int Warpout_sound = -1;
void camera_move();
int Use_joy_mouse = 0;
#ifndef NDEBUG
int Use_fullscreen_at_startup = 0;
#endif
int Show_area_effect = 0;
object	*Last_view_target = NULL;

int dogfight_blown = 0;

int	frame_int = -1;
float frametimes[FRAME_FILTER];
float frametotal = 0.0f;
float flFrametime;

#ifdef RELEASE_REAL
	int	Show_framerate = 0;
#else 
	int	Show_framerate = 1;
#endif

int	Framerate_cap = 120;
int	Show_mem = 0;
int	Show_cpu = 0;
int	Show_target_debug_info = 0;
int	Show_target_weapons = 0;
int	Game_font = -1;
static int Show_player_pos = 0;		// debug console command to show player world pos on HUD

int Debug_octant = -1;

fix Game_time_compression = F1_0;

// if the ships.tbl the player has is valid
int Game_ships_tbl_valid = 0;

// if the weapons.tbl the player has is valid
int Game_weapons_tbl_valid = 0;

#ifndef NDEBUG
int Test_begin = 0;
extern int	Player_attacking_enabled;
int Show_net_stats;
#endif

int Pre_player_entry;

int	Fred_running = 0;
char Game_current_mission_filename[MAX_FILENAME_LEN];
int game_single_step = 0;
int last_single_step=0;

extern int MSG_WINDOW_X_START;	// used to position mission_time and shields output
extern int MSG_WINDOW_Y_START;
extern int MSG_WINDOW_HEIGHT;

int game_zbuffer = 1;

int Game_level_seed;

#define EXPIRE_BAD_CHECKSUM			1
#define EXPIRE_BAD_TIME					2

extern void ssm_init();
extern void ssm_level_init();
extern void ssm_process();

// static variable to contain the time this version was built
// commented out for now until
// I figure out how to get the username into the file
//LOCAL char freespace_build_time[] = "Compiled on:"__DATE__" "__TIME__" by "__USER__;

// defines and variables used for dumping frame for making trailers.
#ifndef NDEBUG
int Debug_dump_frames = 0;			// Set to 0 to not dump frames, else equal hz to dump. (15 or 30 probably)
int Debug_dump_trigger = 0;
int Debug_dump_frame_count;
int Debug_dump_frame_num = 0;
#define DUMP_BUFFER_NUM_FRAMES	1			// store every 15 frames
#endif

// amount of time to wait after the player has died before we display the death died popup
#define PLAYER_DIED_POPUP_WAIT		2500
int Player_died_popup_wait = -1;
int Player_multi_died_check = -1;

// builtin mission list stuff
#ifdef FS2_DEMO
	int Game_builtin_mission_count = 6;
	fs_builtin_mission Game_builtin_mission_list[MAX_BUILTIN_MISSIONS] = {
		{ "SPDemo-01.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							""		},
		{ "SPDemo-02.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							""		},
		{ "DemoTrain.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							""		},
		{ "Demo.fc2",						(FSB_FROM_VOLITION | FSB_CAMPAIGN_FILE),					""		},
		{ "MPDemo-01.fs2",				(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "Demo-DOG-01.fs2",				(FSB_FROM_VOLITION | FSB_MULTI),								""		},
	};
#elif defined(PD_BUILD)
	int Game_builtin_mission_count = 4;
	fs_builtin_mission Game_builtin_mission_list[MAX_BUILTIN_MISSIONS] = {
		{ "sm1-01.fs2",					(FSB_FROM_VOLITION),												""		},
		{ "sm1-05.fs2",					(FSB_FROM_VOLITION),												""		},		
		{ "sm1-01",							(FSB_FROM_VOLITION),												""		},
		{ "sm1-05",							(FSB_FROM_VOLITION),												""		},		
	};
#elif defined(MULTIPLAYER_BETA)
	int Game_builtin_mission_count = 17;
	fs_builtin_mission Game_builtin_mission_list[MAX_BUILTIN_MISSIONS] = {
		// multiplayer beta
		{ "md-01.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "md-02.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "md-03.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "md-04.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "md-05.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "md-06.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "md-07.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "mt-02.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "mt-03.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "m-03.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "m-04.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "m-05.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""		},
		{ "templar-01.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""		},
		{ "templar-02.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""		},
		{ "templar-03a.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""		},
		{ "templar-04a.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""		},
		{ "templar.fc2",					(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN_FILE),	""		},	
	};
#elif defined(OEM_BUILD)
	int Game_builtin_mission_count = 17;
	fs_builtin_mission Game_builtin_mission_list[MAX_BUILTIN_MISSIONS] = {
		// oem version - act 1 only
		{ "freespace2oem.fc2",			(FSB_FROM_VOLITION | FSB_CAMPAIGN_FILE),					"" },
			
		// act 1
		{ "sm1-01.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "sm1-02.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "sm1-03.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "sm1-04.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "sm1-05.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "sm1-06.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "sm1-07.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "sm1-08.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "sm1-09.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "sm1-10.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "training-1.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "training-2.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "training-3.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "tsm-104.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "tsm-105.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	},
		{ "tsm-106.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_1	}
	};
#else 
	int Game_builtin_mission_count = 92;
	fs_builtin_mission Game_builtin_mission_list[MAX_BUILTIN_MISSIONS] = {
		// single player campaign
		{ "freespace2.fc2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN_FILE),					"" },
			
		// act 1
		{ "sm1-01.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "sm1-02.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "sm1-03.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "sm1-04.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "sm1-05.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "sm1-06.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "sm1-07.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "sm1-08.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "sm1-09.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "sm1-10.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "loop1-1.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "loop1-2.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "loop1-3.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "training-1.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "training-2.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "training-3.fs2",				(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "tsm-104.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "tsm-105.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},
		{ "tsm-106.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_2	},

		// act 2
		{ "sm2-01.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm2-02.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm2-03.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm2-04.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm2-05.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm2-06.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm2-07.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm2-08.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm2-09.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm2-10.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},

		// act 3
		{ "sm3-01.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm3-02.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm3-03.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm3-04.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm3-05.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm3-06.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm3-07.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm3-08.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm3-09.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "sm3-10.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},
		{ "loop2-1.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},		
		{ "loop2-2.fs2",					(FSB_FROM_VOLITION | FSB_CAMPAIGN),							FS_CDROM_VOLUME_3	},

		// multiplayer missions

		// gauntlet
		{ "g-shi.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "g-ter.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "g-vas.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},

		// coop
		{ "m-01.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "m-02.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "m-03.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "m-04.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},		

		// dogfight
		{ "mdh-01.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdh-02.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdh-03.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdh-04.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdh-05.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdh-06.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdh-07.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdh-08.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdh-09.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdl-01.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdl-02.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdl-03.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdl-04.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdl-05.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdl-06.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdl-07.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdl-08.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdl-09.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdm-01.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdm-02.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdm-03.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdm-04.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdm-05.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdm-06.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdm-07.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdm-08.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "mdm-09.fs2",					(FSB_FROM_VOLITION | FSB_MULTI),								""						},		
		{ "osdog.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},

		// TvT		
		{ "mt-01.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "mt-02.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "mt-03.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "mt-04.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "mt-05.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "mt-06.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "mt-07.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "mt-08.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "mt-09.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},
		{ "mt-10.fs2",						(FSB_FROM_VOLITION | FSB_MULTI),								""						},				

		// campaign
		{ "templar.fc2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN_FILE),					"" },
		{ "templar-01.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""						},				
		{ "templar-02.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""						},				
		{ "templar-03.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""						},				
		{ "templar-04.fs2",				(FSB_FROM_VOLITION | FSB_MULTI | FSB_CAMPAIGN),			""						},				
	};
#endif

char transfer_text[128];

float	Start_time = 0.0f;

float Framerate = 0.0f;

float Timing_total = 0.0f;
float Timing_render2 = 0.0f;
float Timing_render3 = 0.0f;
float Timing_flip = 0.0f;
float Timing_clear = 0.0f;

// Internal function prototypes
void game_maybe_draw_mouse(float frametime);
void init_animating_pointer();
void load_animating_pointer(char *filename, int dx, int dy);
void unload_animating_pointer();
void game_do_training_checks();
void game_shutdown(void);
void game_show_event_debug(float frametime);
void game_event_debug_init();
void game_frame();
void demo_upsell_show_screens();
void game_start_subspace_ambient_sound();
void game_stop_subspace_ambient_sound();
void verify_ships_tbl();
void verify_weapons_tbl();
void display_title_screen();

// loading background filenames
static char *Game_loading_bground_fname[GR_NUM_RESOLUTIONS] = {
	"LoadingBG",		// GR_640
	"2_LoadingBG"		// GR_1024
};


static char *Game_loading_ani_fname[GR_NUM_RESOLUTIONS] = {
	"Loading.ani",		// GR_640
	"2_Loading.ani"		// GR_1024
};

#if defined(FS2_DEMO)
static char *Game_demo_title_screen_fname[GR_NUM_RESOLUTIONS] = {
	"PreLoad",
	"2_PreLoad"
};
#elif defined(OEM_BUILD)
static char *Game_demo_title_screen_fname[GR_NUM_RESOLUTIONS] = {
	"OEMPreLoad",
	"2_OEMPreLoad"
};
#endif

// cdrom stuff
char Game_CDROM_dir[MAX_PATH_LEN];
int init_cdrom();

// How much RAM is on this machine. Set in WinMain
unsigned long Freespace_total_ram = 0;
 
// EAX stuff
sound_env Game_sound_env;
//sound_env Game_default_sound_env = {SND_ENV_AUDITORIUM, 0.25f, 0.35f, 3.0f};
sound_env Game_default_sound_env = {SND_ENV_GENERIC, 0.2F,0.2F,1.0F};

int Game_sound_env_update_timestamp;

// WARPIN CRAP BEGIN --------------------------------------------------------------------------------------------


// WARPIN CRAP END --------------------------------------------------------------------------------------------

fs_builtin_mission *game_find_builtin_mission(char *filename)
{
	int idx;

	// look through all existing builtin missions
	for(idx=0; idx<Game_builtin_mission_count; idx++){
		if(!stricmp(Game_builtin_mission_list[idx].filename, filename)){
			return &Game_builtin_mission_list[idx];
		}
	}

	// didn't find it
	return NULL;
}

int game_get_default_skill_level()
{
	return DEFAULT_SKILL_LEVEL;
}


float Gf_critical = -1.0f;					// framerate we should be above on the average for this mission
float Gf_critical_time = 0.0f;			// how much time we've been at the critical framerate

void game_framerate_check_init()
{
	// zero critical time
	Gf_critical_time = 0.0f;
		
	// nebula missions
	if(The_mission.flags & MISSION_FLAG_FULLNEB){
		// if this is a glide card
		Gf_critical = 15.0f;			
	} else {
		Gf_critical = 25.0f;
	}
}

extern float Framerate;
void game_framerate_check()
{
	int y_start = 100;
	
	// if the current framerate is above the critical level, add frametime
	if(Framerate >= Gf_critical){
		Gf_critical_time += flFrametime;
	}	

	if(!Show_framerate){
		return;
	}

	// display if we're above the critical framerate
	if(Framerate < Gf_critical){
		gr_set_color_fast(&Color_bright_red);
		gr_string(200, y_start, "Framerate warning");

		y_start += 10;
	}

	// display our current pct of good frametime
	if(f2fl(Missiontime) >= 0.0f){
		float pct = (Gf_critical_time / f2fl(Missiontime)) * 100.0f;

		if(pct >= 85.0f){
			gr_set_color_fast(&Color_bright_green);
		} else {
			gr_set_color_fast(&Color_bright_red);
		}

		gr_printf(200, y_start, "%d%%", (int)pct);

		y_start += 10;
	}
}


void game_level_close()
{
	// De-Initialize the game subsystems
	message_mission_shutdown();
	event_music_level_close();
	game_stop_looped_sounds();
	snd_stop_all();
	obj_snd_level_close();					// uninit object-linked persistant sounds
	gamesnd_unload_gameplay_sounds();	// unload gameplay sounds from memory
	anim_level_close();						// stop and clean up any anim instances
	shockwave_level_close();
	fireball_level_close();	
	shield_hit_close();
	mission_event_shutdown();
	asteroid_level_close();
	model_cache_reset();						// Reset/free all the model caching stuff
	flak_level_close();						// unload flak stuff
	neb2_level_close();						// shutdown gaseous nebula stuff
	ct_level_close();
	beam_level_close();
	mflash_level_close();

	audiostream_unpause_all();
}


// intializes game stuff and loads the mission.  Returns 0 on failure, 1 on success
// input: seed =>	DEFAULT PARAMETER (value -1).  Only set by demo playback code.
void game_level_init(int seed)
{
	// seed the random number generator
	if ( seed == -1 ) {
		// if no seed was passed, seed the generator either from the time value, or from the
		// netgame security flags -- ensures that all players in multiplayer game will have the
		// same randon number sequence (with static rand functions)
		if ( Game_mode & GM_NORMAL ) {
			Game_level_seed = time(NULL);
		} else {
			Game_level_seed = Netgame.security;
		}
	} else {
		// mwa 9/17/98 -- maybe this assert isn't needed????
		Assert( !(Game_mode & GM_MULTIPLAYER) );
		Game_level_seed = seed;
	}
	srand( Game_level_seed );

	// semirand function needs to get re-initted every time in multiplayer
	if ( Game_mode & GM_MULTIPLAYER ){
		init_semirand();
	}

	Framecount = 0;

	Key_normal_game = (Game_mode & GM_NORMAL);
	Cheats_enabled = 0;

	// Initialize the game subsystems
//	timestamp_reset();			// Must be inited before everything else
	if(!Is_standalone){
		game_reset_time();			// resets time, and resets saved time too
	}
	obj_init();						// Must be inited before the other systems
	model_free_all();				// Free all existing models
	mission_brief_common_init();		// Free all existing briefing/debriefing text
	weapon_level_init();
	ai_level_init();				//	Call this before ship_init() because it reads ai.tbl.
	ship_level_init();
	player_level_init();	
	shipfx_flash_init();			// Init the ship gun flash system.
	particle_init();				// Reset the particle system
	fireball_init();
	debris_init();
	cmeasure_init();
	shield_hit_init();				//	Initialize system for showing shield hits
	radar_mission_init();
	mission_init_goals();
	mission_log_init();
	messages_init();
	obj_snd_level_init();					// init object-linked persistant sounds
	anim_level_init();
	shockwave_level_init();
	afterburner_level_init();
	scoring_level_init( &Player->stats );
	key_level_init();
	asteroid_level_init();
	control_config_clear_used_status();
	collide_ship_ship_sounds_init();
	Missiontime = 0;
	Pre_player_entry = 1;			//	Means the player has not yet entered.
	Entry_delay_time = 0;			//	Could get overwritten in mission read.
	fireball_preload();				//	page in warphole bitmaps
	observer_init();
	flak_level_init();				// initialize flak - bitmaps, etc
	ct_level_init();					// initialize ships contrails, etc
	awacs_level_init();				// initialize AWACS
	beam_level_init();				// initialize beam weapons
	mflash_level_init();
	ssm_level_init();	
	supernova_level_init();

	// multiplayer dogfight hack
	dogfight_blown = 0;

	shipfx_engine_wash_level_init();

	nebl_level_init();

	Last_view_target = NULL;

	Game_no_clear = 0;

	// campaign wasn't ended
	Campaign_ended_in_mission = 0;
}

// called when a mission is over -- does server specific stuff.
void freespace_stop_mission()
{	
	game_level_close();
	Game_mode &= ~GM_IN_MISSION;
}

// called at frame interval to process networking stuff
void game_do_networking()
{
	Assert( Net_player != NULL );
	if (!(Game_mode & GM_MULTIPLAYER)){
		return;
	}

	// see if this player should be reading/writing data.  Bit is set when at join
	// screen onward until quits back to main menu.
	if ( !(Net_player->flags & NETINFO_FLAG_DO_NETWORKING) ){
		return;
	}

	if(gameseq_get_state()!=GS_STATE_MULTI_PAUSED){
		multi_do_frame();
	} else {
		multi_pause_do_frame();
	}	
}


// Loads the best palette for this level, based
// on nebula color and hud color.  You could just call palette_load_table with
// the appropriate filename, but who wants to do that.
void game_load_palette()
{
	char palette_filename[1024];

	// We only use 3 hud colors right now
	// Assert( HUD_config.color >= 0 );
	// Assert( HUD_config.color <= 2 );

	Assert( Mission_palette >= 0 );
	Assert( Mission_palette <= 98 );

	// if ( The_mission.flags & MISSION_FLAG_SUBSPACE )	{
		strcpy( palette_filename, NOX("gamepalette-subspace") );
	// } else {
		// sprintf( palette_filename, NOX("gamepalette%d-%02d"), HUD_config.color+1, Mission_palette+1 );
	// }

	mprintf(( "Loading palette %s\n", palette_filename ));

	// palette_load_table(palette_filename);
}

void game_post_level_init()
{
	// Stuff which gets called after mission is loaded.  Because player isn't created until
	// after mission loads, some things must get initted after the level loads

	model_level_post_init();

	// for each player
	const int lNumLocalPlayers = GetNumLocalPlayers();

	for(int i = 0; i < lNumLocalPlayers; i++)
	{
		SetPlayerShip(i);

 		HUD_init();
		hud_setup_escort_list();
		mission_hotkey_set_defaults();	// set up the default hotkeys (from mission file)
	}
	SetPlayerShip(0);

	stars_level_init();	
	neb2_level_init();		

#ifndef NDEBUG
	game_event_debug_init();
#endif

	training_mission_init();
	asteroid_create_all();
	
	game_framerate_check_init();
}

int Game_loading_callback_inited = 0;

int Game_loading_background = -1;
int Game_loading_time;
AnimInstanceGroup g_loadingBarAnim;

static int Game_loading_ani_coords[GR_NUM_RESOLUTIONS][2] = {
	{
		63, 316  // GR_640
	},
	{
		101, 505	// GR_1024
	}
};

void game_loading_callback()
{	
	game_do_networking();

	Assert( Game_loading_callback_inited==1 );

	int time = timer_get_seconds();

	if(Game_loading_time == -1)
	{
		Game_loading_time = time;
	}

	int framenum = (time - Game_loading_time);
	
	if ( Game_loading_background > -1 )	{
		gr_set_bitmap( Game_loading_background );
		gr_bitmap(0,0);
	}

	int frame = (framenum / 10000) % g_loadingBarAnim.GetNumFrames();
	frame = max(frame, 0);
	gr_set_bitmap( g_loadingBarAnim.GetBitmap(frame), GR_ALPHABLEND_NONE, GR_BITBLT_MODE_NORMAL);
	gr_bitmap(Game_loading_ani_coords[gr_screen.res][0],Game_loading_ani_coords[gr_screen.res][1]);

	gr_flip();
}

void game_loading_callback_init()
{
	Assert( Game_loading_callback_inited==0 );

	bitmap_entry::SetLockEnabledForBitmapsCreatedFromNow(true);
	{
		Game_loading_background = bm_load(Game_loading_bground_fname[gr_screen.res]);
		g_loadingBarAnim.Load(Game_loading_ani_fname[gr_screen.res]);
	}
	bitmap_entry::SetLockEnabledForBitmapsCreatedFromNow(false);

	Game_loading_time = -1;

	Game_loading_callback_inited = 1;
	g_MouseController.SetMouseHidden(1);
	game_busy_callback( game_loading_callback);	
}

void game_loading_callback_close()
{
	Assert( Game_loading_callback_inited==1 );
	
	//int real_count = 
	game_busy_callback( NULL );
	g_MouseController.SetMouseHidden(0);

	Game_loading_callback_inited = 0;

	g_loadingBarAnim.Unload();
	bm_release( Game_loading_background );
	Game_loading_background = -1;

	common_free_interface_palette();		// restore game palette


	gr_set_font( FONT1 );
}

// Update the sound environment (ie change EAX settings based on proximity to large ships)
//
void game_maybe_update_sound_environment()
{
	// do nothing for now
}

// Assign the sound environment for the game, based on the current mission
//
void game_assign_sound_environment()
{
	/*
	if (The_mission.flags & MISSION_FLAG_SUBSPACE) {
		Game_sound_env.id = SND_ENV_DRUGGED;
		Game_sound_env.volume = 0.800f;
		Game_sound_env.damping = 1.188f;
		Game_sound_env.decay = 6.392f;
#ifndef FS2_DEMO
	} else if (Num_asteroids > 30) {
		Game_sound_env.id = SND_ENV_AUDITORIUM;
		Game_sound_env.volume = 0.603f;
		Game_sound_env.damping = 0.5f;
		Game_sound_env.decay = 4.279f;
#endif
	} else {
		Game_sound_env = Game_default_sound_env;
	}
	*/

	Game_sound_env = Game_default_sound_env;
	Game_sound_env_update_timestamp = timestamp(1);
}

class LoadGame : public Thread
{
public:
	LoadGame() : Thread() {}
	~LoadGame() {}

	bool IsRunning() 
	{
		return canRun();
	}
private:
	void run();

};

void LoadGame::run()
{
	mprintf(( "=================== STARTING LEVEL DATA LOAD ==================\n" )); 
	
	event_music_level_init();	// preloads the first 2 seconds for each event music track

	gamesnd_unload_interface_sounds();		// unload interface sounds from memory

	gamesnd_preload_common_sounds();			// load in sounds that are expected to play

	ship_assign_sound_all();	// assign engine sounds to ships
	game_assign_sound_environment();	 // assign the sound environment for this mission

	// call function in missionparse.cpp to fixup player/ai stuff.
	mission_parse_fixup_players();

	// Load in all the bitmaps for this level
	level_page_in();
}

// function which gets called before actually entering the mission.  It is broken down into a funciton
// since it will get called in one place from a single player game and from another place for
// a multiplayer game
void freespace_mission_load_stuff()
{
	// called if we're not on a freespace dedicated (non rendering, no pilot) server
	// IE : we _don't_ want to load any sounds or bitmap/texture info on this machine.
	if(!(Game_mode & GM_STANDALONE_SERVER)){	
	
		game_loading_callback_init();

		LoadGame newLoad;
		
		if (newLoad.create())
		{
			newLoad.start();
		}

		while(newLoad.IsRunning())
		{
			// Use os_poll for now but in the end it should work properly
			// and pass control back to the root systems
			os_poll(); 
			const int sleepInThreadWait = 10;
			Sleep(sleepInThreadWait);
			game_busy();
		}	

		const int sleepBeforeJoin = 100;
		Sleep(sleepBeforeJoin);
		newLoad.join();
		game_loading_callback_close();	

	} 
	// the only thing we need to call on the standalone for now.
	else {
		// call function in missionparse.cpp to fixup player/ai stuff.
		mission_parse_fixup_players();

		// Load in all the bitmaps for this level
		level_page_in();
	}
}

uint load_gl_init;
uint load_mission_load;
uint load_post_level_init;
uint load_mission_stuff;

// tells the server to load the mission and initialize structures
int game_start_mission()
{	
	mprintf(( "=================== STARTING LEVEL LOAD ==================\n" ));
	
	load_gl_init = time(NULL);
	game_level_init();
	load_gl_init = time(NULL) - load_gl_init;
	
	if (Game_mode & GM_MULTIPLAYER) {
		Player->flags |= PLAYER_FLAGS_IS_MULTI;

		// clear multiplayer stats
		init_multiplayer_stats();
	}

	load_mission_load = time(NULL);
	if (mission_load()) {
		if ( !(Game_mode & GM_MULTIPLAYER) ) {
			popup(PF_BODY_BIG, 1, POPUP_OK, XSTR( "Attempt to load the mission failed", 169));
			gameseq_post_event(GS_EVENT_MAIN_MENU);
		} else {
			multi_quit_game(PROMPT_NONE, MULTI_END_NOTIFY_NONE, MULTI_END_ERROR_LOAD_FAIL);
		}

		return 0;
	}
	load_mission_load = time(NULL) - load_mission_load;

	// If this is a red alert mission in campaign mode, bash wingman status
	if ( (Game_mode & GM_CAMPAIGN_MODE) && red_alert_mission() ) {
		red_alert_bash_wingman_status();
	}

	// the standalone server in multiplayer doesn't do any rendering, so we will not even bother loading the palette
	if ( !(Game_mode & GM_STANDALONE_SERVER) ) {
		mprintf(( "=================== LOADING GAME PALETTE ================\n" ));
		// game_load_palette();
	}

	load_post_level_init = time(NULL);
	game_post_level_init();
	load_post_level_init = time(NULL) - load_post_level_init;

	#ifndef NDEBUG
	{
		void Do_model_timings_test();
		Do_model_timings_test();	
	}
	#endif

	load_mission_stuff = time(NULL);
	freespace_mission_load_stuff();
	load_mission_stuff = time(NULL) - load_mission_stuff;

	return 1;
}

int Interface_framerate = 0;
#ifndef NDEBUG

//DCF_BOOL( mouse_control, Use_mouse_to_fly )
DCF_BOOL( show_framerate, Show_framerate )
DCF_BOOL( show_target_debug_info, Show_target_debug_info )
DCF_BOOL( show_target_weapons, Show_target_weapons )
DCF_BOOL( lead_target_cheat, Players[Player_num].lead_target_cheat )
DCF_BOOL( sound, Sound_enabled )
DCF_BOOL( zbuffer, game_zbuffer )
DCF_BOOL( shield_system, New_shield_system )
DCF_BOOL( show_shield_mesh, Show_shield_mesh)
DCF_BOOL( player_attacking, Player_attacking_enabled )
DCF_BOOL( show_waypoints, Show_waypoints )
DCF_BOOL( show_area_effect, Show_area_effect )
DCF_BOOL( show_net_stats, Show_net_stats )
DCF_BOOL( log, Log_debug_output_to_file )
DCF_BOOL( training_msg_method, Training_msg_method )
DCF_BOOL( show_player_pos, Show_player_pos )
DCF_BOOL(i_framerate, Interface_framerate )

DCF(show_mem,"Toggles showing mem usage")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	Show_mem = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) Show_mem = 0;	
		else if ( Dc_arg_type & ARG_NONE ) Show_mem ^= 1;	

		if ( Show_mem )	{
			Show_cpu = 0;
		}
	}	
	if ( Dc_help )	dc_printf( "Usage: Show_mem\nSets show_mem to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	{
		dc_printf( "Show_mem is %s\n", (Show_mem?"TRUE":"FALSE") );	
		dc_printf( "Show_cpu is %s\n", (Show_cpu?"TRUE":"FALSE") );	
	}
}

DCF(show_cpu,"Toggles showing cpu usage")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	Show_cpu = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) Show_cpu = 0;	
		else if ( Dc_arg_type & ARG_NONE ) Show_cpu ^= 1;	

		if ( Show_cpu )	{
			Show_mem = 0;
		}
	}	
	if ( Dc_help )	dc_printf( "Usage: Show_cpu\nSets show_cpu to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	{
		dc_printf( "Show_mem is %s\n", (Show_mem?"TRUE":"FALSE") );	
		dc_printf( "Show_cpu is %s\n", (Show_cpu?"TRUE":"FALSE") );	

	}
}

#else

	// AL 4-8-98: always allow players to display their framerate

	#ifdef FS2_DEMO
		DCF_BOOL( show_framerate, Show_framerate )
	#endif

#endif	// NDEBUG

			int Game_init_seed;

DCF(use_joy_mouse,"Makes joystick move mouse cursor")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	Use_joy_mouse = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) Use_joy_mouse = 0;	
		else if ( Dc_arg_type & ARG_NONE ) Use_joy_mouse ^= 1;	
	}	
	if ( Dc_help )	dc_printf( "Usage: use_joy_mouse [bool]\nSets use_joy_mouse to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	dc_printf( "use_joy_mouse is %s\n", (Use_joy_mouse?"TRUE":"FALSE") );	

	os_config_write_uint( NULL, NOX("JoystickMovesCursor"), Use_joy_mouse );
}

int	Framerate_delay = 0;

float Freespace_gamma = 1.0f;

DCF(gamma,"Sets Gamma factor")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_FLOAT|ARG_NONE);
		if ( Dc_arg_type & ARG_FLOAT )	{
			Freespace_gamma = Dc_arg_float;
		} else {
			dc_printf( "Gamma reset to 1.0f\n" );
			Freespace_gamma = 1.0f;
		}
		if ( Freespace_gamma < 0.1f )	{
			Freespace_gamma = 0.1f;
		} else if ( Freespace_gamma > 5.0f )	{
			Freespace_gamma = 5.0f;
		}
		gr_set_gamma(Freespace_gamma);

		char tmp_gamma_string[32];
		sprintf( tmp_gamma_string, NOX("%.2f"), Freespace_gamma );
		os_config_write_string( NULL, NOX("Gamma"), tmp_gamma_string );
	}

	if ( Dc_help )	{
		dc_printf( "Usage: gamma <float>\n" );
		dc_printf( "Sets gamma in range 1-3, no argument resets to default 1.2\n" );
		Dc_status = 0;	// don't print status if help is printed.  Too messy.
	}

	if ( Dc_status )	{
		dc_printf( "Gamma = %.2f\n", Freespace_gamma );
	}
}

void game_init()
{
	char *ptr;

	Game_current_mission_filename[0] = 0;

	// seed the random number generator
	Game_init_seed = time(NULL);
	srand( Game_init_seed );

	Framerate_delay = 0;

	#ifndef NDEBUG
	load_filter_info();
	#endif

	extern void bm_init();
	bm_init();

	// encrypt stuff
	encrypt_init();

	// Initialize the timer before the os
	timer_init();

	int s1, e1;
	// int s2, e2;

	char whee[1024];
	GetCurrentDirectoryA(1024, whee);
	strcat(whee, "\\");
	strcat(whee, EXE_FNAME);

	//Initialize the libraries
	s1 = timer_get_milliseconds();
	if(cfile_init(whee, Game_CDROM_dir)){			// initialize before calling any cfopen stuff!!!
		exit(1);
	}		
	e1 = timer_get_milliseconds();

	// time a bunch of cfopens	
	/*
	s2 = timer_get_milliseconds();	
	CFILE *whee;
	for(int idx=0; idx<10000; idx++){
		whee = cfopen("capital01.pof", "rb", CFILE_NORMAL, CF_TYPE_MODELS);
		if(whee != NULL){
			cfclose(whee);
		}
		whee = NULL;
		//cf_exist("capital01.pof", CF_TYPE_MODELS);
	}
	e2 = timer_get_milliseconds();	
	*/

	if (Is_standalone) {
#if !defined(FS2_UE)
		std_init_standalone();
#endif
	} else {		
#if defined(FS2_UE)
		os_init("", "");
#else
		os_init( Osreg_class_name, Osreg_app_name );
		os_set_title(Osreg_title);
#endif
	}

	// initialize localization module. Make sure this is down AFTER initialzing OS.
//	int t1 = timer_get_milliseconds();
	lcl_init();	
	lcl_xstr_init();
//	mprintf(("LCL_INIT() TOOK %d MS\n", timer_get_milliseconds()-t1));

	// verify that he has a valid ships.tbl (will Game_ships_tbl_valid if so)
	verify_ships_tbl();

	// verify that he has a valid weapons.tbl
	verify_weapons_tbl();

	Use_joy_mouse = 0;		//os_config_read_uint( NULL, NOX("JoystickMovesCursor"), 1 );

#ifndef FS2_DEMO
	Asteroids_enabled = 1;		
#endif

/////////////////////////////
// SOUND INIT START
/////////////////////////////

	int use_a3d = 0;
	int use_eax = 0;
/*
	ptr = os_config_read_string(NULL, NOX("Soundcard"), NULL);
	mprintf(("soundcard = %s\n", ptr ? ptr : "<nothing>"));
	if (ptr) {
		if (!stricmp(ptr, NOX("no sound"))) {
			Cmdline_freespace_no_sound = 1;

		} else if (!stricmp(ptr, NOX("Aureal A3D"))) {
			use_a3d = 1;
		} else if (!stricmp(ptr, NOX("EAX"))) {
			use_eax = 1;
		}
	}
*/
	if (!Is_standalone) {
		const int SoundInitResult = snd_init(use_a3d, use_eax);
		Assert(SoundInitResult);
	}

#if !defined(FS2_UE)
	g_TextToSpeech.Init();
#endif
	if(Cmdline_voicer == 1)
	{
		//const bool voiceRectOn = 
		g_VoiceRecognition.Init(os_get_voice_recognition_event_id(), GRAMMARID1, IDR_CMD_CFG);
//		Assert(voiceRectOn);
	}

/////////////////////////////
// SOUND INIT END
/////////////////////////////
/*	
	ptr = os_config_read_string(NULL, NOX("Videocard"), NULL);	
	if (ptr == NULL) {
		MessageBox((HWND)os_get_window(), XSTR("Please configure your system in the Launcher before running FS2.\n\n The Launcher will now be started!", 1446), XSTR("Attention!", 1447), MB_OK);

		// fire up the UpdateLauncher executable
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		memset( &si, 0, sizeof(STARTUPINFO) );
		si.cb = sizeof(si);

		BOOL ret = CreateProcess(	LAUNCHER_FNAME,	// pointer to name of executable module 
									NULL,							// pointer to command line string
									NULL,							// pointer to process security attributes 
									NULL,							// pointer to thread security attributes 
									FALSE,							// handle inheritance flag 
									CREATE_DEFAULT_ERROR_MODE,		// creation flags 
									NULL,							// pointer to new environment block 
									NULL,	// pointer to current directory name 
									&si,	// pointer to STARTUPINFO 
									&pi 	// pointer to PROCESS_INFORMATION  
								);			

		// If the Launcher could not be started up, let the user know
		if (!ret) {
			MessageBox((HWND)os_get_window(), XSTR("The Launcher could not be restarted.", 1450), XSTR("Error", 1451), MB_OK);
		}
		exit(1);
	}

	if(!Is_standalone){
		
		if(!stricmp(ptr, "Aucune acc�l�ration 3D") || !stricmp(ptr, "Keine 3D-Beschleunigerkarte") || !stricmp(ptr, "No 3D acceleration"			MessageBox((HWND)os_get_window(), XSTR("Warning, Freespace 2 requires Glide or Direct3D hardware accleration. You will not be able to run Freespace 2 without it.", 1448), XSTR("Warning", 1449), MB_OK);		
			exit(1);
		}
	}
*/
	// check for hi res pack file 
	int has_sparky_hi = 0;

	// check if sparky_hi exists -- access mode 0 means does file exist
	if ( _access("sparky_hi_fs2.vp", 0) == 0) {
		has_sparky_hi = 1;
	} else {
		char dir[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH,dir);
		mprintf(("No sparky_hi_fs2.vp in directory %s\n", dir));
	}

	int lDefaultMode = GR_DIRECT3D5;
	if(Cmdline_forceDirect3d9)
	{
		lDefaultMode = GR_DIRECT3D9;
	}
	if(Cmdline_forceDirect3d5)
	{
		lDefaultMode = GR_DIRECT3D5;
	}
	if (Cmdline_forceDummy)
	{
		lDefaultMode = GR_DUMMY;
	}

#if defined(FS2_UE)
	lDefaultMode = GR_DUMMY;
#endif

	assert(has_sparky_hi);
	int initResult = gr_init(GR_1024, lDefaultMode, 32);
	assert(initResult == 0);
	
	extern int Gr_inited;
	if(!Gr_inited){

#if !defined(FS2_UE)

		extern char Device_init_error[512];		
		MessageBoxA( NULL, Device_init_error, "Error intializing Direct3D", MB_OK|MB_TASKMODAL|MB_SETFOREGROUND );
#endif
		exit(1);
		return;
	}

	// Set the gamma
	//ptr = os_config_read_string(NULL,NOX("Gamma"),NOX("1.80"));
	Freespace_gamma = 1.80f;// (float)atof(ptr);
/*
	if ( Freespace_gamma < 0.1f )	{
		Freespace_gamma = 0.1f;
	} else if ( Freespace_gamma > 5.0f )	{
		Freespace_gamma = 5.0f;
	}
	char tmp_gamma_string[32];
	sprintf( tmp_gamma_string, NOX("%.2f"), Freespace_gamma );
	os_config_write_string( NULL, NOX("Gamma"), tmp_gamma_string );
*/
	gr_set_gamma(Freespace_gamma);

#if defined(FS2_DEMO) || defined(OEM_BUILD)
	// add title screen
	if(!Is_standalone){
		display_title_screen();
	}
#endif
	
	// attempt to load up master tracker registry info (login and password)
	Multi_tracker_id = -1;		

	// pxo login and password
	ptr = os_config_read_string(NOX("PXO"),NOX("Login"),NULL);
	if(ptr == NULL){
		nprintf(("Network","Error reading in PXO login data\n"));
		strcpy(Multi_tracker_login,"");
	} else {		
		strcpy(Multi_tracker_login,ptr);
	}
	ptr = os_config_read_string(NOX("PXO"),NOX("Password"),NULL);
	if(ptr == NULL){		
		nprintf(("Network","Error reading PXO password\n"));
		strcpy(Multi_tracker_passwd,"");
	} else {		
		strcpy(Multi_tracker_passwd,ptr);
	}	

	// pxo squad name and password
	ptr = os_config_read_string(NOX("PXO"),NOX("SquadName"),NULL);
	if(ptr == NULL){
		nprintf(("Network","Error reading in PXO squad name\n"));
		strcpy(Multi_tracker_squad_name, "");
	} else {		
		strcpy(Multi_tracker_squad_name, ptr);
	}

	// load non-darkening pixel defs
	palman_load_pixels();

	// hud shield icon stuff
	hud_shield_game_init();

	control_config_common_init();				// sets up localization stuff in the control config
	parse_rank_tbl();
#ifndef FS2_DEMO
	parse_medal_tbl();
#endif
	cutscene_init();
	key_init();

	GetPrimaryPad()->Init(0, 0);
	GetSecondryPad()->Init(1, 1);

	g_MouseController.Init(Cmdline_360pad ? InputController::kIN_360PAD : InputController::kIN_MOUSE);

	// standalone's don't use hte joystick and it seems to sometimes cause them to not get shutdown properly
	if(!Is_standalone){
		joy_init(Cmdline_360pad ? InputController::kIN_360PAD : InputController::kIN_JOYSTICK);
	}

	gamesnd_parse_soundstbl();
	radar_init();
	gameseq_init();
	multi_init();	

	player_controls_init();
	model_init();	

	//if(!Is_standalone){
		event_music_init();
	//}	

	obj_init();	
	mflash_game_init();	
	weapon_init();	
	ai_init();		
	ship_init();						// read in ships.tbl	
	player_init();	
	mission_campaign_init();		// load in the default campaign	
	anim_init();
//	navmap_init();						// init the navigation map system
	context_help_init();			
	techroom_intel_init();			// parse species.tbl, load intel info	
	// initialize psnet
	psnet_init( Multi_options_g.protocol, Multi_options_g.port );						// initialize the networking code		
	init_animating_pointer();	
	asteroid_init();
	mission_brief_common_init();	// Mark all the briefing structures as empty.		
	gr_font_init();					// loads up all fonts		

	neb2_init();						// fullneb stuff
	nebl_init();
	stars_init();
	ssm_init();	
	player_tips_init();				// helpful tips
	beam_init();
	
	// load the list of pilot pic filenames (for barracks and pilot select popup quick reference)
	pilot_load_pic_list();	
	pilot_load_squad_pic_list();

	load_animating_pointer(NOX("cursor"), 0, 0);	

	// initialize alpha colors
	alpha_colors_init();	

	Viewer_mode = 0;

	timeBeginPeriod(1);	

	nprintf(("General", "Ships.tbl is : %s\n", Game_ships_tbl_valid ? "VALID" : "INVALID!!!!"));
	nprintf(("General", "Weapons.tbl is : %s\n", Game_weapons_tbl_valid ? "VALID" : "INVALID!!!!"));

	mprintf(("cfile_init() took %d\n", e1 - s1));
	// mprintf(("1000 cfopens() took %d\n", e2 - s2));	
}

MONITOR(NumPolysDrawn);
MONITOR(NumPolys);
MONITOR(NumVerts);
MONITOR(BmpUsed);
MONITOR(BmpNew);

void game_get_framerate()
{	
	char text[128] = "";

	if ( frame_int == -1 )	{
		int i;
		for (i=0; i<FRAME_FILTER; i++ )	{
			frametimes[i] = 0.0f;
		}
		frametotal = 0.0f;
		frame_int = 0;
	}
	frametotal -= frametimes[frame_int];
	frametotal += flFrametime;
	frametimes[frame_int] = flFrametime;
	frame_int = (frame_int + 1 ) % FRAME_FILTER;

	if ( frametotal != 0.0 )	{
		if ( Framecount >= FRAME_FILTER )
			Framerate = FRAME_FILTER / frametotal;
		else
			Framerate = Framecount / frametotal;
		sprintf( text, NOX("FPS: %.1f"), Framerate );
	} else {
		sprintf( text, NOX("FPS: ?") );
	}
	Framecount++;

	if (Show_framerate)	{
		gr_set_color_fast(&HUD_color_debug);
		gr_string( 570, 2, text );
	}
}

void game_show_framerate()
{	
	float	cur_time;

	cur_time = f2fl(timer_get_seconds());
	if (cur_time - Start_time > 30.0f) {
		mprintf(("%i frames executed in %7.3f seconds, %7.3f frames per second.\n", Framecount, cur_time - Start_time, Framecount/(cur_time - Start_time)));
		Start_time += 1000.0f;
	}

	//mprintf(( "%s\n", text ));

#ifndef NDEBUG
	if ( Debug_dump_frames )
		return;
#endif	

	// possibly show control checking info
	control_check_indicate();

//	int bitmaps_used_this_frame, bitmaps_new_this_frame;
//	bm_get_frame_usage(&bitmaps_used_this_frame,&bitmaps_new_this_frame);
//	MONITOR_INC(BmpUsed, bitmaps_used_this_frame);
// MONITOR_INC(BmpNew, bitmaps_new_this_frame);

	if(Cmdline_fps)
	{
		if(Framerate > 30)
		{
			gr_set_color_fast(&HUD_color_debug);
		}
		else
		{
			gr_set_color_fast(&Color_bright_red);
		}
		gr_printf( 100, 100, NOX("FPS: %f\n"), Framerate );
		gr_printf( 100, 120, NOX("Time: %f\n"), cur_time );
	}
	

#ifndef NDEBUG
	if ( Show_cpu == 1 ) {
		
		int sx,sy,dy;
		sx = 530;
		sy = 15;
		dy = gr_get_font_height() + 1;

		gr_set_color_fast(&HUD_color_debug);

		{
			extern int D3D_textures_in;
			extern int D3D_textures_in_frame;

			gr_printf( sx, sy, NOX("VRAM: %d KB\n"), (D3D_textures_in)/1024 );
			sy += dy;
			gr_printf( sx, sy, NOX("VRAM: +%d KB\n"), (D3D_textures_in_frame)/1024 );
			sy += dy;
		}
//		gr_printf( sx, sy, "BPP: %d", gr_screen.bits_per_pixel );
//		sy += dy;
		gr_printf( sx, sy, NOX("DMA: %s"), transfer_text );
		sy += dy;
		gr_printf( sx, sy, NOX("POLYP: %d"), modelstats_num_polys );
		sy += dy;
		gr_printf( sx, sy, NOX("POLYD: %d"), modelstats_num_polys_drawn );
		sy += dy;
		gr_printf( sx, sy, NOX("VERTS: %d"), modelstats_num_verts );
		sy += dy;

		{

			extern int Num_pairs;		// Number of object pairs that were checked.
			gr_printf( sx, sy, NOX("PAIRS: %d"), Num_pairs );
			sy += dy;

			extern int Num_pairs_checked;	// What percent of object pairs were checked.
			gr_printf( sx, sy, NOX("FVI: %d"), Num_pairs_checked );
			sy += dy;
			Num_pairs_checked = 0;

		}

		gr_printf( sx, sy, NOX("Snds: %d"), snd_num_playing() );
		sy += dy;

		if ( Timing_total > 0.01f )	{
			gr_printf(  sx, sy, NOX("CLEAR: %.0f%%"), Timing_clear*100.0f/Timing_total );
			sy += dy;
			gr_printf( sx, sy, NOX("REND2D: %.0f%%"), Timing_render2*100.0f/Timing_total );
			sy += dy;
			gr_printf( sx, sy, NOX("REND3D: %.0f%%"), Timing_render3*100.0f/Timing_total );
			sy += dy;
			gr_printf( sx, sy, NOX("FLIP: %.0f%%"), Timing_flip*100.0f/Timing_total );
			sy += dy;
			gr_printf( sx, sy, NOX("GAME: %.0f%%"), (Timing_total-(Timing_render2+Timing_render3+Timing_flip+Timing_clear))*100.0f/Timing_total );
			sy += dy;
		}
	}
	 	
	if ( Show_mem  ) {
		
		int sx,sy,dy;
		sx = 530;
		sy = 15;
		dy = gr_get_font_height() + 1;

		gr_set_color_fast(&HUD_color_debug);

		{
			extern int TotalRam;
			gr_printf( sx, sy, NOX("DYN: %d KB\n"), TotalRam/1024 );
			sy += dy;
		}	

		{
			extern int Model_ram;
			gr_printf( sx, sy, NOX("POF: %d KB\n"), Model_ram/1024 );
			sy += dy;
		}	

		gr_printf( sx, sy, NOX("BMP: %d KB\n"), bm_texture_ram/1024 );
		sy += dy;
		gr_printf( sx, sy, NOX("S-SRAM: %d KB\n"), Snd_sram/1024 );		// mem used to store game sound
		sy += dy;
		gr_printf( sx, sy, NOX("S-HRAM: %d KB\n"), Snd_hram/1024 );		// mem used to store game sound
		sy += dy;
		{
			extern int D3D_textures_in;
			gr_printf( sx, sy, NOX("VRAM: %d KB\n"), (D3D_textures_in)/1024 );
			sy += dy;
		}
	}


	if ( Show_player_pos ) {
		int sx, sy;
		sx = 320;
		sy = 100;
		gr_printf(sx, sy, NOX("Player Pos: (%d,%d,%d)"), fl2i(Player_obj->pos.x), fl2i(Player_obj->pos.y), fl2i(Player_obj->pos.z));
	}

	MONITOR_INC(NumPolys, modelstats_num_polys);
	MONITOR_INC(NumPolysDrawn, modelstats_num_polys_drawn );
	MONITOR_INC(NumVerts, modelstats_num_verts );

	modelstats_num_polys = 0;
	modelstats_num_polys_drawn = 0;
	modelstats_num_verts = 0;
	modelstats_num_sortnorms = 0;
#endif
}

void game_show_standalone_framerate()
{
	float frame_rate=30.0f;
	if ( frame_int == -1 )	{
		int i;
		for (i=0; i<FRAME_FILTER; i++ )	{
			frametimes[i] = 0.0f;
		}
		frametotal = 0.0f;
		frame_int = 0;
	}
	frametotal -= frametimes[frame_int];
	frametotal += flFrametime;
	frametimes[frame_int] = flFrametime;
	frame_int = (frame_int + 1 ) % FRAME_FILTER;

	if ( frametotal != 0.0 )	{
		if ( Framecount >= FRAME_FILTER ){
			frame_rate = FRAME_FILTER / frametotal;
		} else {
			frame_rate = Framecount / frametotal;
		}
	}

	gr_set_color_fast(&Color_text_normal);
	gr_printf( 50, 40, "FPS: %.1f", frame_rate );

#if !defined(FS2_UE)
	std_set_standalone_fps(frame_rate);
#endif
	Framecount++;
}

// function to show the time remaining in a mission.  Used only when the end-mission sexpression is used
void game_show_time_left()
{
	int diff;

	// mission_end_time is a global from missionparse.cpp that contains the mission time at which the
	// mission should end (in fixed seconds).  There is code in missionparse.cpp which actually handles
	// checking how much time is left

	if ( Mission_end_time == -1 ){
		return;
	}

	diff = f2i(Mission_end_time - Missiontime);
	// be sure to bash to 0.  diff could be negative on frame that we quit mission
	if ( diff < 0 ){
		diff = 0;
	}

	hud_set_default_color();
	gr_printf( 5, 40, XSTR( "Mission time remaining: %d seconds", 179), diff );
}

//========================================================================================
//=================== NEW DEBUG CONSOLE COMMANDS TO REPLACE OLD DEBUG PAUSE MENU =========
//========================================================================================

#ifndef NDEBUG

DCF(ai_pause,"Pauses ai")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	ai_paused = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) ai_paused = 0;	
		else if ( Dc_arg_type & ARG_NONE ) ai_paused = !ai_paused;	

		if (ai_paused)	{	
			obj_init_all_ships_physics();
		}
	}	
	if ( Dc_help )	dc_printf( "Usage: ai_paused [bool]\nSets ai_paused to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	dc_printf( "ai_paused is %s\n", (ai_paused?"TRUE":"FALSE") );	
}

DCF(single_step,"Single steps the game")
{
	if ( Dc_command )	{	
		dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
		if ( Dc_arg_type & ARG_TRUE )	game_single_step = 1;	
		else if ( Dc_arg_type & ARG_FALSE ) game_single_step = 0;	
		else if ( Dc_arg_type & ARG_NONE ) game_single_step = !game_single_step;	

		last_single_step = 0;	// Make so single step waits a frame before stepping

	}	
	if ( Dc_help )	dc_printf( "Usage: single_step [bool]\nSets single_step to true or false.  If nothing passed, then toggles it.\n" );	
	if ( Dc_status )	dc_printf( "single_step is %s\n", (game_single_step?"TRUE":"FALSE") );	
}

DCF_BOOL(physics_pause, physics_paused)
DCF_BOOL(ai_rendering, Ai_render_debug_flag)
DCF_BOOL(ai_firing, Ai_firing_enabled )

// Create some simple aliases to these commands...
debug_command dc_s("s","shortcut for single_step",dcf_single_step);
debug_command dc_p("p","shortcut for physics_pause", dcf_physics_pause );
debug_command dc_r("r","shortcut for ai_rendering", dcf_ai_rendering );
debug_command dc_f("f","shortcut for ai_firing", dcf_ai_firing);
debug_command dc_a("a","shortcut for ai_pause", dcf_ai_pause);
#endif

//========================================================================================
//========================================================================================


void game_training_pause_do()
{
	int key;

	key = game_check_key();
	if (key > 0){
		gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
	}

	gr_flip();
}


void game_increase_skill_level()
{
	Game_skill_level++;
	if (Game_skill_level >= NUM_SKILL_LEVELS){
		Game_skill_level = 0;
	}
}

int	Player_died_time;

DCF(view, "Sets the percent of the 3d view to render.")
{
	if ( Dc_command ) {
		dc_get_arg(ARG_INT);
		if ( (Dc_arg_int >= 5 ) || (Dc_arg_int <= 100) ) {
			GameSetViewPercent(Dc_arg_int);
		} else {
			dc_printf( "Illegal value for view. (Must be from 5-100) \n\n");
			Dc_help = 1;
		}
	}

	if ( Dc_help ) {
		dc_printf("Usage: view [n]\nwhere n is percent of view to show (5-100).\n");
	}
	
	if ( Dc_status ) {
		dc_printf("View is set to %d%%\n", GameGetViewPercent() );
	}
}

void show_debug_stuff()
{
	int	i;
	int	laser_count = 0, missile_count = 0;

	for (i=0; i<MAX_OBJECTS; i++) {
		if (Objects[i].type == OBJ_WEAPON){
			if (Weapon_info[Weapons[Objects[i].instance].weapon_info_index].subtype == WP_LASER){
				laser_count++;
			} else if (Weapon_info[Weapons[Objects[i].instance].weapon_info_index].subtype == WP_MISSILE){
				missile_count++;
			}
		}
	}

	nprintf(("Mike", "Frame: %i Lasers: %4i, Missiles: %4i\n", Framecount, laser_count, missile_count));
}

extern int Tool_enabled;
int tst = 0;
int tst_time = 0;
int tst_big = 0;
vector tst_pos;
int tst_bitmap = -1;
float tst_x, tst_y;
float tst_offset, tst_offset_total;
int tst_mode;
int tst_stamp;
void game_tst_frame_pre()
{
	// start tst
	if(tst == 3){
		tst = 0;

		// screen position
		vertex v;
		g3_rotate_vertex(&v, &tst_pos);
		g3_project_vertex(&v);	
	
		// offscreen
		if(!((v.sx >= 0) && (v.sx <= gr_screen.max_w) && (v.sy >= 0) && (v.sy <= gr_screen.max_h))){
			return;
		}	

		// big ship? always tst
		if(tst_big){
			// within 3000 meters
			if( vm_vec_dist_quick(&tst_pos, &Eye_position) <= 3000.0f){
				tst = 2;				
			}
		} else {			
			// within 300 meters
			if( (vm_vec_dist_quick(&tst_pos, &Eye_position) <= 300.0f) && ((tst_time == 0) || ((time(NULL) - tst_time) >= 10)) ){
				tst = 2;				
			} 
		}			
	}

}
void game_tst_frame()
{
	int left = 0;

	if(!Tool_enabled){
		return;
	}
	
	// setup tst
	if(tst == 2){		
		tst_time = time(NULL);

		// load the tst bitmap		
		switch((int)frand_range(0.0f, 3.0)){
		case 0:			
			tst_bitmap = bm_load("ig_jim");
			left = 1;
			mprintf(("TST 0\n"));
			break;

		case 1:
			tst_bitmap = bm_load("ig_kan");
			left = 0;
			mprintf(("TST 1\n"));
			break;

		case 2:
			tst_bitmap = bm_load("ig_jim");
			left = 1;
			mprintf(("TST 2\n"));
			break;
			
		default:			
			tst_bitmap = bm_load("ig_kan");
			left = 0;
			mprintf(("TST 3\n"));
			break;
		}

		if(tst_bitmap < 0){
			tst = 0;
			return;
		}		

		// get the tst bitmap dimensions
		int w, h;
		bm_get_info(tst_bitmap, &w, &h);

		// tst y
		tst_y = frand_range(0.0f, (float)gr_screen.max_h - h);

		snd_play(&Snds[SND_VASUDAN_BUP]);

		// tst x and direction
		tst_mode = 0;
		if(left){
			tst_x = (float)-w;
			tst_offset_total = (float)w;
			tst_offset = (float)w;
		} else {
			tst_x = (float)gr_screen.max_w;
			tst_offset_total = (float)-w;
			tst_offset = (float)w;
		}

		tst = 1;
	}

	// run tst
	if(tst == 1){
		float diff = (tst_offset_total / 0.5f) * flFrametime;

		// move the bitmap
		if(tst_mode == 0){
			tst_x += diff;
			
			tst_offset -= fl_abs(diff);
		} else if(tst_mode == 2){
			tst_x -= diff;
			
			tst_offset -= fl_abs(diff);
		}

		// draw the bitmap
		gr_set_bitmap(tst_bitmap);
		gr_bitmap((int)tst_x, (int)tst_y);

		if(tst_mode == 1){
			if(timestamp_elapsed_safe(tst_stamp, 1100)){
				tst_mode = 2;
			}
		} else {
			// if we passed the switch point
			if(tst_offset <= 0.0f){
				// switch modes
				switch(tst_mode){
				case 0:
					tst_mode = 1;
					tst_stamp = timestamp(1000);
					tst_offset = fl_abs(tst_offset_total);
					break;				

				case 2:				
					tst = 0;
					return;
				}
			}				
		}
	}
}
void game_tst_mark(object *objp, ship *shipp)
{
	ship_info *sip;	

	if(!Tool_enabled){
		return;
	}

	// bogus
	if((objp == NULL) || (shipp == NULL) || (shipp->ship_info_index < 0) || (shipp->ship_info_index >= Num_ship_types)){
		return;
	}
	sip = &Ship_info[shipp->ship_info_index];

	// already tst
	if(tst){
		return;
	}

	tst_pos = objp->pos;
	if(sip->flags & (SIF_BIG_SHIP | SIF_HUGE_SHIP)){
		tst_big = 1;
	}
	tst = 3;
}

extern void render_shields();

void player_repair_frame(float frametime)
{
	if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)){
		int idx;
		for(idx=0;idx<MAX_PLAYERS;idx++){
			net_player *np;

			np = &Net_players[idx];

			if(MULTI_CONNECTED(Net_players[idx]) && (Net_player != NULL) && (Net_player->player_id != Net_players[idx].player_id) && (Net_players[idx].player != NULL) && (Net_players[idx].player->objnum >= 0) && (Net_players[idx].player->objnum < MAX_OBJECTS)){

				// don't rearm/repair if the player is dead or dying/departing
				if ( !NETPLAYER_IS_DEAD(np) && !(Ships[Objects[np->player->objnum].instance].flags & (SF_DYING|SF_DEPARTING)) ) {
					ai_do_repair_frame(&Objects[Net_players[idx].player->objnum],&Ai_info[Ships[Objects[Net_players[idx].player->objnum].instance].ai_index],frametime);
				}
			}
		}
	}	
	if ( (Player_obj != NULL) && (Player_obj->type == OBJ_SHIP) && !(Game_mode & GM_STANDALONE_SERVER) && (Player_ship != NULL) && !(Player_ship->flags & SF_DYING) ) {
		ai_do_repair_frame(Player_obj, &Ai_info[Ships[Player_obj->instance].ai_index], frametime);
	}
}


#ifndef NDEBUG
#define NUM_FRAMES_TEST		300
#define NUM_MIXED_SOUNDS	16
void do_timing_test(float flFrametime)
{
	static int framecount = 0;
	static int test_running = 0;
	static float test_time = 0.0f;

	static int snds[NUM_MIXED_SOUNDS];
	int i;

	if ( test_running ) {
		framecount++;
		test_time += flFrametime;
		if ( framecount >= NUM_FRAMES_TEST ) {
			test_running = 0;
			nprintf(("General", "%d frames took %.3f seconds\n", NUM_FRAMES_TEST, test_time));
			for ( i = 0; i < NUM_MIXED_SOUNDS; i++ )
				snd_stop(snds[i]);
		}
	}

	if ( Test_begin == 1 ) {
		framecount = 0;
		test_running = 1;
		test_time = 0.0f;
		Test_begin = 0;

		for ( i = 0; i < NUM_MIXED_SOUNDS; i++ )
			snds[i] = -1;

		// start looping digital sounds
		for ( i = 0; i < NUM_MIXED_SOUNDS; i++ )
			snds[i] = snd_play_looping( &Snds[i], 0.0f, -1, -1);
	}
	

}
#endif

DCF(dcf_fov, "Change the field of view")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_FLOAT|ARG_NONE);
		if ( Dc_arg_type & ARG_NONE )	{
			Viewer_zoom = VIEWER_ZOOM_DEFAULT;
			dc_printf( "Zoom factor reset\n" );
		}
		if ( Dc_arg_type & ARG_FLOAT )	{
			if (Dc_arg_float < 0.25f) {
				Viewer_zoom = 0.25f;
				dc_printf("Zoom factor pinned at 0.25.\n");
			} else if (Dc_arg_float > 1.25f) {
				Viewer_zoom = 1.25f;
				dc_printf("Zoom factor pinned at 1.25.\n");
			} else {
				Viewer_zoom = Dc_arg_float;
			}
		}
	}

	if ( Dc_help )	
		dc_printf( "Usage: fov [factor]\nFactor is the zoom factor btwn .25 and 1.25\nNo parameter resets it to default.\n" );

	if ( Dc_status )				
		dc_printf("Zoom factor set to %6.3f (original = 0.5, John = 0.75)", Viewer_zoom);
}


DCF(framerate_cap, "Sets the framerate cap")
{
	if ( Dc_command ) {
		dc_get_arg(ARG_INT);
		if ( (Dc_arg_int >= 1 ) || (Dc_arg_int <= 120) ) {
			Framerate_cap = Dc_arg_int;
		} else {
			dc_printf( "Illegal value for framerate cap. (Must be from 1-120) \n\n");
			Dc_help = 1;
		}
	}

	if ( Dc_help ) {
		dc_printf("Usage: framerate_cap [n]\nwhere n is the frames per second to cap framerate at.\n");
		dc_printf("If n is 0 or omitted, then the framerate cap is removed\n");
		dc_printf("[n] must be from 1 to 120.\n");
	}
	
	if ( Dc_status ) {
		if ( Framerate_cap )
			dc_printf("Framerate cap is set to %d fps\n", Framerate_cap );
		else
			dc_printf("There is no framerate cap currently active.\n");
	}
}

#define	MIN_DIST_TO_DEAD_CAMERA		50.0f
int Show_viewing_from_self = 0;

void say_view_target()
{
	object	*view_target;

	if ((Viewer_mode & VM_OTHER_SHIP) && (Player_ai->target_objnum != -1))
		view_target = &Objects[Player_ai->target_objnum];
	else
		view_target = Player_obj;

	if (Game_mode & GM_DEAD) {
		if (Player_ai->target_objnum != -1)
			view_target = &Objects[Player_ai->target_objnum];
	}

	if (!(Game_mode & GM_DEAD_DIED) && ((Game_mode & (GM_DEAD_BLEW_UP)) || ((Last_view_target != NULL) && (Last_view_target != view_target)))) {
		if (view_target != Player_obj){

			char *view_target_name = NULL;
			switch(Objects[Player_ai->target_objnum].type) {
			case OBJ_SHIP:
				view_target_name = Ships[Objects[Player_ai->target_objnum].instance].ship_name;
				break;
			case OBJ_WEAPON:
				view_target_name = Weapon_info[Weapons[Objects[Player_ai->target_objnum].instance].weapon_info_index].name;
				Viewer_mode &= ~VM_OTHER_SHIP;
				break;
			case OBJ_JUMP_NODE: {
				char	jump_node_name[128];
				strcpy(jump_node_name, XSTR( "jump node", 184));
				view_target_name = jump_node_name;
				Viewer_mode &= ~VM_OTHER_SHIP;
				break;
				}

			default:
				Int3();
				break;
			}

			if ( view_target_name ) {
				HUD_fixed_printf(0.0f, XSTR( "Viewing %s%s\n", 185), (Viewer_mode & VM_OTHER_SHIP) ? XSTR( "from ", 186) : "", view_target_name);
				Show_viewing_from_self = 1;
			}
		} else {
			if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_OBSERVER) && (Player_obj->type == OBJ_OBSERVER)){
				HUD_fixed_printf(2.0f,XSTR( "Viewing from observer\n", 187));
				Show_viewing_from_self = 1;
			} else {
				if (Show_viewing_from_self)
					HUD_fixed_printf(2.0f, XSTR( "Viewing from self\n", 188));
			}
		}
	}

	Last_view_target = view_target;
}


float Game_hit_x = 0.0f;
float Game_hit_y = 0.0f;

// Reset at the beginning of each frame
void game_whack_reset()
{
	Game_hit_x = 0.0f;
	Game_hit_y = 0.0f;
}

// Apply a 2d whack to the player
void game_whack_apply( float x, float y )
{
	// Do some force feedback
	g_ForceFeedback.PlayDirEffect(x * 80.0f, y * 80.0f);

	// Move the eye 
	Game_hit_x += x;
	Game_hit_y += y;

//	mprintf(( "WHACK = %.1f, %.1f\n", Game_hit_x, Game_hit_y ));
}

#define FF_SCALE	10000
void apply_hud_shake(matrix *eye_orient)
{
	if (Viewer_obj == Player_obj) {
		physics_info	*pi = &Player_obj->phys_info;

		angles	tangles;

		tangles.p = 0.0f;
		tangles.h = 0.0f;
		tangles.b = 0.0f;

		//	Make eye shake due to afterburner
		if ( !timestamp_elapsed(pi->afterburner_decay) ) {			
			int		dtime;

			dtime = timestamp_until(pi->afterburner_decay);
			
			int r1 = myrand();
			int r2 = myrand();
			tangles.p += 0.07f * (float) (r1-RAND_MAX/2)/RAND_MAX * (0.5f - fl_abs(0.5f - (float) dtime/ABURN_DECAY_TIME));
			tangles.h += 0.07f * (float) (r2-RAND_MAX/2)/RAND_MAX * (0.5f - fl_abs(0.5f - (float) dtime/ABURN_DECAY_TIME));
		}

		// Make eye shake due to engine wash
		extern int Wash_on;
		if (Player_obj->type == OBJ_SHIP && (Ships[Player_obj->instance].wash_intensity > 0) && Wash_on ) {
			int r1 = myrand();
			int r2 = myrand();
			tangles.p += 0.07f * Ships[Player_obj->instance].wash_intensity * (float) (r1-RAND_MAX/2)/RAND_MAX;
			tangles.h += 0.07f * Ships[Player_obj->instance].wash_intensity * (float) (r2-RAND_MAX/2)/RAND_MAX;

			// get the   intensity
			float intensity = FF_SCALE * Ships[Player_obj->instance].wash_intensity;

			// vector rand_vec
			vector rand_vec;
			vm_vec_rand_vec_quick(&rand_vec);

			// play the effect
			g_ForceFeedback.PlayDirEffect(intensity*rand_vec.x, intensity*rand_vec.y);
		}

		// make hud shake due to shuddering
		MsgShudderGetAngles lMsg;
		Player_obj->PostAMessage(&lMsg);

		tangles.p += lMsg.GetAngles()->p;
		tangles.h += lMsg.GetAngles()->h;
		tangles.b += lMsg.GetAngles()->b;

		matrix	tm, tm2;
		vm_angles_2_matrix(&tm, &tangles);
		Assert(vm_vec_mag(&tm.fvec) > 0.0f);
		Assert(vm_vec_mag(&tm.rvec) > 0.0f);
		Assert(vm_vec_mag(&tm.uvec) > 0.0f);
		vm_matrix_x_matrix(&tm2, eye_orient, &tm);
		*eye_orient = tm2;
	}
}

extern void compute_slew_matrix(matrix *orient, angles *a);	// TODO: move code to proper place and extern in header file

//	Player's velocity just before he blew up.  Used to keep camera target moving.
vector	Dead_player_last_vel(1.0f, 1.0f, 1.0f);

//	Set eye_pos and eye_orient based on view mode.
void game_render_frame_setup(vector *eye_pos, matrix *eye_orient, int lViewer_mode)
{
	vector	eye_dir;

	static int last_Viewer_mode = 0;
	static int last_Game_mode = 0;
	static int last_Viewer_objnum = -1;

	// This code is supposed to detect camera "cuts"... like going between
	// different views.

	// determine if we need to regenerate the nebula
	if(	(!(last_Viewer_mode & VM_EXTERNAL) && (lViewer_mode & VM_EXTERNAL)) ||							// internal to external 
			((last_Viewer_mode & VM_EXTERNAL) && !(lViewer_mode & VM_EXTERNAL)) ||							// external to internal
			(!(last_Viewer_mode & VM_DEAD_VIEW) && (lViewer_mode & VM_DEAD_VIEW)) ||							// non dead-view to dead-view
			((last_Viewer_mode & VM_DEAD_VIEW) && !(lViewer_mode & VM_DEAD_VIEW)) ||							// dead-view to non dead-view
			(!(last_Viewer_mode & VM_WARP_CHASE) && (lViewer_mode & VM_WARP_CHASE)) ||						// non warp-chase to warp-chase
			((last_Viewer_mode & VM_WARP_CHASE) && !(lViewer_mode & VM_WARP_CHASE)) ||						// warp-chase to non warp-chase
			(!(last_Viewer_mode & VM_OTHER_SHIP) && (lViewer_mode & VM_OTHER_SHIP)) ||						// non other-ship to other-ship
			((last_Viewer_mode & VM_OTHER_SHIP) && !(lViewer_mode & VM_OTHER_SHIP)) ||						// other-ship to non-other ship
			((lViewer_mode & VM_OTHER_SHIP) && (last_Viewer_objnum != Player_ai->target_objnum)) 		// other ship mode, but targets changes
			) {

		// regenerate the nebula
		neb2_eye_changed();
	}		

	if ( (last_Viewer_mode != lViewer_mode) || (last_Game_mode != Game_mode) )	{
		//mprintf(( "************** Camera cut! ************\n" ));
		last_Viewer_mode = lViewer_mode;
		last_Game_mode = Game_mode;

		// Camera moved.  Tell stars & debris to not do blurring.
		stars_camera_cut();		
	}

	say_view_target();

	if ( lViewer_mode & VM_PADLOCK_ANY ) {
		player_display_packlock_view();
	}
	
	GameSetViewClip();

	if (Game_mode & GM_DEAD) {
		vector	vec_to_deader, view_pos;
		float		dist;

		lViewer_mode |= VM_DEAD_VIEW;

		if (Player_ai->target_objnum != -1) {
			int view_from_player = 1;

			if (lViewer_mode & VM_OTHER_SHIP) {
				//	View from target.
				Viewer_obj = &Objects[Player_ai->target_objnum];

				last_Viewer_objnum = Player_ai->target_objnum;

				if ( Viewer_obj->type == OBJ_SHIP ) {
					ship_get_eye( eye_pos, eye_orient, Viewer_obj );
					view_from_player = 0;
				}
			} else {
				last_Viewer_objnum = -1;
			}

			if ( view_from_player ) {
				//	View target from player ship.
				Viewer_obj = NULL;
				*eye_pos = Player_obj->pos;
				vm_vec_normalized_dir(&eye_dir, &Objects[Player_ai->target_objnum].pos, eye_pos);
				vm_vector_2_matrix(eye_orient, &eye_dir, NULL, NULL);
			}
		} else {
			dist = vm_vec_normalized_dir(&vec_to_deader, &Player_obj->pos, &Dead_camera_pos);
			
			if (dist < MIN_DIST_TO_DEAD_CAMERA)
				dist += flFrametime * 16.0f;

			vm_vec_scale(&vec_to_deader, -dist);
			vm_vec_add(&Dead_camera_pos, &Player_obj->pos, &vec_to_deader);
			
			view_pos = Player_obj->pos;

			if (!(Game_mode & GM_DEAD_BLEW_UP)) {
				lViewer_mode &= ~(VM_EXTERNAL | VM_CHASE);
				vm_vec_scale_add2(&Dead_camera_pos, &Original_vec_to_deader, 25.0f * flFrametime);
				Dead_player_last_vel = Player_obj->phys_info.vel;
				//nprintf(("AI", "Player death roll vel = %7.3f %7.3f %7.3f\n", Player_obj->phys_info.vel.x, Player_obj->phys_info.vel.y, Player_obj->phys_info.vel.z));
			} else if (Player_ai->target_objnum != -1) {
				view_pos = Objects[Player_ai->target_objnum].pos;
			} else {
				//	Make camera follow explosion, but gradually slow down.
				vm_vec_scale_add2(&Player_obj->pos, &Dead_player_last_vel, flFrametime);
				view_pos = Player_obj->pos;
				vm_vec_scale(&Dead_player_last_vel, 0.99f);
				vm_vec_scale_add2(&Dead_camera_pos, &Original_vec_to_deader, min(25.0f, vm_vec_mag_quick(&Dead_player_last_vel)) * flFrametime);
			}

			*eye_pos = Dead_camera_pos;

			vm_vec_normalized_dir(&eye_dir, &Player_obj->pos, eye_pos);

			vm_vector_2_matrix(eye_orient, &eye_dir, NULL, NULL);
			Viewer_obj = NULL;
		}
	} 

	// if supernova shockwave
	if(supernova_camera_cut()){
		// no viewer obj
		Viewer_obj = NULL;

		// call it dead view
		lViewer_mode |= VM_DEAD_VIEW;

		// set eye pos and orient
		supernova_set_view(eye_pos, eye_orient);
	} else {	
		//	If already blown up, these other modes can override.
		if (!IsInDeadState(Player, GM_DEAD | GM_DEAD_BLEW_UP)) 
		{
			lViewer_mode &= ~VM_DEAD_VIEW;

			Viewer_obj = Player_obj;
 
			if (lViewer_mode & VM_OTHER_SHIP) {
				if (Player_ai->target_objnum != -1){
					Viewer_obj = &Objects[Player_ai->target_objnum];
					last_Viewer_objnum = Player_ai->target_objnum;
				} else {
					lViewer_mode &= ~VM_OTHER_SHIP;
					last_Viewer_objnum = -1;
				}
			} else {
				last_Viewer_objnum = -1;
			}

			if (lViewer_mode & VM_EXTERNAL) {
				matrix	tm, tm2;

				vm_angles_2_matrix(&tm2, &Viewer_external_info.angles);
				vm_matrix_x_matrix(&tm, &Viewer_obj->orient, &tm2);

				vm_vec_scale_add(eye_pos, &Viewer_obj->pos, &tm.fvec, 2.0f * Viewer_obj->radius + Viewer_external_info.distance);

				vm_vec_sub(&eye_dir, &Viewer_obj->pos, eye_pos);
				vm_vec_normalize(&eye_dir);
				vm_vector_2_matrix(eye_orient, &eye_dir, &Viewer_obj->orient.uvec, NULL);
				Viewer_obj = NULL;

				//	Modify the orientation based on head orientation.
				compute_slew_matrix(eye_orient, &Viewer_slew_angles);

			} else if ( lViewer_mode & VM_CHASE ) {
				vector	move_dir;

				if ( Viewer_obj->phys_info.speed < 0.1 )
					move_dir = Viewer_obj->orient.fvec;
				else {
					move_dir = Viewer_obj->phys_info.vel;
					vm_vec_normalize(&move_dir);
				}

				vm_vec_scale_add(eye_pos, &Viewer_obj->pos, &move_dir, -3.0f * Viewer_obj->radius - Viewer_chase_info.distance);
				vm_vec_scale_add2(eye_pos, &Viewer_obj->orient.uvec, 0.75f * Viewer_obj->radius);
				vm_vec_sub(&eye_dir, &Viewer_obj->pos, eye_pos);
				vm_vec_normalize(&eye_dir);

				// JAS: I added the following code because if you slew up using
				// Descent-style physics, eye_dir and Viewer_obj->orient.uvec are
				// equal, which causes a zero-length vector in the vm_vector_2_matrix
				// call because the up and the forward vector are the same.   I fixed
				// it by adding in a fraction of the right vector all the time to the
				// up vector.
				vector tmp_up = Viewer_obj->orient.uvec;
				vm_vec_scale_add2( &tmp_up, &Viewer_obj->orient.rvec, 0.00001f );

				vm_vector_2_matrix(eye_orient, &eye_dir, &tmp_up, NULL);
				Viewer_obj = NULL;

				//	Modify the orientation based on head orientation.
				compute_slew_matrix(eye_orient, &Viewer_slew_angles);
			} else if ( lViewer_mode & VM_WARP_CHASE ) {
					*eye_pos = Camera_pos;

					ship * shipp = &Ships[Player_obj->instance];

					vm_vec_sub(&eye_dir, &shipp->warp_effect_pos, eye_pos);
					vm_vec_normalize(&eye_dir);
					vm_vector_2_matrix(eye_orient, &eye_dir, &Player_obj->orient.uvec, NULL);
					Viewer_obj = NULL;
			} else {
				// get an eye position based upon the correct type of object
				switch(Viewer_obj->type){
				case OBJ_SHIP:
					// make a call to get the eye point for the player object
					ship_get_eye( eye_pos, eye_orient, Viewer_obj );
					break;
				case OBJ_OBSERVER:
					// make a call to get the eye point for the player object
					observer_get_eye( eye_pos, eye_orient, Viewer_obj );				
					break;
				default :
					Int3();
				}
			}
		}
	}

	apply_hud_shake(eye_orient);

	// setup neb2 rendering
	neb2_render_setup(eye_pos, eye_orient);
}

#ifndef NDEBUG
extern void ai_debug_render_stuff();
#endif

int Game_subspace_effect = 0;
DCF_BOOL( subspace, Game_subspace_effect );

// Does everything needed to render a frame
void game_render_frame( vector * eye_pos, matrix * eye_orient )
{
	int dont_offset;

	g3_start_frame(game_zbuffer);
	g3_set_view_matrix( eye_pos, eye_orient, Viewer_zoom );

	// maybe offset the HUD (jitter stuff)
	dont_offset = ((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_OBSERVER));
	HUD_set_offsets(Viewer_obj, !dont_offset);
	
	// for multiplayer clients, call code in Shield.cpp to set up the Shield_hit array.  Have to
	// do this becaues of the disjointed nature of this system (in terms of setup and execution).
	// must be done before ships are rendered
	if ( MULTIPLAYER_CLIENT ) {
		shield_point_multi_setup();
	}

	if ( Game_subspace_effect )	{
		stars_draw(0,0,0,1);
	} else {
		stars_draw(1,1,1,0);
	}

	// FOG ON HERE
	if(The_mission.flags & MISSION_FLAG_FULLNEB)
	{
		int r, g, b;
		neb2_get_pixel(gr_screen.max_w / 2, gr_screen.max_h / 2, &r, &g, &b);
		gr_fog_set(GR_FOGMODE_FOG, r, g, b);
	}

	obj_render_all(obj_render, (int) OBJ_RENDER_OPAQUE);

	// FOG OFF HERE
	if(The_mission.flags & MISSION_FLAG_FULLNEB)
	{
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	}

	beam_render_all();						// render all beam weapons
	obj_render_all(obj_render, (int) OBJ_RENDER_TRANSPARENT);
#ifndef FS2_UE
	particle_render_all();					// render particles after everything else.
#endif
	trail_render_all();						// render missilie trails after everything else.	
	mflash_render_all();						// render all muzzle flashes	

	//	Why do we not show the shield effect in these modes?  Seems ok.
	//if (!(Viewer_mode & (VM_EXTERNAL | VM_SLEWED | VM_CHASE | VM_DEAD_VIEW))) {
	render_shields();
	//}	

	// render nebula lightning
	nebl_render_all();

	// render local player nebula
	neb2_render_player();	

#ifndef NDEBUG
	ai_debug_render_stuff();
#endif

#ifndef RELEASE_REAL
	// game_framerate_check();
#endif

#ifndef NDEBUG
	extern void snd_spew_debug_info();
	snd_spew_debug_info();
#endif

	//================ END OF 3D RENDERING STUFF ====================

	hud_show_radar();

	if( (Game_detail_flags & DETAIL_FLAG_HUD) && !(Game_mode & GM_MULTIPLAYER) || ( (Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_OBSERVER) ) ) {
		hud_maybe_clear_head_area();
		anim_render_all(0, flFrametime);
	}

	extern int Multi_display_netinfo;
	if(Multi_display_netinfo){
		extern void multi_display_netinfo();
		multi_display_netinfo();
	}	

	game_tst_frame_pre();

#ifndef NDEBUG
	do_timing_test(flFrametime);
#endif

#ifndef NDEBUG
	extern int OO_update_index;	
	multi_rate_display(OO_update_index, 375, 0);
#endif

#ifndef NDEBUG
	// test
	extern void oo_display();
	oo_display();			
#endif
	
	g3_end_frame();
}

// following function for dumping frames for purposes of building trailers.
#ifndef NDEBUG

// function to toggle state of dumping every frame into PCX when playing the game
DCF(dump_frames, "Starts/stop frame dumping at 15 hz")
{
	if ( Dc_command )	{

		if ( Debug_dump_frames == 0 )	{
			// Turn it on
			Debug_dump_frames = 15;
			Debug_dump_trigger = 0;
			gr_dump_frame_start( Debug_dump_frame_num, DUMP_BUFFER_NUM_FRAMES );
			dc_printf( "Frame dumping at 15 hz is now ON\n" );
		} else {
			// Turn it off
			Debug_dump_frames = 0;
			Debug_dump_trigger = 0;
			gr_dump_frame_stop();
			dc_printf( "Frame dumping is now OFF\n" );
		}
		
	}
}

DCF(dump_frames_trigger, "Starts/stop frame dumping at 15 hz")
{
	if ( Dc_command )	{

		if ( Debug_dump_frames == 0 )	{
			// Turn it on
			Debug_dump_frames = 15;
			Debug_dump_trigger = 1;
			gr_dump_frame_start( Debug_dump_frame_num, DUMP_BUFFER_NUM_FRAMES );
			dc_printf( "Frame dumping at 15 hz is now ON\n" );
		} else {
			// Turn it off
			Debug_dump_frames = 0;
			Debug_dump_trigger = 0;
			gr_dump_frame_stop();
			dc_printf( "Frame dumping is now OFF\n" );
		}
		
	}
}

DCF(dump_frames30, "Starts/stop frame dumping at 30 hz")
{
	if ( Dc_command )	{

		if ( Debug_dump_frames == 0 )	{
			// Turn it on
			Debug_dump_frames = 30;
			Debug_dump_trigger = 0;
			gr_dump_frame_start( Debug_dump_frame_num, DUMP_BUFFER_NUM_FRAMES );
			dc_printf( "Frame dumping at 30 hz is now ON\n" );
		} else {
			// Turn it off
			Debug_dump_frames = 0;
			Debug_dump_trigger = 0;
			gr_dump_frame_stop();
			dc_printf( "Frame dumping is now OFF\n" );
		}
		
	}
}

DCF(dump_frames30_trigger, "Starts/stop frame dumping at 30 hz")
{
	if ( Dc_command )	{

		if ( Debug_dump_frames == 0 )	{
			// Turn it on
			Debug_dump_frames = 30;
			Debug_dump_trigger = 1;
			gr_dump_frame_start( Debug_dump_frame_num, DUMP_BUFFER_NUM_FRAMES );
			dc_printf( "Triggered frame dumping at 30 hz is now ON\n" );
		} else {
			// Turn it off
			Debug_dump_frames = 0;
			Debug_dump_trigger = 0;
			gr_dump_frame_stop();
			dc_printf( "Triggered frame dumping is now OFF\n" );
		}
		
	}
}

void game_maybe_dump_frame()
{
	if ( !Debug_dump_frames ){
		return;
	}

	if( Debug_dump_trigger && !keyd_pressed[KEY_Q] ){
		return;
	}

	game_stop_time();

	gr_dump_frame();
	Debug_dump_frame_num++;

	game_start_time();
}
#endif

extern int Player_dead_state;

//	Flip the page and time how long it took.
void game_flip_page_and_time_it()
{	
	fix t1, t2,d;
	int t;
	t1 = timer_get_fixed_seconds();
	gr_flip();
	t2 = timer_get_fixed_seconds();
	d = t2 - t1;
	t = (gr_screen.max_w*gr_screen.max_h*gr_screen.bytes_per_pixel)/1024;
	sprintf( transfer_text, NOX("%d MB/s"), (int) fixmuldiv(t,65,d) );
}

void game_simulation_frame()
{
	// blow ships up in multiplayer dogfight
	if((Game_mode & GM_MULTIPLAYER) && (Net_player != NULL) && (Net_player->flags & NETINFO_FLAG_AM_MASTER) && (Netgame.type_flags & NG_TYPE_DOGFIGHT) && (f2fl(Missiontime) >= 2.0f) && !dogfight_blown){
		// blow up all non-player ships
		ship_obj *moveup = GET_FIRST(&Ship_obj_list);
		ship *shipp;
		ship_info *sip;
		while((moveup != END_OF_LIST(&Ship_obj_list)) && (moveup != NULL)){
			// bogus
			if((moveup->objnum < 0) || (moveup->objnum >= MAX_OBJECTS) || (Objects[moveup->objnum].type != OBJ_SHIP) || (Objects[moveup->objnum].instance < 0) || (Objects[moveup->objnum].instance >= MAX_SHIPS) || (Ships[Objects[moveup->objnum].instance].ship_info_index < 0) || (Ships[Objects[moveup->objnum].instance].ship_info_index >= Num_ship_types)){
				moveup = GET_NEXT(moveup);
				continue;
			}
			shipp = &Ships[Objects[moveup->objnum].instance];
			sip = &Ship_info[shipp->ship_info_index];

			// only blow up small ships			
			if((sip->flags & SIF_SMALL_SHIP) && (multi_find_player_by_object(&Objects[moveup->objnum]) < 0) ){							
				// function to simply explode a ship where it is currently at
				ship_self_destruct( &Objects[moveup->objnum] );					
			}

			moveup = GET_NEXT(moveup);
		}

		dogfight_blown = 1;
	}

	// process AWACS stuff - do this first thing
	awacs_process();

	// single player, set Player hits_this_frame to 0
	if ( !(Game_mode & GM_MULTIPLAYER) && Player ) {
		Player->damage_this_burst -= (flFrametime * MAX_BURST_DAMAGE  / (0.001f * BURST_DURATION));
		Player->damage_this_burst = max(Player->damage_this_burst, 0.0f);
	}

	// supernova
	supernova_process();
	if(supernova_active() >= 5){
		return;
	}

	// fire targeting lasers now so that 
	// 1 - created this frame
	// 2 - collide this frame
	// 3 - render this frame
	// 4 - ignored and deleted next frame
	// the basic idea being that because it con be confusing to deal with them on a multi-frame basis, they are only valid for
	// frame
	ship_process_targeting_lasers();	

	// do this here so that it works for multiplayer
	if ( Viewer_obj ) {
		// get viewer direction
		int viewer_direction = PHYSICS_VIEWER_REAR;

		if(Viewer_mode == 0){
			viewer_direction = PHYSICS_VIEWER_FRONT;
		}
		if(Viewer_mode & VM_PADLOCK_UP){
			viewer_direction = PHYSICS_VIEWER_UP;
		}
		else if(Viewer_mode & VM_PADLOCK_REAR){
			viewer_direction = PHYSICS_VIEWER_REAR;
		} 
		else if(Viewer_mode & VM_PADLOCK_LEFT){
			viewer_direction = PHYSICS_VIEWER_LEFT;
		} 
		else if(Viewer_mode & VM_PADLOCK_RIGHT){
			viewer_direction = PHYSICS_VIEWER_RIGHT;
		}

		physics_set_viewer( &Viewer_obj->phys_info, viewer_direction );
	} else {
		physics_set_viewer( NULL, PHYSICS_VIEWER_FRONT );
	}

#define	VM_PADLOCK_UP					(1 << 7)
#define	VM_PADLOCK_REAR				(1 << 8)
#define	VM_PADLOCK_LEFT				(1 << 9)
#define	VM_PADLOCK_RIGHT				(1 << 10)
		
	// evaluate mission departures and arrivals before we process all objects.
	if ( !(Game_mode & GM_MULTIPLAYER) || ((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER) && !multi_endgame_ending()) ) {

		// we don't want to evaluate mission stuff when any ingame joiner in multiplayer is receiving
		// ships/wing packets.
		if ( !((Game_mode & GM_MULTIPLAYER) && (Netgame.flags & NG_FLAG_INGAME_JOINING_CRITICAL)) && !(Game_mode & GM_DEMO_PLAYBACK)){
			mission_parse_eval_stuff();
		}

		// if we're an observer, move ourselves seperately from the standard physics
		if((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_OBSERVER)){
			obj_observer_move(flFrametime);
		}
		
		// move all the objects now
		obj_move_all(flFrametime);

		// check for cargo reveal (this has an internal timestamp, so only runs every N ms)
		// AL: 3-15-98: It was decided to not let AI ships inspect cargo
		//	ship_check_cargo_all();
		if(!(Game_mode & GM_DEMO_PLAYBACK)){
			mission_eval_goals();
		}
	}

	// always check training objectives, even in multiplayer missions. we need to do this so that the directives gauge works properly on clients
	if(!(Game_mode & GM_DEMO_PLAYBACK)){
		training_check_objectives();
	}
	
	// do all interpolation now
	if ( (Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER) && !multi_endgame_ending() && !(Netgame.flags & NG_FLAG_SERVER_LOST)) {
		// client side processing of warping in effect stages
		multi_do_client_warp(flFrametime);     
	
		// client side movement of an observer
		if((Net_player->flags & NETINFO_FLAG_OBSERVER) || (Player_obj->type == OBJ_OBSERVER)){
			obj_observer_move(flFrametime);   
		}

		// move all objects - does interpolation now as well
		obj_move_all(flFrametime);
	}

	// only process the message queue when the player is "in" the game
	if ( !Pre_player_entry ){
		message_queue_process();				// process any messages send to the player
	}

	if(!(Game_mode & GM_DEMO_PLAYBACK)){
		message_maybe_distort();				// maybe distort incoming message if comms damaged
		player_repair_frame(flFrametime);	//	AI objects get repaired in ai_process, called from move code...deal with player.
		player_process_pending_praise();		// maybe send off a delayed praise message to the player
		player_maybe_play_all_alone_msg();	// mabye tell the player he is all alone	
	}

	if(!(Game_mode & GM_STANDALONE_SERVER)){		
		// process some stuff every frame (before frame is rendered)
		emp_process_local();

		hud_update_frame();						// update hud systems

		if (!physics_paused)	{
			// Move particle system
			particle_move_all(flFrametime);	

			// Move missile trails
			trail_move_all(flFrametime);		

			// process muzzle flashes
			mflash_process_all();

			// Flash the gun flashes
			shipfx_flash_do_frame(flFrametime);			

			shockwave_move_all(flFrametime);	// update all the shockwaves
		}

		// subspace missile strikes
		ssm_process();

		obj_snd_do_frame();						// update the object-linked persistant sounds
		game_maybe_update_sound_environment();
		snd_update_listener(&View_position, &Player_obj->phys_info.vel, &Player_obj->orient);

// AL: debug code used for testing ambient subspace sound (ie when enabling subspace through debug console)
#ifndef NDEBUG
		if ( Game_subspace_effect ) {
			game_start_subspace_ambient_sound();
		}
#endif
	}		

}

// Maybe render and process the dead-popup
void game_maybe_do_dead_popup(float frametime)
{
	if ( popupdead_is_active() ) {
		int leave_popup=1;
		int choice = popupdead_do_frame(frametime);

		if ( Game_mode & GM_NORMAL ) {
			switch(choice) {
			case 0:
				// CD CHECK				
				if(game_do_cd_mission_check(Game_current_mission_filename)){
					gameseq_post_event(GS_EVENT_ENTER_GAME);
				} else {
					gameseq_post_event(GS_EVENT_MAIN_MENU);
				}					
				break;

			case 1:
				gameseq_post_event(GS_EVENT_END_GAME);
				break;

			case 2:
				// CD CHECK
				if(game_do_cd_mission_check(Game_current_mission_filename)){
					gameseq_post_event(GS_EVENT_START_GAME);					
				} else {
					gameseq_post_event(GS_EVENT_MAIN_MENU);
				}					
				break;

			// this should only happen during a red alert mission
			case 3:				
				// bogus?
				Assert(The_mission.red_alert);
				if(!The_mission.red_alert){
					// CD CHECK
					if(game_do_cd_mission_check(Game_current_mission_filename)){
						gameseq_post_event(GS_EVENT_START_GAME);
					} else {
						gameseq_post_event(GS_EVENT_MAIN_MENU);
					}
					break;
				}
				
				// choose the previous mission
				mission_campaign_previous_mission();
				// CD CHECK
				if(game_do_cd_mission_check(Game_current_mission_filename)){
					gameseq_post_event(GS_EVENT_START_GAME);
				} else {
					gameseq_post_event(GS_EVENT_MAIN_MENU);
				}				
				break;

			default:
				leave_popup=0;
				break;
			}
		} else {
			switch( choice ) {

			case POPUPDEAD_DO_MAIN_HALL:
				multi_quit_game(PROMPT_NONE,-1);
				break;

			case POPUPDEAD_DO_RESPAWN:				
				multi_respawn_normal();
				event_music_player_respawn();
				break;

			case POPUPDEAD_DO_OBSERVER:
				multi_respawn_observer();
				event_music_player_respawn_as_observer();
				break;

			default:
				leave_popup = 0;
				break;
			}
		}

		if ( leave_popup ) {
			popupdead_close();
		}
	}
}

// returns true if player is actually in a game_play stats
int game_actually_playing()
{
	int state;

	state = gameseq_get_state();
	if ( (state != GS_STATE_GAME_PLAY) && (state != GS_STATE_DEATH_DIED) && (state != GS_STATE_DEATH_BLEW_UP) )
		return 0;
	else
		return 1;
}

// Draw the 2D HUD gauges
void game_render_hud_2d()
{
	if(Player_obj->type != OBJ_SHIP)
	{
		return;
	}

	const int supernova_active_stage3 = 3;
	if(supernova_active() >=	supernova_active_stage3)
	{
		return;
	}

	if ( !(Game_detail_flags & DETAIL_FLAG_HUD) ) {
		return;
	}
	
	HUD_render_2d(flFrametime);
	gr_reset_clip_splitscreen_safe();
}

// Draw the 3D-dependant HUD gauges
void game_render_hud_3d(vector *eye_pos, matrix *eye_orient)
{
	g3_start_frame(0);		// 0 = turn zbuffering off
	g3_set_view_matrix( eye_pos, eye_orient, Viewer_zoom );

	if ( (Game_detail_flags & DETAIL_FLAG_HUD) && (supernova_active() < 3)/* && !(Game_mode & GM_MULTIPLAYER) || ( (Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_OBSERVER) )*/ ) {
		HUD_render_3d(flFrametime);
	}

	MsgScreenEffectsUpdate lMsg(flFrametime);
	Player_obj->PostAMessage(&lMsg);

	g3_end_frame();
}

void MultiPlayerPopups()
{
	vector eye_pos;
	matrix eye_orient;

	game_render_frame_setup(&eye_pos, &eye_orient, Viewer_mode);

	// save the eye position and orientation
	if ( Game_mode & GM_MULTIPLAYER ) {
		Net_player->s_info.eye_pos = eye_pos;
		Net_player->s_info.eye_orient = eye_orient;
	}

	// check to see if we should display the death died popup
	if(Game_mode & GM_DEAD_BLEW_UP)
	{				
		if(Game_mode & GM_MULTIPLAYER){
			// catch the situation where we're supposed to be warping out on this transition
			if(Net_player->flags & NETINFO_FLAG_WARPING_OUT){
				gameseq_post_event(GS_EVENT_DEBRIEF);
			} else if((Player_died_popup_wait != -1) && (timestamp_elapsed(Player_died_popup_wait))){
				Player_died_popup_wait = -1;
				popupdead_start();
			}
		} else {
			if((Player_died_popup_wait != -1) && (timestamp_elapsed(Player_died_popup_wait))){
				Player_died_popup_wait = -1;
				popupdead_start();
			}
		}
	}


	// hack - sometimes this seems to slip by in multiplayer. this should guarantee that we catch it
	if((Game_mode & GM_MULTIPLAYER) && (Player_multi_died_check != -1) && (Game_mode & GM_DEAD_BLEW_UP) ){
		if(fl_abs(time(NULL) - Player_multi_died_check) > 4){
			if(!popupdead_is_active()){
				popupdead_start();
			}

			Player_multi_died_check = -1;
		}
	}
}


void game_frame()
{
	SetPlayerShip(0);

#ifndef NDEBUG
	if (Framerate_delay) {
		int	start_time = timer_get_milliseconds();
		while (timer_get_milliseconds() < start_time + Framerate_delay)
			;
	}
#endif

#ifdef DEMO_SYSTEM
	demo_do_frame_start();
	if(Demo_error){
		mprintf(("Error (%d) while processing demo!\n", Demo_error));
		demo_close();
	}
#endif
	
	// start timing frame
	timing_frame_start();

	// var to hold which state we are in
	int actually_playing = game_actually_playing();
	
	if ((!(Game_mode & GM_MULTIPLAYER)) || ((Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_OBSERVER))) {
		if (!(Game_mode & GM_STANDALONE_SERVER)){
			Assert( OBJ_INDEX(Player_obj) >= 0 );
		}
	}

	if (Missiontime > Entry_delay_time){
		Pre_player_entry = 0;
	} else {
		; //nprintf(("AI", "Framecount = %i, time = %7.3f\n", Framecount, f2fl(Missiontime)));
	}

	shield_frame_init();

	if ( Player->control_mode != PCM_NORMAL )
		camera_move();

	if ( !Pre_player_entry && actually_playing ) {		   		
		if (! (Game_mode & GM_STANDALONE_SERVER) ) {

			if( (!popup_running_state()) && (!popupdead_is_active()) ){

				SetPlayerShip(0);
				game_process_keys();

				if(Cmdline_splitscreen)
				{
					SetPlayerShip(1);
					game_process_keys();
				}

				// don't read flying controls if we're playing a demo back
				if(!(Game_mode & GM_DEMO_PLAYBACK)){

					static int lastGameTime = 0;
					const int now = timer_get_milliseconds();

					const int lNumPlayers = Cmdline_splitscreen ? 2 : 1;
					for(int i = 0; i < lNumPlayers; i++)
					{
						SetPlayerShip(i);
						read_player_controls( Player_obj, flFrametime);

						g_ForceFeedback.Update(flFrametime * 1000);//now - lastGameTime);

						MsgGetPad lMsg;
						Player_obj->PostAMessage(&lMsg);
						GameController *pPad = lMsg.GetPad();
						Assert(pPad);
						pPad->UpdateGameControls(flFrametime * 1000);//now - lastGameTime);
					}

					lastGameTime = now;
				}
			}
			
			// if we're not the master, we may have to send the server-critical ship status button_info bits
			if ((Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER) && !(Net_player->flags & NETINFO_FLAG_OBSERVER)){
				multi_maybe_send_ship_status();
			}
		}
	}

	// Reset the whack stuff
	game_whack_reset();

	if ((Game_mode & GM_MULTIPLAYER) && (Netgame.game_state == NETGAME_STATE_SERVER_TRANSFER)){
		return;
	}
	
	RadarStart();
	game_simulation_frame();	
	RadarEnd();

	// if not actually in a game play state, then return.  This condition could only be true in 
	// a multiplayer game.
	if ( !actually_playing ) {
		Assert( Game_mode & GM_MULTIPLAYER );
		return;
	}

	if(Cmdline_splitscreen && AllPlayersAreDead(true) && (GM_IN_MISSION & GM_IN_MISSION))
	{
		gameseq_post_event(GS_EVENT_DEATH_DIED);	
	}

	if (!Pre_player_entry) {
		if (! (Game_mode & GM_STANDALONE_SERVER)) {

			// clear the screen to black
			gr_reset_clip();
			if ( (Game_detail_flags & DETAIL_FLAG_CLEAR) ) {
				gr_clear();
			}

			const int lNumPlayers = Cmdline_splitscreen ? 2 : 1;
			for(int i = 0; i < lNumPlayers; i++)
			{
				// Must be outside of Pre_player_entry code,
				// otherwise too many lights are added.
				light_reset();

				SetPlayerShip(i);

				vector eye_pos;
				matrix eye_orient;

				gr_reset_clip_set_player_num(i);
				game_render_frame_setup(&eye_pos, &eye_orient, Viewer_mode);
				game_render_frame( &eye_pos, &eye_orient );
					
				GameSetViewClip();

				hud_show_target_model();
				game_show_time_left();

				// Draw the 2D HUD gauges
				game_render_hud_2d();

				GameSetViewClip();

				// Draw 3D HUD gauges			
				game_render_hud_3d(&eye_pos, &eye_orient);	
			}

			SetPlayerShip(0);

			gr_reset_clip_set_player_num(0);
			MultiPlayerPopups();

			game_get_framerate();
			if(Cmdline_fps)
			{
				game_show_framerate();	
			}

			// maybe render and process the dead popup
			game_maybe_do_dead_popup(flFrametime);

			// start timing frame

			// If a regular popup is active, don't flip (popup code flips)
			if( !popup_running_state() ){
				game_flip_page_and_time_it();
			}

#ifndef NDEBUG
			game_maybe_dump_frame();			// used to dump pcx files for building trailers
#endif		
		} else {
			game_show_standalone_framerate();
		}
	}

	game_do_training_checks();
	asteroid_frame();

	// process lightning (nebula only)
	nebl_process();

#ifdef DEMO_SYSTEM
	demo_do_frame_end();
	if(Demo_error){
		mprintf(("Error (%d) while processing demo!\n", Demo_error));
		demo_close();
	}
#endif

	SetPlayerShip(0);
}

#define	MAX_FRAMETIME	(F1_0/4)		// Frametime gets saturated at this.  Changed by MK on 11/1/97.
												//	Some bug was causing Frametime to always get saturated at 2.0 seconds after the player
												//	died.  This resulted in screwed up death sequences.

fix Last_time = 0;						// The absolute time of game at end of last frame (beginning of this frame)
fix Last_delta_time = 0;				// While game is paused, this keeps track of how much elapsed in the frame before paused.
static int timer_paused=0;
static int stop_count,start_count;
static int time_stopped,time_started;
int saved_timestamp_ticker = -1;

void game_reset_time()
{
	if((Game_mode & GM_MULTIPLAYER) && (Netgame.game_state == NETGAME_STATE_SERVER_TRANSFER)){
		return ;
	}
	
	//	Last_time = timer_get_fixed_seconds();
	game_start_time();
	timestamp_reset();
	game_stop_time();
}

void game_stop_time()
{
	if (timer_paused==0) {
		fix time;
		time = timer_get_fixed_seconds();
		// Save how much time progressed so far in the frame so we can
		// use it when we unpause.
		Last_delta_time = time - Last_time;		

		//mprintf(("Last_time in game_stop_time = %7.3f\n", f2fl(Last_delta_time)));
		if (Last_delta_time < 0) {
			#if defined(TIMER_TEST) && !defined(NDEBUG)
			Int3();		//get Matt!!!!
			#endif
			Last_delta_time = 0;
		}
		#if defined(TIMER_TEST) && !defined(NDEBUG)
		time_stopped = time;
		#endif

		// Stop the timer_tick stuff...
		// Normally, you should never access 'timestamp_ticker', consider this a low-level routine
		saved_timestamp_ticker = timestamp_ticker;
	}
	timer_paused++;

	#if defined(TIMER_TEST) && !defined(NDEBUG)
	stop_count++;
	#endif
}

void game_start_time()
{
	timer_paused--;
	Assert(timer_paused >= 0);
	if (timer_paused==0) {
		fix time;
		time = timer_get_fixed_seconds();
		#if defined(TIMER_TEST) && !defined(NDEBUG)
		if (Last_time < 0)
			Int3();		//get Matt!!!!
		}
		#endif
		// Take current time, and set it backwards to account for time	
		// that the frame already executed, so that timer_get_fixed_seconds() - Last_time
		// will be correct when it goes to calculate the frametime next
		// frame.
		Last_time = time - Last_delta_time;		
		#if defined(TIMER_TEST) && !defined(NDEBUG)
		time_started = time;
		#endif

		// Restore the timer_tick stuff...
		// Normally, you should never access 'timestamp_ticker', consider this a low-level routine
		Assert( saved_timestamp_ticker > -1 );		// Called out of order, get JAS
		timestamp_ticker = saved_timestamp_ticker;
		saved_timestamp_ticker = -1;
	}

	#if defined(TIMER_TEST) && !defined(NDEBUG)
	start_count++;
	#endif
}


void game_set_frametime(int state)
{
	fix thistime;
	float frame_cap_diff;

	thistime = timer_get_fixed_seconds();

	if ( Last_time == 0 )	
		Frametime = F1_0 / 30;
	else
		Frametime = thistime - Last_time;

	if(Frametime < 0)
	{
		//Int3();
		Frametime = 0;
	}

//	Frametime = F1_0 / 30;

//	fix	debug_frametime = Frametime;	//	Just used to display frametime.

	//	If player hasn't entered mission yet, make frame take 1/4 second.
	if ((Pre_player_entry) && (state == GS_STATE_GAME_PLAY))
		Frametime = F1_0/4;
#ifndef NDEBUG
	else if ((Debug_dump_frames) && (state == GS_STATE_GAME_PLAY)) {				// note link to above if!!!!!
	
		fix frame_speed = F1_0 / Debug_dump_frames;

		if (Frametime > frame_speed ){
			nprintf(("warning","slow frame: %x\n",Frametime));
		} else {			
			do {
				thistime = timer_get_fixed_seconds();
				Frametime = thistime - Last_time;
			} while (Frametime < frame_speed );			
		}
		Frametime = frame_speed;
	}
#endif

	Assert( Framerate_cap > 0 );

	if((Game_mode & GM_STANDALONE_SERVER) && 
		(f2fl(Frametime) < ((float)1.0/(float)Multi_options_g.std_framecap))){

		frame_cap_diff = ((float)1.0/(float)Multi_options_g.std_framecap) - f2fl(Frametime);		
		Sleep((DWORD)(frame_cap_diff*1000)); 				
		
		thistime += fl2f((frame_cap_diff));		

		Frametime = thistime - Last_time;
   }

	Frametime = fixmul(Frametime, Game_time_compression);

	Last_time = thistime;
	//mprintf(("Frame %i, Last_time = %7.3f\n", Framecount, f2fl(Last_time)));

	flFrametime = f2fl(Frametime);

	if(flFrametime < 0)
	{
		Int3();
		assert(flFrametime);
	}
	//if(!(Game_mode & GM_PLAYING_DEMO)){
	timestamp_inc(flFrametime);

/*	if ((Framecount > 0) && (Framecount < 10)) {
		mprintf(("Frame %2i: frametime = %.3f (%.3f)\n", Framecount, f2fl(Frametime), f2fl(debug_frametime)));
	}
*/
}

// This is called from game_do_frame(), and from navmap_do_frame() 
void game_update_missiontime()
{
	// TODO JAS: Put in if and move this into game_set_frametime, 
	// fix navmap to call game_stop/start_time
	//if ( !timer_paused )	
		Missiontime += Frametime;
}

void game_do_frame()
{	
	game_set_frametime(GS_STATE_GAME_PLAY);
	game_update_missiontime();

#if !defined(FS2_UE)
	if (Game_mode & GM_STANDALONE_SERVER) {
		std_multi_set_standalone_missiontime(f2fl(Missiontime));
	}
#endif

	if ( game_single_step && (last_single_step == game_single_step) ) {
		os_set_title( NOX("SINGLE STEP MODE (Pause exits, any other key steps)") );
		while( key_checkch() == 0 )
			os_sleep(10);
		os_set_title( XSTR( "FreeSpace", 171) );
  		Last_time = timer_get_fixed_seconds();
	}

	last_single_step = game_single_step;

	if ((gameseq_get_state() == GS_STATE_GAME_PLAY) && g_MouseController.GetMouseState(MouseController::MOUSE_STATE_USE_TO_FLY)){
		g_MouseController.SetMouseState(MouseController::MOUSE_STATE_KEEP_CENTERED);
		// force mouse to center of our window (so we don't hit movement limits)
	}
	game_frame();

	g_MouseController.UnsetMouseState(MouseController::MOUSE_STATE_KEEP_CENTERED);
	monitor_update();			// Update monitor variables
}

void multi_maybe_do_frame()
{
	if ( (Game_mode & GM_MULTIPLAYER) && (Game_mode & GM_IN_MISSION) && !Multi_pause_status){
		game_do_frame(); 
	}
}

int Joymouse_button_status = 0;

// Flush all input devices
void game_flush()
{
	key_flush();
	g_MouseController.Flush();
	joy_flush();
	snazzy_flush();

	Joymouse_button_status = 0;

	//mprintf(("Game flush!\n" ));
}

// function for multiplayer only which calls game_do_state_common() when running the
// debug console
void game_do_dc_networking()
{
	Assert( Game_mode & GM_MULTIPLAYER );

	game_do_state_common( gameseq_get_state() );
}

// Call this whenever in a loop, or when you need to check for a keystroke.
int game_check_key()
{
	int k;

	k = game_poll();

	// convert keypad enter to normal enter
	if ((k & KEY_MASK) == KEY_PADENTER)
		k = (k & ~KEY_MASK) | KEY_ENTER;

	return k;
}

#ifdef FS2_DEMO

#define DEMO_TRAILER_TIMEOUT_MS		45000			// 45 seconds of no input, play trailer
static int Demo_show_trailer_timestamp = 0;

void demo_reset_trailer_timer()
{
	Demo_show_trailer_timestamp = timer_get_milliseconds();
}

void demo_maybe_show_trailer(int k)
{
	/*
	// if key pressed, reset demo trailer timer
	if ( k > 0 ) {
		demo_reset_trailer_timer();
		return;
	}

	// if mouse moved, reset demo trailer timer
	int dx = 0, dy = 0;

	g_MouseController.GetDelta(&dx, &dy);
	if ( (dx > 0) || (dy > 0) ) {
		demo_reset_trailer_timer();
		return;
	}

	// if joystick has moved, reset demo trailer timer
	dx = 0;
	dy = 0;
	joy_get_delta(&dx, &dy);
	if ( (dx > 0) || (dy > 0) ) {
		demo_reset_trailer_timer();
		return;
	}

	// NOTE: reseting the trailer timer on mouse/joystick presses is handled in
	//       the low-level code.  Ugly, I know... but was the simplest and most
	//       robust solution.
		
	// if 30 seconds since last demo trailer time reset, launch movie
	if ( os_foreground() ) {
		int now = timer_get_milliseconds();
		if ( (now - Demo_show_trailer_timestamp) > DEMO_TRAILER_TIMEOUT_MS ) {
//		if ( (now - Demo_show_trailer_timestamp) > 10000 ) {
			// play movie here
			movie_play( NOX("fstrailer2.mve") );
			demo_reset_trailer_timer();
		}
	}
	*/
}

#endif

// same as game_check_key(), except this is used while actually in the game.  Since there
// generally are differences between game control keys and general UI keys, makes sense to
// have seperate functions for each case.  If you are not checking a game control while in a
// mission, you should probably be using game_check_key() instead.
int game_poll()
{
	int k, state;

	if (!os_foreground()) {		
		game_stop_time();
		os_sleep(100);
		game_start_time();

		// If we're in a single player game, pause it.
		if (!(Game_mode & GM_MULTIPLAYER)){
			if ( (gameseq_get_state() == GS_STATE_GAME_PLAY) && (!popup_active()) && (!popupdead_is_active()) )	{
				game_process_pause_key();
			}
		}
	}

   k = key_inkey();

#ifdef FS2_DEMO
	demo_maybe_show_trailer(k);
#endif

	// Move the mouse cursor with the joystick.
	if (os_foreground() && (!g_MouseController.GetMouseHidden()) && (Use_joy_mouse) )	{
		// Move the mouse cursor with the joystick
		int mx, my, dx, dy;
		int jx, jy, jz, jr;

		joy_get_pos( &jx, &jy, &jz, &jr );

		dx = fl2i(f2fl(jx)*flFrametime*500.0f);
		dy = fl2i(f2fl(jy)*flFrametime*500.0f);

		if ( dx || dy )	{
			g_MouseController.GetRealPos( &mx, &my );
			g_MouseController.SetPos( mx+dx, my+dy );
		}

		int j, m;
		j = joy_down(0);
		m = g_MouseController.IsDown(MouseController::MOUSE_LEFT_BUTTON);

		if ( j != Joymouse_button_status )	{
			//mprintf(( "Joy went from %d to %d, mouse is %d\n", Joymouse_button_status, j, m ));
			Joymouse_button_status = j;
			if ( j && (!m) )	{
				g_MouseController.MarkButton( MouseController::MOUSE_LEFT_BUTTON, 1 );
			} else if ( (!j) && (m) )	{
				g_MouseController.MarkButton( MouseController::MOUSE_LEFT_BUTTON, 0 );
			}
		}
	}

	// if we should be ignoring keys because of some multiplayer situations
	if((Game_mode & GM_MULTIPLAYER) && multi_ignore_controls(k)){
		return 0;
	}

	// If a popup is running, don't process all the Fn keys
	if( popup_active() ) {
		return k;
	}

	state = gameseq_get_state();

//	if ( k ) nprintf(( "General", "Key = %x\n", k ));

	switch (k) {
		case KEY_DEBUGGED + KEY_BACKSP:
			Int3();
			break;

		case KEY_F1:
			launch_context_help();
			k = 0;
			break;

		case KEY_F2:
//			if (state != GS_STATE_INITIAL_PLAYER_SELECT) {

			// don't allow f2 while warping out in multiplayer	
			if((Game_mode & GM_MULTIPLAYER) && (Net_player != NULL) && (Net_player->flags & NETINFO_FLAG_WARPING_OUT)){
				break;
			}

			switch (state) {
				case GS_STATE_INITIAL_PLAYER_SELECT:
				case GS_STATE_OPTIONS_MENU:
				case GS_STATE_HUD_CONFIG:
				case GS_STATE_CONTROL_CONFIG:
				case GS_STATE_DEATH_DIED:
				case GS_STATE_DEATH_BLEW_UP:		
				case GS_STATE_VIEW_MEDALS:
					break;

				default:
					gameseq_post_event(GS_EVENT_OPTIONS_MENU);
					k = 0;
					break;
			}

			break;

			// hotkey selection screen -- only valid from briefing and beyond.
		case KEY_F3:	
			#ifndef FS2_DEMO
				if ( (state == GS_STATE_TEAM_SELECT) || (state == GS_STATE_BRIEFING) || (state == GS_STATE_SHIP_SELECT) || (state == GS_STATE_WEAPON_SELECT) || (state == GS_STATE_GAME_PLAY) || (state == GS_STATE_GAME_PAUSED) ) {
					gameseq_post_event( GS_EVENT_HOTKEY_SCREEN );
					k = 0;
				}
			#endif
			break;

		case KEY_DEBUGGED + KEY_F3:
			gameseq_post_event( GS_EVENT_TOGGLE_FULLSCREEN );
			break;

		case KEY_DEBUGGED + KEY_F4:
			gameseq_post_event( GS_EVENT_TOGGLE_GLIDE );
			break;
		
		case KEY_F4:
			if(Game_mode & GM_MULTIPLAYER){
				if((state == GS_STATE_GAME_PLAY) || (state == GS_STATE_MULTI_PAUSED)){
					gameseq_post_event( GS_EVENT_MISSION_LOG_SCROLLBACK );
					k = 0;
				} 
			} else {
				if ((state == GS_STATE_GAME_PLAY) || (state == GS_STATE_DEATH_DIED) || (state == GS_STATE_DEATH_BLEW_UP) || (state == GS_STATE_GAME_PAUSED) ) {
					gameseq_post_event( GS_EVENT_MISSION_LOG_SCROLLBACK );
					k = 0;
				}
			}
			break;

		case KEY_ESC | KEY_SHIFTED:
			// make sure to quit properly out of multiplayer
			if(Game_mode & GM_MULTIPLAYER){
				multi_quit_game(PROMPT_NONE);
			}

			gameseq_post_event( GS_EVENT_QUIT_GAME );
			k = 0;

			break;

		case KEY_DEBUGGED + KEY_P:			
			break;			

		case KEY_PRINT_SCRN: 
			{
				static int counter = 0;
				char tmp_name[127];

				game_stop_time();

				sprintf( tmp_name, NOX("screen%02d"), counter );
				counter++;
				mprintf(( "Dumping screen to '%s'\n", tmp_name ));
				gr_print_screen(tmp_name);

				game_start_time();
			}

			k = 0;
			break;

		case KEY_SHIFTED | KEY_ENTER: {

#if !defined(NDEBUG)

			if ( Game_mode & GM_NORMAL ){
				game_stop_time();
			}

			// if we're in multiplayer mode, do some special networking
			if(Game_mode & GM_MULTIPLAYER){
				debug_console(game_do_dc_networking);
			} else {				
				debug_console();
			}

			game_flush();

			if ( Game_mode & GM_NORMAL )
				game_start_time();

#endif

			break;
		}
	}

	return k;
}

void os_close()
{
	gameseq_post_event(GS_EVENT_QUIT_GAME);
}

void apply_physics( float damping, float desired_vel, float initial_vel, float t, float * new_vel, float * delta_pos );


void camera_set_position( vector *pos )
{
	Camera_pos = *pos;
}

void camera_set_orient( matrix *orient )
{
	Camera_orient = *orient;
}

void camera_set_velocity( vector *vel, int instantaneous )
{
	Camera_desired_velocity.x = 0.0f;
	Camera_desired_velocity.y = 0.0f;
	Camera_desired_velocity.z = 0.0f;

	vm_vec_scale_add2( &Camera_desired_velocity, &Camera_orient.rvec, vel->x );
	vm_vec_scale_add2( &Camera_desired_velocity, &Camera_orient.uvec, vel->y );
	vm_vec_scale_add2( &Camera_desired_velocity, &Camera_orient.fvec, vel->z );

	if ( instantaneous )	{
		Camera_velocity = Camera_desired_velocity;
	}

}

//
void camera_move()
{
	vector new_vel, delta_pos;

	apply_physics( Camera_damping, Camera_desired_velocity.x, Camera_velocity.x, flFrametime, &new_vel.x, &delta_pos.x );
	apply_physics( Camera_damping, Camera_desired_velocity.y, Camera_velocity.y, flFrametime, &new_vel.y, &delta_pos.y );
	apply_physics( Camera_damping, Camera_desired_velocity.z, Camera_velocity.z, flFrametime, &new_vel.z, &delta_pos.z );

	Camera_velocity = new_vel;

//	mprintf(( "Camera velocity = %.1f,%.1f, %.1f\n", Camera_velocity.x, Camera_velocity.y, Camera_velocity.z ));

	vm_vec_add2( &Camera_pos, &delta_pos );

	float ot = Camera_time+0.0f;

	Camera_time += flFrametime;

	if ( (ot < 0.667f) && ( Camera_time >= 0.667f ) )	{
		vector tmp;
		
		tmp.z = 4.739f;		// always go this fast forward.

		// pick x and y velocities so they are always on a 
		// circle with a 25 m radius.

		float tmp_angle = frand()*PI2;
	
		tmp.x = 22.0f * (float)sin(tmp_angle);
		tmp.y = -22.0f * (float)cos(tmp_angle);

		//mprintf(( "Angle = %.1f, vx=%.1f, vy=%.1f\n", tmp_angle, tmp.x, tmp.y ));

		//mprintf(( "Changing velocity!\n" ));
		camera_set_velocity( &tmp, 0 );
	}

	if ( (ot < 3.0f ) && ( Camera_time >= 3.0f ) )	{
		vector tmp ( 0.0f, 0.0f, 0.0f );
		camera_set_velocity( &tmp, 0 );
	}
	
}

void end_demo_campaign_do()
{
#if defined(FS2_DEMO)
	// show upsell screens
	demo_upsell_show_screens();
#elif defined(OEM_BUILD)
	// show oem upsell screens
	oem_upsell_show_screens();
#endif

	// drop into main hall
	gameseq_post_event( GS_EVENT_MAIN_MENU );
}

// All code to process events.   This is the only place
// that you should change the state of the game.
void game_process_event( int current_state, int event )
{
	mprintf(("Got event %s in state %s\n", GS_event_text[event], GS_state_text[current_state]));

	switch (event) {
		case GS_EVENT_SIMULATOR_ROOM:
			gameseq_set_state(GS_STATE_SIMULATOR_ROOM);
			break;

		case GS_EVENT_MAIN_MENU:
			gameseq_set_state(GS_STATE_MAIN_MENU);	
			g_MouseController.SetPos(gr_screen.max_w / 2, gr_screen.max_h *4 / 6);
			break;

		case GS_EVENT_OPTIONS_MENU:
			gameseq_push_state( GS_STATE_OPTIONS_MENU );
			break;

		case GS_EVENT_BARRACKS_MENU:
			gameseq_set_state(GS_STATE_BARRACKS_MENU);		
			break;

		case GS_EVENT_TECH_MENU:
			gameseq_set_state(GS_STATE_TECH_MENU);		
			break;

		case GS_EVENT_TRAINING_MENU:
			gameseq_set_state(GS_STATE_TRAINING_MENU);		
			break;

		case GS_EVENT_START_GAME:
			Select_default_ship = 0;			
			Player_multi_died_check = -1;
			gameseq_set_state(GS_STATE_CMD_BRIEF);
			break;

		case GS_EVENT_START_BRIEFING:
			gameseq_set_state(GS_STATE_BRIEFING);		
			break;

		case GS_EVENT_DEBRIEF:
			// did we end the campaign in the main freespace 2 single player campaign?
			if(Campaign_ended_in_mission && (Game_mode & GM_CAMPAIGN_MODE) && !stricmp(Campaign.filename, "freespace2")) {
				gameseq_post_event(GS_EVENT_END_CAMPAIGN);
			} else {
				gameseq_set_state(GS_STATE_DEBRIEF);		
			}

			Player_multi_died_check = -1;
			break;

		case GS_EVENT_SHIP_SELECTION:
			gameseq_set_state( GS_STATE_SHIP_SELECT );
			break;

		case GS_EVENT_WEAPON_SELECTION:
			gameseq_set_state( GS_STATE_WEAPON_SELECT );
			break;

		case GS_EVENT_ENTER_GAME:		
#ifdef DEMO_SYSTEM
			// maybe start recording a demo
			if(Demo_make){
				demo_start_record("test.fsd");
			}
#endif

			if (Game_mode & GM_MULTIPLAYER) {
				// if we're respawning, make sure we change the view mode so that the hud shows up
				if (current_state == GS_STATE_DEATH_BLEW_UP) {
					Viewer_mode = 0;
				}

				gameseq_set_state(GS_STATE_GAME_PLAY);
			} else {
				gameseq_set_state(GS_STATE_GAME_PLAY, 1);
			}

			Player_multi_died_check = -1;

			// clear multiplayer button info			
			extern button_info Multi_ship_status_bi;
			memset(&Multi_ship_status_bi, 0, sizeof(button_info));

			Start_time = f2fl(timer_get_seconds());
			//Framecount = 0;
			mprintf(("Entering game at time = %7.3f\n", Start_time));
			break;


		case GS_EVENT_START_GAME_QUICK:
			Select_default_ship = 1;
			gameseq_post_event(GS_EVENT_ENTER_GAME);
			break;


		case GS_EVENT_END_GAME:
			if ( (current_state == GS_STATE_GAME_PLAY) || (current_state == GS_STATE_DEATH_DIED) ||
				(current_state == GS_STATE_DEATH_BLEW_UP) ||	(current_state == GS_STATE_DEBRIEF) || (current_state == GS_STATE_MULTI_DOGFIGHT_DEBRIEF)) {
					gameseq_set_state(GS_STATE_MAIN_MENU);

			} else
				Int3();

			Player_multi_died_check = -1;
			break;

		case GS_EVENT_QUIT_GAME:
			main_hall_stop_music();
			main_hall_stop_ambient();
			gameseq_set_state(GS_STATE_QUIT_GAME);

			Player_multi_died_check = -1;
			break;

		case GS_EVENT_GAMEPLAY_HELP:
			gameseq_push_state( GS_STATE_GAMEPLAY_HELP );
			break;

		case GS_EVENT_PAUSE_GAME:
			gameseq_push_state(GS_STATE_GAME_PAUSED);
			break;

		case GS_EVENT_DEBUG_PAUSE_GAME:
			gameseq_push_state(GS_STATE_DEBUG_PAUSED);
			break;

		case GS_EVENT_TRAINING_PAUSE:
			gameseq_push_state(GS_STATE_TRAINING_PAUSED);
			break;

		case GS_EVENT_PREVIOUS_STATE:
			gameseq_pop_state();
			break;

		case GS_EVENT_TOGGLE_FULLSCREEN:
			#ifndef HARDWARE_ONLY
				#ifndef NDEBUG
				if ( gr_screen.mode == GR_SOFTWARE )	{
					gr_init( GR_640, GR_DIRECTDRAW );
				} else if ( gr_screen.mode == GR_DIRECTDRAW )	{
					gr_init( GR_640, GR_SOFTWARE );
				}
				#endif
			#endif
			break;			
 
		case GS_EVENT_LOAD_MISSION_MENU:
			gameseq_set_state(GS_STATE_LOAD_MISSION_MENU);
			break;

		case GS_EVENT_MISSION_LOG_SCROLLBACK:
			gameseq_push_state( GS_STATE_MISSION_LOG_SCROLLBACK );
			break;

		case GS_EVENT_HUD_CONFIG:
			gameseq_push_state( GS_STATE_HUD_CONFIG );
			break;

		case GS_EVENT_CONTROL_CONFIG:
			gameseq_push_state( GS_STATE_CONTROL_CONFIG );
			break;	

		case GS_EVENT_DEATH_DIED:
			gameseq_set_state( GS_STATE_DEATH_DIED );
			break;

		case GS_EVENT_DEATH_BLEW_UP:
			if (  current_state == GS_STATE_DEATH_DIED )	{
				gameseq_set_state( GS_STATE_DEATH_BLEW_UP );
				event_music_player_death();

				// multiplayer clients set their extra check here
				if(Game_mode & GM_MULTIPLAYER){
					// set the multi died absolute last chance check					
					Player_multi_died_check = time(NULL);
				}					
			} else {
				mprintf(( "Ignoring GS_EVENT_DEATH_BLEW_UP because we're in state %d\n", current_state ));
			}
			break;

		case GS_EVENT_NEW_CAMPAIGN:
			if (!mission_load_up_campaign()){
				readyroom_continue_campaign();
			}

			Player_multi_died_check = -1;
			break;

		case GS_EVENT_CAMPAIGN_CHEAT:
			if (!mission_load_up_campaign()){
				/*
				// bash campaign value
				extern char Main_hall_campaign_cheat[512];
				int idx;
				
				// look for the mission
				for(idx=0; idx<Campaign.num_missions; idx++){
					if(!stricmp(Campaign.missions[idx].name, Main_hall_campaign_cheat)){
						Campaign.next_mission = idx;
						Campaign.prev_mission = idx - 1;
						break;
					}
				}
				*/

				// continue
				readyroom_continue_campaign();
			}

			Player_multi_died_check = -1;
			break;

		case GS_EVENT_CAMPAIGN_ROOM:
			gameseq_set_state(GS_STATE_CAMPAIGN_ROOM);
			break;

		case GS_EVENT_CMD_BRIEF:
			gameseq_set_state(GS_STATE_CMD_BRIEF);
			break;

		case GS_EVENT_RED_ALERT:
			gameseq_set_state(GS_STATE_RED_ALERT);
			break;

		case GS_EVENT_CREDITS:
			gameseq_set_state( GS_STATE_CREDITS );
			break;

		case GS_EVENT_VIEW_MEDALS:
			gameseq_push_state( GS_STATE_VIEW_MEDALS );
			break;

		case GS_EVENT_SHOW_GOALS:
			gameseq_push_state( GS_STATE_SHOW_GOALS );	// use push_state() since we might get to this screen through a variety of states
			break;

		case GS_EVENT_HOTKEY_SCREEN:
			gameseq_push_state( GS_STATE_HOTKEY_SCREEN );	// use push_state() since we might get to this screen through a variety of states
			break;
		
	// multiplayer stuff follow these comments

		case GS_EVENT_MULTI_JOIN_GAME:
			gameseq_set_state( GS_STATE_MULTI_JOIN_GAME );
			break;

		case GS_EVENT_MULTI_HOST_SETUP:
			gameseq_set_state( GS_STATE_MULTI_HOST_SETUP );
			break;

		case GS_EVENT_MULTI_CLIENT_SETUP:
			gameseq_set_state( GS_STATE_MULTI_CLIENT_SETUP );
			break;

  		case GS_EVENT_GOTO_VIEW_CUTSCENES_SCREEN:
			gameseq_set_state(GS_STATE_VIEW_CUTSCENES);
			break;

		case GS_EVENT_MULTI_STD_WAIT:
			gameseq_set_state( GS_STATE_MULTI_STD_WAIT );
			break;

		case GS_EVENT_STANDALONE_MAIN:
			gameseq_set_state( GS_STATE_STANDALONE_MAIN );
			break;   

		case GS_EVENT_MULTI_PAUSE:
			gameseq_push_state( GS_STATE_MULTI_PAUSED );
			break;			

		case GS_EVENT_INGAME_PRE_JOIN:
			gameseq_set_state( GS_STATE_INGAME_PRE_JOIN );
			break;
		
		case GS_EVENT_EVENT_DEBUG:
			gameseq_push_state(GS_STATE_EVENT_DEBUG);
			break;

		// Start a warpout where player automatically goes 70 no matter what
		// and can't cancel out of it.
		case GS_EVENT_PLAYER_WARPOUT_START_FORCED:
			Warpout_forced = 1;							// If non-zero, bash the player to speed and go through effect

			// Same code as in GS_EVENT_PLAYER_WARPOUT_START only ignores current mode
			Player->saved_viewer_mode = Viewer_mode;
			Player->control_mode = PCM_WARPOUT_STAGE1;
			Warpout_sound = snd_play(&Snds[SND_PLAYER_WARP_OUT]);
			Warpout_time = 0.0f;			// Start timer!
			break;

		case GS_EVENT_PLAYER_WARPOUT_START:
			if ( Player->control_mode != PCM_NORMAL )	{
				mprintf(( "Player isn't in normal mode; cannot warp out.\n" ));
			} else {
				Player->saved_viewer_mode = Viewer_mode;
				Player->control_mode = PCM_WARPOUT_STAGE1;
				Warpout_sound = snd_play(&Snds[SND_PLAYER_WARP_OUT]);
				Warpout_time = 0.0f;			// Start timer!
				Warpout_forced = 0;				// If non-zero, bash the player to speed and go through effect
			}
			break;

		case GS_EVENT_PLAYER_WARPOUT_STOP:
			if ( Player->control_mode != PCM_NORMAL )	{
				if ( !Warpout_forced )	{		// cannot cancel forced warpout
					Player->control_mode = PCM_NORMAL;
					Viewer_mode = Player->saved_viewer_mode;
					hud_subspace_notify_abort();
					mprintf(( "Player put back to normal mode.\n" ));
					if ( Warpout_sound > -1 )	{
						snd_stop( Warpout_sound );
						Warpout_sound = -1;
					}
				}
			}
			break;

		case GS_EVENT_PLAYER_WARPOUT_DONE_STAGE1:		// player ship got up to speed
			if ( Player->control_mode != PCM_WARPOUT_STAGE1 )	{
				gameseq_post_event( GS_EVENT_PLAYER_WARPOUT_STOP );
				mprintf(( "Player put back to normal mode, because of invalid sequence in stage1.\n" ));
			} else {
				mprintf(( "Hit target speed.  Starting warp effect and moving to stage 2!\n" ));
				shipfx_warpout_start( Player_obj );
				Player->control_mode = PCM_WARPOUT_STAGE2;
				Player->saved_viewer_mode = Viewer_mode;
				Viewer_mode |= VM_WARP_CHASE;
				
				vector tmp = Player_obj->pos;
				matrix tmp_m;
				ship_get_eye( &tmp, &tmp_m, Player_obj );
				vm_vec_scale_add2( &tmp, &Player_obj->orient.rvec, 0.0f );
				vm_vec_scale_add2( &tmp, &Player_obj->orient.uvec, 0.952f );
				vm_vec_scale_add2( &tmp, &Player_obj->orient.fvec, -1.782f );
				Camera_time = 0.0f;
				camera_set_position( &tmp );
				camera_set_orient( &Player_obj->orient );
				vector tmp_vel( 0.0f, 5.1919f, 14.7f );

				//mprintf(( "Rad = %.1f\n", Player_obj->radius ));
				camera_set_velocity( &tmp_vel, 1);
			}
			break;

		case GS_EVENT_PLAYER_WARPOUT_DONE_STAGE2:		// player ship got into the warp effect
			if ( Player->control_mode != PCM_WARPOUT_STAGE2 )	{
				gameseq_post_event( GS_EVENT_PLAYER_WARPOUT_STOP );
				mprintf(( "Player put back to normal mode, because of invalid sequence in stage2.\n" ));
			} else {
				mprintf(( "Hit warp effect.  Moving to stage 3!\n" ));
				Player->control_mode = PCM_WARPOUT_STAGE3;
			}
			break;

		case GS_EVENT_PLAYER_WARPOUT_DONE:	// player ship got through the warp effect
			mprintf(( "Player warped out.  Going to debriefing!\n" ));
			Player->control_mode = PCM_NORMAL;
			Viewer_mode = Player->saved_viewer_mode;
			Warpout_sound = -1;

			// we have a special debriefing screen for multiplayer furballs
			if((Game_mode & GM_MULTIPLAYER) && (The_mission.game_type & MISSION_TYPE_MULTI_DOGFIGHT)){
				gameseq_post_event(GS_EVENT_MULTI_DOGFIGHT_DEBRIEF);
			}
			// do the normal debriefing for all other situations
			else {
				gameseq_post_event(GS_EVENT_DEBRIEF);
			}
			break;

		case GS_EVENT_STANDALONE_POSTGAME:
			gameseq_set_state(GS_STATE_STANDALONE_POSTGAME);
			break;

		case GS_EVENT_INITIAL_PLAYER_SELECT:
			gameseq_set_state(GS_STATE_INITIAL_PLAYER_SELECT);
			break;

		case GS_EVENT_GAME_INIT:
	#if defined(FS2_DEMO) || defined(OEM_BUILD)
			gameseq_set_state(GS_STATE_INITIAL_PLAYER_SELECT);
	#else			
			// see if the command line option has been set to use the last pilot, and act acoordingly
			if( player_select_get_last_pilot() ) {								
				// always enter the main menu -- do the automatic network startup stuff elsewhere
				// so that we still have valid checks for networking modes, etc.
				gameseq_set_state(GS_STATE_MAIN_MENU);
			} else {
				gameseq_set_state(GS_STATE_INITIAL_PLAYER_SELECT);
			}
	#endif
			break;

		case GS_EVENT_MULTI_MISSION_SYNC:
			gameseq_set_state(GS_STATE_MULTI_MISSION_SYNC);
			break;		

		case GS_EVENT_MULTI_START_GAME:
			gameseq_set_state(GS_STATE_MULTI_START_GAME);
			break;

		case GS_EVENT_MULTI_HOST_OPTIONS:
			gameseq_set_state(GS_STATE_MULTI_HOST_OPTIONS);
			break;

		case GS_EVENT_MULTI_DOGFIGHT_DEBRIEF:
			gameseq_set_state(GS_STATE_MULTI_DOGFIGHT_DEBRIEF);
			break;

		case GS_EVENT_TEAM_SELECT:
			gameseq_set_state(GS_STATE_TEAM_SELECT); 
			break;

		case GS_EVENT_END_CAMPAIGN:			
			gameseq_set_state(GS_STATE_END_OF_CAMPAIGN);
			break;		

		case GS_EVENT_END_DEMO:
			gameseq_set_state(GS_STATE_END_DEMO);
			break;

		case GS_EVENT_LOOP_BRIEF:
			gameseq_set_state(GS_STATE_LOOP_BRIEF);
			break;

		default:
			Int3();
			break;
	}
}

// Called when a state is being left.
// The current state is still at old_state, but as soon as
// this function leaves, then the current state will become
// new state.     You should never try to change the state
// in here... if you think you need to, you probably really
// need to post an event, not change the state.
void game_leave_state( int old_state, int new_state )
{
	int end_mission = 1;

	switch (new_state) {
		case GS_STATE_GAME_PAUSED:
		case GS_STATE_DEBUG_PAUSED:
		case GS_STATE_OPTIONS_MENU:
		case GS_STATE_CONTROL_CONFIG:		
		case GS_STATE_MISSION_LOG_SCROLLBACK:
		case GS_STATE_DEATH_DIED:
		case GS_STATE_SHOW_GOALS:
		case GS_STATE_HOTKEY_SCREEN:		
		case GS_STATE_MULTI_PAUSED:
		case GS_STATE_TRAINING_PAUSED:
		case GS_STATE_EVENT_DEBUG:				
		case GS_STATE_GAMEPLAY_HELP:
			end_mission = 0;  // these events shouldn't end a mission
			break;
	}

	switch (old_state) {
		case GS_STATE_BRIEFING:
			brief_stop_voices();
			if ( (new_state != GS_STATE_OPTIONS_MENU) && (new_state != GS_STATE_WEAPON_SELECT)
				  && (new_state != GS_STATE_SHIP_SELECT) && (new_state != GS_STATE_HOTKEY_SCREEN)
				  && (new_state != GS_STATE_TEAM_SELECT) ){
				common_select_close();
				if ( new_state == GS_STATE_MAIN_MENU ) {
					freespace_stop_mission();	
				}
			}
			
			// COMMAND LINE OPTION
			if (Cmdline_multi_stream_chat_to_file){
				cfwrite_string(NOX("-------------------------------------------\n"),Multi_chat_stream);
				cfclose(Multi_chat_stream);
			}
			break;

		case GS_STATE_DEBRIEF:
			if ( (new_state != GS_STATE_VIEW_MEDALS) && (new_state != GS_STATE_OPTIONS_MENU) ) {
				debrief_close();				
			}
			break;

		case GS_STATE_MULTI_DOGFIGHT_DEBRIEF:
			multi_df_debrief_close();
			break;

		case GS_STATE_LOAD_MISSION_MENU:
			mission_load_menu_close();
			break;

		case GS_STATE_SIMULATOR_ROOM:
			sim_room_close();
			break;

		case GS_STATE_CAMPAIGN_ROOM:
			campaign_room_close();
			break;

		case GS_STATE_CMD_BRIEF:
			if (new_state == GS_STATE_OPTIONS_MENU) {
				cmd_brief_hold();

			} else {
				cmd_brief_close();
				if (new_state == GS_STATE_MAIN_MENU)
					freespace_stop_mission();	
			}

			break;

		case GS_STATE_RED_ALERT:
			red_alert_close();
			break;

		case GS_STATE_SHIP_SELECT:
			if ( new_state != GS_STATE_OPTIONS_MENU && new_state != GS_STATE_WEAPON_SELECT &&
				  new_state != GS_STATE_HOTKEY_SCREEN &&
				  new_state != GS_STATE_BRIEFING && new_state != GS_STATE_TEAM_SELECT) {
				common_select_close();
				if ( new_state == GS_STATE_MAIN_MENU ) {
					freespace_stop_mission();	
				}
			}
			break;

		case GS_STATE_WEAPON_SELECT:
			if ( new_state != GS_STATE_OPTIONS_MENU && new_state != GS_STATE_SHIP_SELECT &&
				  new_state != GS_STATE_HOTKEY_SCREEN &&
				  new_state != GS_STATE_BRIEFING && new_state != GS_STATE_TEAM_SELECT) {
				common_select_close();
				if ( new_state == GS_STATE_MAIN_MENU ) {
					freespace_stop_mission();	
				}
			}
			break;

		case GS_STATE_TEAM_SELECT:
			if ( new_state != GS_STATE_OPTIONS_MENU && new_state != GS_STATE_SHIP_SELECT &&
				  new_state != GS_STATE_HOTKEY_SCREEN &&
				  new_state != GS_STATE_BRIEFING && new_state != GS_STATE_WEAPON_SELECT) {
				common_select_close();
				if ( new_state == GS_STATE_MAIN_MENU ) {
					freespace_stop_mission();	
				}
			}					
			break;

		case GS_STATE_MAIN_MENU:
#if defined(PRESS_TOUR_BUILD) || defined(PD_BUILD)
			mht_close();
#else
			main_hall_close();
#endif
			break;

		case GS_STATE_OPTIONS_MENU:
			//game_start_time();
			if(new_state == GS_STATE_MULTI_JOIN_GAME){
				multi_join_clear_game_list();
			}
			options_menu_close();
			break;

		case GS_STATE_BARRACKS_MENU:
			if(new_state != GS_STATE_VIEW_MEDALS){
				barracks_close();
			}
			break;

		case GS_STATE_MISSION_LOG_SCROLLBACK:
			hud_scrollback_close();
			break;

		case GS_STATE_TRAINING_MENU:
			training_menu_close();
			break;

		case GS_STATE_GAME_PLAY:
			if ( !(Game_mode & GM_STANDALONE_SERVER) ) {
				player_save_target_and_weapon_link_prefs();
				game_stop_looped_sounds();
			}

			sound_env_disable();
			g_ForceFeedback.StopEffects();

			// stop game time under certain conditions
			if ( end_mission || (Game_mode & GM_NORMAL) || ((Game_mode & GM_MULTIPLAYER) && (new_state == GS_STATE_MULTI_PAUSED)) ){
				game_stop_time();
			}

			if (end_mission) {
			// shut down any recording or playing demos
#ifdef DEMO_SYSTEM
				demo_close();
#endif

				// when in multiplayer and going back to the main menu, send a leave game packet
				// right away (before calling stop mission).  stop_mission was taking to long to
				// close mission down and I want people to get notified ASAP.
				if ( (Game_mode & GM_MULTIPLAYER) && (new_state == GS_STATE_MAIN_MENU) ){
					multi_quit_game(PROMPT_NONE);
				}

				freespace_stop_mission();			
				Game_time_compression = F1_0;
			}
			break;

		case GS_STATE_TECH_MENU:
			techroom_close();
			break;

		case GS_STATE_TRAINING_PAUSED:
			Training_num_lines = 0;
			// fall through to GS_STATE_GAME_PAUSED

		case GS_STATE_GAME_PAUSED:
			game_start_time();
			if ( end_mission ) {
				pause_close(0);
			}
			break;

		case GS_STATE_DEBUG_PAUSED:
			#ifndef NDEBUG
				game_start_time();
				pause_debug_close();
			#endif
			break;

		case GS_STATE_HUD_CONFIG:
			hud_config_close();
			break;

		// join/start a game
		case GS_STATE_MULTI_JOIN_GAME:
			if(new_state != GS_STATE_OPTIONS_MENU){
				multi_join_game_close();
			}
			break;

		case GS_STATE_MULTI_HOST_SETUP:
		case GS_STATE_MULTI_CLIENT_SETUP:
			// if this is just the host going into the options screen, don't do anything
			if((new_state == GS_STATE_MULTI_HOST_OPTIONS) || (new_state == GS_STATE_OPTIONS_MENU)){
				break;
			}

			// close down the proper state
			if(old_state == GS_STATE_MULTI_HOST_SETUP){
				multi_create_game_close();
			} else {
				multi_game_client_setup_close();
			}

			// COMMAND LINE OPTION
			if (Cmdline_multi_stream_chat_to_file){
				if( (new_state != GS_STATE_TEAM_SELECT) && (Multi_chat_stream!=NULL) ) {
					cfwrite_string(NOX("-------------------------------------------\n"),Multi_chat_stream);
					cfclose(Multi_chat_stream);
				}
			}			
			break;

		case GS_STATE_CONTROL_CONFIG:
			control_config_close();
			break;

		case GS_STATE_DEATH_DIED:
			Game_mode &= ~GM_DEAD_DIED;
			
			// early end while respawning or blowing up in a multiplayer game
			if((Game_mode & GM_MULTIPLAYER) && ((new_state == GS_STATE_DEBRIEF) || (new_state == GS_STATE_MULTI_DOGFIGHT_DEBRIEF)) ){
				game_stop_time();
				freespace_stop_mission();
			}
			break;

		case GS_STATE_DEATH_BLEW_UP:
			Game_mode &= ~GM_DEAD_BLEW_UP;

			// for single player, we might reload mission, etc.  For multiplayer, look at my new state
			// to determine if I should do anything.
			if ( !(Game_mode & GM_MULTIPLAYER) ) {
				if ( end_mission ){
					freespace_stop_mission();
				}
			} else {
				// if we are not respawing as an observer or as a player, our new state will not
				// be gameplay state.
				if ( (new_state != GS_STATE_GAME_PLAY) && (new_state != GS_STATE_MULTI_PAUSED) ) {
					game_stop_time();									// hasn't been called yet!!
					freespace_stop_mission();
				}
			}
			break;


		case GS_STATE_CREDITS:
			credits_close();
			break;

		case GS_STATE_VIEW_MEDALS:
			medal_main_close();
			break;

		case GS_STATE_SHOW_GOALS:
			mission_show_goals_close();
			break;

		case GS_STATE_HOTKEY_SCREEN:
			if ( new_state != GS_STATE_OPTIONS_MENU ) {
				mission_hotkey_close();
			}
			break;

		case GS_STATE_MULTI_MISSION_SYNC:
			// if we're moving into the options menu, don't do anything
			if(new_state == GS_STATE_OPTIONS_MENU){
				break;
			}

			Assert( Game_mode & GM_MULTIPLAYER );
			multi_sync_close();
			if ( new_state == GS_STATE_GAME_PLAY ){
				// palette_restore_palette();

				// change a couple of flags to indicate our state!!!
				Net_player->state = NETPLAYER_STATE_IN_MISSION;
				send_netplayer_update_packet();

				// set the game mode
				Game_mode |= GM_IN_MISSION;
			}			
			break;		
   
		case GS_STATE_VIEW_CUTSCENES:
			cutscenes_screen_close();
			break;

		case GS_STATE_MULTI_STD_WAIT:
			multi_standalone_wait_close();
	  		break;

		case GS_STATE_STANDALONE_MAIN:			
			standalone_main_close();
			if(new_state == GS_STATE_MULTI_STD_WAIT){		
				init_multiplayer_stats();										
			}			
			break;

		case GS_STATE_MULTI_PAUSED:
			// if ( end_mission ){
				pause_close(1);
			// }
			break;			

		case GS_STATE_INGAME_PRE_JOIN:
			multi_ingame_select_close();
			break;

		case GS_STATE_STANDALONE_POSTGAME:
			multi_standalone_postgame_close();
			break;

		case GS_STATE_INITIAL_PLAYER_SELECT:			
			player_select_close();			
			break;		

		case GS_STATE_MULTI_START_GAME:
			multi_start_game_close();
			break;

		case GS_STATE_MULTI_HOST_OPTIONS:
			multi_host_options_close();
			break;				

		case GS_STATE_END_OF_CAMPAIGN:
			mission_campaign_end_close();
			break;

		case GS_STATE_LOOP_BRIEF:
			loop_brief_close();
			break;
	}
}

// Called when a state is being entered.
// The current state is set to the state we're entering at
// this point, and old_state is set to the state we're coming
// from.    You should never try to change the state
// in here... if you think you need to, you probably really
// need to post an event, not change the state.

void game_enter_state( int old_state, int new_state )
{
	switch (new_state) {
		case GS_STATE_MAIN_MENU:				
			// in multiplayer mode, be sure that we are not doing networking anymore.
			if ( Game_mode & GM_MULTIPLAYER ) {
				Assert( Net_player != NULL );
				Net_player->flags &= ~NETINFO_FLAG_DO_NETWORKING;
			}

			Game_time_compression = F1_0;
	
			// determine which ship this guy is currently based on
#if defined(PRESS_TOUR_BUILD) || defined(PD_BUILD)
			mht_init();
#else
			if (Player->on_bastion) {
				main_hall_init(1);
			} else {
				main_hall_init(0);
			}
#endif
			break;

		case GS_STATE_BRIEFING:
			main_hall_stop_music();
			main_hall_stop_ambient();
			
			if (Game_mode & GM_NORMAL) {
				// AL: Don't call freespace_start_mission() if re-entering from ship or weapon select
				// MWA: or from options or hotkey screens
				// JH: or if the command brief state already did this
				if ( (old_state != GS_STATE_OPTIONS_MENU) && (old_state != GS_STATE_HOTKEY_SCREEN)
					&& (old_state != GS_STATE_SHIP_SELECT) && (old_state != GS_STATE_WEAPON_SELECT)
					&& (old_state != GS_STATE_CMD_BRIEF) ) {
					if ( !game_start_mission() )			// this should put us into a new state on failure!
						break;
				}
			}
			// maybe play a movie before the briefing.  don't play if entering briefing screen from ship or weapon select.
			if ( (old_state == GS_STATE_DEBRIEF) || (old_state == GS_STATE_SIMULATOR_ROOM) || (old_state == GS_STATE_MAIN_MENU))
				mission_campaign_maybe_play_movie(CAMPAIGN_MOVIE_PRE_MISSION);

			Game_time_compression = F1_0;

			if ( red_alert_mission() ) {
				gameseq_post_event(GS_EVENT_RED_ALERT);
			} else {
				brief_init();
			}

			break;

		case GS_STATE_DEBRIEF:
			game_stop_looped_sounds();
			mission_goal_fail_incomplete();				// fail all incomplete goals before entering debriefing
			if ( (old_state != GS_STATE_VIEW_MEDALS) && (old_state != GS_STATE_OPTIONS_MENU) ){
				debrief_init();
			}
			break;

		case GS_STATE_MULTI_DOGFIGHT_DEBRIEF:
			multi_df_debrief_init();
			break;

		case GS_STATE_LOAD_MISSION_MENU:
			mission_load_menu_init();
			break;

		case GS_STATE_SIMULATOR_ROOM:
			sim_room_init();
			break;

		case GS_STATE_CAMPAIGN_ROOM:
			campaign_room_init();
			break;

		case GS_STATE_RED_ALERT:
			mission_campaign_maybe_play_movie(CAMPAIGN_MOVIE_PRE_MISSION);
			red_alert_init();
			break;

		case GS_STATE_CMD_BRIEF: {
			int team_num = 0;  // team number used as index for which cmd brief to use.

			if (old_state == GS_STATE_OPTIONS_MENU) {
				cmd_brief_unhold();

			} else {
				main_hall_stop_music();
				main_hall_stop_ambient();

				if (Game_mode & GM_NORMAL) {
					// AL: Don't call freespace_start_mission() if re-entering from ship or weapon select
					// MWA: or from options or hotkey screens
					// JH: or if the command brief state already did this
					if ( (old_state != GS_STATE_OPTIONS_MENU) && (old_state != GS_STATE_HOTKEY_SCREEN)
						&& (old_state != GS_STATE_SHIP_SELECT) && (old_state != GS_STATE_WEAPON_SELECT) ) {
						if ( !game_start_mission() )			// this should put us into a new state on failure!
							break;
					}
				}

				// maybe play a movie before the briefing.  don't play if entering briefing screen from ship or weapon select.
				if ( (old_state == GS_STATE_DEBRIEF) || (old_state == GS_STATE_SIMULATOR_ROOM) || (old_state == GS_STATE_MAIN_MENU))
					mission_campaign_maybe_play_movie(CAMPAIGN_MOVIE_PRE_MISSION);

				cmd_brief_init(team_num);
			}

			break;
		}

		case GS_STATE_SHIP_SELECT:
			ship_select_init();
			break;

		case GS_STATE_WEAPON_SELECT:
			weapon_select_init();
			break;

		case GS_STATE_TEAM_SELECT:		
			multi_ts_init();
			break;

		case GS_STATE_GAME_PAUSED:
			game_stop_time();
			pause_init(0);
			break;

		case GS_STATE_DEBUG_PAUSED:
	//		game_stop_time();
	//		os_set_title("FreeSpace - PAUSED");
	//		break;
	//
		case GS_STATE_TRAINING_PAUSED:
			#ifndef NDEBUG
				game_stop_time();
				pause_debug_init();
			#endif
			break;

		case GS_STATE_OPTIONS_MENU:
			//game_stop_time();
			options_menu_init();
			break;
 
		case GS_STATE_GAME_PLAY:
			// coming from the gameplay state or the main menu, we might need to load the mission
			if ( (Game_mode & GM_NORMAL) && ((old_state == GS_STATE_MAIN_MENU) || (old_state == GS_STATE_GAME_PLAY) || (old_state == GS_STATE_DEATH_BLEW_UP)) ) {
				if ( !game_start_mission() )		// this should put us into a new state.
					// Failed!!!
					break;
			}

			// if we are coming from the briefing, ship select, weapons loadout, or main menu (in the
			// case of quick start), then do bitmap loads, etc  Don't do any of the loading stuff
			// if we are in multiplayer -- this stuff is all handled in the multi-wait section
			if ( !(Game_mode & GM_MULTIPLAYER) && (old_state == GS_STATE_BRIEFING) || (old_state == GS_STATE_SHIP_SELECT) ||
				(old_state == GS_STATE_WEAPON_SELECT) || (old_state == GS_STATE_MAIN_MENU) || (old_state == GS_STATE_MULTI_STD_WAIT)	|| (old_state == GS_STATE_SIMULATOR_ROOM) ) {
					// JAS: Used to do all paging here.

					#ifndef NDEBUG
					//XSTR:OFF
						HUD_printf("Skill level is set to ** %s **", Skill_level_names(Game_skill_level));
					//XSTR:ON
					#endif

					main_hall_stop_music();
					main_hall_stop_ambient();
					event_music_first_pattern();	// start the first pattern
			}

			// special code that restores player ship selection and weapons loadout when doing a quick start
			if ( !(Game_mode & GM_MULTIPLAYER) && (old_state == GS_STATE_MAIN_MENU) || (old_state == GS_STATE_DEATH_BLEW_UP)  || (old_state == GS_STATE_GAME_PLAY) ) {
				if ( !stricmp(Player_loadout.filename, Game_current_mission_filename) ) {
					wss_direct_restore_loadout();
				}
			}

			// single-player, quick-start after just died... we need to set weapon linking and kick off the event music
			if (!(Game_mode & GM_MULTIPLAYER) && (old_state == GS_STATE_DEATH_BLEW_UP) ) {
				event_music_first_pattern();	// start the first pattern
			}

			if ( !(Game_mode & GM_STANDALONE_SERVER) && (old_state != GS_STATE_GAME_PAUSED) && (old_state != GS_STATE_MULTI_PAUSED) ) {
				event_music_first_pattern();	// start the first pattern
			}			
			player_restore_target_and_weapon_link_prefs();

			Game_mode |= GM_IN_MISSION;

#ifndef NDEBUG
			// required to truely make mouse deltas zeroed in debug mouse code
void mouse_force_pos(int x, int y);
			mouse_force_pos(gr_screen.max_w / 2, gr_screen.max_h / 2);
#endif

			game_flush();

			// only start time if in single player, or coming from multi wait state
			if (
					(
						(Game_mode & GM_NORMAL) && 
						(old_state != GS_STATE_VIEW_CUTSCENES)
					) || (
						(Game_mode & GM_MULTIPLAYER) && (
							(old_state == GS_STATE_MULTI_PAUSED) ||
							(old_state == GS_STATE_MULTI_MISSION_SYNC)
						)
					)
				)
					game_start_time();

			// when coming from the multi paused state, reset the timestamps
			if ( (Game_mode & GM_MULTIPLAYER) && (old_state == GS_STATE_MULTI_PAUSED) ){
				multi_reset_timestamps();
			}

			if ((Game_mode & GM_MULTIPLAYER) && (old_state != GS_STATE_DEATH_BLEW_UP) ) {
				// initialize all object update details
				multi_oo_gameplay_init();
			}
	
			// under certain circumstances, the server should reset the object update rate limiting stuff
			if( ((Game_mode & GM_MULTIPLAYER) && (Net_player->flags & NETINFO_FLAG_AM_MASTER)) &&
				 (old_state == GS_STATE_MULTI_PAUSED) || (old_state == GS_STATE_MULTI_MISSION_SYNC) ){
				
				// reinitialize the rate limiting system for all clients
				multi_oo_rate_init_all();
			}

			// multiplayer clients should always re-initialize their control info rate limiting system			
			if((Game_mode & GM_MULTIPLAYER) && !(Net_player->flags & NETINFO_FLAG_AM_MASTER)){
				multi_oo_rate_init_all();
			}
			
			// reset ping times
			if(Game_mode & GM_MULTIPLAYER){
				multi_ping_reset_players();
			}

			Game_subspace_effect = 0;
			if (The_mission.flags & MISSION_FLAG_SUBSPACE) {
				Game_subspace_effect = 1;
				if( !(Game_mode & GM_STANDALONE_SERVER) ){	
					game_start_subspace_ambient_sound();
				}
			}

			sound_env_set(&Game_sound_env);
			g_ForceFeedback.MissionInit(Ship_info[Player_ship->ship_info_index].rotation_time);

			// clear multiplayer button info			i
			extern button_info Multi_ship_status_bi;
			memset(&Multi_ship_status_bi, 0, sizeof(button_info));
			break;

		case GS_STATE_HUD_CONFIG:
			hud_config_init();
			break;

		case GS_STATE_MULTI_JOIN_GAME:
			multi_join_clear_game_list();

			if (old_state != GS_STATE_OPTIONS_MENU) {
				multi_join_game_init();
			}

			break;

		case GS_STATE_MULTI_HOST_SETUP:		
			// don't reinitialize if we're coming back from the host options screen
			if ((old_state != GS_STATE_MULTI_HOST_OPTIONS) && (old_state != GS_STATE_OPTIONS_MENU)) {
				multi_create_game_init();
			}

			break;

		case GS_STATE_MULTI_CLIENT_SETUP:		
			if (old_state != GS_STATE_OPTIONS_MENU) {
				multi_game_client_setup_init();
			}

			break;

		case GS_STATE_CONTROL_CONFIG:
			control_config_init();
			break;

		case GS_STATE_TECH_MENU:
			techroom_init();
			break;

		case GS_STATE_BARRACKS_MENU:
			if(old_state != GS_STATE_VIEW_MEDALS){
				barracks_init();
			}
			break;

		case GS_STATE_MISSION_LOG_SCROLLBACK:
			hud_scrollback_init();
			break;

		case GS_STATE_DEATH_DIED:
 			Player_died_time = timestamp(10);

			if(!(Game_mode & GM_MULTIPLAYER)){
				player_show_death_message();
			}
			Game_mode |= GM_DEAD_DIED;
			break;

		case GS_STATE_DEATH_BLEW_UP:
			if ( !popupdead_is_active() ) {
				Player_ai->target_objnum = -1;
			}

			// stop any local EMP effect
			emp_stop_local();

			Player->flags &= ~PLAYER_FLAGS_AUTO_TARGETING;	//	Prevent immediate switch to a hostile ship.
			Game_mode |= GM_DEAD_BLEW_UP;		
			Show_viewing_from_self = 0;

			// timestamp how long we should wait before displaying the died popup
			if ( !popupdead_is_active() ) {
				Player_died_popup_wait = timestamp(PLAYER_DIED_POPUP_WAIT);
			}
			break;

		case GS_STATE_GAMEPLAY_HELP:
			gameplay_help_init();
			break;

		case GS_STATE_CREDITS:
			main_hall_stop_music();
			main_hall_stop_ambient();
			credits_init();
			break;

		case GS_STATE_VIEW_MEDALS:
			medal_main_init(Player);
			break;

		case GS_STATE_SHOW_GOALS:
			mission_show_goals_init();
			break;

		case GS_STATE_HOTKEY_SCREEN:
			mission_hotkey_init();
			break;

		case GS_STATE_MULTI_MISSION_SYNC:
			// if we're coming from the options screen, don't do any
			if(old_state == GS_STATE_OPTIONS_MENU){
				break;
			}

			switch(Multi_sync_mode){
			case MULTI_SYNC_PRE_BRIEFING:
				// if moving from game forming to the team select state						
				multi_sync_init();			
				break;
			case MULTI_SYNC_POST_BRIEFING:
				// if moving from briefing into the mission itself			
				multi_sync_init();
			
				// tell everyone that we're now loading data
				Net_player->state = NETPLAYER_STATE_DATA_LOAD;
				send_netplayer_update_packet();

				// JAS: Used to do all paging here!!!!
								
				Net_player->state = NETPLAYER_STATE_WAITING;			
				send_netplayer_update_packet();				
				Missiontime = 0;
				Game_time_compression = F1_0;
				break;
			case MULTI_SYNC_INGAME:
				multi_sync_init();
				break;
			}
			break;		
   
		case GS_STATE_VIEW_CUTSCENES:
			cutscenes_screen_init();
			break;

		case GS_STATE_MULTI_STD_WAIT:
			multi_standalone_wait_init();
			break;

		case GS_STATE_STANDALONE_MAIN:
			// don't initialize if we're coming from one of these 2 states unless there are no 
			// players left (reset situation)
			if((old_state != GS_STATE_STANDALONE_POSTGAME) || multi_endgame_ending()){
				standalone_main_init();
			}
			break;	

		case GS_STATE_MULTI_PAUSED:
			pause_init(1);
			break;
		
		case GS_STATE_INGAME_PRE_JOIN:
			multi_ingame_select_init();
			break;

		case GS_STATE_STANDALONE_POSTGAME:
			multi_standalone_postgame_init();
			break;

		case GS_STATE_INITIAL_PLAYER_SELECT:
			player_select_init();
			break;		

		case GS_STATE_MULTI_START_GAME:
			multi_start_game_init();
			break;

		case GS_STATE_MULTI_HOST_OPTIONS:
			multi_host_options_init();
			break;		

		case GS_STATE_END_OF_CAMPAIGN:
			mission_campaign_end_init();
			break;		

		case GS_STATE_LOOP_BRIEF:
			loop_brief_init();
			break;

	} // end switch
}

// do stuff that may need to be done regardless of state
void game_do_state_common(int state,int no_networking)
{
	game_maybe_draw_mouse(flFrametime);		// determine if to draw the mouse this frame
	snd_do_frame();								// update sound system
	event_music_do_frame();						// music needs to play across many states

	multi_log_process();	

	if (no_networking) {
		return;
	}

	// maybe do a multiplayer frame based on game mode and state type	
	if (Game_mode & GM_MULTIPLAYER) {
		switch (state) {
			case GS_STATE_OPTIONS_MENU:
			case GS_STATE_GAMEPLAY_HELP:
			case GS_STATE_HOTKEY_SCREEN:
			case GS_STATE_HUD_CONFIG:
			case GS_STATE_CONTROL_CONFIG:
			case GS_STATE_MISSION_LOG_SCROLLBACK:
			case GS_STATE_SHOW_GOALS:
			case GS_STATE_VIEW_CUTSCENES:
			case GS_STATE_EVENT_DEBUG:
				multi_maybe_do_frame();
				break;
		}
		
		game_do_networking();
	}
}

// Called once a frame.
// You should never try to change the state
// in here... if you think you need to, you probably really
// need to post an event, not change the state.
int Game_do_state_should_skip = 0;
void game_do_state(int state)
{
	// always lets the do_state_common() function determine if the state should be skipped
	Game_do_state_should_skip = 0;
	
	// legal to set the should skip state anywhere in this function
	game_do_state_common(state);	// do stuff that may need to be done regardless of state

	if(Game_do_state_should_skip){
		return;
	}
	
	switch (state) {
		case GS_STATE_MAIN_MENU:

			if(Cmdline_autoload)
			{
				// No support for skipping setup
				//Assert(Cmdline_splitscreen);

				// Set defaults
				strcpy_s(Game_current_mission_filename, MAX_FILENAME_LEN, Cmdline_autoload);
				Select_default_ship = 1;

				gameseq_post_event(GS_EVENT_START_GAME_QUICK);
				// Disable autoload
				Cmdline_autoload = 0;
			}
			else
			{

				game_set_frametime(GS_STATE_MAIN_MENU);
#if defined(PRESS_TOUR_BUILD) || defined(PD_BUILD)
				mht_do();
#else
				main_hall_do(flFrametime);
#endif
			}
			break;

		case GS_STATE_OPTIONS_MENU:
			game_set_frametime(GS_STATE_OPTIONS_MENU);
			options_menu_do_frame(flFrametime);
			break;

		case GS_STATE_BARRACKS_MENU:
			game_set_frametime(GS_STATE_BARRACKS_MENU);
			barracks_do_frame(flFrametime);
			break;

		case GS_STATE_TRAINING_MENU:
			game_set_frametime(GS_STATE_TRAINING_MENU);
			training_menu_do_frame(flFrametime);
			break;

		case GS_STATE_TECH_MENU:
			game_set_frametime(GS_STATE_TECH_MENU);
			techroom_do_frame(flFrametime);
			break;

		case GS_STATE_GAMEPLAY_HELP:
			game_set_frametime(GS_STATE_GAMEPLAY_HELP);
			gameplay_help_do_frame(flFrametime);
			break;

		case GS_STATE_GAME_PLAY:	// do stuff that should be done during gameplay
			game_do_frame();
			break;

		case GS_STATE_GAME_PAUSED:
			pause_do(0);
			break;

		case GS_STATE_DEBUG_PAUSED:
			#ifndef NDEBUG
				game_set_frametime(GS_STATE_DEBUG_PAUSED);
				pause_debug_do();
			#endif
			break;

		case GS_STATE_TRAINING_PAUSED:
			game_training_pause_do();
			break;

		case GS_STATE_LOAD_MISSION_MENU:
			game_set_frametime(GS_STATE_LOAD_MISSION_MENU);
			mission_load_menu_do();
			break;
		
		case GS_STATE_BRIEFING:
			game_set_frametime(GS_STATE_BRIEFING);
			brief_do_frame(flFrametime);
			break;

		case GS_STATE_DEBRIEF:
			game_set_frametime(GS_STATE_DEBRIEF);
			debrief_do_frame(flFrametime);
			break;

		case GS_STATE_MULTI_DOGFIGHT_DEBRIEF:
			game_set_frametime(GS_STATE_MULTI_DOGFIGHT_DEBRIEF);
			multi_df_debrief_do();
			break;

		case GS_STATE_SHIP_SELECT:
			game_set_frametime(GS_STATE_SHIP_SELECT);
			ship_select_do(flFrametime);
			break;

		case GS_STATE_WEAPON_SELECT:
			game_set_frametime(GS_STATE_WEAPON_SELECT);
			weapon_select_do(flFrametime);
			break;

		case GS_STATE_MISSION_LOG_SCROLLBACK:
			game_set_frametime(GS_STATE_MISSION_LOG_SCROLLBACK);
			hud_scrollback_do_frame(flFrametime);
			break;

		case GS_STATE_HUD_CONFIG:
			game_set_frametime(GS_STATE_HUD_CONFIG);
			hud_config_do_frame(flFrametime);
			break;

		case GS_STATE_MULTI_JOIN_GAME:
			game_set_frametime(GS_STATE_MULTI_JOIN_GAME);
			multi_join_game_do_frame();
			break;

		case GS_STATE_MULTI_HOST_SETUP:
			game_set_frametime(GS_STATE_MULTI_HOST_SETUP);
			multi_create_game_do();
			break;

		case GS_STATE_MULTI_CLIENT_SETUP:
			game_set_frametime(GS_STATE_MULTI_CLIENT_SETUP);
			multi_game_client_setup_do_frame();
			break;

		case GS_STATE_CONTROL_CONFIG:
			game_set_frametime(GS_STATE_CONTROL_CONFIG);
			control_config_do_frame(flFrametime);
			break;	

		case GS_STATE_DEATH_DIED:
			game_do_frame();			
			break;

		case GS_STATE_DEATH_BLEW_UP:
			game_do_frame();
			break;

		case GS_STATE_SIMULATOR_ROOM:
			game_set_frametime(GS_STATE_SIMULATOR_ROOM);
			sim_room_do_frame(flFrametime);
			break;

		case GS_STATE_CAMPAIGN_ROOM:
			game_set_frametime(GS_STATE_CAMPAIGN_ROOM);
			campaign_room_do_frame(flFrametime);
			break;

		case GS_STATE_RED_ALERT:
			game_set_frametime(GS_STATE_RED_ALERT);
			red_alert_do_frame(flFrametime);
			break;

		case GS_STATE_CMD_BRIEF:
			game_set_frametime(GS_STATE_CMD_BRIEF);
			cmd_brief_do_frame(flFrametime);
			break;

		case GS_STATE_CREDITS:
			game_set_frametime(GS_STATE_CREDITS);
			credits_do_frame(flFrametime);
			break;

		case GS_STATE_VIEW_MEDALS:
			game_set_frametime(GS_STATE_VIEW_MEDALS);
			medal_main_do();
			break;

		case GS_STATE_SHOW_GOALS:
			game_set_frametime(GS_STATE_SHOW_GOALS);
			mission_show_goals_do_frame(flFrametime);
			break;

		case GS_STATE_HOTKEY_SCREEN:
			game_set_frametime(GS_STATE_HOTKEY_SCREEN);
			mission_hotkey_do_frame(flFrametime);
			break;	
   
		case GS_STATE_VIEW_CUTSCENES:
			game_set_frametime(GS_STATE_VIEW_CUTSCENES);
			cutscenes_screen_do_frame();
			break;

		case GS_STATE_MULTI_STD_WAIT:
			game_set_frametime(GS_STATE_STANDALONE_MAIN);
			multi_standalone_wait_do();
			break;

		case GS_STATE_STANDALONE_MAIN:
			game_set_frametime(GS_STATE_STANDALONE_MAIN);
			standalone_main_do();
			break;	

		case GS_STATE_MULTI_PAUSED:
			game_set_frametime(GS_STATE_MULTI_PAUSED);
			pause_do(1);
			break;

		case GS_STATE_TEAM_SELECT:
			game_set_frametime(GS_STATE_TEAM_SELECT);
			multi_ts_do();
			break;

		case GS_STATE_INGAME_PRE_JOIN:
			game_set_frametime(GS_STATE_INGAME_PRE_JOIN);
			multi_ingame_select_do();
			break;

		case GS_STATE_EVENT_DEBUG:
	#ifndef NDEBUG
			game_set_frametime(GS_STATE_EVENT_DEBUG);
			game_show_event_debug(flFrametime);
	#endif
			break;

		case GS_STATE_STANDALONE_POSTGAME:
			game_set_frametime(GS_STATE_STANDALONE_POSTGAME);
			multi_standalone_postgame_do();
			break;

		case GS_STATE_INITIAL_PLAYER_SELECT:
			game_set_frametime(GS_STATE_INITIAL_PLAYER_SELECT);
			player_select_do();
			break;

		case GS_STATE_MULTI_MISSION_SYNC:
			game_set_frametime(GS_STATE_MULTI_MISSION_SYNC);
			multi_sync_do();
			break;		

		case GS_STATE_MULTI_START_GAME:
			game_set_frametime(GS_STATE_MULTI_START_GAME);
			multi_start_game_do();
			break;
		
		case GS_STATE_MULTI_HOST_OPTIONS:
			game_set_frametime(GS_STATE_MULTI_HOST_OPTIONS);
			multi_host_options_do();
			break;		

		case GS_STATE_END_OF_CAMPAIGN:
			mission_campaign_end_do();
			break;		

		case GS_STATE_END_DEMO:
			game_set_frametime(GS_STATE_END_DEMO);
			end_demo_campaign_do();
			break;

		case GS_STATE_LOOP_BRIEF:
			game_set_frametime(GS_STATE_LOOP_BRIEF);
			loop_brief_do();
			break;

   } // end switch(gs_current_state)
}


// return 0 if there is enough RAM to run FreeSpace, otherwise return -1
int game_do_ram_check(unsigned long ram_in_bytes)
{
	if ( ram_in_bytes < 30*1024*1024 )	{
		int allowed_to_run = 1;
		if ( ram_in_bytes < 25*1024*1024 ) {
			allowed_to_run = 0;
		}

		char tmp[1024];
		int Freespace_total_ram_MB;
		Freespace_total_ram_MB = fl2i(ram_in_bytes/(1024*1024));

		if ( allowed_to_run ) {

			sprintf( tmp, XSTR( "FreeSpace has detected that you only have %dMB of free memory.\n\nFreeSpace requires at least 32MB of memory to run.  If you think you have more than %dMB of physical memory, ensure that you aren't running SmartDrive (SMARTDRV.EXE).  Any memory allocated to SmartDrive is not usable by applications\n\nPress 'OK' to continue running with less than the minimum required memory\n", 193), Freespace_total_ram_MB, Freespace_total_ram_MB);

			int msgbox_rval;
			msgbox_rval = MessageBoxA( NULL, tmp, XSTR( "Not Enough RAM", 194), MB_OKCANCEL );
			if ( msgbox_rval == IDCANCEL ) {
				return -1;
			}

		} else {
			sprintf( tmp, XSTR( "FreeSpace has detected that you only have %dMB of free memory.\n\nFreeSpace requires at least 32MB of memory to run.  If you think you have more than %dMB of physical memory, ensure that you aren't running SmartDrive (SMARTDRV.EXE).  Any memory allocated to SmartDrive is not usable by applications\n", 195), Freespace_total_ram_MB, Freespace_total_ram_MB);
			MessageBoxA( NULL, tmp, XSTR( "Not Enough RAM", 194), MB_OK );
			return -1;
		}
	}

	return 0;
}

// Check if there is a freespace.exe in the /update directory (relative to where fs.exe is installed).
// If so, copy it over and remove the update directory.
void game_maybe_update_launcher(char *exe_dir)
{
	char src_filename[MAX_PATH];
	char dest_filename[MAX_PATH];

	strcpy(src_filename, exe_dir);
	strcat(src_filename, NOX("\\update\\freespace.exe"));

	strcpy(dest_filename, exe_dir);
	strcat(dest_filename, NOX("\\freespace.exe"));

	// see if src_filename exists
	FILE *fp;
	fp = fopen(src_filename, "rb");
	if ( !fp ) {
		return;
	}
	fclose(fp);

	SetFileAttributesA(dest_filename, FILE_ATTRIBUTE_NORMAL);

	// copy updated freespace.exe to freespace exe dir
	if ( CopyFileA(src_filename, dest_filename, 0) == 0 ) {
		MessageBoxA( NULL, XSTR("Unable to copy freespace.exe from update directory to installed directory.  You should copy freespace.exe from the update directory (located in your FreeSpace install directory) to your install directory", 988), NULL, MB_OK|MB_TASKMODAL|MB_SETFOREGROUND );
		return;
	}

	// delete the file in the update directory
	DeleteFileA(src_filename);

	// safe to assume directory is empty, since freespace.exe should only be the file ever in the update dir
	char update_dir[MAX_PATH];
	strcpy(update_dir, exe_dir);
	strcat(update_dir, NOX("\\update"));
	RemoveDirectoryA(update_dir);
}

void game_spew_pof_info_sub(int model_num, polymodel *pm, int sm, CFILE *out, int *out_total, int *out_destroyed_total)
{
	int i;
	int sub_total = 0;
	int sub_total_destroyed = 0;
	int total = 0;
	char str[255] = "";		
	
	// get the total for all his children
	for (i=pm->submodel[sm].first_child; i>-1; i = pm->submodel[i].next_sibling )	{
		game_spew_pof_info_sub(model_num, pm, i, out, &sub_total, &sub_total_destroyed);
	}	

	// find the # of faces for this _individual_ object	
	total = submodel_get_num_polys(model_num, sm);
	if(strstr(pm->submodel[sm].name, "-destroyed")){
		sub_total_destroyed = total;
	}
	
	// write out total
	sprintf(str, "Submodel %s total : %d faces\n", pm->submodel[sm].name, total);
	cfputs(str, out);		

	*out_total += total + sub_total;
	*out_destroyed_total += sub_total_destroyed;
}

#define BAIL()			do { int idx; for(idx=0; idx<num_files; idx++){ if(pof_list[idx] != NULL){free(pof_list[idx]); pof_list[idx] = NULL;}} return;} while(0);
void game_spew_pof_info()
{
	char *pof_list[1000];
	int num_files;	
	CFILE *out;
	int idx, model_num, i, j;
	polymodel *pm;
	int total, root_total, model_total, destroyed_total, counted;
	char str[255] = "";

	// get file list
	num_files = cf_get_file_list(1000, pof_list, CF_TYPE_MODELS, "*.pof");

	// spew info on all the pofs
	if(!num_files){
		return;
	}

	// go
	out = cfopen("pofspew.txt", "wt", CFILE_NORMAL, CF_TYPE_DATA);
	if(out == NULL){
		BAIL();
	}	
	counted = 0;	
	for(idx=0; idx<num_files; idx++, counted++){
		sprintf(str, "%s.pof", pof_list[idx]);
		model_num = model_load(str, 0, NULL);
		if(model_num >= 0){
			pm = model_get(model_num);

			// if we have a real model
			if(pm != NULL){				
				cfputs(str, out);
				cfputs("\n", out);
				
				// go through and print all raw submodels
				cfputs("RAW\n", out);
				total = 0;
				model_total = 0;				
				for (i=0; i<pm->n_models; i++)	{					
					total = submodel_get_num_polys(model_num, i);					
					
					model_total += total;
					sprintf(str, "Submodel %s total : %d faces\n", pm->submodel[i].name, total);
					cfputs(str, out);
				}				
				sprintf(str, "Model total %d\n", model_total);				
				cfputs(str, out);				

				// now go through and do it by LOD
				cfputs("BY LOD\n\n", out);				
				for(i=0; i<pm->n_detail_levels; i++){
					sprintf(str, "LOD %d\n", i);
					cfputs(str, out);

					// submodels
					root_total = submodel_get_num_polys(model_num, pm->detail[i] );
					total = 0;
					destroyed_total = 0;
					for (j=pm->submodel[pm->detail[i]].first_child; j>-1; j = pm->submodel[j].next_sibling )	{
						game_spew_pof_info_sub(model_num, pm, j, out, &total, &destroyed_total);
					}

					sprintf(str, "Submodel %s total : %d faces\n", pm->submodel[pm->detail[i]].name, root_total);
					cfputs(str, out);

					sprintf(str, "TOTAL: %d\n", total + root_total);					
					cfputs(str, out);
					sprintf(str, "TOTAL not counting destroyed faces %d\n", (total + root_total) - destroyed_total);
					cfputs(str, out);
					sprintf(str, "TOTAL destroyed faces %d\n\n", destroyed_total);
					cfputs(str, out);
				}				
				cfputs("------------------------------------------------------------------------\n\n", out);				
			}
		}

		if(counted >= MAX_POLYGON_MODELS - 5){
			model_free_all();
			counted = 0;
		}
	}

	cfclose(out);
	model_free_all();
	BAIL();
}

DCF(pofspew, "")
{
	game_spew_pof_info();
}

bool FREESPACE_Init(
#if !defined(FS2_UE)
	HINSTANCE hInst,
	LPSTR szCmdLine)
#else
	char *szCmdLine)
#endif
{
	// Don't let more than one instance of Freespace run.
	HWND hwnd = FindWindowA(NOX("FreeSpaceClass"), NULL);
	if (hwnd) {
		SetForegroundWindow(hwnd);
		return 0;
	}

	// Find out how much RAM is on this machine
	MEMORYSTATUS ms;
	ms.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&ms);
	Freespace_total_ram = ms.dwTotalPhys;

	if (game_do_ram_check(Freespace_total_ram) == -1) {
		return 0;
	}

	if (ms.dwTotalVirtual < 1024) {
		MessageBoxA(NULL, XSTR("FreeSpace requires virtual memory to run.\r\n", 196), XSTR("No Virtual Memory", 197), MB_OK);
		return 0;
	}

#ifndef FS2_UE
	if (!vm_init(24 * 1024 * 1024)) {
		MessageBoxA(NULL, XSTR("Not enough memory to run Freespace.\r\nTry closing down some other applications.\r\n", 198), XSTR("Not Enough Memory", 199), MB_OK);
		return 0;
	}
#endif

	char *tmp_mem = (char *)malloc(16 * 1024 * 1024);
	if (!tmp_mem) {
		MessageBoxA(NULL, XSTR("Not enough memory to run Freespace.\r\nTry closing down some other applications.\r\n", 198), XSTR("Not Enough Memory", 199), MB_OK);
		return 0;
	}

	free(tmp_mem);
	tmp_mem = NULL;

	/* this code doesn't work, and we will hit an error about being unable to load the direct draw
		dll before we get here anyway if it's not installed (unless we load it manually, which doesn't
		seem worth bothering with.

		LONG lResult;

		lResult = RegOpenKeyEx(
			HKEY_LOCAL_MACHINE,					// Where it is
			"Software\\Microsoft\\DirectX",	// name of key
			NULL,										// DWORD reserved
			KEY_QUERY_VALUE,						// Allows all changes
			&hKey										// Location to store key
		);

		if (lResult == ERROR_SUCCESS) {
			char version[32];
			DWORD dwType, dwLen;

			dwLen = 32;
			lResult = RegQueryValueEx(
				hKey,									// Handle to key
				"Version",							// The values name
				NULL,									// DWORD reserved
				&dwType,								// What kind it is
				(ubyte *) version, 				// value to set
				&dwLen								// How many bytes to set
			);

			if (lResult == ERROR_SUCCESS) {
				dx_version = atoi(strstr(version, ".") + 1);

			} else {
				int val;
				DWORD dwType, dwLen;

				dwLen = 4;
				lResult = RegQueryValueEx(
					hKey,									// Handle to key
					"InstalledVersion",				// The values name
					NULL,									// DWORD reserved
					&dwType,								// What kind it is
					(ubyte *) &val,					// value to set
					&dwLen								// How many bytes to set
				);

				if (lResult == ERROR_SUCCESS) {
					dx_version = val;
				}
			}

			RegCloseKey(hKey);
		}

		if (dx_version < 3) {
			MessageBox(NULL, "DirectX 3.0 or higher is required and wasn't detected.  You can get the\n"
				"latest version of DirectX at:\n\n"
				"http://www.microsoft.com/msdownload/directx/dxf/enduser5.0/default.htm", "DirectX required", MB_OK);

			MessageBox(NULL, "DirectX 3.0 or higher is required and wasn't detected.  You can install\n"
				"DirectX 5.2 by pressing the 'Install DirectX' button on the FreeSpace Launcher", "DirectX required", MB_OK);

			return 0;
		}
	*/
#if !defined(FS2_UE)
	//=====================================================
	// Make sure we're running in the right directory.
	char exe_dir[1024];

	if (GetModuleFileNameA(hInst, exe_dir, 1023) > 0) {
		char *p = exe_dir + strlen(exe_dir);

		// chop off the filename
		while ((p > exe_dir) && (*p != '\\') && (*p != '/') && (*p != ':')) {
			p--;
		}
		*p = 0;

		// Set directory
		if (strlen(exe_dir) > 0) {
			SetCurrentDirectoryA(exe_dir);
		}

		// check for updated freespace.exe
		game_maybe_update_launcher(exe_dir);
	}
#endif


#ifndef NDEBUG				
	{
		extern void windebug_memwatch_init();
		windebug_memwatch_init();
	}
#endif

	parse_cmdline(szCmdLine);

#ifdef STANDALONE_ONLY_BUILD
	Is_standalone = 1;
	nprintf(("Network", "Standalone running"));
#else
	if (Is_standalone) {
		nprintf(("Network", "Standalone running"));
	}
#endif

	init_cdrom();
	game_init();
	game_stop_time();

	// maybe spew pof stuff
	if (Cmdline_spew_pof_info) {
		game_spew_pof_info();
		game_shutdown();
		return 1;
	}

	// non-demo, non-standalone, play the intro movie
	if (!Is_standalone) {
#ifndef DEMO
#ifdef RELEASE_REAL
		char *plist[5];
		if ((cf_get_file_list(2, plist, CF_TYPE_MULTI_PLAYERS, NOX("*.plr")) <= 0) && (cf_get_file_list(2, plist, CF_TYPE_SINGLE_PLAYERS, NOX("*.plr")) <= 0)) {
			// prompt for cd 2
#if defined(OEM_BUILD)
			game_do_cd_check_specific(FS_CDROM_VOLUME_1, 1);
#else
			game_do_cd_check_specific(FS_CDROM_VOLUME_2, 2);
#endif // defined(OEM_BUILD)
		}
#endif
	}

	if (!Is_standalone) {

		// release -- movies always play
#if defined(NDEBUG)

// in RELEASE_REAL builds make the user stick in CD2 if there are no pilots on disk so that we guarantee he plays the movie
// no soup for you!
// movie_play( NOX("intro.mve"), 0 );

// debug version, movie will only play with -showmovies
#else if !defined(NDEBUG)

// no soup for you!
// movie_play( NOX("intro.mve"), 0);
/*
#ifndef NDEBUG
		if ( Cmdline_show_movies )
			movie_play( NOX("intro.mve"), 0 );
#endif
*/
#endif
	}

#endif	

	if (Is_standalone) {
		gameseq_post_event(GS_EVENT_STANDALONE_MAIN);
	}
	else {
		gameseq_post_event(GS_EVENT_GAME_INIT);		// start the game rolling -- check for default pilot, or go to the pilot select screen
	}

	return 1;
}

bool FREESPACE_Update(const float DeltaTime)
{
	// only important for non THREADED mode
	os_poll();

	static int lastGameTime = 0;
	const int now = timer_get_milliseconds();
	GetPrimaryPad()->Update();
	g_MouseController.Update(now - lastGameTime, 0, 0, gr_screen.max_w, gr_screen.max_h);
	lastGameTime = now;

	int state = gameseq_process_events();
	if (state == GS_STATE_QUIT_GAME) {
		return false;
	}

	return true;
}

void FREESPACE_Shutdown()
{
#ifdef FS2_DEMO
	if(!Is_standalone){
		demo_upsell_show_screens();
	}
#elif defined(OEM_BUILD)
	// show upsell screens on exit
	oem_upsell_show_screens();
#endif

	game_shutdown();
}

#if !defined(FS2_UE)
int PASCAL WinMainSub(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int nCmdShow)
{
	if (FREESPACE_Init(hInst, szCmdLine) == false)
	{
		return 0;
	}

	while (1) 
	{
		if (FREESPACE_Update(0) == false)
		{
			break;
		}
	}

	FREESPACE_Shutdown();
	return 1;
}

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int nCmdShow)
{
	int result = -1;
	__try
	{
		::CoInitialize(NULL);
		result = WinMainSub(hInst, hPrev, szCmdLine, nCmdShow);
		::CoUninitialize();
	}
	__except(RecordExceptionInfo(GetExceptionInformation(), "Freespace 2 Main Thread"))
	{
		// Do nothing here - RecordExceptionInfo() has already done
		// everything that is needed. Actually this code won't even
		// get called unless you return EXCEPTION_EXECUTE_HANDLER from
		// the __except clause.
	}
	return !result;
}
#endif

// launcher the fslauncher program on exit
void game_launch_launcher_on_exit()
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	char cmd_line[2048];
	char original_path[MAX_PATH] = "";
	
	memset( &si, 0, sizeof(STARTUPINFOA) );
	si.cb = sizeof(si);

	// directory
	GetCurrentDirectoryA(MAX_PATH, original_path);
	//_getcwd(original_path, 1023);

	// set up command line
	strcpy(cmd_line, original_path);
	strcat(cmd_line, "\\");
	strcat(cmd_line, LAUNCHER_FNAME);
	strcat(cmd_line, " -straight_to_update");		

	BOOL ret = CreateProcessA(	NULL,									// pointer to name of executable module 
										cmd_line,							// pointer to command line string
										NULL,									// pointer to process security attributes 
										NULL,									// pointer to thread security attributes 
										FALSE,								// handle inheritance flag 
										CREATE_DEFAULT_ERROR_MODE,		// creation flags 
										NULL,									// pointer to new environment block 
										NULL,									// pointer to current directory name 
										&si,									// pointer to STARTUPINFO 
										&pi									// pointer to PROCESS_INFORMATION  
										);			
	// to eliminate build warnings
	ret;
}


// game_shutdown()
//
// This function is called when FreeSpace terminates normally.  
//
void game_shutdown(void)
{
	timeEndPeriod(1);

	g_VoiceRecognition.Deinit();
#if !defined(FS2_UE)
	g_TextToSpeech.Deinit();
#endif

	// don't ever flip a page on the standalone!
	if(!(Game_mode & GM_STANDALONE_SERVER)){
		gr_reset_clip();
		gr_clear();
		gr_flip();
	}

#if defined(FS2_UE)
	gr_close();
	freespace_stop_mission();
#endif

   // if the player has left the "player select" screen and quit the game without actually choosing
	// a player, Player will be NULL, in which case we shouldn't write the player file out!
	if (!(Game_mode & GM_STANDALONE_SERVER) && (Player!=NULL) && !Is_standalone){
		write_pilot_file();
	}

	// load up common multiplayer icons
	multi_unload_common_icons();

	HUD_deinit();
	
	shockwave_close();			// release any memory used by shockwave system	
	fireball_close();				// free fireball system
	ship_close();					// free any memory that was allocated for the ships
	hud_free_scrollback_list();// free space allocated to store hud messages in hud scrollback
	unload_animating_pointer();// frees the frames used for the animating mouse pointer
	bm_unload_all();				// free bitmaps
	bm_close();
	mission_campaign_close();	// close out the campaign stuff
	multi_voice_close();			// close down multiplayer voice (including freeing buffers, etc)
	multi_log_close();
#ifdef MULTI_USE_LAG
	multi_lag_close();
#endif

	// the menu close functions will unload the bitmaps if they were displayed during the game
#if !defined(PRESS_TOUR_BUILD) && !defined(PD_BUILD)
	main_hall_close();
#endif
	training_menu_close();
	gr_close();

	weapon_deinit();

	extern void joy_close();
	joy_close();

	audiostream_close();
	snd_close();
	event_music_close();
	psnet_close();

	lcl_close();
	//lcl_xstr_close();

	FlushAllAllocatedBeahviours();

#ifdef FS2_UE
	ship_deinit();
	multi_deinit();
	model_free_all();
#endif

	gr_font_deinit();
	cf_free_leaks();
	vm_strdup_free();

	os_cleanup();

	// HACKITY HACK HACK
	// if this flag is set, we should be firing up the launcher when exiting freespace
	extern int Multi_update_fireup_launcher_on_exit;
	if(Multi_update_fireup_launcher_on_exit){
		game_launch_launcher_on_exit();
	}
}

// game_stop_looped_sounds()
//
// This function will call the appropriate stop looped sound functions for those
// modules which use looping sounds.  It is not enough just to stop a looping sound
// at the DirectSound level, the game is keeping track of looping sounds, and this
// function is used to inform the game that looping sounds are being halted.
//
void game_stop_looped_sounds()
{
	hud_stop_looped_locking_sounds();
	hud_stop_looped_engine_sounds();
	afterburner_stop_sounds();
	player_stop_looped_sounds();
	obj_snd_stop_all();		// stop all object-linked persistant sounds
	game_stop_subspace_ambient_sound();
	snd_stop(Radar_static_looping);
	Radar_static_looping = -1;
	snd_stop(Target_static_looping);
	shipfx_stop_engine_wash_sound();
	Target_static_looping = -1;
}

//////////////////////////////////////////////////////////////////////////
//
// Code for supporting an animating mouse pointer
//
//
//////////////////////////////////////////////////////////////////////////

typedef struct animating_obj
{
	int	first_frame;
	int	num_frames;
	int	current_frame;
	float time;
	float elapsed_time;
} animating_obj;

static animating_obj Animating_mouse;

// ----------------------------------------------------------------------------
// init_animating_pointer()
//
// Called by load_animating_pointer() to ensure the Animating_mouse struct
// gets properly initialized
//
void init_animating_pointer()
{
	Animating_mouse.first_frame	= -1;
	Animating_mouse.num_frames		= 0;
	Animating_mouse.current_frame	= -1;
	Animating_mouse.time				= 0.0f;
	Animating_mouse.elapsed_time	= 0.0f;
}

// ----------------------------------------------------------------------------
// load_animating_pointer()
//
// Called at game init to load in the frames for the animating mouse pointer
//
// input:	filename	=>	filename of animation file that holds the animation
// 
void load_animating_pointer(char *filename, int dx, int dy)
{
	int				fps;
	animating_obj *am;

	init_animating_pointer();

	am = &Animating_mouse;
	am->first_frame = bm_load_animation(filename, &am->num_frames, &fps);
	if ( am->first_frame == -1 ) 
		Error(LOCATION, "Could not load animation %s for the mouse pointer\n", filename);
	am->current_frame = 0;
	am->time = am->num_frames / i2fl(fps);
}

// ----------------------------------------------------------------------------
// unload_animating_pointer()
//
// Called at game shutdown to free the memory used to store the animation frames
//
void unload_animating_pointer()
{
	int				i;
	animating_obj	*am;

	am = &Animating_mouse;
	for ( i = 0; i < am->num_frames; i++ ) {
		Assert( (am->first_frame+i) >= 0 );
		bm_release(am->first_frame + i);
	}

	am->first_frame	= -1;
	am->num_frames		= 0;
	am->current_frame = -1;
}

// draw the correct frame of the game mouse... called from game_maybe_draw_mouse()
void game_render_mouse(float frametime)
{
	int				mx, my;
	animating_obj	*am;

	// if animating cursor exists, play the next frame
	am = &Animating_mouse;
	if ( am->first_frame != -1 ) {
		g_MouseController.GetPos(&mx, &my);
		am->elapsed_time += frametime;
		am->current_frame = fl2i( ( am->elapsed_time / am->time ) * (am->num_frames-1) );
		if ( am->current_frame >= am->num_frames ) {
			am->current_frame = 0;
			am->elapsed_time = 0.0f;
		}
		gr_set_cursor_bitmap(am->first_frame + am->current_frame);
	}
}

// ----------------------------------------------------------------------------
// game_maybe_draw_mouse()
//
// determines whether to draw the mouse pointer at all, and what frame of
// animation to use if the mouse is animating
//
// Sets mouse.cpp globals Mouse_hidden and Mouse_moved based on the state of the game.
//
// input:	frametime => elapsed frame time in seconds since last call
//
void game_maybe_draw_mouse(float frametime)
{
	int game_state;

	game_state = gameseq_get_state();

	switch ( game_state ) {
		case GS_STATE_GAME_PAUSED:
		// case GS_STATE_MULTI_PAUSED:
		case GS_STATE_GAME_PLAY:
		case GS_STATE_DEATH_DIED:
		case GS_STATE_DEATH_BLEW_UP:
			if ( popup_active() || popupdead_is_active() ) {
				g_MouseController.SetMouseHidden(0);
			} else {
				g_MouseController.SetMouseHidden(1);
			}
			break;

		default:
			g_MouseController.SetMouseHidden(0);
			break;
	}	// end switch

	if ( !g_MouseController.GetMouseHidden() ) 
		game_render_mouse(frametime);

}

void game_do_training_checks()
{
	int i, s;
	float d;
	waypoint_list *wplp;

	if (Training_context & TRAINING_CONTEXT_SPEED) {
		s = (int) Player_obj->phys_info.fspeed;
		if ((s >= Training_context_speed_min) && (s <= Training_context_speed_max)) {
			if (!Training_context_speed_set) {
				Training_context_speed_set = 1;
				Training_context_speed_timestamp = timestamp();
			}

		} else
			Training_context_speed_set = 0;
	}

	if (Training_context & TRAINING_CONTEXT_FLY_PATH) {
		wplp = &Waypoint_lists[Training_context_path];
		if (wplp->count > Training_context_goal_waypoint) {
			i = Training_context_goal_waypoint;
			do {
				d = vm_vec_dist(&wplp->waypoints[i], &Player_obj->pos);
				if (d <= Training_context_distance) {
					Training_context_at_waypoint = i;
					if (Training_context_goal_waypoint == i) {
						Training_context_goal_waypoint++;
						snd_play(&Snds[SND_CARGO_REVEAL], 0.0f);
					}

					break;
				}

				i++;
				if (i == wplp->count)
					i = 0;

			} while (i != Training_context_goal_waypoint);
		}
	}

	if ((Players_target == UNINITIALIZED) || (Player_ai->target_objnum != Players_target) || (Player_ai->targeted_subsys != Players_targeted_subsys)) {
		Players_target = Player_ai->target_objnum;
		Players_targeted_subsys = Player_ai->targeted_subsys;
		Players_target_timestamp = timestamp();
	}
}

/////////// Following is for event debug view screen

#ifndef NDEBUG

#define EVENT_DEBUG_MAX	5000
#define EVENT_DEBUG_EVENT 0x8000

int Event_debug_index[EVENT_DEBUG_MAX];
int ED_count;

void game_add_event_debug_index(int n, int indent)
{
	if (ED_count < EVENT_DEBUG_MAX)
		Event_debug_index[ED_count++] = n | (indent << 16);
}

void game_add_event_debug_sexp(int n, int indent)
{
	if (n < 0)
		return;

	if (Sexp_nodes[n].first >= 0) {
		game_add_event_debug_sexp(Sexp_nodes[n].first, indent);
		game_add_event_debug_sexp(Sexp_nodes[n].rest, indent);
		return;
	}

	game_add_event_debug_index(n, indent);
	if (Sexp_nodes[n].subtype == SEXP_ATOM_OPERATOR)
		game_add_event_debug_sexp(Sexp_nodes[n].rest, indent + 1);
	else
		game_add_event_debug_sexp(Sexp_nodes[n].rest, indent);
}

void game_event_debug_init()
{
	int e;

	ED_count = 0;
	for (e=0; e<Num_mission_events; e++) {
		game_add_event_debug_index(e | EVENT_DEBUG_EVENT, 0);
		game_add_event_debug_sexp(Mission_events[e].formula, 1);
	}
}

void game_show_event_debug(float frametime) 
{
	char buf[256];
	int i, k, z;
	int font_height, font_width;	
	int y_index, y_max;
	static int scroll_offset = 0;
	
	k = game_check_key();
	if (k)
		switch (k) {
			case KEY_UP:
			case KEY_PAD8:
				scroll_offset--;
				if (scroll_offset < 0)
					scroll_offset = 0;
				break;

			case KEY_DOWN:
			case KEY_PAD2:
				scroll_offset++;
				break;

			case KEY_PAGEUP:
				scroll_offset -= 20;
				if (scroll_offset < 0)
					scroll_offset = 0;
				break;

			case KEY_PAGEDOWN:
				scroll_offset += 20;	// not font-independent, hard-coded since I counted the lines!
				break;

			default:
				gameseq_post_event(GS_EVENT_PREVIOUS_STATE);
				key_flush();
				break;
		} // end switch

	gr_clear();
	gr_set_color_fast(&Color_bright);
	gr_set_font(FONT1);
	gr_printf(0x8000, 5, NOX("EVENT DEBUG VIEW"));

	gr_set_color_fast(&Color_normal);
	gr_set_font(FONT1);
	gr_get_string_size(&font_width, &font_height, NOX("test"));
	y_max = gr_screen.max_h - font_height - 5;
	y_index = 45;

	k = scroll_offset;
	while (k < ED_count) {
		if (y_index > y_max)
			break;

		z = Event_debug_index[k];
		if (z & EVENT_DEBUG_EVENT) {
			z &= 0x7fff;
			sprintf(buf, NOX("%s%s (%s) %s%d %d"), (Mission_events[z].flags & MEF_CURRENT) ? NOX("* ") : "",
				Mission_events[z].name, Mission_events[z].result ? NOX("True") : NOX("False"),
				(Mission_events[z].chain_delay < 0) ? "" : NOX("x "),
				Mission_events[z].repeat_count, Mission_events[z].interval);

		} else {
			i = (z >> 16) * 3;
			buf[i] = 0;
			while (i--)
				buf[i] = ' ';

			strcat(buf, Sexp_nodes[z & 0x7fff].text);
			switch (Sexp_nodes[z & 0x7fff].value) {
				case SEXP_TRUE:
					strcat(buf, NOX(" (True)"));
					break;

				case SEXP_FALSE:
					strcat(buf, NOX(" (False)"));
					break;

				case SEXP_KNOWN_TRUE:
					strcat(buf, NOX(" (Always true)"));
					break;

				case SEXP_KNOWN_FALSE:
					strcat(buf, NOX(" (Always false)"));
					break;

				case SEXP_CANT_EVAL:
					strcat(buf, NOX(" (Can't eval)"));
					break;

				case SEXP_NAN:
				case SEXP_NAN_FOREVER:
					strcat(buf, NOX(" (Not a number)"));
					break;
			}
		}

		gr_printf(10, y_index, buf);
		y_index += font_height;
		k++;
	}

	gr_flip();
}

#endif // NDEBUG

#ifndef NDEBUG
FILE * Time_fp;
FILE * Texture_fp;

int Tmap_num_too_big = 0;
int Num_models_needing_splitting = 0;

void Time_model( int modelnum )
{
//	mprintf(( "Timing ship '%s'\n", si->name ));

	vector eye_pos, model_pos;
	matrix eye_orient, model_orient;

	polymodel *pm = model_get( modelnum );

	int l = strlen(pm->filename);
	while( (l>0) )	{
		if ( (l == '/') || (l=='\\') || (l==':'))	{
			l++;
			break;
		}
		l--;
	}
	char *pof_file = &pm->filename[l];

	int model_needs_splitting = 0;

	//fprintf( Texture_fp, "Model: %s\n", pof_file );
	int i;
	for (i=0; i<pm->n_textures; i++ )	{
		char filename[1024];
		ubyte pal[768];

		int bmp_num = pm->original_textures[i];
		if ( bmp_num > -1 )	{
			bm_get_palette(pm->original_textures[i], pal, filename );		
			int w,h;
			bm_get_info( pm->original_textures[i],&w, &h );


			if ( (w > 512) || (h > 512) )	{
				fprintf( Texture_fp, "%s\t%s\t%d\t%d\n", pof_file, filename, w, h );
				Tmap_num_too_big++;
				model_needs_splitting++;
			}
		} else {
			//fprintf( Texture_fp, "\tTexture %d is bogus\n", i );
		}
	}

	if ( model_needs_splitting )	{
		Num_models_needing_splitting++;
	}
	eye_orient = model_orient = vmd_identity_matrix;
	eye_pos = model_pos = vmd_zero_vector;

	eye_pos.z = -pm->rad*2.0f;

	vector eye_to_model;

	vm_vec_sub( &eye_to_model, &model_pos, &eye_pos );
	vm_vector_2_matrix( &eye_orient, &eye_to_model, NULL, NULL );

	fix t1 = timer_get_fixed_seconds();

	angles ta;
	ta.p = ta.b = ta.h = 0.0f; 
	int framecount = 0;

	int bitmaps_used_this_frame, bitmaps_new_this_frame;
		
	bm_get_frame_usage(&bitmaps_used_this_frame,&bitmaps_new_this_frame);

	modelstats_num_polys = modelstats_num_verts = 0;

	while( ta.h < PI2 )	{

		matrix m1;
		vm_angles_2_matrix(&m1, &ta );
		vm_matrix_x_matrix( &model_orient, &vmd_identity_matrix, &m1 );

		gr_reset_clip();
//		gr_clear();

		g3_start_frame(1);

		g3_set_view_matrix( &eye_pos, &eye_orient, Viewer_zoom );	

		model_clear_instance( modelnum );
		model_set_detail_level(0);		// use highest detail level
		model_render( modelnum, &model_orient, &model_pos, MR_LOCK_DETAIL);	//|MR_NO_POLYS );

		g3_end_frame();
//		gr_flip();

		framecount++;
		ta.h += 0.1f;

		int k = key_inkey();
		if ( k == KEY_ESC ) {
			exit(1);
		}
	}

	fix t2 = timer_get_fixed_seconds();

	bm_get_frame_usage(&bitmaps_used_this_frame,&bitmaps_new_this_frame);
	//bitmaps_used_this_frame /= framecount;

	modelstats_num_polys /= framecount;
	modelstats_num_verts /= framecount;

	mprintf(( "'%s' is %.2f FPS\n", pof_file, i2fl(framecount)/f2fl(t2-t1) ));
//	fprintf( Time_fp, "\"%s\"\t%.0f\t%d\t%d\t%d\t%d\n", pof_file, i2fl(framecount)/f2fl(t2-t1), bitmaps_used_this_frame, modelstats_num_polys, modelstats_num_verts, Tmap_npixels );
//	fprintf( Time_fp, "%.0f\t%d\t%d\t%d\t%d\n", i2fl(framecount)/f2fl(t2-t1), bitmaps_used_this_frame, modelstats_num_polys, modelstats_num_verts, Tmap_npixels );

		
//	key_getch();
}

int Time_models = 0;
DCF_BOOL( time_models, Time_models );

void Do_model_timings_test()
{
	

	if ( !Time_models ) return;

	mprintf(( "Timing models!\n" ));

	int i;

	ubyte model_used[MAX_POLYGON_MODELS];
	int model_id[MAX_POLYGON_MODELS];
	for (i=0; i<MAX_POLYGON_MODELS; i++ )	{
		model_used[i] = 0;
	}
	
	// Load them all
	for (i=0; i<Num_ship_types; i++ )	{
		Ship_info[i].modelnum = model_load( Ship_info[i].pof_file, NULL, NULL );

		model_used[Ship_info[i].modelnum%MAX_POLYGON_MODELS]++;
		model_id[Ship_info[i].modelnum%MAX_POLYGON_MODELS] = Ship_info[i].modelnum;
	}

	Texture_fp = fopen( NOX("ShipTextures.txt"), "wt" );
	if ( !Texture_fp ) return;

	Time_fp = fopen( NOX("ShipTimings.txt"), "wt" );
	if ( !Time_fp ) return;

	fprintf( Time_fp, "Name\tFPS\tTRAM\tPolys\tVerts\tPixels\n" );
//	fprintf( Time_fp, "FPS\tTRAM\tPolys\tVerts\tPixels\n" );
	
	for (i=0; i<MAX_POLYGON_MODELS; i++ )	{
		if ( model_used[i] )	{
			Time_model( model_id[i] );
		}
	}
	
	fprintf( Texture_fp, "Number too big: %d\n", Tmap_num_too_big );
	fprintf( Texture_fp, "Number of models needing splitting: %d\n", Num_models_needing_splitting );
	
	fclose(Time_fp);
	fclose(Texture_fp);

	exit(1);
}
#endif

// Call this function when you want to inform the player that a feature is not
// enabled in the DEMO version of FreSpace
void game_feature_not_in_demo_popup()
{
	popup(PF_USE_AFFIRMATIVE_ICON|PF_BODY_BIG, 1, POPUP_OK, XSTR( "Sorry, this feature is available only in the retail version", 200));
}

// format the specified time (fixed point) into a nice string
void game_format_time(fix m_time,char *time_str)
{
	float mtime;
	int hours,minutes,seconds;
	char tmp[10];

	mtime = f2fl(m_time);		

	// get the hours, minutes and seconds	
	hours = (int)(mtime / 3600.0f);
	if(hours > 0){
		mtime -= (3600.0f * (float)hours);
	}
	seconds = (int)mtime%60;
	minutes = (int)mtime/60;			

	// print the hour if necessary
	if(hours > 0){		
		sprintf(time_str,XSTR( "%d:", 201),hours);
		// if there are less than 10 minutes, print a leading 0
		if(minutes < 10){
			strcpy(tmp,NOX("0"));
			strcat(time_str,tmp);
		}		
	}	
	
	// print the minutes
	if(hours){
		sprintf(tmp,XSTR( "%d:", 201),minutes);
		strcat(time_str,tmp);
	} else {
		sprintf(time_str,XSTR( "%d:", 201),minutes);
	}

	// print the seconds
	if(seconds < 10){
		strcpy(tmp,NOX("0"));
		strcat(time_str,tmp);
	} 
	sprintf(tmp,"%d",seconds);
	strcat(time_str,tmp);
}

//	Stuff version string in *str.
void get_version_string(char *str)
{
//XSTR:OFF
if ( FS_VERSION_BUILD == 0 ) {
	sprintf(str,"v%d.%02d",FS_VERSION_MAJOR, FS_VERSION_MINOR);
} else {
	sprintf(str,"v%d.%02d.%02d",FS_VERSION_MAJOR, FS_VERSION_MINOR, FS_VERSION_BUILD );
}

#if defined (FS2_DEMO)
	strcat(str, " D");
#elif defined (OEM_BUILD)
	strcat(str, " (OEM)");
#endif
//XSTR:ON
	/*
	HMODULE hMod;
	DWORD bogus_handle;
	char myname[_MAX_PATH];
	int namelen, major, minor, build, waste;
	unsigned int buf_size;
	DWORD version_size;
	char *infop;
	VOID *bufp;
	BOOL result;

	// Find my EXE file name
	hMod = GetModuleHandle(NULL);
	namelen = GetModuleFileName( hMod, myname, _MAX_PATH );

	version_size = GetFileVersionInfoSize(myname, &bogus_handle );
	infop = (char *)malloc(version_size);
	result = GetFileVersionInfo( myname, 0, version_size, (LPVOID)infop );

	// get the product version
	result = VerQueryValue((LPVOID)infop, TEXT("\\StringFileInfo\\040904b0\\ProductVersion"), &bufp, &buf_size );
	sscanf( (char *)bufp, "%d, %d, %d, %d", &major, &minor, &build, &waste );
#ifdef DEMO
	sprintf(str,"Dv%d.%02d",major, minor);
#else
	sprintf(str,"v%d.%02d",major, minor);
#endif
	*/
}

void get_version_string_short(char *str)
{
	sprintf(str,"v%d.%02d",FS_VERSION_MAJOR, FS_VERSION_MINOR);
}

// ----------------------------------------------------------------
//
// OEM UPSELL SCREENS BEGIN
//
// ----------------------------------------------------------------
#if defined(OEM_BUILD)

#define NUM_OEM_UPSELL_SCREENS				3
#define OEM_UPSELL_SCREEN_DELAY				10000

static int Oem_upsell_bitmaps_loaded = 0;
static int Oem_upsell_bitmaps[GR_NUM_RESOLUTIONS][NUM_OEM_UPSELL_SCREENS];
static int Oem_upsell_screen_number = 0;
static int Oem_upsell_show_next_bitmap_time;

//XSTR:OFF
static char *Oem_upsell_bitmap_filenames[GR_NUM_RESOLUTIONS][NUM_OEM_UPSELL_SCREENS] = 
{
	{	"OEMUpSell02",
		"OEMUpSell01",
		"OEMUpSell03",
	},
	{	"2_OEMUpSell02",
		"2_OEMUpSell01",
		"2_OEMUpSell03",
	},
};
//XSTR:ON

static int Oem_normal_cursor = -1;
static int Oem_web_cursor = -1;
//#define OEM_UPSELL_URL		"http://www.interplay-store.com/"
#define OEM_UPSELL_URL		"http://www.interplay.com/cgi-bin/oemlinks.pl/pid=483421&cid=18384"

void oem_upsell_next_screen()
{
	Oem_upsell_screen_number++;
	if ( Oem_upsell_screen_number == (NUM_OEM_UPSELL_SCREENS-1) ) {
		// extra long delay, mouse shown on last upsell
		Oem_upsell_show_next_bitmap_time = timer_get_milliseconds() + OEM_UPSELL_SCREEN_DELAY*2;
		Mouse_hidden = 0;

	} else {
		Oem_upsell_show_next_bitmap_time = timer_get_milliseconds() + OEM_UPSELL_SCREEN_DELAY;
	}
}

void oem_upsell_load_bitmaps()
{
	int i;

	for ( i = 0; i < NUM_OEM_UPSELL_SCREENS; i++ ) {
		Oem_upsell_bitmaps[gr_screen.res][i] = bm_load(Oem_upsell_bitmap_filenames[gr_screen.res][i]);
	}
}

void oem_upsell_unload_bitmaps()
{
	int i;

	for ( i = 0; i < NUM_OEM_UPSELL_SCREENS; i++ ) {
		if(Oem_upsell_bitmaps[gr_screen.res][i] >= 0){
			bm_unload(Oem_upsell_bitmaps[gr_screen.res][i]);
		}
	}

	// unloaded
	Oem_upsell_bitmaps_loaded = 0;
}

// clickable hotspot on 3rd OEM upsell screen
static int Oem_upsell3_button_coords[GR_NUM_RESOLUTIONS][4] = {
	{	// GR_640
		28, 350, 287, 96					// x, y, w, h
	},
	{	// GR_1024
		45, 561, 460, 152					// x, y, w, h
	}
};

void oem_upsell_show_screens()
{
	int current_time, k;
	int done = 0;

	if ( !Oem_upsell_bitmaps_loaded ) {
		oem_upsell_load_bitmaps();
		Oem_upsell_bitmaps_loaded = 1;
	}

	// may use upsell screens more than once
	Oem_upsell_show_next_bitmap_time = timer_get_milliseconds() + OEM_UPSELL_SCREEN_DELAY;
	Oem_upsell_screen_number = 0;
	
	key_flush();
	Mouse_hidden = 1;

	// set up cursors
	int nframes;						// used to pass, not really needed (should be 1)
	Oem_normal_cursor = gr_get_cursor_bitmap();
	Oem_web_cursor = bm_load_animation("cursorweb", &nframes);
	Assert(Oem_web_cursor >= 0);
	if (Oem_web_cursor < 0) {
		Oem_web_cursor = Oem_normal_cursor;
	}

	while(!done) {

		//oem_reset_trailer_timer();

		current_time = timer_get_milliseconds();

		os_poll();
		k = key_inkey();

		// advance screen on keypress or timeout
		if (( k > 0 ) || (mouse_up_count(MouseController::MOUSE_LEFT_BUTTON) > 0) || (current_time > Oem_upsell_show_next_bitmap_time)) {
			oem_upsell_next_screen();
		}

		// check if we are done
		if ( Oem_upsell_screen_number >= NUM_OEM_UPSELL_SCREENS ) {
			Oem_upsell_screen_number--;
			done = 1;
		} else {
			if ( Oem_upsell_bitmaps[gr_screen.res][Oem_upsell_screen_number] < 0 ) {
				done = 1;
			}
		}

		// show me the upsell
		if ( Oem_upsell_bitmaps[gr_screen.res][Oem_upsell_screen_number] >= 0 ) {		
			gr_set_bitmap(Oem_upsell_bitmaps[gr_screen.res][Oem_upsell_screen_number]);
			gr_bitmap(0,0);
		}

		// if this is the 3rd upsell, make it clickable, d00d
		if ( Oem_upsell_screen_number == NUM_OEM_UPSELL_SCREENS-1 ) {
			int mx, my;
			int button_state = g_MouseController.GetPos(&mx, &my);
			if ( (mx >= Oem_upsell3_button_coords[gr_screen.res][0]) && (mx <= Oem_upsell3_button_coords[gr_screen.res][0] + Oem_upsell3_button_coords[gr_screen.res][2])
				&& (my >= Oem_upsell3_button_coords[gr_screen.res][1]) && (my <= Oem_upsell3_button_coords[gr_screen.res][1] + Oem_upsell3_button_coords[gr_screen.res][3]) )
			{
				// switch cursors
				gr_set_cursor_bitmap(Oem_web_cursor); //, GR_CURSOR_LOCK);

				// check for clicks
				if (button_state & MouseController::MOUSE_LEFT_BUTTON) {
					// launch URL
					multi_pxo_url(OEM_UPSELL_URL);
					done = 1;
				} 
			} else {
				// switch cursor back to normal one
				gr_set_cursor_bitmap(Oem_normal_cursor); //, GR_CURSOR_UNLOCK);
			}
		}

		if ( done ) {
			if (gameseq_get_state() != GS_STATE_END_DEMO) {
				gr_fade_out(0);
				Sleep(300);
			}
		}

		gr_flip();
	}

	// unload bitmap
	oem_upsell_unload_bitmaps();

	// switch cursor back to normal one
	gr_set_cursor_bitmap(Oem_normal_cursor); //, GR_CURSOR_UNLOCK);

}

#endif // defined(OEM_BUILD)
// ----------------------------------------------------------------
//
// OEM UPSELL SCREENS END
//
// ----------------------------------------------------------------



// ----------------------------------------------------------------
//
// DEMO UPSELL SCREENS BEGIN
//
// ----------------------------------------------------------------

#ifdef FS2_DEMO

//#define NUM_DEMO_UPSELL_SCREENS				4

#define NUM_DEMO_UPSELL_SCREENS				2
#define DEMO_UPSELL_SCREEN_DELAY				3000

static int Demo_upsell_bitmaps_loaded = 0;
static int Demo_upsell_bitmaps[GR_NUM_RESOLUTIONS][NUM_DEMO_UPSELL_SCREENS];
static int Demo_upsell_screen_number = 0;
static int Demo_upsell_show_next_bitmap_time;

//XSTR:OFF
static char *Demo_upsell_bitmap_filenames[GR_NUM_RESOLUTIONS][NUM_DEMO_UPSELL_SCREENS] = 
{
	{	"UpSell02",
		"UpSell01",
	},
	{	"2_UpSell02",
		"2_UpSell01",
	},
	// "DemoUpsell3",
	// "DemoUpsell4",
};
//XSTR:ON

void demo_upsell_next_screen()
{
	Demo_upsell_screen_number++;
	if ( Demo_upsell_screen_number == (NUM_DEMO_UPSELL_SCREENS-1) ) {
		Demo_upsell_show_next_bitmap_time = timer_get_milliseconds() + DEMO_UPSELL_SCREEN_DELAY*4;
	} else {
		Demo_upsell_show_next_bitmap_time = timer_get_milliseconds() + DEMO_UPSELL_SCREEN_DELAY;
	}

	/*
	if ( Demo_upsell_screen_number < NUM_DEMO_UPSELL_SCREENS ) {
		if ( Demo_upsell_bitmap_filenames[gr_screen.res][Demo_upsell_screen_number] >= 0 ) {
#ifndef HARDWARE_ONLY
			palette_use_bm_palette(Demo_upsell_bitmaps[gr_screen.res][Demo_upsell_screen_number]);
#endif
		}
	}
	*/
}

void demo_upsell_load_bitmaps()
{
	int i;

	for ( i = 0; i < NUM_DEMO_UPSELL_SCREENS; i++ ) {
		Demo_upsell_bitmaps[gr_screen.res][i] = bm_load(Demo_upsell_bitmap_filenames[gr_screen.res][i]);
	}
}

void demo_upsell_unload_bitmaps()
{
	int i;

	for ( i = 0; i < NUM_DEMO_UPSELL_SCREENS; i++ ) {
		if(Demo_upsell_bitmaps[gr_screen.res][i] >= 0){
			bm_unload(Demo_upsell_bitmaps[gr_screen.res][i]);
		}
	}

	// unloaded
	Demo_upsell_bitmaps_loaded = 0;
}

void demo_upsell_show_screens()
{
	int current_time, k;
	int done = 0;

	if ( !Demo_upsell_bitmaps_loaded ) {
		demo_upsell_load_bitmaps();
		Demo_upsell_bitmaps_loaded = 1;
	}

	// may use upsell screens more than once
	Demo_upsell_show_next_bitmap_time = timer_get_milliseconds() + DEMO_UPSELL_SCREEN_DELAY;
	Demo_upsell_screen_number = 0;
	
	key_flush();
	Mouse_hidden = 1;

	while(!done) {

		demo_reset_trailer_timer();

		current_time = timer_get_milliseconds();

// #ifndef THREADED
		os_poll();
// #endif
		k = key_inkey();

		// don't time out, wait for keypress
		/*
		if ( current_time > Demo_upsell_show_next_bitmap_time ) {
			demo_upsell_next_screen();
			k = 0;
		}*/

		if ( k > 0 ) {
			demo_upsell_next_screen();
		}

		if ( Demo_upsell_screen_number >= NUM_DEMO_UPSELL_SCREENS ) {
			Demo_upsell_screen_number--;
			done = 1;
		} else {
			if ( Demo_upsell_bitmaps[gr_screen.res][Demo_upsell_screen_number] < 0 ) {
				done = 1;
			}
		}

		if ( Demo_upsell_bitmaps[gr_screen.res][Demo_upsell_screen_number] >= 0 ) {		
			gr_set_bitmap(Demo_upsell_bitmaps[gr_screen.res][Demo_upsell_screen_number]);
			gr_bitmap(0,0);
		}

		if ( done ) {
			if (gameseq_get_state() != GS_STATE_END_DEMO) {
				gr_fade_out(0);
				Sleep(300);
			}
		}

		gr_flip();
	}

	// unload bitmap
	demo_upsell_unload_bitmaps();
}

#endif // DEMO

// ----------------------------------------------------------------
//
// DEMO UPSELL SCREENS END
//
// ----------------------------------------------------------------


// ----------------------------------------------------------------
//
// Subspace Ambient Sound START
//
// ----------------------------------------------------------------

static int Subspace_ambient_left_channel = -1;
static int Subspace_ambient_right_channel = -1;

// 
void game_start_subspace_ambient_sound()
{
	if ( Subspace_ambient_left_channel < 0 ) {
		Subspace_ambient_left_channel = snd_play_looping(&Snds[SND_SUBSPACE_LEFT_CHANNEL], -1.0f);
	}

	if ( Subspace_ambient_right_channel < 0 ) {
		Subspace_ambient_right_channel = snd_play_looping(&Snds[SND_SUBSPACE_RIGHT_CHANNEL], 1.0f);
	}
}

void game_stop_subspace_ambient_sound()
{
	if ( Subspace_ambient_left_channel >= 0 ) {
		snd_stop(Subspace_ambient_left_channel);
		Subspace_ambient_left_channel = -1;
	}

	if ( Subspace_ambient_right_channel >= 0 ) {
		snd_stop(Subspace_ambient_right_channel);
		Subspace_ambient_right_channel = -1;
	}
}

// ----------------------------------------------------------------
//
// Subspace Ambient Sound END
//
// ----------------------------------------------------------------

// ----------------------------------------------------------------
//
// CDROM detection code START
//
// ----------------------------------------------------------------

#define CD_SIZE_72_MINUTE_MAX			(697000000)

uint game_get_cd_used_space(char *path)
{
	uint total = 0;
	char use_path[512] = "";
	char sub_path[512] = "";
	WIN32_FIND_DATAA	find;
	HANDLE find_handle;

	// recurse through all files and directories
	strcpy(use_path, path);
	strcat(use_path, "*.*");
	find_handle = FindFirstFileA(use_path, &find);

	// bogus
	if(find_handle == INVALID_HANDLE_VALUE){
		return 0;
	}	

	// whee
	do {
		// subdirectory. make sure to ignore . and ..
		if((find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && stricmp(find.cFileName, ".") && stricmp(find.cFileName, "..")){
			// subsearch
			strcpy(sub_path, path);
			strcat(sub_path, find.cFileName);
			strcat(sub_path, "\\");
			total += game_get_cd_used_space(sub_path);	
		} else {
			total += (uint)find.nFileSizeLow;
		}				
	} while(FindNextFileA(find_handle, &find));	

	// close
	FindClose(find_handle);

	// total
	return total;
}


// if volume_name is non-null, the CD name must match that
int find_freespace_cd(char *volume_name)
{
	char oldpath[MAX_PATH];
	char volume[256];
	int i;
	int cdrom_drive=-1;
	int volume_match = 0;
	_finddata_t find;
	intptr_t find_handle;

	GetCurrentDirectoryA(MAX_PATH, oldpath);

	for (i = 0; i < 26; i++) 
	{
//XSTR:OFF
		char path[]="d:\\";
//XSTR:ON

		path[0] = (char)('A'+i);
		if (GetDriveTypeA(path) == DRIVE_CDROM) {
			cdrom_drive = -3;
			if ( GetVolumeInformationA(path, volume, 256, NULL, NULL, NULL, NULL, 0) == TRUE ) {
				nprintf(("CD", "CD volume: %s\n", volume));
			
				// check for any CD volume
				int volume1_present = 0;
				int volume2_present = 0;
				int volume3_present = 0;		

				char full_check[512] = "";

				// look for setup.exe
				strcpy(full_check, path);
				strcat(full_check, "setup.exe");				
				find_handle = _findfirst(full_check, &find);
				if(find_handle != -1){
					volume1_present = 1;				
					_findclose(find_handle);				
				}

				// look for intro.mve
				strcpy(full_check, path);
				strcat(full_check, "intro.mve");				
				find_handle = _findfirst(full_check, &find);
				if(find_handle != -1){
					volume2_present = 1;
					_findclose(find_handle);						
				}				

				// look for endpart1.mve
				strcpy(full_check, path);
				strcat(full_check, "endpart1.mve");				
				find_handle = _findfirst(full_check, &find);
				if(find_handle != -1){
					volume3_present = 1;
					_findclose(find_handle);				
				}				
			
				// see if we have the specific CD we're looking for
				if ( volume_name ) {
					// volume 1
					if ( !stricmp(volume_name, FS_CDROM_VOLUME_1) && volume1_present) {
						volume_match = 1;
					}
					// volume 2
					if ( !stricmp(volume_name, FS_CDROM_VOLUME_2) && volume2_present) {
						volume_match = 1;
					}
					// volume 3
					if ( !stricmp(volume_name, FS_CDROM_VOLUME_3) && volume3_present) {
						volume_match = 1;
					}
				} else {										
					if ( volume1_present || volume2_present || volume3_present ) {
						volume_match = 1;
					}
				}
				
				// here's where we make sure that CD's 2 and 3 are not just ripped - check to make sure its capacity is > 697,000,000 bytes				
				if ( volume_match ){
#ifdef RELEASE_REAL					
					// we don't care about CD1 though. let it be whatever size it wants, since the game will demand CD's 2 and 3 at the proper time
					if(volume2_present || volume3_present) {
						// first step - check to make sure its a cdrom
						if(GetDriveTypeA(path) != DRIVE_CDROM){							
							break;
						}

#if !defined(OEM_BUILD)
						// oem not on 80 min cds, so dont check tha size
						// check its size
						uint used_space = game_get_cd_used_space(path);											
						if(used_space < CD_SIZE_72_MINUTE_MAX){							
							break;
						}
#endif // !defined(OEM_BUILD)
					}					

					cdrom_drive = i;
					break;
#else
					cdrom_drive = i;
					break;
#endif // RELEASE_REAL
				}
			}
		}
	}	

	SetCurrentDirectoryA(oldpath);
	return cdrom_drive;
}

int set_cdrom_path(int drive_num)
{
	int rval;

	if (drive_num < 0) {			//no CD
//		#ifndef NDEBUG
//		strcpy(CDROM_dir,"j:\\FreeSpaceCD\\");				//set directory
//		rval = 1;
//		#else
		strcpy(Game_CDROM_dir,"");				//set directory
		rval = 0;
//		#endif
	} else {
		sprintf(Game_CDROM_dir,NOX("%c:\\"), 'a' + drive_num );			//set directory
		rval = 1;
	}

	return rval;
}

int init_cdrom()
{
	int i, rval;

	//scan for CD, etc.

	rval = 1;

	#ifndef DEMO
	i = find_freespace_cd();

	rval = set_cdrom_path(i);

	/*
	if ( rval ) {
		nprintf(("CD", "Using %s for FreeSpace CD\n", CDROM_dir));
	} else {
		nprintf(("CD", "FreeSpace CD not found\n"));
	}
	*/
	#endif

	return rval;
}

int Last_cd_label_found = 0;
char Last_cd_label[256];

int game_cd_changed()
{
	char label[256];
	int found;
	int changed = 0;
	
	if ( strlen(Game_CDROM_dir) == 0 ) {
		init_cdrom();
	}

	found = GetVolumeInformationA(Game_CDROM_dir, label, 256, NULL, NULL, NULL, NULL, 0);

	if ( found != Last_cd_label_found )	{
		Last_cd_label_found = found;
		if ( found )	{
			mprintf(( "CD '%s' was inserted\n", label ));
			changed = 1;
		} else {
			mprintf(( "CD '%s' was removed\n", Last_cd_label ));
			changed = 1;
		}
	} else {
		if ( Last_cd_label_found )	{
			if ( !stricmp( Last_cd_label, label ))	{
				//mprintf(( "CD didn't change\n" ));
			} else {
				mprintf(( "CD was changed from '%s' to '%s'\n", Last_cd_label, label ));
				changed = 1;
			}
		} else {
			// none found before, none found now.
			//mprintf(( "still no CD...\n" ));
		}
	}
	
	Last_cd_label_found = found;
	if ( found )	{
		strcpy( Last_cd_label, label );
	} else {
		strcpy( Last_cd_label, "" );
	}

	return changed;
}

// check if _any_ FreeSpace2 CDs are in the drive
// return: 1	=> CD now in drive
//			  0	=>	Could not find CD, they refuse to put it in the drive
int game_do_cd_check(char *volume_name)
{	
#if !defined(GAME_CD_CHECK)
	return 1;
#else
	int cd_present = 0;
	int cd_drive_num;

	int num_attempts = 0;
	int refresh_files = 0;
	while(1) {
		int path_set_ok, popup_rval;

		cd_drive_num = find_freespace_cd(volume_name);
		path_set_ok = set_cdrom_path(cd_drive_num);
		if ( path_set_ok ) {
			cd_present = 1;
			if ( refresh_files ) {
				cfile_refresh();
				refresh_files = 0;
			}
			break;
		}

		// standalone mode
		if(Is_standalone){
			cd_present = 0;
			break;
		} else {
			// no CD found, so prompt user
			popup_rval = popup(PF_BODY_BIG, 1, POPUP_OK, XSTR( "FreeSpace 2 CD not found\n\nInsert a FreeSpace 2 CD to continue", 202));
			refresh_files = 1;
			if ( popup_rval != 1 ) {
				cd_present = 0;
				break;
			}

			if ( num_attempts++ > 5 ) {
				cd_present = 0;
				break;
			}
		}
	}

	return cd_present;
#endif
}

// check if _any_ FreeSpace2 CDs are in the drive
// return: 1	=> CD now in drive
//			  0	=>	Could not find CD, they refuse to put it in the drive
int game_do_cd_check_specific(char *volume_name, int cdnum)
{	
	int cd_present = 0;
	int cd_drive_num;

	int num_attempts = 0;
	int refresh_files = 0;
	while(1) {
		int path_set_ok, popup_rval;

		cd_drive_num = find_freespace_cd(volume_name);
		path_set_ok = set_cdrom_path(cd_drive_num);
		if ( path_set_ok ) {
			cd_present = 1;
			if ( refresh_files ) {
				cfile_refresh();
				refresh_files = 0;
			}
			break;
		}

		if(Is_standalone){
			cd_present = 0;
			break;
		} else {
			// no CD found, so prompt user
#if defined(DVD_MESSAGE_HACK)
			popup_rval = popup(PF_BODY_BIG, 1, POPUP_OK, XSTR("Please insert DVD", 1468));
#else
			popup_rval = popup(PF_BODY_BIG, 1, POPUP_OK, XSTR("Please insert CD %d", 1468), cdnum);
#endif
			refresh_files = 1;
			if ( popup_rval != 1 ) {
				cd_present = 0;
				break;
			}

			if ( num_attempts++ > 5 ) {
				cd_present = 0;
				break;
			}
		}
	}

	return cd_present;
}

// only need to do this in RELEASE_REAL
int game_do_cd_mission_check(char *filename)
{	
	if(GlobalConst::kCDCheckOn == false)
	{
		return 1;
	}
#ifdef RELEASE_REAL
	int cd_num;
	int cd_present = 0;
	int cd_drive_num;
	fs_builtin_mission *m = game_find_builtin_mission(filename);

	// check for changed CD
	if(game_cd_changed()){
		cfile_refresh();
	}

	// multiplayer
	if((Game_mode & GM_MULTIPLAYER) || Is_standalone){
		return 1;
	}

	// not builtin, so do a general check (any FS2 CD will do)
	if(m == NULL){
		return game_do_cd_check();
	}

	// does not have any CD requirement, do a general check
	if(strlen(m->cd_volume) <= 0){
		return game_do_cd_check();
	}

	// get the volume
	if(!stricmp(m->cd_volume, FS_CDROM_VOLUME_1)){
		cd_num = 1;
	} else if(!stricmp(m->cd_volume, FS_CDROM_VOLUME_2)){
		cd_num = 2;
	} else if(!stricmp(m->cd_volume, FS_CDROM_VOLUME_3)){
		cd_num = 3; 
	} else {
		return game_do_cd_check();
	}

	// did we find the cd?
	if(find_freespace_cd(m->cd_volume) >= 0){
		return 1;
	}

	// make sure the volume exists
	int num_attempts = 0;
	int refresh_files = 0;
	while(1){
		int path_set_ok, popup_rval;

		cd_drive_num = find_freespace_cd(m->cd_volume);
		path_set_ok = set_cdrom_path(cd_drive_num);
		if ( path_set_ok ) {
			cd_present = 1;
			if ( refresh_files ) {
				cfile_refresh();
				refresh_files = 0;
			}
			break;
		}

		// no CD found, so prompt user
#if defined(DVD_MESSAGE_HACK)
		popup_rval = popup(PF_BODY_BIG, 1, POPUP_OK, XSTR("Please insert DVD", 1468));
#else
		popup_rval = popup(PF_BODY_BIG, 1, POPUP_OK, XSTR("Please insert CD %d", 1468), cd_num);
#endif

		refresh_files = 1;
		if ( popup_rval != 1 ) {
			cd_present = 0;
			break;
		}

		if ( num_attempts++ > 5 ) {
			cd_present = 0;
			break;
		}
	}	

	return cd_present;
#else
	return 1;
#endif
}

// ----------------------------------------------------------------
//
// CDROM detection code END
//
// ----------------------------------------------------------------

// ----------------------------------------------------------------
// SHIPS TBL VERIFICATION STUFF
//

// checksums, just keep a list of all valid ones, if it matches any of them, keep it
#define NUM_SHIPS_TBL_CHECKSUMS		1
/*
int Game_ships_tbl_checksums[NUM_SHIPS_TBL_CHECKSUMS] = {
	-463907578,						// US - beta 1
	1696074201,						// FS2 demo
};
*/
int Game_ships_tbl_checksums[NUM_SHIPS_TBL_CHECKSUMS] = {
//	-1022810006,					// 1.0 FULL
	-1254285366						// 1.2 FULL (German)
};

void verify_ships_tbl()
{	
	/*
#ifdef NDEBUG
	Game_ships_tbl_valid = 1;
#else
	*/
	uint file_checksum;		
	int idx;

	// detect if the packfile exists
	CFILE *detect = cfopen("ships.tbl", "rb");
	Game_ships_tbl_valid = 0;	 
	
	// not mission-disk
	if(!detect){
		Game_ships_tbl_valid = 0;
		return;
	}	

	// get the long checksum of the file
	file_checksum = 0;
	cfseek(detect, 0, SEEK_SET);	
	cf_chksum_long(detect, &file_checksum);
	cfclose(detect);
	detect = NULL;	

	// now compare the checksum/filesize against known #'s
	for(idx=0; idx<NUM_SHIPS_TBL_CHECKSUMS; idx++){
		if(Game_ships_tbl_checksums[idx] == (int)file_checksum){
			Game_ships_tbl_valid = 1;
			return;
		}
	}
// #endif
}

DCF(shipspew, "display the checksum for the current ships.tbl")
{
	uint file_checksum;
	CFILE *detect = cfopen("ships.tbl", "rb");
	// get the long checksum of the file
	file_checksum = 0;
	cfseek(detect, 0, SEEK_SET);	
	cf_chksum_long(detect, &file_checksum);
	cfclose(detect);

	dc_printf("%d", file_checksum);
}

// ----------------------------------------------------------------
// WEAPONS TBL VERIFICATION STUFF
//

// checksums, just keep a list of all valid ones, if it matches any of them, keep it
#define NUM_WEAPONS_TBL_CHECKSUMS		1
/*
int Game_weapons_tbl_checksums[NUM_WEAPONS_TBL_CHECKSUMS] = {
	141718090,				// US - beta 1
	-266420030,				// demo 1
};
*/
int Game_weapons_tbl_checksums[NUM_WEAPONS_TBL_CHECKSUMS] = {
//	399297860,				// 1.0 FULL	
	-553984927				// 1.2 FULL (german)
};

void verify_weapons_tbl()
{	
	/*
#ifdef NDEBUG
	Game_weapons_tbl_valid = 1;
#else
	*/
	uint file_checksum;		
	int idx;

	// detect if the packfile exists
	CFILE *detect = cfopen("weapons.tbl", "rb");
	Game_weapons_tbl_valid = 0;	 
	
	// not mission-disk
	if(!detect){
		Game_weapons_tbl_valid = 0;
		return;
	}	

	// get the long checksum of the file
	file_checksum = 0;
	cfseek(detect, 0, SEEK_SET);	
	cf_chksum_long(detect, &file_checksum);
	cfclose(detect);
	detect = NULL;	

	// now compare the checksum/filesize against known #'s
	for(idx=0; idx<NUM_WEAPONS_TBL_CHECKSUMS; idx++){
		if(Game_weapons_tbl_checksums[idx] == (int)file_checksum){
			Game_weapons_tbl_valid = 1;
			return;
		}
	}
// #endif
}

DCF(wepspew, "display the checksum for the current weapons.tbl")
{
	uint file_checksum;
	CFILE *detect = cfopen("weapons.tbl", "rb");
	// get the long checksum of the file
	file_checksum = 0;
	cfseek(detect, 0, SEEK_SET);	
	cf_chksum_long(detect, &file_checksum);
	cfclose(detect);

	dc_printf("%d", file_checksum);
}

// if the game is running using hacked data
int game_hacked_data()
{
	// hacked!
	if(!Game_weapons_tbl_valid || !Game_ships_tbl_valid){
		return 1;
	}

	// not hacked
	return 0;
}

void display_title_screen()
{
#if defined(FS2_DEMO) || defined(OEM_BUILD)
	///int title_bitmap;

	// load bitmap
	int title_bitmap = bm_load(Game_demo_title_screen_fname[gr_screen.res]);
	if (title_bitmap == -1) {
		return;
	}

	gr_start_frame();

	// set
	gr_set_bitmap(title_bitmap);

	// draw
	gr_bitmap(0, 0);

	gr_stop_frame();

	// flip
	gr_flip();

	bm_unload(title_bitmap);
#endif  // FS2_DEMO || OEM_BUILD
}
