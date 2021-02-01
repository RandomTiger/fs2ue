/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#ifndef UNITY_BUILD
#include "Lighting.h"

#include "vecmat.h"
#include "3d.h"
#include "Fvi.h"
#include "Key.h"
#include "Timer.h"
#include "SystemVars.h"
#endif

#define MAX_LIGHTS 256
#define MAX_LIGHT_LEVELS 16

#define LT_DIRECTIONAL	0		// A light like a sun
#define LT_POINT			1		// A point light, like an explosion
#define LT_TUBE			2		// A tube light, like a fluorescent light

typedef struct light {
	int		type;							// What type of light this is
	vector	vec;							// location in world space of a point light or the direction of a directional light or the first point on the tube for a tube light
	vector	vec2;							// second point on a tube light
	vector	local_vec;					// rotated light vector
	vector	local_vec2;					// rotated 2nd light vector for a tube light
	float		intensity;					// How bright the light is.
	float		m_rad1, m_rad1_squared;		// How big of an area a point light affect.  Is equal to l->intensity / MIN_LIGHT;
	float		m_rad2, m_rad2_squared;		// How big of an area a point light affect.  Is equal to l->intensity / MIN_LIGHT;
	float		r,g,b;						// The color components of the light
	int		ignore_objnum;				// Don't light this object.  Used to optimize weapons casting light on parents.
	int		affected_objnum;			// for "unique lights". ie, lights which only affect one object (trust me, its useful)
} light;

light Lights[MAX_LIGHTS];
int Num_lights=0;

light *Relevent_lights[MAX_LIGHTS][MAX_LIGHT_LEVELS];
int Num_relevent_lights[MAX_LIGHT_LEVELS];
int Num_light_levels = 0;

#define MAX_STATIC_LIGHTS			10
light * Static_light[MAX_STATIC_LIGHTS];
int Static_light_count = 0;

static int Light_in_shadow = 0;	// If true, this means we're in a shadow

#define LM_BRIGHTEN  0
#define LM_DARKEN    1

#define MIN_LIGHT 0.03f	// When light drops below this level, ignore it.  Must be non-zero! (1/32)


int Lighting_off = 0;

// For lighting values, 0.75 is full intensity

#if 1		// ADAM'S new stuff
	int Lighting_mode = LM_BRIGHTEN;
	#define AMBIENT_LIGHT_DEFAULT		0.15f		//0.10f
	#define REFLECTIVE_LIGHT_DEFAULT 0.75f		//0.90f
#else
	int Lighting_mode = LM_DARKEN;
	#define AMBIENT_LIGHT_DEFAULT		0.75f		//0.10f
	#define REFLECTIVE_LIGHT_DEFAULT 0.50f		//0.90f
#endif

float Ambient_light = AMBIENT_LIGHT_DEFAULT;
float Reflective_light = REFLECTIVE_LIGHT_DEFAULT;

int Lighting_flag = 1;

