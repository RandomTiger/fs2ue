/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#ifndef UNITY_BUILD
#include "HUDlock.h"

#include "HUD.h"
#include "HUDtarget.h"
#include "HUDreticle.h"
#include "Player.h"
#include "Ship.h"
#include "weapon.h"
#include "sound.h"
#include "timer.h"
#include "Freespace.h"
#include "GameSequence.h"
#include "GameSnd.h"
#include "ai.h"
#include "bmpman.h"
#include "3d.h"
#include "LinkList.h"
#include "multi.h"
#include "Emp.h"
#endif

#define SCREEN_CENTER_X ((gr_screen.clip_left + gr_screen.clip_right)	/ 2.0f)
#define SCREEN_CENTER_Y ((gr_screen.clip_top + gr_screen.clip_bottom)   / 2.0f)

static float Lock_start_dist;
static int Rotate_time_id = 1;	// timer id for controlling how often to rotate triangles around lock indicator

int Missile_track_loop = -1;
int Missile_lock_loop = -1;

int Lock_target_box_width[GR_NUM_RESOLUTIONS] = {
	19,
	30
};
int Lock_target_box_height[GR_NUM_RESOLUTIONS] = {
	19,
	30
};

// the locked triangles (that orbit lock indicator) dimensions
float Lock_triangle_base[GR_NUM_RESOLUTIONS] = {
	4.0f,
	6.5f
};
float Lock_triangle_height[GR_NUM_RESOLUTIONS] = {
	4.0f,
	6.5f
};

int Lock_gauge_half_w[GR_NUM_RESOLUTIONS] = {
	17,
	28
};
int Lock_gauge_half_h[GR_NUM_RESOLUTIONS] = {
	15, 
	25
};

// hud_frames Lock_gauge;
int Lock_gauge_loaded = 0;
hud_anim Lock_gauge;
int Lock_gauge_draw = 0;
int Lock_gauge_draw_stamp = -1;
#define LOCK_GAUGE_BLINK_RATE			5			// blinks/sec

int Lockspin_half_w[GR_NUM_RESOLUTIONS] = {
	31,
	50
};
int Lockspin_half_h[GR_NUM_RESOLUTIONS] = {
	32, 
	52
};
hud_anim	Lock_anim;

char Lock_fname[GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN] = {
	"lock1",
	"2_lock1"
};

char Lockspin_fname[GR_NUM_RESOLUTIONS][MAX_FILENAME_LEN] = {
	"lockspin",
	"2_lockspin"
};

void hud_lock_determine_lock_point(vector *lock_world_pos_out);

// hud_init_missile_lock() is called at the beginning of a mission
//
void hud_init_missile_lock()
{
	Player->lock_indicator_start_x = -1;
	Player->lock_indicator_start_y = -1;
	Player->lock_indicator_visible = 0;
	Player_ai->current_target_is_locked = 0;

	Player_ai->last_secondary_index = -1;

	Rotate_time_id = 1;

	// Load in the frames need for the lead indicator
	if (!Lock_gauge_loaded) {
		/*
		Lock_gauge.first_frame = bm_load_animation(Lock_fname[gr_screen.res], &Lock_gauge.num_frames);
		if ( Lock_gauge.first_frame < 0 ) {
			Warning(LOCATION,"Cannot load hud ani: Lock_fname[gr_screen.res]\n");
		}
		*/
		hud_anim_init(&Lock_gauge, 0, 0, Lock_fname[gr_screen.res]);
		hud_anim_load(&Lock_gauge);

		hud_anim_init(&Lock_anim, 0, 0, Lockspin_fname[gr_screen.res]);
		hud_anim_load(&Lock_anim);

		Lock_gauge_loaded = 1;
		
		Lock_gauge_draw_stamp = -1;
		Lock_gauge_draw = 0;
	}
}

void hud_draw_diamond(int x, int y, int width, int height)
{
	Assert(height>0);
	Assert(width>0);

	int x1,x2,x3,x4,y1,y2,y3,y4;

	x1=x;
	y1=y-height/2;

	x2=x+width/2;
	y2=y;

	x3=x;
	y3=y+height/2;

	x4=x-width/2;
	y4=y;

	gr_line(x1,y1,x2,y2);
	gr_line(x2,y2,x3,y3);
	gr_line(x3,y3,x4,y4);
	gr_line(x4,y4,x1,y1);
}


