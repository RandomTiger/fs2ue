/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#ifndef UNITY_BUILD
#include "Freespace.h"
#include "Weapon.h"
#include "LinkList.h"
#include "3d.h" 
#include "3dInternal.h" 
#include "BmpMan.h"
#include "Trails.h"
#include "Timer.h"
#endif

#define MAX_TRAILS MAX_WEAPONS

// Stuff for missile trails doesn't need to be saved or restored... or does it?
typedef struct trail {
	int		head, tail;						// pointers into the queue for the trail points
	vector	pos[NUM_TRAIL_SECTIONS];	// positions of trail points
	float		val[NUM_TRAIL_SECTIONS];	// for each point, a value that tells how much to fade out	
	int		object_died;					// set to zero as long as object	
	int		trail_stamp;					// trail timestamp	

	// trail info
	trail_info info;							// this is passed when creating a trail

	struct	trail * prev;
	struct	trail * next;
} trail;


int Num_trails = 0;
trail Trails[MAX_TRAILS];

trail Trail_free_list;
trail Trail_used_list;

// Reset everything between levels
void trail_level_init()
{
	int i;

	Num_trails = 0;
	list_init( &Trail_free_list );
	list_init( &Trail_used_list );

	// Link all object slots into the free list
	for (i=0; i<MAX_TRAILS; i++)	{
		list_append(&Trail_free_list, &Trails[i] );
	}
}

//returns the number of a free trail
//returns -1 if no free trails
int trail_create(trail_info info)
{
	int trail_num;
	trail *trailp;

	// standalone server should never create trails
	if(Game_mode & GM_STANDALONE_SERVER){
		return -1;
	}

	if ( !Detail.weapon_extras )	{
		// No trails at slot 0
		return -1;
	}

	if (Num_trails >= MAX_TRAILS ) {
		#ifndef NDEBUG
		mprintf(("Trail creation failed - too many trails!\n" ));
		#endif
		return -1;
	}

	// Find next available trail
	trailp = GET_FIRST(&Trail_free_list);
	Assert( trailp != &Trail_free_list );		// shouldn't have the dummy element

	// remove trailp from the free list
	list_remove( &Trail_free_list, trailp );
	
	// insert trailp onto the end of used list
	list_append( &Trail_used_list, trailp );

	// increment counter
	Num_trails++;

	// get objnum
	trail_num = trailp-Trails;

	// Init the trail data
	trailp->info = info;
	trailp->tail = 0;
	trailp->head = 0;	
	trailp->object_died = 0;		
	trailp->trail_stamp = timestamp(trailp->info.stamp);

	return trail_num;
}

// output top and bottom vectors
// fvec == forward vector (eye viewpoint basically. in world coords)
// pos == world coordinate of the point we're calculating "around"
// w == width of the diff between top and bottom around pos
void trail_calc_facing_pts( vector *top, vector *bot, vector *fvec, vector *pos, float w )
{
	vector uvec, rvec;

	vm_vec_sub( &rvec, &Eye_position, pos );
	vm_vec_normalize( &rvec );

	vm_vec_crossprod(&uvec,fvec,&rvec);
	vm_vec_normalize(&uvec);

	vm_vec_scale_add( top, pos, &uvec, w/2.0f );
	vm_vec_scale_add( bot, pos, &uvec, -w/2.0f );
}

// trail is on ship
int trail_is_on_ship(int trail_index, ship *shipp)
{
	int idx;

	for(idx=0; idx<MAX_SHIP_CONTRAILS; idx++){
		if(shipp->trail_num[idx] == (short)trail_index){
			return 1;
		}
	}

	// nope
	return 0;
}