DCF(light,"Changes lighting parameters")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_STRING);
		if ( !strcmp( Dc_arg, "ambient" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( (Dc_arg_float < 0.0f) || (Dc_arg_float > 1.0f) )	{
				Dc_help = 1;
			} else {
				Ambient_light = Dc_arg_float;
			} 
		} else if ( !strcmp( Dc_arg, "reflect" ))	{
			dc_get_arg(ARG_FLOAT);
			if ( (Dc_arg_float < 0.0f) || (Dc_arg_float > 1.0f) )	{
				Dc_help = 1;
			} else {
				Reflective_light = Dc_arg_float;
			} 
		} else if ( !strcmp( Dc_arg, "default" ))	{
			Lighting_mode = LM_BRIGHTEN;
			Ambient_light = AMBIENT_LIGHT_DEFAULT;
			Reflective_light = REFLECTIVE_LIGHT_DEFAULT;
			Lighting_flag = 0;
		} else if ( !strcmp( Dc_arg, "mode" ))	{
			dc_get_arg(ARG_STRING);
			if ( !strcmp(Dc_arg, "light") )	{
				Lighting_mode = LM_BRIGHTEN;
			} else if ( !strcmp(Dc_arg, "darken"))	{ 
				Lighting_mode = LM_DARKEN;
			} else {
				Dc_help = 1;
			}
		} else if ( !strcmp( Dc_arg, "dynamic" ))	{
			dc_get_arg(ARG_TRUE|ARG_FALSE|ARG_NONE);		
			if ( Dc_arg_type & ARG_TRUE )	Lighting_flag = 1;	
			else if ( Dc_arg_type & ARG_FALSE ) Lighting_flag = 0;	
			else if ( Dc_arg_type & ARG_NONE ) Lighting_flag ^= 1;	
		} else if ( !strcmp( Dc_arg, "on" ) )	{
			Lighting_off = 0;
		} else if ( !strcmp( Dc_arg, "off" ) )	{
			Lighting_off = 1;
		} else {
			// print usage, not stats
			Dc_help = 1;
		}
	}

	if ( Dc_help )	{
		dc_printf( "Usage: light keyword\nWhere keyword can be in the following forms:\n" );
		dc_printf( "light on|off          Turns all lighting on/off\n" );
		dc_printf( "light default         Resets lighting to all default values\n" );
		dc_printf( "light ambient X       Where X is the ambient light between 0 and 1.0\n" );
		dc_printf( "light reflect X       Where X is the material reflectiveness between 0 and 1.0\n" );
		dc_printf( "light dynamic [bool]  Toggles dynamic lighting on/off\n" );
		dc_printf( "light mode [light|darken]   Changes the lighting mode.\n" );
		dc_printf( "   Where 'light' means the global light adds light.\n");
		dc_printf( "   and 'darken' means the global light subtracts light.\n");
		Dc_status = 0;	// don't print status if help is printed.  Too messy.
	}

	if ( Dc_status )	{
		dc_printf( "Ambient light is set to %.2f\n", Ambient_light );
		dc_printf( "Reflective light is set to %.2f\n", Reflective_light );
		dc_printf( "Dynamic lighting is: %s\n", (Lighting_flag?"on":"off") );
		switch( Lighting_mode )	{
		case LM_BRIGHTEN:   dc_printf( "Lighting mode is: light\n" ); break;
		case LM_DARKEN:   dc_printf( "Lighting mode is: darken\n" ); break;
		default: dc_printf( "Lighting mode is: UNKNOWN\n" ); break;
		}
	}
}

void light_reset()
{
	int idx;

	// reset static (sun) lights
	for(idx=0; idx<MAX_STATIC_LIGHTS; idx++){
		Static_light[idx] = NULL;
	}
	Static_light_count = 0;

	Num_lights = 0;
	light_filter_reset();
}

// Rotates the light into the current frame of reference
void light_rotate(light * l)
{
	switch( l->type )	{
	case LT_DIRECTIONAL:
		// Rotate the light direction into local coodinates
		vm_vec_rotate(&l->local_vec, &l->vec, &Light_matrix );
		break;
	
	case LT_POINT:	{
			vector tempv;
			// Rotate the point into local coordinates
			vm_vec_sub(&tempv, &l->vec, &Light_base );
			vm_vec_rotate(&l->local_vec, &tempv, &Light_matrix );
		}
		break;
	
	case LT_TUBE:{
			vector tempv;
			// Rotate the point into local coordinates
			vm_vec_sub(&tempv, &l->vec, &Light_base );
			vm_vec_rotate(&l->local_vec, &tempv, &Light_matrix );
			
			// Rotate the point into local coordinates
			vm_vec_sub(&tempv, &l->vec2, &Light_base );
			vm_vec_rotate(&l->local_vec2, &tempv, &Light_matrix );
		}
		break;

	default:
		Int3();	// Invalid light type
	}
}