// hud_show_lock_indicator() will display the lock indicator for homing missiles
void hud_show_lock_indicator(float frametime)
{
	int			target_objnum, sx, sy;
	object		*targetp;

	if (!Player->lock_indicator_visible){
		return;
	}

	target_objnum = Player_ai->target_objnum;
	Assert(target_objnum != -1);
	targetp = &Objects[target_objnum];

	// check to see if there are any missile to fire.. we don't want to show the 
	// lock indicator if there are missiles to fire.
	if ( !ship_secondary_bank_has_ammo(Player_obj->instance) ) {
		return;
	}
	
	hud_set_iff_color(targetp);
//	nprintf(("Alan","lockx: %d, locky: %d TargetX: %d, TargetY: %d\n", Player->lock_indicator_x, Player->lock_indicator_y, Player->current_target_sx, Player->current_target_sy));

	if (Player_ai->current_target_is_locked) {
		sx = Player->current_target_sx;
		sy = Player->current_target_sy;
		// show the rotating triangles if target is locked
		hud_draw_lock_triangles(sx, sy, frametime);
	} else {
		sx = Player->lock_indicator_x;
		sy = Player->lock_indicator_y;
	}

	// show locked indicator
	/*
	if ( Lock_gauge.first_frame >= 0 ) {
		gr_set_bitmap(Lock_gauge.first_frame);
		gr_aabitmap(sx - Lock_gauge_half_w[gr_screen.res], sy - Lock_gauge_half_h[gr_screen.res]);
	} else {
		hud_draw_diamond(sx, sy, Lock_target_box_width[gr_screen.res], Lock_target_box_height[gr_screen.res]);
	}
	*/
	Lock_gauge.sx = sx - Lock_gauge_half_w[gr_screen.res];
	Lock_gauge.sy = sy - Lock_gauge_half_h[gr_screen.res];
	if(Player_ai->current_target_is_locked){
		Lock_gauge.time_elapsed = 0.0f;			
		hud_anim_render(&Lock_gauge, 0.0f, 1);		
	} else {
		hud_anim_render(&Lock_gauge, frametime, 1);
	}
}

// Reset data used for player lock indicator
void hud_lock_reset(float lock_time_scale)
{
	weapon_info	*wip;
	ship_weapon	*swp;

	swp = &Player_ship->weapons;
	wip = &Weapon_info[swp->secondary_bank_weapons[swp->current_secondary_bank]];

	Player_ai->current_target_is_locked = 0;
	Player->lock_indicator_visible = 0;
	Player->target_in_lock_cone = 0;
	Player->lock_time_to_target = i2fl(wip->min_lock_time*lock_time_scale);
	Player->current_target_sx = -1;
	Player->current_target_sy = -1;
	Player->locking_subsys=NULL;
	Player->locking_on_center=0;
	Player->locking_subsys_parent=-1;
	hud_stop_looped_locking_sounds();

	Lock_gauge_draw_stamp = -1;
	Lock_gauge_draw = 0;

	// reset the lock anim time elapsed
	Lock_anim.time_elapsed = 0.0f;
}

// Determine if the locking code has a point to track
int hud_lock_has_homing_point()
{
	if ( Player_ai->targeted_subsys || Player->locking_subsys || Player->locking_on_center ) {
		return 1;
	}
	return 0;
}

int Nebula_sec_range = 0;
DCF_BOOL(nebula_sec_range, Nebula_sec_range)

int hud_lock_world_pos_in_range(vector *target_world_pos, vector *vec_to_target)
{
	float			dist_to_target, weapon_range;
	weapon_info	*wip;
	ship_weapon	*swp;

	int target_in_range=1;

	swp = &Player_ship->weapons;
	wip = &Weapon_info[swp->secondary_bank_weapons[swp->current_secondary_bank]];

	vm_vec_sub(vec_to_target, target_world_pos, &Player_obj->pos);
	dist_to_target = vm_vec_mag(vec_to_target);

	// calculate the range of the weapon, and only display the lead target indicator when
	// if the weapon can actually hit the target
	weapon_range = wip->max_speed * wip->lifetime;

	// reduce firing range in nebula
	if ((The_mission.flags & MISSION_FLAG_FULLNEB) && Nebula_sec_range) {
		weapon_range *= 0.8f;
	}

	if (dist_to_target > weapon_range) {
		target_in_range=0;
	}

	return target_in_range;
}

