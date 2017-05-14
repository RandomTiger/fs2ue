/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef FS_CMDLINE_HEADER_FILE
#define FS_CMDLINE_HEADER_FILE

int parse_cmdline(char *cmdline);

// COMMAND LINE SETTINGS
// This section is for reference by all the *_init() functions. For example, the multiplayer init function
// could check to see if (int Cmdline_multi_stream_chat_to_file) has been set by the command line parser.
//
// Add any extern definitions here and put the actual variables inside of cmdline.cpp for ease of use
// Also, check to make sure anything you add doesn't break Fred or TestCode

extern int Cmdline_multi_stream_chat_to_file;
extern int Cmdline_freespace_no_sound;
extern int Cmdline_freespace_no_music;
extern int Cmdline_gimme_all_medals;
extern int Cmdline_use_last_pilot;
extern int Cmdline_cd_check;
extern int Cmdline_start_netgame;
extern char *Cmdline_autoload;
extern int Cmdline_closed_game;
extern int Cmdline_restricted_game;
extern int Cmdline_network_port;
extern char *Cmdline_game_name;
extern char *Cmdline_game_password;
extern char *Cmdline_rank_above;
extern char *Cmdline_rank_below;
extern char *Cmdline_connect_addr;
extern int Cmdline_multi_log;
extern int Cmdline_server_firing;
extern int Cmdline_client_dodamage;
extern int Cmdline_spew_pof_info;
extern int Cmdline_mouse_coords;
extern int Cmdline_timeout;

extern int Cmdline_window;
extern int Cmdline_360pad;
extern int Cmdline_voicer;
extern int Cmdline_fps;
extern int Cmdline_bootcheck;

extern bool Cmdline_anttweakbar;
extern bool Cmdline_forceDirect3d9;
extern bool Cmdline_forceDirect3d5;
extern bool Cmdline_splitscreen;
extern bool Cmdline_horde;


#endif