void light_add_directional( vector *dir, float intensity, float r, float g, float b )
{
	light * l;

	if ( Lighting_off ) return;

	if ( Num_lights >= MAX_LIGHTS ) return;

	l = &Lights[Num_lights++];

	l->type = LT_DIRECTIONAL;

	if ( Lighting_mode == LM_BRIGHTEN )	{
		vm_vec_copy_scale( &l->vec, dir, -1.0f );
	} else {
		vm_vec_copy_scale( &l->vec, dir, 1.0f );
	}

	l->r = r;
	l->g = g;
	l->b = b;
	l->intensity = intensity;
	l->m_rad1 = 0.0f;
	l->m_rad2 = 0.0f;
	l->m_rad1_squared = l->m_rad1*l->m_rad1;
	l->m_rad2_squared = l->m_rad2*l->m_rad2;
	l->ignore_objnum = -1;
	l->affected_objnum = -1;
		
	Assert( Num_light_levels <= 1 );
//	Relevent_lights[Num_relevent_lights[Num_light_levels-1]++][Num_light_levels-1] = l;

	if(Static_light_count < MAX_STATIC_LIGHTS){		
		Static_light[Static_light_count++] = l;
	}
}


void light_add_point( vector * pos, float lRad1, float lRad2, float intensity, float r, float g, float b, int ignore_objnum )
{
	light * l;

	if ( Lighting_off ) return;

	if (!Lighting_flag) return;

//	if ( keyd_pressed[KEY_LSHIFT] ) return;

	if ( Num_lights >= MAX_LIGHTS ) {
		mprintf(( "Out of lights!\n" ));
		return;
	}

	l = &Lights[Num_lights++];

	l->type = LT_POINT;
	l->vec = *pos;
	l->r = r;
	l->g = g;
	l->b = b;
	l->intensity = intensity;
	l->m_rad1 = lRad1;
	l->m_rad2 = lRad2;
	l->m_rad1_squared = l->m_rad1*l->m_rad1;
	l->m_rad2_squared = l->m_rad2*l->m_rad2;
	l->ignore_objnum = ignore_objnum;
	l->affected_objnum = -1;

	Assert( Num_light_levels <= 1 );
//	Relevent_lights[Num_relevent_lights[Num_light_levels-1]++][Num_light_levels-1] = l;
}

void light_add_point_unique( vector * pos, float lRad1, float lRad2, float intensity, float r, float g, float b, int affected_objnum)
{
	light * l;

	if ( Lighting_off ) return;

	if (!Lighting_flag) return;

//	if ( keyd_pressed[KEY_LSHIFT] ) return;

	if ( Num_lights >= MAX_LIGHTS ) {
		mprintf(( "Out of lights!\n" ));
		return;
	}

	l = &Lights[Num_lights++];

	l->type = LT_POINT;
	l->vec = *pos;
	l->r = r;
	l->g = g;
	l->b = b;
	l->intensity = intensity;
	l->m_rad1 = lRad1;
	l->m_rad2 = lRad2;
	l->m_rad1_squared = l->m_rad1*l->m_rad1;
	l->m_rad2_squared = l->m_rad2*l->m_rad2;
	l->ignore_objnum = -1;
	l->affected_objnum = affected_objnum;

	Assert( Num_light_levels <= 1 );
}

// for now, tube lights only affect one ship (to keep the filter stuff simple)
void light_add_tube(vector *p0, vector *p1, float r1, float r2, float intensity, float r, float g, float b, int affected_objnum)
{
	light * l;

	if ( Lighting_off ) return;

	if (!Lighting_flag) return;

	if ( Num_lights >= MAX_LIGHTS ) {
		mprintf(( "Out of lights!\n" ));
		return;
	}

	l = &Lights[Num_lights++];

	l->type = LT_TUBE;
	l->vec = *p0;
	l->vec2 = *p1;
	l->r = r;
	l->g = g;
	l->b = b;
	l->intensity = intensity;
	l->m_rad1 = r1;
	l->m_rad2 = r2;
	l->m_rad1_squared = l->m_rad1*l->m_rad1;
	l->m_rad2_squared = l->m_rad2*l->m_rad2;
	l->ignore_objnum = -1;
	l->affected_objnum = affected_objnum;

	Assert( Num_light_levels <= 1 );
}

// Reset the list of lights to point to all lights.
void light_filter_reset()
{
	int i;
	light *l;

	if ( Lighting_off ) return;

	Num_light_levels = 1;

	int n = Num_light_levels-1;
	Num_relevent_lights[n] = 0;

	l = Lights;
	for (i=0; i<Num_lights; i++, l++ )	{
		Relevent_lights[Num_relevent_lights[n]++][n] = l;
	}
}