// Determine if point to lock on is in range
int hud_lock_target_in_range()
{
	vector		target_world_pos, vec_to_target;
	object		*targetp;

	if ( !hud_lock_has_homing_point() ) {
		return 0;
	}

	targetp = &Objects[Player_ai->target_objnum];

	if ( Player_ai->targeted_subsys != NULL ) {
		vm_vec_unrotate(&target_world_pos, &Player_ai->targeted_subsys->system_info->pnt, &targetp->orient);
		vm_vec_add2(&target_world_pos, &targetp->pos);
	} else {
		if ( Player->locking_subsys ) {
			vm_vec_unrotate(&target_world_pos, &Player->locking_subsys->system_info->pnt, &targetp->orient);
			vm_vec_add2(&target_world_pos, &targetp->pos);
		} else {
			Assert(Player->locking_on_center);
			target_world_pos = targetp->pos;
		}
	}

	return hud_lock_world_pos_in_range(&target_world_pos, &vec_to_target);
}

int hud_abort_lock()
{
	int target_team;

	target_team = obj_team(&Objects[Player_ai->target_objnum]);

	if ( Player_ship->weapons.num_secondary_banks <= 0 ) {
		return 1;
	}

	if ( Player_ship->weapons.current_secondary_bank < 0 ) {
		return 1;
	}

	// check to see if there are any missile to fire.. we don't want to show the 
	// lock indicator if there are no missiles to fire.
	if ( !ship_secondary_bank_has_ammo(Player_obj->instance) ) {
		return 1;
	}

	// if the target is friendly, don't lock!
	if ( hud_team_matches_filter(Player_ship->team, target_team)) {
		// if we're in multiplayer dogfight, ignore this
		if(!((Game_mode & GM_MULTIPLAYER) && (Netgame.type_flags & NG_TYPE_DOGFIGHT))){
			return 1;
		}
	}

	return 0;
}

// determine if the subsystem to lock on to has a direct line of sight
int hud_lock_on_subsys_ok()
{
	ship_subsys		*subsys;
	vector			subobj_pos;
	object			*target_objp;
	int				in_sight=0;
	
	Assert(Player_ai->target_objnum >= 0);
	target_objp	= &Objects[Player_ai->target_objnum];

	subsys = Player_ai->targeted_subsys;
	if ( !subsys ) {
		return 0;
	}

	vm_vec_unrotate(&subobj_pos, &subsys->system_info->pnt, &target_objp->orient);
	vm_vec_add2(&subobj_pos, &target_objp->pos);

	if ( Player->subsys_in_view < 0 ) {
		in_sight = ship_subsystem_in_sight(target_objp, subsys, &View_position, &subobj_pos);
	} else {
		in_sight = Player->subsys_in_view;
	}

	return in_sight;
}

// Determine if locking point is in the locking cone
void hud_lock_check_if_target_in_lock_cone(vector *lock_world_pos)
{
	float		dist, dot;
	vector	vec_to_target;

	dist = vm_vec_normalized_dir(&vec_to_target, lock_world_pos, &Player_obj->pos);
	dot = vm_vec_dot(&Player_obj->orient.fvec, &vec_to_target);

	if ( dot > 0.85) {
		Player->target_in_lock_cone = 1;
	} else {
		Player->target_in_lock_cone = 0;
	}

}

// return 1 if current secondary weapon is different than previous secondary weapon
int hud_lock_secondary_weapon_changed(ship_weapon *swp)
{

	if ( swp->current_secondary_bank != Player_ai->last_secondary_index ) {
		return 1;
	}

	return 0;
/*
	int last_wi_index = -1;
	int current_wi_index = -1;


	// do a quick out if same bank is selected
	if ( swp->current_secondary_bank == Player_ai->last_secondary_index ) {
		return 0;
	}

	// bank has changed, but it still may be the same weapon type
	if ( swp->current_secondary_bank >= 0 ) {
		current_wi_index = swp->secondary_bank_weapons[swp->current_secondary_bank];
	}

	if ( Player_ai->last_secondary_index >= 0 ) {
		last_wi_index = swp->secondary_bank_weapons[Player_ai->last_secondary_index];
	}

	if ( current_wi_index != last_wi_index ) {
		return 1;
	}

	return 0;
*/

}

