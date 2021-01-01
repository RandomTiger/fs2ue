/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#ifndef UNITY_BUILD
#include "HUDreticle.h"

#include "2d.h"
#include "HUD.h"
//#include "Pixel.h"
#include "math.h"
#include "Player.h"
#include "Ship.h"
#include "FreeSpace.h"
#include "ai.h"
#include "BmpMan.h"
#include "key.h"
#include "timer.h"
#include "math.h"
#include "GameSnd.h"
#include "HUDtargetbox.h"
#include "Multi.h"
#include "Emp.h"
#include "Localize.h"
#endif

static int Reticle_inited = 0;

#define NUM_RETICLE_ANIS			6		// keep up to date when modifying the number of reticle ani files

#define RETICLE_TOP_ARC				0
#define RETICLE_LASER_WARN			1
#define RETICLE_LOCK_WARN			2
#define RETICLE_LEFT_ARC			3
#define RETICLE_RIGHT_ARC			4
//#define RETICLE_ONE_PRIMARY		5
//#define RETICLE_TWO_PRIMARY		6
//#define RETICLE_ONE_SECONDARY		7
//#define RETICLE_TWO_SECONDARY		8
//#define RETICLE_THREE_SECONDARY	9
// #define RETICLE_LAUNCH_LABEL		5
#define RETICLE_CENTER				5

int Hud_throttle_frame_h[GR_NUM_RESOLUTIONS] = {
	85,
	136
};
int Hud_throttle_frame_w[GR_NUM_RESOLUTIONS] = {
	49, 
	78
};
int Hud_throttle_frame_bottom_y[GR_NUM_RESOLUTIONS] = {
	325,
	520
};
int Hud_throttle_h[GR_NUM_RESOLUTIONS] = {
	50,
	80
};
int Hud_throttle_bottom_y[GR_NUM_RESOLUTIONS] = {
	307,
	491
};
int Hud_throttle_aburn_h[GR_NUM_RESOLUTIONS] = {
	17,
	27
};
int Hud_throttle_aburn_button[GR_NUM_RESOLUTIONS] = {
	259,
	414
};

int Outer_circle_radius[GR_NUM_RESOLUTIONS] = {
	104,
	166
};

int Hud_reticle_center[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		320, 242
	},
	{ // GR_1024
		512, 385
	}
};

char Reticle_frame_names[GR_NUM_RESOLUTIONS][NUM_RETICLE_ANIS][MAX_FILENAME_LEN] = 
{
//XSTR:OFF
	{ // GR_640
		"toparc1",
		"toparc2",
		"toparc3",
		"leftarc",
		"rightarc1",
/*		"rightarc2",
		"rightarc3",
		"rightarc4",
		"rightarc5",
		"rightarc6",	
		"toparc4",	*/
		"reticle1",	
	}, 
	{ // GR_1024
		"2_toparc1",
		"2_toparc2",
		"2_toparc3",
		"2_leftarc",
		"2_rightarc1",
/*		"2_rightarc2",
		"2_rightarc3",
		"2_rightarc4",
		"2_rightarc5",
		"2_rightarc6",	
		"2_toparc4",	*/
		"2_reticle1",	
	}
//XSTR:ON
};

// reticle frame coords
int Reticle_frame_coords[GR_NUM_RESOLUTIONS][NUM_RETICLE_ANIS][2] = {
	{ // GR_640
		{241, 137},
		{400, 245},
		{394, 261},
		{216, 168},
		{359, 168},
//		{406, 253},
//		{406, 253},
//		{391, 276},
//		{391, 276},
//		{391, 276},
//		{297, 161},
		{308, 235}
	}, 
	{ // GR_1024
		{386, 219},
		{640, 393},
		{631, 419},
		{346, 269},
		{574, 269},
//		{649, 401},
//		{649, 401},
//		{625, 438},
//		{625, 438},
//		{625, 438},
//		{475, 258},
		{493, 370}
	}
};

// "launch" gauge coords
int Reticle_launch_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		297,	161
	},
	{ // GR_1024
		475,	258
	}
};

hud_frames Reticle_gauges[NUM_RETICLE_ANIS];

#define THREAT_DUMBFIRE				(1<<0)
#define THREAT_ATTEMPT_LOCK		(1<<1)
#define THREAT_LOCK					(1<<2)