// Makes a list of only the lights that will affect
// the sphere specified by 'pos' and 'rad' and 'objnum'
int light_filter_push( int objnum, vector *pos, float rad )
{
	int i;
	light *l;

	if ( Lighting_off ) return 0;

	light_filter_reset();

	int n1,n2;
	n1 = Num_light_levels-1;
	n2 = Num_light_levels;
	Num_light_levels++;
	Assert( Num_light_levels < MAX_LIGHT_LEVELS );

	Num_relevent_lights[n2] = 0;

	for (i=0; i<Num_relevent_lights[n1]; i++ )	{
		l = Relevent_lights[i][n1];

		switch( l->type )	{
		case LT_DIRECTIONAL:
			//Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
			break;

		case LT_POINT:	{
				// if this is a "unique" light source, it only affects one guy
				if(l->affected_objnum >= 0){
					if(objnum == l->affected_objnum){
						vector to_light;
						float dist_squared, max_dist_squared;
						vm_vec_sub( &to_light, &l->vec, pos );
						dist_squared = vm_vec_mag_squared(&to_light);

						max_dist_squared = l->m_rad2+rad;
						max_dist_squared *= max_dist_squared;
						
						if ( dist_squared < max_dist_squared )	{
							Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
						}
					}
				}
				// otherwise check all relevant objects
				else {
					// if ( (l->ignore_objnum<0) || (l->ignore_objnum != objnum) )	{
						vector to_light;
						float dist_squared, max_dist_squared;
						vm_vec_sub( &to_light, &l->vec, pos );
						dist_squared = vm_vec_mag_squared(&to_light);

						max_dist_squared = l->m_rad2+rad;
						max_dist_squared *= max_dist_squared;
						
						if ( dist_squared < max_dist_squared )	{
							Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
						}
					// }
				}
			}
			break;

		// hmm. this could probably be more optimal
		case LT_TUBE:
			// all tubes are "unique" light sources for now
			if((l->affected_objnum >= 0) && (objnum == l->affected_objnum)){
				Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
			}
			break;

		default:
			Int3();	// Invalid light type
		}
	}

	return Num_relevent_lights[n2];
}

int is_inside( vector *min, vector *max, vector * p0, float rad )
{
	float *origin = (float *)&p0->x;
	float *minB = (float *)min;
	float *maxB = (float *)max;
	int i;

	for (i=0; i<3; i++ )	{
		if ( origin[i] < minB[i] - rad )	{
			return 0;
		} else if (origin[i] > maxB[i] + rad )	{
			return 0;
		}
	}
	return 1;
}


int light_filter_push_box( vector *min, vector *max )
{
	int i;
	light *l;

	if ( Lighting_off ) return 0;

	int n1,n2;
	n1 = Num_light_levels-1;
	n2 = Num_light_levels;
	Num_light_levels++;

//	static int mll = -1;
//	if ( Num_light_levels > mll )	{
//		mll = Num_light_levels;
//		mprintf(( "Max level = %d\n", mll ));
//	}

	Assert( Num_light_levels < MAX_LIGHT_LEVELS );

	Num_relevent_lights[n2] = 0;

	for (i=0; i<Num_relevent_lights[n1]; i++ )	{
		l = Relevent_lights[i][n1];

		switch( l->type )	{
		case LT_DIRECTIONAL:
			Int3();	//Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
			break;

		case LT_POINT:	{
				// l->local_vec
				if ( is_inside( min, max, &l->local_vec, l->m_rad2 ) )	{
					Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
				}
			}
			break;

		case LT_TUBE:
			if ( is_inside(min, max, &l->local_vec, l->m_rad2) || is_inside(min, max, &l->local_vec2, l->m_rad2) )	{
				Relevent_lights[Num_relevent_lights[n2]++][n2] = l;
			}
			break;

		default:
			Int3();	// Invalid light type
		}
	}

	return Num_relevent_lights[n2];
}

void light_filter_pop()
{
	if ( Lighting_off ) return;

	Num_light_levels--;
	Assert( Num_light_levels > 0 );
}