// hud_update_lock_indicator() will manage the non-rendering dependant part of
// missle locking
void hud_update_lock_indicator(float frametime)
{
	ship_weapon *swp;
	weapon_info	*wip;
	vector		lock_world_pos;

	// if i'm a multiplayer observer, bail here
	if((Game_mode & GM_MULTIPLAYER) && ((Net_player->flags & NETINFO_FLAG_OBSERVER) || (Player_obj->type == OBJ_OBSERVER)) ){
		return;
	}

	Assert(Player_ai->target_objnum != -1);

	// be sure to unset this flag, then possibly set later in this function so that
	// threat indicators work properly.
	Player_ai->ai_flags &= ~AIF_SEEK_LOCK;

	if ( hud_abort_lock() ) {
		hud_lock_reset();
		return;
	}

	// if there is an EMP effect active, never update lock
	if(emp_active_local()){
		hud_lock_reset();
		return;
	}

	swp = &Player_ship->weapons;
	wip = &Weapon_info[swp->secondary_bank_weapons[swp->current_secondary_bank]];

	Lock_start_dist = wip->min_lock_time * wip->lock_pixels_per_sec;

	// if secondary weapons change, reset the lock
	if ( hud_lock_secondary_weapon_changed(swp) ) {
		hud_lock_reset();
	}
		
	Player_ai->last_secondary_index = swp->current_secondary_bank;

	if ( !(wip->wi_flags & WIF_HOMING_ASPECT) ) {
		hud_lock_reset();
		return;		
	}

	// Allow locking on ships and bombs (only targeted weapon allowed is a bomb, so don't bother checking flags)
	if ( (Objects[Player_ai->target_objnum].type != OBJ_SHIP) && (Objects[Player_ai->target_objnum].type != OBJ_WEAPON) ) {	
		hud_lock_reset();
		return;
	}

	hud_lock_determine_lock_point(&lock_world_pos);

	if ( !hud_lock_has_homing_point() ) {
		Player->target_in_lock_cone=0;
	}

	hud_lock_check_if_target_in_lock_cone(&lock_world_pos);

	// check if the target is within range of the current secondary weapon.  If it is not,
	// a lock will not be detected
	if ( !hud_lock_target_in_range() ) {
		Player->target_in_lock_cone = 0;
	}

	// If locking on a subsystem, and not in sight... can't lock
	//	Changed by MK on 4/3/98.  It was confusing me that my hornets would not lock on my target.
	//	It will now be confusing that they lock, but don't home on your subsystem, but I think that's preferable.
	//	Often you really care about destroying the target, not just the subsystem.
	/*if ( Player_ai->targeted_subsys ) {
		if ( !hud_lock_on_subsys_ok() ) {
			Player->target_in_lock_cone=0;
		}
	}*/

	if ( !Player->target_in_lock_cone ) {
		Player->locking_on_center=0;
		Player->locking_subsys_parent=-1;
		Player->locking_subsys=NULL;
	}
		
	hud_calculate_lock_position(frametime);

	if (!Player->lock_indicator_visible)
		return;

	if (Player_ai->current_target_is_locked) {
		if ( Missile_track_loop > -1 )	{
			snd_chg_loop_status(Missile_track_loop, 0);
			Missile_track_loop = -1;
			Missile_lock_loop = snd_play(&Snds[SND_MISSILE_LOCK]);
		}
	}
	else {
		Player_ai->ai_flags |= AIF_SEEK_LOCK;		// set this flag so multiplayer's properly track lock on other ships
		if ( Missile_lock_loop != -1 && snd_is_playing(Missile_lock_loop) ) {
			snd_stop(Missile_lock_loop);
			Missile_lock_loop = -1;
		}
	}
}