#define THREAT_UPDATE_DUMBFIRE_TIME		1000		// time between checking for dumbfire threats
#define THREAT_UPDATE_LOCK_TIME			500		// time between checking for lock threats

#define THREAT_DUMBFIRE_FLASH				180
#define THREAT_LOCK_FLASH					180
static int Threat_dumbfire_timer;		// timestamp for when to show next flashing frame for dumbfire threat
static int Threat_lock_timer;				// timestamp for when to show next flashing frame for lock threat

static int Threat_dumbfire_frame;			// frame offset of current dumbfire flashing warning
static int Threat_lock_frame;				// frame offset of current lock flashing warning

// coordinates
static int Max_speed_coords[GR_NUM_RESOLUTIONS][2] = 
{
	{ // GR_640
		236, 254
	}, 
	{ // GR_1024
		377, 406
	}
};
static int Zero_speed_coords[GR_NUM_RESOLUTIONS][2] = {
	{ // GR_640
		252, 303
	},
	{ // GR_1024
		403, 485
	}
};

// Called at the start of each level.. use Reticle_inited so we only load frames once
void hud_init_reticle()
{
	int			i;
	hud_frames	*hfp;

	Threat_dumbfire_timer = timestamp(0);
	Threat_lock_timer = timestamp(0);
	Threat_dumbfire_frame = 1;
	Threat_lock_frame = 1;
	Player->threat_flags = 0;
	Player->update_dumbfire_time = timestamp(0);		
	Player->update_lock_time = timestamp(0);

	if ( Reticle_inited ) {
		return;
	}

	for ( i = 0; i < NUM_RETICLE_ANIS; i++ ) {
		hfp = &Reticle_gauges[i];
		hfp->first_frame = bm_load_animation(Reticle_frame_names[gr_screen.res][i], &hfp->num_frames);
		if ( hfp->first_frame < 0 ) {
			Warning(LOCATION,"Cannot load hud ani: %s\n", Reticle_frame_names[gr_screen.res][i]);
		}
	}

	Reticle_inited = 1;
}

void hud_deinit_reticle()
{
	Reticle_inited = 0;
}

// called once per frame to update the reticle gauges.  Makes calls to
// ship_dumbfire_threat() and ship_lock_threat() and updates Threat_flags.
void hud_update_reticle( player *pp )
{
	int rval;
	ship *shipp;

	// multiplayer clients won't call this routine
	if ( MULTIPLAYER_CLIENT || MULTI_OBSERVER(Net_players[MY_NET_PLAYER_NUM]))
		return;

	shipp = &Ships[Objects[pp->objnum].instance];

	if ( ship_dumbfire_threat(shipp) ) {
		pp->threat_flags |= THREAT_DUMBFIRE;
		pp->update_dumbfire_time = timestamp(THREAT_UPDATE_DUMBFIRE_TIME);
	}

	if ( timestamp_elapsed(pp->update_dumbfire_time) ) {
		pp->update_dumbfire_time = timestamp(THREAT_UPDATE_DUMBFIRE_TIME);
		pp->threat_flags &= ~THREAT_DUMBFIRE;
	}

	if ( timestamp_elapsed(pp->update_lock_time) ) {
		pp->threat_flags &= ~(THREAT_LOCK | THREAT_ATTEMPT_LOCK);
		pp->update_lock_time = timestamp(THREAT_UPDATE_LOCK_TIME);
		rval = ship_lock_threat(shipp);
		if ( rval == 1 ) {
			pp->threat_flags |= THREAT_ATTEMPT_LOCK;
		} else if ( rval == 2 ) {
			pp->threat_flags |= THREAT_LOCK;
		}
	}
}

// draw left arc (the dark portion of the throttle gauge)
void hud_render_throttle_background(int y_end)
{
	int x,y,w,h;

	hud_set_gauge_color(HUD_THROTTLE_GAUGE);

	x = Reticle_frame_coords[gr_screen.res][RETICLE_LEFT_ARC][0];
	y = Reticle_frame_coords[gr_screen.res][RETICLE_LEFT_ARC][1];

	bm_get_info( Reticle_gauges[RETICLE_LEFT_ARC].first_frame+1,&w,&h);	

	if ( y_end > y ) {
		GR_AABITMAP_EX(Reticle_gauges[RETICLE_LEFT_ARC].first_frame+1, GetHUDCoordsCentred(x), y, w, y_end-y+1, 0, 0);		
	}
}

