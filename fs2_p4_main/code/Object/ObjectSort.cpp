/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef UNITY_BUILD
#include "object.h"
#include "3d.h"
#include "2d.h"
#include "SystemVars.h"
#include "Fireballs.h"
#include "MissionParse.h"
#include "Neb.h"
#endif

typedef struct sorted_obj {
	object			*obj;					// a pointer to the original object
	float				z, min_z, max_z;	// The object's z values relative to viewer
} sorted_obj;

int Num_sorted_objects = 0;
sorted_obj Sorted_objects[MAX_OBJECTS];
int Object_sort_order[MAX_OBJECTS];


// Used to (fairly) quicky find the 8 extreme
// points around an object.
vector check_offsets[8] = { 
  vector( -1.0f, -1.0f, -1.0f ),
  vector( -1.0f, -1.0f,  1.0f ),
  vector( -1.0f,  1.0f, -1.0f ),
  vector( -1.0f,  1.0f,  1.0f ),
  vector(  1.0f, -1.0f, -1.0f ),
  vector(  1.0f, -1.0f,  1.0f ),
  vector(  1.0f,  1.0f, -1.0f ),
  vector(  1.0f,  1.0f,  1.0f )
};

// See if an object is in the view cone.
// Returns:
// 0 if object isn't in the view cone
// 1 if object is in cone 
// This routine could possibly be optimized.  Right now, for an
// offscreen object, it has to rotate 8 points to determine it's
// offscreen.  Not the best considering we're looking at a sphere.
int obj_in_view_cone( object * objp )
{
	int i;
	vector tmp,pt; 
	ubyte codes;

// Use this to hack out player for testing.
// if ( objp == Player_obj ) return 0;

// OLD INCORRECT CODE!!!
//	g3_rotate_vector(&tmp,&objp->pos);
//	codes=g3_code_vector_radius(&tmp, objp->radius);
//	if ( !codes )	{
//		return 1;		// center is in, so return 1
//	}
//	return 0;

// This I commented out because it will quickly out for
// objects in the center, but cause one more rotation
// for objects outside the center.  So I figured it
// would be best to slow down objects inside by a bit
// and not penalize the offscreen ones, which require
// 8 rotatations to throw out.
//	g3_rotate_vector(&tmp,&objp->pos);
//	codes=g3_code_vector(&tmp);
//	if ( !codes )	{
//		//mprintf(( "Center is in, so render it\n" ));
//		return 1;		// center is in, so return 1
//	}

	// Center isn't in... are other points?

	ubyte and_codes = 0xff;

	for (i=0; i<8; i++ )	{
		vm_vec_scale_add( &pt, &objp->pos, &check_offsets[i], objp->radius );
		g3_rotate_vector(&tmp,&pt);
		codes=g3_code_vector(&tmp);
		if ( !codes )	{
			//mprintf(( "A point is inside, so render it.\n" ));
			return 1;		// this point is in, so return 1
		}
		and_codes &= codes;
	}

	if (and_codes)	{
		//mprintf(( "All points offscreen, so don't render it.\n" ));
		return 0;	//all points off screen
	}

	//mprintf(( "All points inside, so render it, but doesn't need clipping.\n" ));
	return 1;	
}


// Sorts all the objects by Z and renders them
extern int Fred_active;
void obj_render_all(void (*render_function)(object *objp), int renderFlags )
{
	object *objp;
	int i, j, incr;
	float fog_near, fog_far;

	objp = Objects;
	Num_sorted_objects = 0;
	for (i=0;i<=Highest_object_index;i++,objp++) {
		if ( (objp->type != OBJ_NONE) && (objp->flags&OF_RENDERS) )	{
			objp->flags &= ~OF_WAS_RENDERED;

			if ( obj_in_view_cone(objp) )	{
				sorted_obj * osp = &Sorted_objects[Num_sorted_objects];
				Object_sort_order[Num_sorted_objects] = Num_sorted_objects;
				Num_sorted_objects++;

				osp->obj = objp;
				vector to_obj;
				vm_vec_sub( &to_obj, &objp->pos, &Eye_position );
				osp->z = vm_vec_dot( &Eye_matrix.fvec, &to_obj );
/*
				if ( objp->type == OBJ_SHOCKWAVE )
					osp->z -= 2*objp->radius;
*/
				// Make warp in effect draw after any ship in it
				if ( objp->type == OBJ_FIREBALL )	{
					//if ( fireball_is_warp(objp) )	{
					osp->z -= 2*objp->radius;
					//}
				}
					
				osp->min_z = osp->z - objp->radius;
				osp->max_z = osp->z + objp->radius;
			}
		}	
	}


	// Sort them by their maximum z value
	if ( Num_sorted_objects > 1 ) {
		incr = Num_sorted_objects / 2;
		while( incr > 0 )	{
			for (i=incr; i<Num_sorted_objects; i++ )	{
				j = i - incr; 
				while (j>=0 )	{
					// compare element j and j+incr
					if ( (Sorted_objects[Object_sort_order[j]].max_z < Sorted_objects[Object_sort_order[j+incr]].max_z)  ) {
						// If not in correct order, them swap 'em
						int tmp;
						tmp = Object_sort_order[j];	
						Object_sort_order[j] = Object_sort_order[j+incr];
						Object_sort_order[j+incr] = tmp;
						j -= incr;
					} else {
						break;
					}
				}
			}
			incr = incr / 2;
		}
	}

	gr_zbuffer_set( GR_ZBUFF_FULL );	

	// now draw them
 	for (i=0; i<Num_sorted_objects; i++)	{
		sorted_obj * os = &Sorted_objects[Object_sort_order[i]];
		os->obj->flags |= OF_WAS_RENDERED;
		os->obj->m_renderFlags = renderFlags;

		// if we're fullneb, fire up the fog - this also generates a fog table
		if((The_mission.flags & MISSION_FLAG_FULLNEB) && (Neb2_render_mode != NEB2_RENDER_NONE) && !Fred_running && os->obj->m_renderFlags == OBJ_RENDER_OPAQUE){
			// get the fog values
			neb2_get_fog_values(&fog_near, &fog_far, os->obj);

			// only reset fog if the fog mode has changed - since regenerating a fog table takes
			// a bit of time
			if(((fog_near != gr_screen.fog_near) || (fog_far != gr_screen.fog_far)) && os->obj->m_renderFlags != OBJ_RENDER_TRANSPARENT){
				gr_fog_set(GR_FOGMODE_FOG, gr_screen.current_fog_color.red, gr_screen.current_fog_color.green, gr_screen.current_fog_color.blue, fog_near, fog_far);
			}

			// maybe skip rendering an object because its obscured by the nebula
			if(neb2_skip_render(os->obj, os->z)){
				continue;
			}
		}
		
		(*render_function)(os->obj);
	}

	// if we're fullneb, switch off the fog effet
	if(The_mission.flags & MISSION_FLAG_FULLNEB && Neb2_render_mode != NEB2_RENDER_NONE && Neb2_render_mode != NEB2_RENDER_SIMPLE)
	{
		gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	}

/*	Show spheres where wingmen should be flying
	{
		extern void render_wing_phantoms_all();
		render_wing_phantoms_all();
	}
	*/
}