// hud_draw_lock_triangles() will draw the 4 rotating triangles around a lock indicator
// (This is done when a lock has been acquired)
#define ROTATE_DELAY 40
void hud_draw_lock_triangles_old(int center_x, int center_y, int radius)
{
	static float ang = 0.0f;

	float end_ang = ang + 2*PI;
	float x3,y3,x4,y4,xpos,ypos;

	if ( timestamp_elapsed(Rotate_time_id) ) {
		Rotate_time_id = timestamp(ROTATE_DELAY);
		ang += PI/12;
	}

	for (ang; ang <= end_ang; ang += PI/2.0f) {

		// draw the orbiting triangles

		//ang = atan2(target_point.y,target_point.x);
		xpos = center_x + (float)cos(ang)*(radius + Lock_triangle_height[gr_screen.res] + 2);
		ypos = center_y - (float)sin(ang)*(radius + Lock_triangle_height[gr_screen.res] + 2);
			
		x3 = xpos - Lock_triangle_base[gr_screen.res] * (float)sin(-ang);
		y3 = ypos + Lock_triangle_base[gr_screen.res] * (float)cos(-ang);
		x4 = xpos + Lock_triangle_base[gr_screen.res] * (float)sin(-ang);
		y4 = ypos - Lock_triangle_base[gr_screen.res] * (float)cos(-ang);

		xpos = xpos - Lock_triangle_base[gr_screen.res] * (float)cos(ang);
		ypos = ypos + Lock_triangle_base[gr_screen.res] * (float)sin(ang);

		hud_tri(x3, y3, xpos, ypos, x4, y4);
	} // end for
}

// draw a frame of the rotating lock triangles animation
void hud_draw_lock_triangles(int center_x, int center_y, float frametime)
{
	if ( Lock_anim.first_frame == -1 ) {
		hud_draw_lock_triangles_old(center_x, center_y, Lock_target_box_width[gr_screen.res]/2);
	} else {
		// render the anim
		Lock_anim.sx = center_x - Lockspin_half_w[gr_screen.res];
		Lock_anim.sy = center_y - Lockspin_half_h[gr_screen.res];

		// if its still animating
		if(Lock_anim.time_elapsed < Lock_anim.total_time){
			hud_anim_render(&Lock_anim, frametime, 1, 0, 1);
		} else {
			// if the timestamp is unset or expired
			if((Lock_gauge_draw_stamp < 0) || timestamp_elapsed(Lock_gauge_draw_stamp)){
				// reset timestamp
				Lock_gauge_draw_stamp = timestamp(1000 / (2 * LOCK_GAUGE_BLINK_RATE));

				// switch between draw and dont-draw
				Lock_gauge_draw = !Lock_gauge_draw;
			}

			// maybe draw the anim
			Lock_gauge.time_elapsed = 0.0f;			
			if(Lock_gauge_draw){
				hud_anim_render(&Lock_anim, frametime, 1, 0, 1);
			}			
		}		
	}
}