// draw left arc (the bright portion of the throttle gauge)
void hud_render_throttle_foreground(int y_end)
{
	int x,y,w,h;

	hud_set_gauge_color(HUD_THROTTLE_GAUGE);

	x = Reticle_frame_coords[gr_screen.res][RETICLE_LEFT_ARC][0];
	y = Reticle_frame_coords[gr_screen.res][RETICLE_LEFT_ARC][1];

	bm_get_info( Reticle_gauges[RETICLE_LEFT_ARC].first_frame+1,&w,&h);

	if ( y_end < Hud_throttle_frame_bottom_y[gr_screen.res] ) {		
		GR_AABITMAP_EX(Reticle_gauges[RETICLE_LEFT_ARC].first_frame+2, GetHUDCoordsCentred(x), y_end, w, h-(y_end-y), 0, y_end-y);		
	}
}

// Draw the throttle speed number
void hud_render_throttle_speed(float current_speed, int y_end)
{
	char buf[32];
	int sx, sy, x_pos, y_pos, w, h;

	hud_set_gauge_color(HUD_THROTTLE_GAUGE);

	// y_end is the y-coordinate of the current throttle setting, calc x-coordinate for edge of 
	// circle (x^2 + y^2 = r^2)
	y_pos = Hud_reticle_center[gr_screen.res][1] - y_end;
	x_pos = (int)sqrt(double(Outer_circle_radius[gr_screen.res] * Outer_circle_radius[gr_screen.res] - y_pos * y_pos) );
	x_pos = Hud_reticle_center[gr_screen.res][0] - x_pos;

	// draw current speed at (x_pos, y_end);
	sprintf(buf, "%d", fl2i(current_speed+0.5f));
	hud_num_make_mono(buf);
	gr_get_string_size(&w, &h, buf);
	sx = x_pos - w - 2;
	sy = fl2i(y_end - h/2.0f + 1.5);
	gr_printf(GetHUDCoordsCentred(sx), sy, buf);

	if ( Players[Player_num].flags & PLAYER_FLAGS_MATCH_TARGET ) {
		int offset;
		if ( current_speed <= 9.5 ) {
			offset = 0;
		} else {
			offset = 3;
		}
#if defined(GERMAN_BUILD)
		// print an m, cuz the voice says its an m.  
		// its a normal m cuz the german font has no special m (its an a)
		gr_string(GetHUDCoordsCentred(sx+offset), sy + h, "m");
#else
		gr_printf(GetHUDCoordsCentred(sx+offset), sy + h, "%c", Lcl_special_chars + 3);
#endif
	}
}

// draw the "desired speed" bar on the throttle
void hud_render_throttle_line(int y)
{
	// hud_set_bright_color();
	hud_set_gauge_color(HUD_THROTTLE_GAUGE, HUD_C_BRIGHT);

	const int x = GetHUDCoordsCentred(Reticle_frame_coords[gr_screen.res][RETICLE_LEFT_ARC][0]);
	GR_AABITMAP_EX(Reticle_gauges[RETICLE_LEFT_ARC].first_frame+3, x, y, 
		Hud_throttle_frame_w[gr_screen.res], 1, 0, y-Reticle_frame_coords[gr_screen.res][RETICLE_LEFT_ARC][1]);	
}

