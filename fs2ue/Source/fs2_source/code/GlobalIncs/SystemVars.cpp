/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#ifndef UNITY_BUILD
#include "SystemVars.h"
#include "Timer.h"
#include "neb.h"
#endif

fix Missiontime;
fix Frametime;
int	Framecount=0;

int Game_mode;

int Game_restoring = 0;		// If set, this means we are restoring data from disk

int	Viewer_mode;		//	Viewer's mode, see VM_xxxx flags.

// The detail level.  Anything below zero draws simple models earlier than it
// should.   Anything above zero draws higher detail models longer than it should.
// -2=lowest
// -1=low
// 0=normal (medium)	
// 1=high
// 2=extra high
int Game_detail_level = 0;
uint Game_detail_flags = DETAIL_DEFAULT;	// see systemvars.h for explanation

angles	Viewer_slew_angles;			//	Angles of viewer relative to forward.
vei		Viewer_external_info;		//	Viewer angles to ship in external view.
vci		Viewer_chase_info;			// View chase camera information

int Is_standalone;
int Rand_count;

int Interface_last_tick = -1;			// last timer tick on flip

// for notifying players of unknown ship types
int Fred_found_unknown_ship_during_parsing = 0;

// If true, then we are using Direct3D hardware.  This is used for game type stuff
// that changes when you're using hardware.
int D3D_enabled = 0;			

// Values used for noise for thruster animations
float Noise[NOISE_NUM_FRAMES] = { 
	0.468225f,
	0.168765f,
	0.318945f,
	0.292866f,
	0.553357f,
	0.468225f,
	0.180456f,
	0.418465f,
	0.489958f,
	1.000000f,
	0.468225f,
	0.599820f,
	0.664718f,
	0.294215f,
	0.000000f
};


int myrand()
{
	int rval;
	rval = rand();
	Rand_count++;
//	nprintf(("Alan","RAND: %d\n", rval));
	return rval;
}


// Variables for the loading callback hooks
static int cf_timestamp = -1;
static void (*cf_callback)(void) = NULL;
static int cf_in_callback = 0;	
static int cb_counter = 0;
static int cb_last_counter = 0;
static int cb_delta_step = -1;

int game_busy_callback( void (*callback)(void))
{
	if ( !callback ) {

		// Call it once more to finalize things
		cf_in_callback++;
		(*cf_callback)();
		cf_in_callback--;

		cf_timestamp = -1;
		cf_callback = NULL;
	} else {
		cb_counter = 0;
		cb_last_counter = 0;
		cf_timestamp = timer_get_milliseconds()+(1000/10);
		cf_callback = callback;

		// Call it once
		cf_in_callback++;
		(*cf_callback)();		// pass 0 first time!
		cf_in_callback--;
	
	}

	return cb_counter;
}

// Call whenever loading to display cursor
void game_busy()
{
	if ( cf_in_callback != 0 ) return;	// don't call callback if we're already in it.
	if ( cf_timestamp < 0 ) return;
	if ( !cf_callback ) return;

	cb_counter++;

//	mprintf(( "CB_COUNTER=%d\n", cb_counter ));

	int t1 = timer_get_milliseconds();

	if ( (t1 > cf_timestamp) || ((cb_counter>cb_last_counter+155)&&(cb_delta_step>0)) )	{
		cb_last_counter = cb_counter;
		cf_in_callback++;
		(*cf_callback)();
		cf_in_callback--;
		cf_timestamp = t1 + +(1000/10);
	}
}

//======================== CODE TO MONITOR EVENTS ======================

#ifndef NDEBUG

const int MAX_SYSTEM_MONITORS = 64;

static int Num_monitors = 0;
static monitor *Monitor[MAX_SYSTEM_MONITORS];

monitor::monitor( char *_name )
{
	int i;

	if ( Num_monitors >= MAX_SYSTEM_MONITORS )	{
		Int3();			// Too many monitor variables!! Increase MAX_SYSTEM_MONITORS!!
		return;
	}

	for (i=0; i<Num_monitors; i++ )	{
		int ret  = stricmp( Monitor[i]->name, _name );

		if ( ret == 0)	{
			Int3();		// This monitor variable already exists!!!! 
			return;
		} else if ( ret > 0 )	{
			break;		// Insert it here

		} else if ( ret < 0 )	{
			// do nothing
		}
	}

	if ( i < Num_monitors )	{
		// Insert it at element i
		int j;
		for (j=Num_monitors; j>i; j-- )	{
			Monitor[j] = Monitor[j-1];
		}
		Monitor[i] = this;		
		Num_monitors++;
	} else {
		Monitor[Num_monitors] = this;		
		Num_monitors++;
	}

	name = _name;
	value = 0;
}