// hud_calculate_lock_position()  will determine where on the screen to draw the lock 
// indicator, and will determine when a lock has occurred.  If the lock indicator is not
// on the screen yet, hud_calculate_lock_start_pos() is called to pick a starting location
void hud_calculate_lock_position(float frametime)
{
	ship_weapon *swp;
	weapon_info	*wip;

	static float pixels_moved_while_locking;
	static float pixels_moved_while_degrading;
	static int Need_new_start_pos = 0;

	static double accumulated_x_pixels, accumulated_y_pixels;
	double int_portion;

	static float last_dist_to_target;
	
	static int catching_up;

	static int maintain_lock_count = 0;

	static float catch_up_distance = 0.0f;

	double hypotenuse, delta_x, delta_y;

	swp = &Player_ship->weapons;
	wip = &Weapon_info[swp->secondary_bank_weapons[swp->current_secondary_bank]];

	if (Player->target_in_lock_cone) {
		if (!Player->lock_indicator_visible) {
			hud_calculate_lock_start_pos();
			last_dist_to_target = 0.0f;

			Player->lock_indicator_x = Player->lock_indicator_start_x;
			Player->lock_indicator_y = Player->lock_indicator_start_y;
			Player->lock_indicator_visible = 1;

			Player->lock_time_to_target = i2fl(wip->min_lock_time);
			catching_up = 0;
		}

		Need_new_start_pos = 1;

		if (Player_ai->current_target_is_locked) {
			Player->lock_indicator_x = Player->current_target_sx;
			Player->lock_indicator_y = Player->current_target_sy;
			return;
		}

		delta_x = Player->lock_indicator_x - Player->current_target_sx;
		delta_y = Player->lock_indicator_y - Player->current_target_sy;

		if (!delta_y && !delta_x) {
			hypotenuse = 0.0f;
		}
		else {
			hypotenuse = _hypot(delta_y, delta_x);
		}

		Player->lock_dist_to_target = (float)hypotenuse;

		if (last_dist_to_target == 0) {
			last_dist_to_target = Player->lock_dist_to_target;
		}

		//nprintf(("Alan","dist to target: %.2f\n",Player->lock_dist_to_target));
		//nprintf(("Alan","last to target: %.2f\n\n",last_dist_to_target));

		if (catching_up) {
			//nprintf(("Alan","IN CATCH UP MODE  catch_up_dist is %.2f\n",catch_up_distance));	
			if ( Player->lock_dist_to_target < catch_up_distance )
				catching_up = 0;
		}
		else {
			//nprintf(("Alan","IN NORMAL MODE\n"));
			if ( (Player->lock_dist_to_target - last_dist_to_target) > 2.0f ) {
				catching_up = 1;
				catch_up_distance = last_dist_to_target + wip->catchup_pixel_penalty;
			}
		}

		last_dist_to_target = Player->lock_dist_to_target;

		if (!catching_up) {
			Player->lock_time_to_target -= frametime;
			if (Player->lock_time_to_target < 0.0f)
				Player->lock_time_to_target = 0.0f;
		}

		float lock_pixels_per_sec;
		if (Player->lock_time_to_target > 0) {
			lock_pixels_per_sec = Player->lock_dist_to_target / Player->lock_time_to_target;
		} else {
			lock_pixels_per_sec = i2fl(wip->lock_pixels_per_sec);
		}

		if (lock_pixels_per_sec > wip->lock_pixels_per_sec) {
			lock_pixels_per_sec = i2fl(wip->lock_pixels_per_sec);
		}
		
		if (catching_up) {
			pixels_moved_while_locking = wip->catchup_pixels_per_sec * frametime;
		} else {
			pixels_moved_while_locking = lock_pixels_per_sec * frametime;
		}
		
		if (delta_x != 0) {
			accumulated_x_pixels += pixels_moved_while_locking * delta_x/hypotenuse; 
		}

		if (delta_y != 0) {
			accumulated_y_pixels += pixels_moved_while_locking * delta_y/hypotenuse; 
		}

		if (fl_abs(accumulated_x_pixels) > 1.0f) {
			modf(accumulated_x_pixels, &int_portion);

			Player->lock_indicator_x -= (int)int_portion;

			if ( fl_abs(Player->lock_indicator_x - Player->current_target_sx) < fl_abs(int_portion) )
				Player->lock_indicator_x = Player->current_target_sx;

			accumulated_x_pixels -= int_portion;
		}

		if (fl_abs(accumulated_y_pixels) > 1.0f) {
			modf(accumulated_y_pixels, &int_portion);

			Player->lock_indicator_y -= (int)int_portion;

			if ( fl_abs(Player->lock_indicator_y - Player->current_target_sy) < fl_abs(int_portion) )
				Player->lock_indicator_y = Player->current_target_sy;

			accumulated_y_pixels -= int_portion;
		}

		if ( Missile_track_loop == -1 ) {	
			Missile_track_loop = snd_play_looping( &Snds[SND_MISSILE_TRACKING], 0.0f , -1, -1);
		}

		if (!Player->lock_time_to_target) {
			if ( (Player->lock_indicator_x == Player->current_target_sx) && (Player->lock_indicator_y == Player->current_target_sy) ) {
				if (maintain_lock_count++ > 1) {
					Player_ai->current_target_is_locked = 1;
				}
			} else {
				maintain_lock_count = 0;
			}
		}

	} else {

		if ( Missile_track_loop > -1 )	{
			snd_chg_loop_status(Missile_track_loop, 0);
			Missile_track_loop = -1;
		}

		Player_ai->current_target_is_locked = 0;

		if (!Player->lock_indicator_visible) {
			return;
		}

		catching_up = 0;
		last_dist_to_target = 0.0f;

		if (Need_new_start_pos) {
			hud_calculate_lock_start_pos();
			Need_new_start_pos = 0;
			accumulated_x_pixels = 0.0f;
			accumulated_y_pixels = 0.0f;
		}

		delta_x = Player->lock_indicator_x - Player->lock_indicator_start_x;
		delta_y = Player->lock_indicator_y - Player->lock_indicator_start_y;

		if (!delta_y && !delta_x) {
			hypotenuse = 0.0f;
		}
		else {
			hypotenuse = _hypot(delta_y, delta_x);
		}

		Player->lock_time_to_target += frametime;

		if (Player->lock_time_to_target > wip->min_lock_time)
			Player->lock_time_to_target = i2fl(wip->min_lock_time);

		pixels_moved_while_degrading = 2.0f * wip->lock_pixels_per_sec * frametime;

		if (delta_x != 0)
			accumulated_x_pixels += pixels_moved_while_degrading * delta_x/hypotenuse; 

		if (delta_y != 0)
			accumulated_y_pixels += pixels_moved_while_degrading * delta_y/hypotenuse; 

		if (fl_abs(accumulated_x_pixels) > 1.0f) {
			modf(accumulated_x_pixels, &int_portion);

			Player->lock_indicator_x -= (int)int_portion;

			if ( fl_abs(Player->lock_indicator_x - Player->lock_indicator_start_x) < fl_abs(int_portion) )
				Player->lock_indicator_x = Player->lock_indicator_start_x;

			accumulated_x_pixels -= int_portion;
		}

		if (fl_abs(accumulated_y_pixels) > 1.0f) {
			modf(accumulated_y_pixels, &int_portion);

			Player->lock_indicator_y -= (int)int_portion;

			if ( fl_abs(Player->lock_indicator_y - Player->lock_indicator_start_y) < fl_abs(int_portion) )
				Player->lock_indicator_y = Player->lock_indicator_start_y;

			accumulated_y_pixels -= int_portion;
		}

		if ( (Player->lock_indicator_x == Player->lock_indicator_start_x) && (Player->lock_indicator_y == Player->lock_indicator_start_y) ) {
			Player->lock_indicator_visible = 0;
		}
	}
}