void light_rotate_all()
{
	int i;
	light *l;

	if ( Lighting_off ) return;

	int n = Num_light_levels-1;

	l = Lights;
	for (i=0; i<Num_relevent_lights[n]; i++ )	{
		l = Relevent_lights[i][n];
		light_rotate(l);
	}

	for(i=0; i<Static_light_count; i++){	
		light_rotate(Static_light[i]);
	}
}

// return the # of global light sources
int light_get_global_count()
{
	return Static_light_count;
}

int light_get_global_dir(vector *pos, int n)
{
	if((n > MAX_STATIC_LIGHTS) || (n > Static_light_count-1)){
		return 0;
	}

	if ( Static_light[n] == NULL ) {
		return 0;
	}

	if (pos) {
		*pos = Static_light[n]->vec;

		if ( Lighting_mode != LM_DARKEN )	{
			vm_vec_scale( pos, -1.0f );
		}
	}
	return 1;
}

void light_set_shadow( int state )
{
	Light_in_shadow = state;
}

ubyte light_apply( vector *pos, vector * norm, float static_light_level )
{
	int i, idx;
	float lval;
	light *l;

	if (Detail.lighting==0) {
		// No static light
		ubyte l = ubyte(fl2i(static_light_level*255.0f));
		return l;
	}

	if ( Lighting_off ) return 191;

	// Factor in ambient light
	lval = Ambient_light;
	
	// Factor in light from suns if there are any
	if ( !Light_in_shadow ){
		// apply all sun lights
		for(idx=0; idx<Static_light_count; idx++){		
			float ltmp;

			// sanity 
			if(Static_light[idx] == NULL){
				continue;
			}

			// calculate light from surface normal
			ltmp = -vm_vec_dot(&Static_light[idx]->local_vec, norm )*Static_light[idx]->intensity*Reflective_light;		// reflective light

			switch(Lighting_mode)	{
			case LM_BRIGHTEN:
				if ( ltmp > 0.0f )
					lval += ltmp;
				break;
			case LM_DARKEN:
				if ( ltmp > 0.0f )
					lval -= ltmp;

				if ( lval < 0.0f ) 
					lval = 0.0f;
				break;
			}
		}
	}

	// At this point, l must be between 0 and 0.75 (0.75-1.0 is for dynamic light only)
	if ( lval < 0.0f ) {
		lval = 0.0f;
	} else if ( lval > 0.75f ) {
		lval = 0.75f;
	}

	lval *= static_light_level;

	int n = Num_light_levels-1;

	Num_relevent_lights[n];

	for (i=0; i<Num_relevent_lights[n]; i++ )	{
		l = Relevent_lights[i][n];

		vector to_light;
		float dot, dist;
		vm_vec_sub( &to_light, &l->local_vec, pos );
		dot = vm_vec_dot(&to_light, norm );
		if ( dot > 0.0f )	{
			dist = vm_vec_mag_squared(&to_light);
			if ( dist < l->m_rad1_squared )	{
				lval += l->intensity*dot;
			} else if ( dist < l->m_rad2_squared )	{
				// dist from 0 to 
				float n = dist - l->m_rad1_squared;
				float d = l->m_rad2_squared - l->m_rad1_squared;
				float ltmp = (1.0f - n / d )*dot*l->intensity;
				lval += ltmp;
			}
			if ( lval > 1.0f ) {
				return 255;
			}
		}
	}

	return ubyte(fl2i(lval*255.0f));
}