int Monitor_inited = 0;
char Monitor_filename[128];
fix monitor_last_time = -1;

DCF(monitor,"Monitors game performace")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_STRING|ARG_NONE);
		if ( Dc_arg_type == ARG_NONE )	{
			if ( Monitor_inited )	{
				Monitor_inited = 0;

/*
				FILE *fp = fopen( Monitor_filename, "at" );
				if ( fp )	{
					fprintf( fp, "\n\n" );
					fprintf( fp, "Name\tMin\tMax\tAvg\n" );
					for (int i=0; i<Num_monitors; i++ )	{
						if ( Monitor[i]->cnt > 0 )	{
							fprintf( fp, "%s\t%d\t%d\t%d\n", Monitor[i]->name, Monitor[i]->min, Monitor[i]->max, Monitor[i]->sum / Monitor[i]->cnt  );
						} else {
							fprintf( fp, "%s\t%d\t%d\t?\n", Monitor[i]->name, Monitor[i]->min, Monitor[i]->max );
						}
					}
					fclose(fp);
				}
*/

				dc_printf( "Monitor to file '%s' turned off\n", Monitor_filename );
			} else {
				dc_printf( "Monitor isn't on\n" );
			}
		} else {
			if ( Monitor_inited )	{
				dc_printf( "Monitor already on\n" );
			} else {
				Monitor_inited = 1;

				strcpy( Monitor_filename, Dc_arg );

				// Reset them all
				int i;
				for (i=0; i<Num_monitors; i++ )	{
					Monitor[i]->value = 0;
					Monitor[i]->sum = 0;
					Monitor[i]->cnt = 0;
					Monitor[i]->min = 0;
					Monitor[i]->max = 0;
				}

				FILE *fp = fopen( Monitor_filename, "wt" );
				if ( fp )	{
					for (i=0; i<Num_monitors; i++ )	{
						if ( i > 0 )	{
							fprintf( fp, "\t" );
						}
						fprintf( fp, "%s", Monitor[i]->name );
					
					}
					fprintf( fp, "\n" );
					fclose(fp);
				}
				dc_printf( "Monitor outputting to file '%s'\n", Monitor_filename );
				monitor_last_time = -1;
			}
		}
	}
	if ( Dc_help )	{
		dc_printf( "Usage: monitor filename\nOutputs monitoring info to filename. No filename turns it off\n" );
	}
	
}


MONITOR(FrameRateX100);

void monitor_update()
{
	int i;
	FILE * fp;

	fix this_time = timer_get_fixed_seconds();
	fix frametime;

	if ( monitor_last_time != -1 )	{
		frametime = this_time - monitor_last_time;
	} else {
		frametime = 0;
	}

	if ( frametime > 0 )	{
		MONITOR_INC(FrameRateX100, (F1_0*100) / frametime );
	} else {
		MONITOR_INC(FrameRateX100, 0 );
	}

		
	if ( !Monitor_inited )	{
		return;
	}

	if ( frametime != 0 )	{
		fp = fopen( Monitor_filename, "at" );
		if ( fp )	{

			for (i=0; i<Num_monitors; i++ )	{
				if (i>0) fprintf( fp, "\t" );
				fprintf( fp, "%d", Monitor[i]->value );
			}
			fprintf( fp, "\n" );
			fclose(fp);
		}

		for (i=0; i<Num_monitors; i++ )	{

			// Record stats
			Monitor[i]->sum += Monitor[i]->value;

			if ( (Monitor[i]->cnt < 1)  || (Monitor[i]->value < Monitor[i]->min ))	{
				Monitor[i]->min = Monitor[i]->value;
			}

			if ( (Monitor[i]->cnt < 1)  || (Monitor[i]->value > Monitor[i]->max ))	{
				Monitor[i]->max = Monitor[i]->value;
			}

			Monitor[i]->cnt++;

			//	Reset the value
			Monitor[i]->value = 0;
		}
	} else {
		for (i=0; i<Num_monitors; i++ )	{
			//	Reset the value
			Monitor[i]->value = 0;
		}
	}

	monitor_last_time = timer_get_fixed_seconds();

}
#endif	//NDEBUG


#if MAX_DETAIL_LEVEL != 4
#error MAX_DETAIL_LEVEL is assumed to be 4 in SystemVars.cpp
#endif