// Render the trail behind a missile.
// Basically a queue of points that face the viewer.
void trail_render( trail * trailp )
{		
	trail_info *ti;	

	if ( trailp->tail == trailp->head ) return;

	ti = &trailp->info;	

	vector topv, botv, *fvec, last_pos, tmp_fvec;
	vertex last_top, last_bot, top, bot;

	int sections[NUM_TRAIL_SECTIONS];
	int num_sections = 0;

	int n = trailp->tail;

	// if this trail is on the player ship, and he's in any padlock view except rear view, don't draw	
	if((Player_ship != NULL) && trail_is_on_ship(trailp - Trails, Player_ship) && (Viewer_mode & (VM_PADLOCK_UP | VM_PADLOCK_LEFT | VM_PADLOCK_RIGHT)) ){
		return;
	}

	do	{
		n--;
		if ( n < 0 ) n = NUM_TRAIL_SECTIONS-1;


		if ( trailp->val[n] > 1.0f ) {
			break;
		}

		sections[num_sections++] = n;

	} while ( n != trailp->head );

	int i;

	for (i=0; i<num_sections; i++ )	{

		n = sections[i];

		float w;
		ubyte l;

		w = trailp->val[n]*(ti->w_end - ti->w_start) + ti->w_start;
		l = (ubyte)fl2i((trailp->val[n]*(ti->a_end - ti->a_start) + ti->a_start)*255.0f);

		vector pos;

		pos = trailp->pos[n];

		if ( i == 0 )	{
			//fvec = 
			//&objp->orient.fvec;
			if ( num_sections > 1 )	{
	
				vm_vec_sub(&tmp_fvec, &pos, &trailp->pos[sections[i+1]] );
				vm_vec_normalize_safe(&tmp_fvec);
				fvec = &tmp_fvec;

			} else {
				fvec = &tmp_fvec;
				fvec->x = 0.0f;
				fvec->y = 0.0f;
				fvec->z = 1.0f;
			}
		} else {
			vm_vec_sub(&tmp_fvec, &last_pos, &pos );
			vm_vec_normalize_safe(&tmp_fvec);
			fvec = &tmp_fvec;
		}
			
		trail_calc_facing_pts( &topv, &botv, fvec, &pos, w );

		g3_rotate_vertex( &top, &topv );
		g3_rotate_vertex( &bot, &botv );
		top.a = bot.a = l;	

		if ( i > 0 )	{

			if ( i == num_sections-1 )	{
				// Last one...
				vector centerv;
				vm_vec_avg( &centerv, &topv, &botv );
				vertex center;
				g3_rotate_vertex( &center, &centerv );
				center.a = l;	

				vertex *vlist[3];
				vlist[0] = &last_top;
				vlist[1] = &last_bot;
				vlist[2] = &center;

				vlist[0]->u = 0.0f;  vlist[0]->v = 1.0f;
				vlist[1]->u = 0.0f;  vlist[1]->v = 0.0f;
				vlist[2]->u = 1.0f;  vlist[2]->v = 0.5f;

				gr_set_bitmap(ti->bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, l/255.0f );
				if ( D3D_enabled )	{
					g3_draw_poly( 3, vlist, TMAP_FLAG_TEXTURED|TMAP_FLAG_ALPHA|TMAP_FLAG_GOURAUD );
				} else {
					g3_draw_poly( 3, vlist, TMAP_FLAG_TEXTURED );
				}


			} else {
				vertex *vlist[4];
				vlist[0] = &last_bot;
				vlist[1] = &bot;
				vlist[2] = &top;
				vlist[3] = &last_top;

				vlist[0]->u = 0.0f;  vlist[0]->v = 0.0f;
				vlist[1]->u = 1.0f;  vlist[1]->v = 0.0f;
				vlist[2]->u = 1.0f;  vlist[2]->v = 1.0f;
				vlist[3]->u = 0.0f;  vlist[3]->v = 1.0f;

				gr_set_bitmap(ti->bitmap, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, l/255.0f );
				if ( D3D_enabled )	{
					g3_draw_poly( 4, vlist, TMAP_FLAG_TEXTURED|TMAP_FLAG_ALPHA|TMAP_FLAG_GOURAUD );
				} else {
					g3_draw_poly( 4, vlist, TMAP_FLAG_TEXTURED );
				}
			}
		}
		last_pos = pos;
		last_top = top;
		last_bot = bot;
	}
}


void trail_add_segment( int trail_num, vector *pos )
{
	if (trail_num < 0 ) return;
	if (trail_num >= MAX_TRAILS ) return;

	trail *trailp = &Trails[trail_num];

	int next = trailp->tail;
	trailp->tail++;
	if ( trailp->tail >= NUM_TRAIL_SECTIONS )
		trailp->tail = 0;

	if ( trailp->head == trailp->tail )	{
		// wrapped!!
		trailp->head++;
		if ( trailp->head >= NUM_TRAIL_SECTIONS )
			trailp->head = 0;
	}
	
	trailp->pos[next] = *pos;
	trailp->val[next] = 0.0f;
}		

void trail_set_segment( int trail_num, vector *pos )
{
	if (trail_num < 0 ) return;
	if (trail_num >= MAX_TRAILS ) return;

	trail *trailp = &Trails[trail_num];

	int next = trailp->tail-1;
	if ( next < 0 )	{
		next = NUM_TRAIL_SECTIONS-1;
	}
	
	trailp->pos[next] = *pos;
}

void trail_move_all(float frametime)
{
	trail *trailp;	

	trailp=GET_FIRST(&Trail_used_list);

	while ( trailp!=END_OF_LIST(&Trail_used_list) )	{

		int num_alive_segments = 0;

		if ( trailp->tail != trailp->head )	{
			int n = trailp->tail;			
			float time_delta = frametime / trailp->info.max_life;
			do	{
				n--;
				if ( n < 0 ) n = NUM_TRAIL_SECTIONS-1;

				trailp->val[n] += time_delta;

				if ( trailp->val[n] <= 1.0f ) {
					num_alive_segments++;	// Record how many still alive.
				}

			} while ( n != trailp->head );
		}		
	
		if ( trailp->object_died && (num_alive_segments < 1) )	{
			// delete it from the list!
			trail *next_one = GET_NEXT(trailp);

			// remove objp from the used list
			list_remove( &Trail_used_list, trailp);

			// add objp to the end of the free
			list_append( &Trail_free_list, trailp );

			// decrement counter
			Num_trails--;

			Assert(Num_trails >= 0);
			
			trailp = next_one;
		} else {
			trailp=GET_NEXT(trailp);
		}
	}
}

void trail_object_died( int trail_num )
{
	if (trail_num < 0 ) return;
	if (trail_num >= MAX_TRAILS ) return;

	trail *trailp = &Trails[trail_num];
	
	trailp->object_died++;
}

void trail_render_all()
{
	trail *trailp;

	if ( !Detail.weapon_extras )	{
		// No trails at slot 0
		return;
	}

	trailp=GET_FIRST(&Trail_used_list);

	while ( trailp!=END_OF_LIST(&Trail_used_list) )	{
		trail_render(trailp);
		trailp=GET_NEXT(trailp);
	}

}
int trail_stamp_elapsed(int trail_num)
{
	return timestamp_elapsed(Trails[trail_num].trail_stamp);
}

void trail_set_stamp(int trail_num)
{
	Trails[trail_num].trail_stamp = timestamp(Trails[trail_num].info.stamp);
}