// Draw the throttle gauge along the left arc of the reticle
void hud_show_throttle()
{
	float	desired_speed, max_speed, current_speed, percent_max, percent_aburn_max;
	int	desired_y_pos, y_end;

	ship_info	*sip;
	sip = &Ship_info[Player_ship->ship_info_index];

	current_speed = Player_obj->phys_info.fspeed;
	if ( current_speed < 0.0f){
		current_speed = 0.0f;
	}

	max_speed = Ships[Player_obj->instance].current_max_speed;
	if ( max_speed <= 0 ) {
		max_speed = sip->max_vel.z;
	}

	desired_speed = Player->ci.forward * max_speed;
	if ( desired_speed < 0.0f ){		// so ships that go backwards don't force the indicators below where they can go
		desired_speed = 0.0f;
	}

	desired_y_pos = Hud_throttle_bottom_y[gr_screen.res] - fl2i(Hud_throttle_h[gr_screen.res]*desired_speed/max_speed+0.5f) - 1;

	Assert(max_speed != 0);
	percent_max = current_speed / max_speed;

	percent_aburn_max = 0.0f;
	if ( percent_max > 1 ) {
		percent_max = 1.0f;
		percent_aburn_max = (current_speed - max_speed) / (sip->afterburner_max_vel.z - max_speed);
		if ( percent_aburn_max > 1.0f ) {
			percent_aburn_max = 1.0f;
		}
		if ( percent_aburn_max < 0 ) {
			percent_aburn_max = 0.0f;
		}
	}

	y_end = Hud_throttle_bottom_y[gr_screen.res] - fl2i(Hud_throttle_h[gr_screen.res]*percent_max+0.5f);
	if ( percent_aburn_max > 0 ) {
		y_end -= fl2i(percent_aburn_max * Hud_throttle_aburn_h[gr_screen.res] + 0.5f);
	}

	if ( Player_obj->phys_info.flags & PF_AFTERBURNER_ON ) {
		desired_y_pos = 240;
	}

	// draw left arc (the dark portion of the throttle gauge)
	// hud_render_throttle_background(y_end);

	// draw throttle speed number
	hud_render_throttle_speed(current_speed, y_end);

	// draw the "desired speed" bar on the throttle
	hud_render_throttle_line(desired_y_pos);

	// draw left arc (the bright portion of the throttle gauge)
	hud_render_throttle_foreground(y_end);

	gr_printf(GetHUDCoordsCentred(Max_speed_coords[gr_screen.res][0]), Max_speed_coords[gr_screen.res][1], "%d",fl2i(max_speed));
	gr_printf(GetHUDCoordsCentred(Zero_speed_coords[gr_screen.res][0]), Zero_speed_coords[gr_screen.res][1], XSTR( "0", 292));
}

/*
// Draw the primary and secondary weapon indicators along the right arc of the reticle
void hud_show_reticle_weapons()
{
	int			gauge_index=0, frame_offset=0;
	ship_weapon	*swp;

	swp = &Player_ship->weapons;

	switch( swp->num_primary_banks ) {
		case 0:
			gauge_index = -1;
			break;

		case 1:
			gauge_index = RETICLE_ONE_PRIMARY;
			if ( Player_ship->weapons.current_primary_bank == -1 ) {
				frame_offset = 0;	
			} else {
				frame_offset = 1;	
			}
			break;

		case 2:
			gauge_index = RETICLE_TWO_PRIMARY;
			if ( swp->current_primary_bank == -1 ) {
				frame_offset = 0;	
			} else {
				if ( Player_ship->flags & SF_PRIMARY_LINKED ) {
					frame_offset = 3;
				} else {
					if ( swp->current_primary_bank == 0 ) {
						frame_offset = 1;
					} else {
						frame_offset = 2;
					}
				}
			}
			break;

		default:
			Int3();	// shouldn't happen (get Alan if it does)
			return;
			break;
	}
	
	if ( gauge_index != -1 ) {
		GR_AABITMAP(Reticle_gauges[gauge_index].first_frame+frame_offset, Reticle_frame_coords[gr_screen.res][gauge_index][0], Reticle_frame_coords[gr_screen.res][gauge_index][1]);		
	}

	int num_banks = swp->num_secondary_banks;
	if ( num_banks <= 0 ) {
		num_banks = Ship_info[Player_ship->ship_info_index].num_secondary_banks;
	}

	switch( num_banks ) {
		case 0:
			Int3();
			gauge_index = -1;
			break;

		case 1:
			gauge_index = RETICLE_ONE_SECONDARY;
			break;

		case 2:
			gauge_index = RETICLE_TWO_SECONDARY;
			break;

		case 3:
			gauge_index = RETICLE_THREE_SECONDARY;
			break;

		default:
			Int3();	// shouldn't happen (get Alan if it does)
			return;
			break;
	}
	
	if ( gauge_index != -1 ) {
		if ( swp->num_secondary_banks <= 0 ) {
			frame_offset = 0;
		} else {
			frame_offset = swp->current_secondary_bank+1;
		}

		GR_AABITMAP(Reticle_gauges[gauge_index].first_frame+frame_offset, Reticle_frame_coords[gr_screen.res][gauge_index][0], Reticle_frame_coords[gr_screen.res][gauge_index][1]);		
	}
}
*/