#if NUM_DEFAULT_DETAIL_LEVELS != 4
#error NUM_DEFAULT_DETAIL_LEVELS is assumed to be 4 in SystemVars.cpp
#endif

// Detail level stuff
detail_levels Detail_defaults[NUM_DEFAULT_DETAIL_LEVELS] = {
	{				// Low
		0,			// setting
					// ===== Analogs (0-MAX_DETAIL_LEVEL) ====
		0,			// nebula_detail;				// 0=lowest detail, MAX_DETAIL_LEVEL=highest detail
		0,			// detail_distance;			// 0=lowest MAX_DETAIL_LEVEL=highest		
		0,			//	hardware_textures;			// 0=max culling, MAX_DETAIL_LEVEL=no culling
		0,			//	num_small_debris;			// 0=min number, MAX_DETAIL_LEVEL=max number
		0,			//	num_particles;				// 0=min number, MAX_DETAIL_LEVEL=max number
		0,			//	num_stars;					// 0=min number, MAX_DETAIL_LEVEL=max number
		0,			//	shield_effects;			// 0=min, MAX_DETAIL_LEVEL=max
		2,			// lighting;					// 0=min, MAX_DETAIL_LEVEL=max		

					// ====  Booleans ====
		0,			//	targetview_model;			// 0=off, 1=on		
		0,			//	planets_suns;				// 0=off, 1=on		
		0,			// weapon_extras
	},
	{				// Medium
		1,			// setting
					// ===== Analogs (0-MAX_DETAIL_LEVEL) ====
		1,			// nebula_detail;				// 0=lowest detail, MAX_DETAIL_LEVEL=highest detail
		1,			// detail_distance;			// 0=lowest MAX_DETAIL_LEVEL=highest		
		1,			//	hardware_textures;			// 0=max culling, MAX_DETAIL_LEVEL=no culling
		2,			//	num_small_debris;			// 0=min number, MAX_DETAIL_LEVEL=max number
		2,			//	num_particles;				// 0=min number, MAX_DETAIL_LEVEL=max number
		2,			//	num_stars;					// 0=min number, MAX_DETAIL_LEVEL=max number
		1,			//	shield_effects;			// 0=min, MAX_DETAIL_LEVEL=max
		3,			// lighting;					// 0=min, MAX_DETAIL_LEVEL=max		

		// ====  Booleans ====
		1,			//	targetview_model;			// 0=off, 1=on		
		1,			//	planets_suns;				// 0=off, 1=on
		1,			// weapon extras				
	},
	{				// High level
		2,			// setting
					// ===== Analogs (0-MAX_DETAIL_LEVEL) ====
		2,			// nebula_detail;				// 0=lowest detail, MAX_DETAIL_LEVEL=highest detail
		3,			// detail_distance;			// 0=lowest MAX_DETAIL_LEVEL=highest		
		3,			//	hardware_textures;			// 0=max culling, MAX_DETAIL_LEVEL=no culling
		3,			//	num_small_debris;			// 0=min number, MAX_DETAIL_LEVEL=max number
		3,			//	num_particles;				// 0=min number, MAX_DETAIL_LEVEL=max number
		4,			//	num_stars;					// 0=min number, MAX_DETAIL_LEVEL=max number
		3,			//	shield_effects;			// 0=min, MAX_DETAIL_LEVEL=max
		4,			// lighting;					// 0=min, MAX_DETAIL_LEVEL=max		

										// ====  Booleans ====
		1,			//	targetview_model;			// 0=off, 1=on		
		1,			//	planets_suns;				// 0=off, 1=on
		1,			// weapon_extras
	},
	{				// Highest level
		3,			// setting
					// ===== Analogs (0-MAX_DETAIL_LEVEL) ====
		3,			// nebula_detail;				// 0=lowest detail, MAX_DETAIL_LEVEL=highest detail
		3,			// detail_distance;			// 0=lowest MAX_DETAIL_LEVEL=highest		
		4,			//	hardware_textures;			// 0=max culling, MAX_DETAIL_LEVEL=no culling
		4,			//	num_small_debris;			// 0=min number, MAX_DETAIL_LEVEL=max number
		3,			//	num_particles;				// 0=min number, MAX_DETAIL_LEVEL=max number
		4,			//	num_stars;					// 0=min number, MAX_DETAIL_LEVEL=max number
		4,			//	shield_effects;			// 0=min, MAX_DETAIL_LEVEL=max
		4,			// lighting;					// 0=min, MAX_DETAIL_LEVEL=max		

										// ====  Booleans ====
		1,			//	targetview_model;			// 0=off, 1=on		
		1,			//	planets_suns;				// 0=off, 1=on
		1,			// weapon_extras
	},
};