// hud_calculate_lock_start_pos() will determine where to draw the starting location of the lock
// indicator.  It does this by picking a location that is Lock_start_dist pixels away from the current
// target (in 2D).  This is accomplished by finding the endpoint of a line that passes through the 
// origin, and connects the target and lock indicator postion (and has a magnitude of Lock_start_dist)
void hud_calculate_lock_start_pos()
{
	double hypotenuse;
	double delta_y;
	double delta_x;
	double target_mag, target_x, target_y;

	delta_x = Player->current_target_sx - SCREEN_CENTER_X;
	delta_y = Player->current_target_sy - SCREEN_CENTER_Y;

	if (!delta_x && !delta_y) {
		Player->lock_indicator_start_x = fl2i(SCREEN_CENTER_X + Lock_start_dist);
		Player->lock_indicator_start_y = fl2i(SCREEN_CENTER_Y);
		return;
	}

	hypotenuse = _hypot(delta_y, delta_x);

	if (hypotenuse >= Lock_start_dist) {
		Player->lock_indicator_start_x = fl2i(SCREEN_CENTER_X);
		Player->lock_indicator_start_y = fl2i(SCREEN_CENTER_Y);
		return;
	}

	target_mag = Lock_start_dist - hypotenuse;
	target_x = target_mag * (delta_x / hypotenuse);
	target_y = target_mag * (delta_y / hypotenuse);

	Player->lock_indicator_start_x = fl2i(SCREEN_CENTER_X - target_x);
	Player->lock_indicator_start_y = fl2i(SCREEN_CENTER_Y - target_y);

	if (Player->lock_indicator_start_x > gr_screen.clip_right)
		Player->lock_indicator_start_x = gr_screen.clip_right;

	if (Player->lock_indicator_start_y > gr_screen.clip_bottom)
		Player->lock_indicator_start_y = gr_screen.clip_bottom;

	if (Player->lock_indicator_start_x < gr_screen.clip_left)
		Player->lock_indicator_start_x = gr_screen.clip_left;

	if (Player->lock_indicator_start_y < gr_screen.clip_top)
		Player->lock_indicator_start_y = gr_screen.clip_top;
}

// hud_stop_looped_locking_sounds() will terminate any hud related looping sounds that are playing
void hud_stop_looped_locking_sounds()
{
	if ( Missile_track_loop > -1 )	{
		snd_stop(Missile_track_loop);
		Missile_track_loop = -1;
	}
}