// Draw the lock threat gauge on the HUD.  Use Threat_flags to determine if a 
// threat exists, and draw flashing frames.
void hud_show_lock_threat()
{
	int frame_offset;

	if ( Player->threat_flags & (THREAT_LOCK | THREAT_ATTEMPT_LOCK) ) {
		if ( timestamp_elapsed(Threat_lock_timer) ) {
			if ( Player->threat_flags & THREAT_LOCK )  {
				Threat_lock_timer = timestamp(fl2i(THREAT_LOCK_FLASH/2.0f));
			} else {
				Threat_lock_timer = timestamp(THREAT_LOCK_FLASH);
			}
			Threat_lock_frame++;
			if ( Threat_lock_frame > 2 ) {
				Threat_lock_frame = 1;
			}
			if ( (Threat_lock_frame == 2) && (Player->threat_flags & THREAT_ATTEMPT_LOCK ) ) {
				snd_play( &Snds[SND_THREAT_FLASH]);
			}
		}
		frame_offset = Threat_lock_frame;
	} else {
		frame_offset = 0;
	}

	hud_set_gauge_color(HUD_THREAT_GAUGE);

	GR_AABITMAP(Reticle_gauges[RETICLE_LOCK_WARN].first_frame+frame_offset, 
		GetHUDCoordsCentred(Reticle_frame_coords[gr_screen.res][RETICLE_LOCK_WARN][0]), 
		Reticle_frame_coords[gr_screen.res][RETICLE_LOCK_WARN][1]);

	// "launch" flash
	if ( (frame_offset > 0) && (Player->threat_flags & THREAT_LOCK) ) {
		if ( hud_targetbox_flash_expired(TBOX_FLASH_CMEASURE) ) {
			// hack
			int bright;
			if(frame_offset % 2){
				bright = 1;
			} else {
				bright = 0;
			}
			// GR_AABITMAP(Reticle_gauges[RETICLE_LAUNCH_LABEL].first_frame+frame_offset%2, Reticle_frame_coords[gr_screen.res][RETICLE_LAUNCH_LABEL][0], Reticle_frame_coords[gr_screen.res][RETICLE_LAUNCH_LABEL][1]);

			// use hud text flash gauge code
			hud_show_text_flash_icon(XSTR("Launch", 1507), Reticle_launch_coords[gr_screen.res][1], bright);
		}
	}
}

// Draw the dumbfire threat gauge on the HUD.  Use Threat_flags to determine if a
// threat exists, and draw flashing frames.
void hud_show_dumbfire_threat()
{
	int frame_offset;

	if ( Player->threat_flags & THREAT_DUMBFIRE ) {
		if ( timestamp_elapsed(Threat_dumbfire_timer) ) {
			Threat_dumbfire_timer = timestamp(THREAT_DUMBFIRE_FLASH);
			Threat_dumbfire_frame++;
			if ( Threat_dumbfire_frame > 2 ) {
				Threat_dumbfire_frame = 1;
			}
		}
		frame_offset = Threat_dumbfire_frame;
	} else {
		frame_offset = 0;
	}

	hud_set_gauge_color(HUD_THREAT_GAUGE);

	GR_AABITMAP(Reticle_gauges[RETICLE_LASER_WARN].first_frame + frame_offset, 
		GetHUDCoordsCentred(Reticle_frame_coords[gr_screen.res][RETICLE_LASER_WARN][0]), 
		Reticle_frame_coords[gr_screen.res][RETICLE_LASER_WARN][1]);	
}

// Draw the center of the reticle
void hud_show_center_reticle()
{
	Assert(Reticle_gauges[RETICLE_CENTER].first_frame != -1 );

//	hud_set_default_color();
	// hud_set_bright_color();
	hud_set_gauge_color(HUD_CENTER_RETICLE, HUD_C_BRIGHT);

	GR_AABITMAP(Reticle_gauges[RETICLE_CENTER].first_frame, 
		GetHUDCoordsCentred(Reticle_frame_coords[gr_screen.res][RETICLE_CENTER][0]), 
		Reticle_frame_coords[gr_screen.res][RETICLE_CENTER][1]);
}