void light_apply_rgb( ubyte *param_r, ubyte *param_g, ubyte *param_b, vector *pos, vector * norm, float static_light_level )
{
	int i, idx;
	float rval, gval, bval;
	light *l;

	if (Detail.lighting==0) {
		// No static light
		ubyte l = ubyte(fl2i(static_light_level*255.0f));
		*param_r = l;
		*param_g = l;
		*param_b = l;
		return;
	}

	if ( Lighting_off ) {
		*param_r = 255;
		*param_g = 255;
		*param_b = 255;
		return;
	}

	// Factor in ambient light
	rval = Ambient_light;
	gval = Ambient_light;
	bval = Ambient_light;

	// Factor in light from sun if there is one
	if ( !Light_in_shadow ){
		// apply all sun lights
		for(idx=0; idx<Static_light_count; idx++){			
			float ltmp;

			// sanity
			if(Static_light[idx] == NULL){
				continue;
			}

			// calculate light from surface normal
			ltmp = -vm_vec_dot(&Static_light[idx]->local_vec, norm )*Static_light[idx]->intensity*Reflective_light;		// reflective light

			switch(Lighting_mode)	{
			case LM_BRIGHTEN:
				if ( ltmp > 0.0f )	{
					rval += Static_light[idx]->r * ltmp;
					gval += Static_light[idx]->g * ltmp;
					bval += Static_light[idx]->b * ltmp;
				}
				break;
			case LM_DARKEN:
				if ( ltmp > 0.0f )	{
					rval -= ltmp; if ( rval < 0.0f ) rval = 0.0f;
					gval -= ltmp; if ( gval < 0.0f ) gval = 0.0f;
					bval -= ltmp; if ( bval < 0.0f ) bval = 0.0f; 
				}
				break;
			}
		}
	}

	// At this point, l must be between 0 and 0.75 (0.75-1.0 is for dynamic light only)
	if ( rval < 0.0f ) {
		rval = 0.0f;
	} else if ( rval > 0.75f ) {
		rval = 0.75f;
	}
	if ( gval < 0.0f ) {
		gval = 0.0f;
	} else if ( gval > 0.75f ) {
		gval = 0.75f;
	}
	if ( bval < 0.0f ) {
		bval = 0.0f;
	} else if ( bval > 0.75f ) {
		bval = 0.75f;
	}

	rval *= static_light_level;
	gval *= static_light_level;
	bval *= static_light_level;

	int n = Num_light_levels-1;

	Num_relevent_lights[n];

	vector to_light;
	float dot, dist;
	vector temp;
	for (i=0; i<Num_relevent_lights[n]; i++ )	{
		l = Relevent_lights[i][n];

		dist = -1.0f;
		switch(l->type){
		// point lights
		case LT_POINT:			
			vm_vec_sub( &to_light, &l->local_vec, pos );
			break;

		// tube lights
		case LT_TUBE:						
			if(vm_vec_dist_to_line(pos, &l->local_vec, &l->local_vec2, &temp, &dist) != 0){
				continue;
			}
			vm_vec_sub(&to_light, &temp, pos);
			dist *= dist;	// since we use radius squared
			break;

		// others. BAD
		default:
			Int3();
		}

		dot = vm_vec_dot(&to_light, norm);
	//		dot = 1.0f;
		if ( dot > 0.0f )	{
			// indicating that we already calculated the distance (vm_vec_dist_to_line(...) does this for us)
			if(dist < 0.0f){
				dist = vm_vec_mag_squared(&to_light);
			}
			if ( dist < l->m_rad1_squared )	{
				float ratio;
				ratio = l->intensity*dot;
				ratio *= 0.25f;
				rval += l->r*ratio;
				gval += l->g*ratio;
				bval += l->b*ratio;
			} else if ( dist < l->m_rad2_squared )	{
				float ratio;
				// dist from 0 to 
				float n = dist - l->m_rad1_squared;
				float d = l->m_rad2_squared - l->m_rad1_squared;
				ratio = (1.0f - n / d)*dot*l->intensity;
				ratio *= 0.25f;
				rval += l->r*ratio;
				gval += l->g*ratio;
				bval += l->b*ratio;
			}
		}
	}

	float m = rval;
	if ( gval > m ) m = gval;
	if ( bval > m ) m = bval;

	if ( m > 1.0f )	{
		float im = 1.0f / m;

		rval *= im;
		gval *= im;
		bval *= im;
	}
	
	if ( rval < 0.0f ) {
		rval = 0.0f;
	} else if ( rval > 1.0f ) {
		rval = 1.0f;
	}
	if ( gval < 0.0f ) {
		gval = 0.0f;
	} else if ( gval > 1.0f ) {
		gval = 1.0f;
	}
	if ( bval < 0.0f ) {
		bval = 0.0f;
	} else if ( bval > 1.0f ) {
		bval = 1.0f;
	}

	*param_r = ubyte(fl2i(rval*255.0f));
	*param_g = ubyte(fl2i(gval*255.0f));
	*param_b = ubyte(fl2i(bval*255.0f));
}