// Get a new world pos for the locking point
void hud_lock_update_lock_pos(object *target_objp, vector *lock_world_pos)
{
	if ( Player_ai->targeted_subsys ) {
		get_subsystem_world_pos(target_objp, Player_ai->targeted_subsys, lock_world_pos);
		return;
	}

	if ( Player->locking_on_center ) {
		*lock_world_pos = target_objp->pos;
	} else {
		Assert(Player->locking_subsys);
		get_subsystem_world_pos(target_objp, Player->locking_subsys, lock_world_pos);
	}
}

// Try and find a new locking point
void hud_lock_get_new_lock_pos(object *target_objp, vector *lock_world_pos)
{
	ship			*target_shipp=NULL;
	int			lock_in_range=0;
	float			best_lock_dot=-1.0f, lock_dot=-1.0f;
	ship_subsys	*ss;
	vector		subsys_world_pos, vec_to_lock;

	if ( target_objp->type == OBJ_SHIP ) {
		target_shipp = &Ships[target_objp->instance];
	}

	// if a large ship, lock to pos closest to center and within range
	if ( (target_shipp) && (Ship_info[target_shipp->ship_info_index].flags & (SIF_BIG_SHIP|SIF_HUGE_SHIP)) ) {
		// check all the subsystems and the center of the ship
		
		// assume best lock pos is the center of the ship
		*lock_world_pos=target_objp->pos;
		Player->locking_on_center=1;
		Player->locking_subsys=NULL;
		Player->locking_subsys_parent=-1;
		lock_in_range = hud_lock_world_pos_in_range(lock_world_pos, &vec_to_lock);
		vm_vec_normalize(&vec_to_lock);
		if ( lock_in_range ) {
			best_lock_dot=vm_vec_dot(&Player_obj->orient.fvec, &vec_to_lock);
		} 
		// take center if reasonable dot
		if ( best_lock_dot > 0.95 ) {
			return;
		}

		// iterate through subsystems to see if we can get a better choice
		ss = GET_FIRST(&target_shipp->subsys_list);
		while ( ss != END_OF_LIST( &target_shipp->subsys_list ) ) {

			// get world pos of subsystem
			get_subsystem_world_pos(target_objp, ss, &subsys_world_pos);

			if ( hud_lock_world_pos_in_range(&subsys_world_pos, &vec_to_lock) ) {
				vm_vec_normalize(&vec_to_lock);
				lock_dot=vm_vec_dot(&Player_obj->orient.fvec, &vec_to_lock);
				if ( lock_dot > best_lock_dot ) {
					best_lock_dot=lock_dot;
					Player->locking_on_center=0;
					Player->locking_subsys=ss;
					Player->locking_subsys_parent=Player_ai->target_objnum;
					*lock_world_pos=subsys_world_pos;
				}
			}
			ss = GET_NEXT( ss );
		}
	} else {
		// if small ship (or weapon), just go for the center
		*lock_world_pos = target_objp->pos;
		Player->locking_on_center=1;
		Player->locking_subsys=NULL;
		Player->locking_subsys_parent=-1;
	}
}

// Decide which point lock should be homing on
void hud_lock_determine_lock_point(vector *lock_world_pos_out)
{
	vector	lock_world_pos;
	vertex	lock_point;
	object	*target_objp;

	Assert(Player_ai->target_objnum >= 0);
	target_objp = &Objects[Player_ai->target_objnum];

	Player->current_target_sx = -1;
	Player->current_target_sx = -1;

	// If subsystem is targeted, we must try to lock on that
	if ( Player_ai->targeted_subsys ) {
		hud_lock_update_lock_pos(target_objp, &lock_world_pos);
		Player->locking_on_center=0;
		Player->locking_subsys=NULL;
		Player->locking_subsys_parent=-1;
	} else {
		// See if we already have a successful locked point
		if ( hud_lock_has_homing_point() ) {
			hud_lock_update_lock_pos(target_objp, &lock_world_pos);
		} else {
			hud_lock_get_new_lock_pos(target_objp, &lock_world_pos);
		}
	}

	*lock_world_pos_out=lock_world_pos;

	g3_rotate_vertex(&lock_point,&lock_world_pos);
	g3_project_vertex(&lock_point);

	if (!(lock_point.flags & PF_OVERFLOW)) {  // make sure point projected
		Player->current_target_sx = (int)lock_point.sx;
		Player->current_target_sy = (int)lock_point.sy;
	}
}

void hudlock_page_in()
{
	bm_page_in_aabitmap( Lock_gauge.first_frame, Lock_gauge.num_frames );
}