// Global used to access detail levels in game and libs
detail_levels Detail = Detail_defaults[NUM_DEFAULT_DETAIL_LEVELS-1];

// Call this with:
// 0 - lowest
// NUM_DETAIL_LEVELS - highest
// To set the parameters in Detail to some set of defaults
void detail_level_set(int level)
{
	if ( level < 0 )	{
		Detail.setting = -1;
		return;
	}
	Assert( level >= 0 );
	Assert( level < NUM_DEFAULT_DETAIL_LEVELS );

	Detail = Detail_defaults[level];

	// reset nebula stuff
	neb2_set_detail_level(level);
}

// Returns the current detail level or -1 if custom.
int current_detail_level()
{
//	return Detail.setting;
	int i;

	for (i=0; i<NUM_DEFAULT_DETAIL_LEVELS; i++ )	{
		if ( memcmp( &Detail, &Detail_defaults[i], sizeof(detail_levels) )==0 )	{
			return i;
		}
	}
	return -1;
}

#ifndef NDEBUG
DCF(detail_level,"Change the detail level")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_INT|ARG_NONE);
		if ( Dc_arg_type & ARG_NONE )	{
			Game_detail_level = 0;
			dc_printf( "Detail level reset\n" );
		}
		if ( Dc_arg_type & ARG_INT )	{
			Game_detail_level = Dc_arg_int;
		}
	}

	if ( Dc_help )	
		dc_printf( "Usage: detail_level [n]\nn is detail level. 0 normal, - lower, + higher, -2 to 2 usually\nNo parameter resets it to default.\n" );

	if ( Dc_status )				
		dc_printf("Detail level set to %d\n", Game_detail_level);
}

DCF(detail, "Turns on/off parts of the game for speed testing" )
{
	if ( Dc_command )	{
		dc_get_arg(ARG_INT|ARG_NONE);
		if ( Dc_arg_type & ARG_NONE )	{
			if ( Game_detail_flags == DETAIL_DEFAULT )	{
				Game_detail_flags = DETAIL_FLAG_CLEAR;
				dc_printf( "Detail flags set lowest (except has screen clear)\n" );
			} else {
				Game_detail_flags = DETAIL_DEFAULT;
				dc_printf( "Detail flags set highest\n" );
			}
		}
		if ( Dc_arg_type & ARG_INT )	{
			Game_detail_flags ^= Dc_arg_int;
		}
	}

	if ( Dc_help )	{
		dc_printf( "Usage: detail [n]\nn is detail bit to toggle.\n" );
		dc_printf( "   1: draw the stars\n" );
		dc_printf( "   2: draw the nebulas\n" );
		dc_printf( "   4: draw the motion debris\n" );
		dc_printf( "   8: draw planets\n" );
		dc_printf( "  16: draw models not as blobs\n" );
		dc_printf( "  32: draw lasers not as pixels\n" );
		dc_printf( "  64: clear screen background after each frame\n" );
		dc_printf( " 128: draw hud stuff\n" );
		dc_printf( " 256: draw fireballs\n" );
		dc_printf( " 512: do collision detection\n" );
	}

	if ( Dc_status )	{
		dc_printf("Detail flags set to 0x%08x\n", Game_detail_flags);
		dc_printf( "   1: draw the stars: %s\n", (Game_detail_flags&1?"on":"off") );
		dc_printf( "   2: draw the nebulas: %s\n", (Game_detail_flags&2?"on":"off") );
		dc_printf( "   4: draw the motion debris: %s\n", (Game_detail_flags&4?"on":"off")  );
		dc_printf( "   8: draw planets: %s\n", (Game_detail_flags&8?"on":"off")  );
		dc_printf( "  16: draw models not as blobs: %s\n", (Game_detail_flags&16?"on":"off")  );
		dc_printf( "  32: draw lasers not as pixels: %s\n", (Game_detail_flags&32?"on":"off")  );
		dc_printf( "  64: clear screen background after each frame: %s\n", (Game_detail_flags&64?"on":"off")  );
		dc_printf( " 128: draw hud stuff: %s\n", (Game_detail_flags&128?"on":"off")  );
		dc_printf( " 256: draw fireballs: %s\n", (Game_detail_flags&256?"on":"off")  );
		dc_printf( " 512: do collision detection: %s\n", (Game_detail_flags&512?"on":"off")  );
	}
}
#endif