// Draw top portion of reticle
void hud_show_top_arc()
{
	hud_set_gauge_color(HUD_CENTER_RETICLE);

	// hud_set_default_color();
	if ( hud_gauge_active(HUD_THREAT_GAUGE) ) {	
		// draw top arc
		GR_AABITMAP(Reticle_gauges[RETICLE_TOP_ARC].first_frame+1, 
			GetHUDCoordsCentred(Reticle_frame_coords[gr_screen.res][RETICLE_TOP_ARC][0]), 
			Reticle_frame_coords[gr_screen.res][RETICLE_TOP_ARC][1]);		

		// draw dumbfire threat
		hud_show_dumbfire_threat();

		// draw lock threat
		hud_show_lock_threat();
	} else {		
		// draw top arc without any holes
		GR_AABITMAP(Reticle_gauges[RETICLE_TOP_ARC].first_frame, 
			GetHUDCoordsCentred(Reticle_frame_coords[gr_screen.res][RETICLE_TOP_ARC][0]), 
			Reticle_frame_coords[gr_screen.res][RETICLE_TOP_ARC][1]);
	}
}

// Draw right portion of reticle
void hud_show_right_arc()
{	
	hud_set_gauge_color(HUD_CENTER_RETICLE);

	GR_AABITMAP(Reticle_gauges[RETICLE_RIGHT_ARC].first_frame+1, 
		GetHUDCoordsCentred(Reticle_frame_coords[gr_screen.res][RETICLE_RIGHT_ARC][0]), 
		Reticle_frame_coords[gr_screen.res][RETICLE_RIGHT_ARC][1]);		
		
	// draw the weapons indicators in the holes along the right arc
	/*
	if ( hud_gauge_active(HUD_WEAPON_LINKING_GAUGE) ) {		
		// draw right arc with holes in it
		GR_AABITMAP(Reticle_gauges[RETICLE_RIGHT_ARC].first_frame+1, Reticle_frame_coords[gr_screen.res][RETICLE_RIGHT_ARC][0], Reticle_frame_coords[gr_screen.res][RETICLE_RIGHT_ARC][1]);		

//		the following line was removed by Jasen to get rid of "undeclared identifier"
//		hehe - DB
//		hud_show_reticle_weapons();
	} else {		
		// draw right arc without any holes
		GR_AABITMAP(Reticle_gauges[RETICLE_RIGHT_ARC].first_frame, Reticle_frame_coords[gr_screen.res][RETICLE_RIGHT_ARC][0], Reticle_frame_coords[gr_screen.res][RETICLE_RIGHT_ARC][1]);
	}
	*/
}

// Draw the left portion of the reticle
void hud_show_left_arc()
{			
	// draw left arc (the dark portion of the throttle gauge)
	hud_set_gauge_color(HUD_CENTER_RETICLE);	
	GR_AABITMAP(Reticle_gauges[RETICLE_LEFT_ARC].first_frame, 
		GetHUDCoordsCentred(Reticle_frame_coords[gr_screen.res][RETICLE_LEFT_ARC][0]), 
		Reticle_frame_coords[gr_screen.res][RETICLE_LEFT_ARC][1]);			
	
	// draw the throttle
	if ( hud_gauge_active(HUD_THROTTLE_GAUGE) ) {
		hud_set_gauge_color(HUD_THROTTLE_GAUGE);
		hud_show_throttle();		
	} 
}

// called once per frame from HUD_render_2d() to draw the reticle gauges
void hud_show_reticle()
{
	if ( !(Viewer_mode & VM_OTHER_SHIP) ) {
		hud_show_top_arc();
		hud_show_right_arc();
		hud_show_left_arc();
	}

	// draw the center of the reticle
	if ( hud_gauge_active(HUD_CENTER_RETICLE) ) {
		hud_show_center_reticle();
	}
}

void hudreticle_page_in()
{
	hud_frames	*hfp;

	int i;
	for ( i = 0; i < NUM_RETICLE_ANIS; i++ ) {
		hfp = &Reticle_gauges[i];
		bm_page_in_aabitmap( hfp->first_frame, hfp->num_frames);
	}

}