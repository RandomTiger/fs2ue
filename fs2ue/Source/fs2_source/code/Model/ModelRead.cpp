/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#ifndef UNITY_BUILD
#include <string.h>
#include <ctype.h>

#define MODEL_LIB

#include "cfile.h"
#include "model.h"
#include "bmpman.h"
#include "floating.h"
#include "3d.h"
#include "ship.h"
#include "ModelsInc.h"
#include "key.h"
#include "2d.h"
#include "3dinternal.h"
#include "LinkList.h"
#include "timer.h"
#include "FreeSpace.h"		// For flFrameTime
#include "fvi.h"
#endif

#define MAX_SUBMODEL_COLLISION_ROT_ANGLE (PI / 6.0f)	// max 30 degrees per frame

// info for special polygon lists

polymodel *Polygon_models[MAX_POLYGON_MODELS];

static int model_initted = 0;

#ifndef NDEBUG
CFILE *ss_fp;			// file pointer used to dump subsystem information
char	debug_name[_MAX_PATH];
int ss_warning_shown;		// have we shown the warning dialog concerning the subsystems?
int Model_ram = 0;			// How much RAM the models use total
#endif

#if defined(FS2_UE) || !defined(NDEBUG)
char  model_filename[_MAX_PATH];		// temp used to store filename
char	Global_filename[256];
#endif


// Anything less than this is considered incompatible.
#define PM_COMPATIBLE_VERSION 1900

// Anything greater than or equal to PM_COMPATIBLE_VERSION and 
// whose major version is less than or equal to this is considered
// compatible.  
#define PM_OBJFILE_MAJOR_VERSION 21

static int Model_signature = 0;

// Free up a model, getting rid of all its memory
// Can't be called from outside of model code because more
// than one person might be using this model so we can't free 
// it.
static void model_unload(int modelnum)
{
	int i,j;

	if ( (modelnum < 0) || (modelnum>MAX_POLYGON_MODELS))	{
		return;
	}

	polymodel *pm = Polygon_models[modelnum];

	if ( !pm )	{
		return;
	}

#ifndef NDEBUG
	Model_ram -= pm->ram_used;
#endif
	
	if (pm->paths)	{
		for (i=0; i<pm->n_paths; i++ )	{
			for (j=0; j<pm->paths[i].nverts; j++ )	{
				if ( pm->paths[i].verts[j].turret_ids )	{
					free(pm->paths[i].verts[j].turret_ids);
					pm->paths[i].verts[j].turret_ids = 0;
				}
			}
			if (pm->paths[i].verts)	{
				free(pm->paths[i].verts);
				pm->paths[i].verts = 0;
			}
		}
		free(pm->paths);
		pm->paths = 0;
		pm->n_paths = 0;
	}

	if ( pm->shield.verts )	{
		free( pm->shield.verts );
		pm->shield.verts = 0;
	}

	if ( pm->shield.tris )	{
		free(pm->shield.tris);
		pm->shield.tris = 0;
	}

	if ( pm->missile_banks )	{
		free(pm->missile_banks);
		pm->missile_banks = 0;
	}

	if ( pm->docking_bays )	{
		for (i=0; i<pm->n_docks; i++ )	{
			if ( pm->docking_bays[i].splines )	{
				free( pm->docking_bays[i].splines );
				pm->docking_bays[i].splines = 0;
			}
		}
		free(pm->docking_bays);
		pm->docking_bays = 0;
	}


	if ( pm->thrusters )	{
		free(pm->thrusters);
	}

#ifndef NDEBUG
	if ( pm->debug_info )	{
		free(pm->debug_info);
	}
#endif

	model_octant_free( pm );

	if (pm->submodel)	{
		for (i=0; i<pm->n_models; i++ )	{
			if ( pm->submodel[i].bsp_data )	{
				free(pm->submodel[i].bsp_data);
			}
		}
		free(pm->submodel);
	}

	if ( pm->xc ) {
		free(pm->xc);
	}

	if ( pm->lights )	{
		free(pm->lights);
	}

	if ( pm->gun_banks )	{
		free(pm->gun_banks);
	}

	pm->id = 0;
	memset( pm, 0, sizeof(polymodel));
	free( pm );

	Polygon_models[modelnum] = NULL;	
}

void model_free_all()
{
	int i;

	if ( !model_initted)	{
		model_init();
		return;
	}

	mprintf(( "Freeing all existing models...\n" ));

	for (i=0;i<MAX_POLYGON_MODELS;i++) {
		model_unload(i);		
	}

}

void model_init()
{
	int i;

	if ( model_initted )		{
		Int3();		// Model_init shouldn't be called twice!
		return;
	}

#ifndef NDEBUG
	Model_ram = 0;
#endif

	for (i=0;i<MAX_POLYGON_MODELS;i++) {
		Polygon_models[i] = NULL;
	}

	// Init the model caching system
	model_cache_init();

	atexit( model_free_all );
	model_initted = 1;
}

// routine to parse out values from a user property field of an object
void get_user_prop_value(char *buf, char *value)
{
	char *p, *p1, c;

	p = buf;
	while ( isspace(*p) || (*p == '=') )		// skip white space and equal sign
		p++;
	p1 = p;
	while ( !iscntrl(*p1) )
		p1++;
	c = *p1;
	*p1 = '\0';
	strcpy(value, p);
	*p1 = c;
}

// funciton to copy model data from one subsystem set to another subsystem set.  This function
// is called when two ships use the same model data, but since the model only gets read in one time,
// the subsystem data is only present in one location.  The ship code will call this routine to fix
// this situation by copying stuff from the source subsystem set to the dest subsystem set.
void model_copy_subsystems( int n_subsystems, model_subsystem *d_sp, model_subsystem *s_sp )
{
	int i, j;
	model_subsystem *source, *dest;

	for (i = 0; i < n_subsystems; i++ ) {
		source = &s_sp[i];
		for ( j = 0; j < n_subsystems; j++ ) {
			dest = &d_sp[j];
			if ( !stricmp( source->subobj_name, dest->subobj_name) ) {
				dest->flags = source->flags;
				dest->subobj_num = source->subobj_num;
				dest->model_num = source->model_num;
				dest->pnt = source->pnt;
				dest->radius = source->radius;
				dest->type = source->type;
				dest->turn_rate = source->turn_rate;
				dest->turret_gun_sobj = source->turret_gun_sobj;

				strcpy( dest->name, source->name );

				if ( dest->type == SUBSYSTEM_TURRET ) {
					int nfp;

					dest->turret_fov = source->turret_fov;
					dest->turret_num_firing_points = source->turret_num_firing_points;
					dest->turret_norm = source->turret_norm;
					dest->turret_matrix = source->turret_matrix;

					for (nfp = 0; nfp < dest->turret_num_firing_points; nfp++ )
						dest->turret_firing_point[nfp] = source->turret_firing_point[nfp];

					if ( dest->flags & MSS_FLAG_CREWPOINT )
						strcpy(dest->crewspot, source->crewspot);
				}
				break;
			}
		}
		if ( j == n_subsystems )
			Int3();							// get allender -- something is amiss with models

	}
}

// routine to get/set subsystem information
static void set_subsystem_info( model_subsystem *subsystemp, char *props, char *dname )
{
	char *p;
	char buf[32];
	char	lcdname[256];

	if ( (p = strstr(props, "$name")) != NULL)
		get_user_prop_value(p+5, subsystemp->name);
	else
		strcpy( subsystemp->name, dname );

	strcpy(lcdname, dname);
	strlwr(lcdname);

	// check the name for it's specific type
	if ( strstr(lcdname, "engine") ) {
		subsystemp->type = SUBSYSTEM_ENGINE;
	} else if ( strstr(lcdname, "radar") ) {
		subsystemp->type = SUBSYSTEM_RADAR;
	} else if ( strstr(lcdname, "turret") ) {
		float angle;

		subsystemp->type = SUBSYSTEM_TURRET;
		if ( (p = strstr(props, "$fov")) != NULL )
			get_user_prop_value(p+4, buf);			// get the value of the fov
		else
			strcpy(buf,"180");
		angle = ANG_TO_RAD(atoi(buf))/2.0f;
		subsystemp->turret_fov = (float)cos(angle);
		subsystemp->turret_num_firing_points = 0;

		if ( (p = strstr(props, "$crewspot")) != NULL) {
			subsystemp->flags |= MSS_FLAG_CREWPOINT;
			get_user_prop_value(p+9, subsystemp->crewspot);
		}

	} else if ( strstr(lcdname, "navigation") ) {
		subsystemp->type = SUBSYSTEM_NAVIGATION;
	} else if ( strstr(lcdname, "communication") )  {
		subsystemp->type = SUBSYSTEM_COMMUNICATION;
	} else if ( strstr(lcdname, "weapons") ) {
		subsystemp->type = SUBSYSTEM_WEAPONS;
	} else if ( strstr(lcdname, "sensors") ) {
		subsystemp->type = SUBSYSTEM_SENSORS;
	} else if ( strstr(lcdname, "solar") ) {
		subsystemp->type = SUBSYSTEM_SOLAR;
	} else if ( strstr(lcdname, "gas") ) {
		subsystemp->type = SUBSYSTEM_GAS_COLLECT;
	} else if ( strstr(lcdname, "activator") ) {
		subsystemp->type = SUBSYSTEM_ACTIVATION;
	}  else { // If unrecognized type, set to unknown so artist can continue working...
		subsystemp->type = SUBSYSTEM_UNKNOWN;
		mprintf(("Warning: Ignoring unrecognized subsystem %s, believed to be in ship %s\n", dname, Global_filename));
	}

	// Rotating subsystem
	if ( (p = strstr(props, "$rotate")) != NULL)	{
		subsystemp->flags |= MSS_FLAG_ROTATES;

		// get time for (a) complete rotation (b) step (c) activation
		float turn_time;
		get_user_prop_value(p+7, buf);
		turn_time = (float)atof(buf);

		// CASE OF STEPPED ROTATION
		if ( (p = strstr(props, "$stepped")) != NULL) {

			subsystemp->stepped_rotation = new(stepped_rotation);
			subsystemp->flags |= MSS_FLAG_STEPPED_ROTATE;

			// get number of steps
			if ( (p = strstr(props, "$steps")) != NULL) {
				get_user_prop_value(p+6, buf);
			   subsystemp->stepped_rotation->num_steps = atoi(buf);
			 } else {
			    subsystemp->stepped_rotation->num_steps = 8;
			 }

			// get pause time
			if ( (p = strstr(props, "$t_paused")) != NULL) {
				get_user_prop_value(p+9, buf);
			   subsystemp->stepped_rotation->t_pause = (float)atof(buf);
			 } else {
			    subsystemp->stepped_rotation->t_pause = 2.0f;
			 }

			// get transition time - time to go between steps
			if ( (p = strstr(props, "$t_transit")) != NULL) {
				get_user_prop_value(p+10, buf);
			    subsystemp->stepped_rotation->t_transit = (float)atof(buf);
			} else {
			    subsystemp->stepped_rotation->t_transit = 2.0f;
			}

			// get fraction of time spent in accel
			if ( (p = strstr(props, "$fraction_accel")) != NULL) {
				get_user_prop_value(p+15, buf);
			    subsystemp->stepped_rotation->fraction = (float)atof(buf);
			   Assert(subsystemp->stepped_rotation->fraction > 0 && subsystemp->stepped_rotation->fraction < 0.5);
			} else {
			    subsystemp->stepped_rotation->fraction = 0.3f;
			}

			int num_steps = subsystemp->stepped_rotation->num_steps;
			float t_trans = subsystemp->stepped_rotation->t_transit;
			float fraction = subsystemp->stepped_rotation->fraction;

			subsystemp->stepped_rotation->max_turn_accel = PI2 / (fraction*(1.0f - fraction) * num_steps * t_trans*t_trans);
			subsystemp->stepped_rotation->max_turn_rate =  PI2 / ((1.0f - fraction) * num_steps *t_trans);

		}

		// CASE OF AI ROTATION
		else if ( (p = strstr(props, "$ai")) != NULL) {
			get_user_prop_value(p+8, buf);
			subsystemp->flags |= MSS_FLAG_AI_ROTATE;

			// get parameters - ie, speed / dist / other ??
			// time to activate
			// condition
		}

		// CASE OF NORMAL CONTINUOUS ROTATION
		else {
			if ( fabs(turn_time) < 1 ) {				
				// Warning(LOCATION, "%s has subsystem %s with rotation time less than 1 sec", dname, Global_filename );
				subsystemp->flags &= ~MSS_FLAG_ROTATES;
				subsystemp->turn_rate = 0.0f;
			} else {
				subsystemp->turn_rate = PI2 / turn_time;
			}
		}
	}

}

// used in collision code to check if submode rotates too far
float get_submodel_delta_angle(submodel_instance_info *sii)
{
	vector diff;
	vm_vec_sub(&diff, (vector*)&sii->angs, (vector*)&sii->prev_angs);

	// find the angle
	float delta_angle = vm_vec_mag(&diff);

	// make sure we get the short way around
	if (delta_angle > PI) {
		delta_angle = (PI2 - delta_angle);
	}

	return delta_angle;
}

void do_new_subsystem( int n_subsystems, model_subsystem *slist, int subobj_num, float rad, vector *pnt, char *props, char *subobj_name, int model_num )
{
	int i;
	model_subsystem *subsystemp;


	if ( slist==NULL ) return;			// For TestCode, POFView, etc don't bother
	
	// try to find the name of the subsystem passed here on the list of subsystems currently on the
	// ship.  Assign the values only when the right subsystem is found

	for (i = 0; i < n_subsystems; i++ ) {
		subsystemp = &slist[i];
		if ( !stricmp(subobj_name, subsystemp->subobj_name) ) {
			subsystemp->flags = 0;
			subsystemp->subobj_num = subobj_num;
			subsystemp->turret_gun_sobj = -1;
			subsystemp->model_num = model_num;
			subsystemp->pnt = *pnt;				// use the offset to get the center point of the subsystem
			subsystemp->radius = rad;
			set_subsystem_info( subsystemp, props, subobj_name);
			strcpy(subsystemp->subobj_name, subobj_name);						// copy the object name
			return;
		}
	}
#ifndef NDEBUG
	if ( !ss_warning_shown) {
		char bname[_MAX_FNAME];

		_splitpath(model_filename, NULL, NULL, bname, NULL);
		Warning(LOCATION, "A subsystem was found in model %s that does not have a record in ships.tbl.\nA list of subsystems for this ship will be dumped to:\n\ndata\\tables\\%s.subsystems for inclusion\n into ships.tbl.", model_filename, bname);
		ss_warning_shown = 1;
	} else
#endif
	nprintf(("warning", "Subsystem %s in ships.tbl not found in model!\n", subobj_name));
#ifndef NDEBUG
	if ( ss_fp )	{
		char tmp_buffer[128];
		sprintf(tmp_buffer, "$Subsystem:\t\t\t%s,1,0.0\n", subobj_name);
		cfputs(tmp_buffer, ss_fp);
	}
#endif

}

void print_family_tree( polymodel *obj, int modelnum, char * ident, int islast )	
{
	char temp[50];

	if ( modelnum < 0 ) return;
	if (obj==NULL) return;

	if (strlen(ident)==0 )	{
		mprintf(( " %s", obj->submodel[modelnum].name ));
		sprintf( temp, " " );
	} else if ( islast ) 	{
		mprintf(( "%s��%s", ident, obj->submodel[modelnum].name ));
		sprintf( temp, "%s  ", ident );
	} else {
		mprintf(( "%s��%s", ident, obj->submodel[modelnum].name ));
		sprintf( temp, "%s� ", ident );
	}

	mprintf(( "\n" ));

	int child = obj->submodel[modelnum].first_child;
	while( child > -1 )	{
		if ( obj->submodel[child].next_sibling < 0 )
			print_family_tree( obj, child, temp,1 );
		else
			print_family_tree( obj, child, temp,0 );
		child = obj->submodel[child].next_sibling;
	}
}

void dump_object_tree(polymodel *obj)
{
	print_family_tree( obj, 0, "", 0 );
	key_getch();
}

void create_family_tree(polymodel *obj)
{
	int i;
	for (i=0; i<obj->n_models; i++ )	{
		obj->submodel[i].num_children = 0;
		obj->submodel[i].first_child = -1;
		obj->submodel[i].next_sibling = -1;
	}

	for (i=0; i<obj->n_models; i++ )	{
		int pn;
		pn = obj->submodel[i].parent;
		if ( pn > -1 )	{
			obj->submodel[pn].num_children++;
			int tmp = obj->submodel[pn].first_child;
			obj->submodel[pn].first_child = i;
			obj->submodel[i].next_sibling = tmp;
		}
	}
}

void model_calc_bound_box( vector *box, vector *big_mn, vector *big_mx)
{
	box[0].x = big_mn->x; box[0].y = big_mn->y; box[0].z = big_mn->z;
	box[1].x = big_mx->x; box[1].y = big_mn->y; box[1].z = big_mn->z;
	box[2].x = big_mx->x; box[2].y = big_mx->y; box[2].z = big_mn->z;
	box[3].x = big_mn->x; box[3].y = big_mx->y; box[3].z = big_mn->z;


	box[4].x = big_mn->x; box[4].y = big_mn->y; box[4].z = big_mx->z;
	box[5].x = big_mx->x; box[5].y = big_mn->y; box[5].z = big_mx->z;
	box[6].x = big_mx->x; box[6].y = big_mx->y; box[6].z = big_mx->z;
	box[7].x = big_mn->x; box[7].y = big_mx->y; box[7].z = big_mx->z;
}


//	Debug thing so we don't repeatedly show warning messages.
#ifndef NDEBUG
int Bogus_warning_flag_1903 = 0;
#endif

//reads a binary file containing a 3d model
int read_model_file(polymodel * pm, char *filename, int n_subsystems, model_subsystem *subsystems)
{
	CFILE *fp;
	int version;
	int id, len, next_chunk;
	int i,j;

#if !defined(NDEBUG) || defined(FS2_UE)
	strcpy(Global_filename, filename);
#endif

	fp = cfopen(filename,"rb");
	if (!fp){
		Error( LOCATION, "Can't open file <%s>",filename);
		return 0;
	}		

	// code to get a filename to write out subsystem information for each model that
	// is read.  This info is essentially debug stuff that is used to help get models
	// into the game quicker
#if 0
	{
		char bname[_MAX_FNAME];

		_splitpath(filename, NULL, NULL, bname, NULL);
		sprintf(debug_name, "%s.subsystems", bname);
		ss_fp = cfopen(debug_name, "wb", CFILE_NORMAL, CF_TYPE_TABLES );
		if ( !ss_fp )	{
			mprintf(( "Can't open debug file for writing subsystems for %s\n", filename));
		} else {
			strcpy(model_filename, filename);
			ss_warning_shown = 0;
		}
	}
#endif

	id = cfread_int(fp);

	if (id!='OPSP')
		Error( LOCATION, "Bad ID in model file <%s>",filename);

	// Version is major*100+minor
	// So, major = version / 100;
	//     minor = version % 100;
	version = cfread_int(fp);

	//Warning( LOCATION, "POF Version = %d", version );
	
	if (version < PM_COMPATIBLE_VERSION || (version/100) > PM_OBJFILE_MAJOR_VERSION)	{
		Warning(LOCATION,"Bad version (%d) in model file <%s>",version,filename);
		return 0;
	}

	pm->version = version;
	Assert( strlen(filename) < FILENAME_LEN );
	strncpy(pm->filename, filename, FILENAME_LEN);

	memset( &pm->view_positions, 0, sizeof(pm->view_positions) );

	// reset insignia counts
	pm->num_ins = 0;

	id = cfread_int(fp);
	len = cfread_int(fp);
	next_chunk = cftell(fp) + len;

	while (!cfeof(fp)) {

//		mprintf(("Processing chunk <%c%c%c%c>, len = %d\n",id,id>>8,id>>16,id>>24,len));
//		key_getch();

		switch (id) {

			case ID_OHDR: {		//Object header
				//vector v;

				//mprintf(0,"Got chunk OHDR, len=%d\n",len);

#if defined( FREESPACE1_FORMAT )
				pm->n_models = cfread_int(fp);
//				mprintf(( "Num models = %d\n", pm->n_models ));
				pm->rad = cfread_float(fp);
				pm->flags = cfread_int(fp);	// 1=Allow tiling
#elif defined( FREESPACE2_FORMAT )
				pm->rad = cfread_float(fp);
				pm->flags = cfread_int(fp);	// 1=Allow tiling
				pm->n_models = cfread_int(fp);
//				mprintf(( "Num models = %d\n", pm->n_models ));
#endif
				
				pm->submodel = (bsp_info *)malloc( sizeof(bsp_info)*pm->n_models );
				Assert(pm->submodel != NULL );
				memset( pm->submodel, 0, sizeof(bsp_info)*pm->n_models );

				//Assert(pm->n_models <= MAX_SUBMODELS);

				cfread_vector(&pm->mins,fp);
				cfread_vector(&pm->maxs,fp);
				model_calc_bound_box(pm->bounding_box,&pm->mins,&pm->maxs);
				
				pm->n_detail_levels = cfread_int(fp);
			//	mprintf(( "There are %d detail levels\n", pm->n_detail_levels ));
				for (i=0; i<pm->n_detail_levels;i++ )	{
					pm->detail[i] = cfread_int(fp);
					pm->detail_depth[i] = 0.0f;
			///		mprintf(( "Detail level %d is model %d.\n", i, pm->detail[i] ));
				}

				pm->num_debris_objects = cfread_int(fp);
				Assert( pm->num_debris_objects <= MAX_DEBRIS_OBJECTS );
				// mprintf(( "There are %d debris objects\n", pm->num_debris_objects ));
				for (i=0; i<pm->num_debris_objects;i++ )	{
					pm->debris_objects[i] = cfread_int(fp);
					// mprintf(( "Debris object %d is model %d.\n", i, pm->debris_objects[i] ));
				}

				if ( pm->version >= 1903 )	{
	
					if ( pm->version >= 2009 )	{
																	
						pm->mass = cfread_float(fp);
						cfread_vector( &pm->center_of_mass, fp );
						cfread_vector( &pm->moment_of_inertia.rvec, fp );
						cfread_vector( &pm->moment_of_inertia.uvec, fp );
						cfread_vector( &pm->moment_of_inertia.fvec, fp );
					} else {
						// old code where mass wasn't based on area, so do the calculation manually

						float vol_mass = cfread_float(fp);
						//	Attn: John Slagel:  The following is better done in bspgen.
						// Convert volume (cubic) to surface area (quadratic) and scale so 100 -> 100
						float area_mass = (float) pow(vol_mass, 0.6667f) * 4.65f;

						pm->mass = area_mass;
						float mass_ratio = vol_mass / area_mass; 
							
						cfread_vector( &pm->center_of_mass, fp );
						cfread_vector( &pm->moment_of_inertia.rvec, fp );
						cfread_vector( &pm->moment_of_inertia.uvec, fp );
						cfread_vector( &pm->moment_of_inertia.fvec, fp );

						// John remove this with change to bspgen
						vm_vec_scale( &pm->moment_of_inertia.rvec, mass_ratio );
						vm_vec_scale( &pm->moment_of_inertia.uvec, mass_ratio );
						vm_vec_scale( &pm->moment_of_inertia.fvec, mass_ratio );
					}	
				} else {
#ifndef NDEBUG
					if (stricmp("fighter04.pof", filename)) {
						if (Bogus_warning_flag_1903 == 0) {
							Warning(LOCATION, "Ship %s is old.  Cannot compute mass.\nSetting to 50.0f.  Talk to John.", filename);
							Bogus_warning_flag_1903 = 1;
						}
					}
#endif
					pm->mass = 50.0f;
					vm_vec_zero( &pm->center_of_mass );
					vm_set_identity( &pm->moment_of_inertia );
					vm_vec_scale(&pm->moment_of_inertia.rvec, 0.001f);
					vm_vec_scale(&pm->moment_of_inertia.uvec, 0.001f);
					vm_vec_scale(&pm->moment_of_inertia.fvec, 0.001f);
				}

				// read in cross section info
				pm->xc = NULL;
				if ( pm->version >= 2014 ) {
					pm->num_xc = cfread_int(fp);
					if (pm->num_xc > 0) {
						pm->xc = (cross_section*) malloc(pm->num_xc*sizeof(cross_section));
						for (int i=0; i<pm->num_xc; i++) {
							pm->xc[i].z = cfread_float(fp);
							pm->xc[i].radius = cfread_float(fp);
						}
					}
				} else {
					pm->num_xc = 0;
				}

				if ( pm->version >= 2007 )	{
					pm->num_lights = cfread_int(fp);
					//mprintf(( "Found %d lights!\n", pm->num_lights ));

					pm->lights = (bsp_light *)malloc( sizeof(bsp_light)*pm->num_lights );
					for (i=0; i<pm->num_lights; i++ )	{			
						cfread_vector(&pm->lights[i].pos,fp);
						pm->lights[i].type = cfread_int(fp);
						pm->lights[i].value = 0.0f;
					}								
				} else {
					pm->num_lights = 0;
					pm->lights = NULL;
				}

				break;
			}
			
			case ID_SOBJ: {		//Subobject header
				int n;
				char *p, props[MAX_PROP_LEN];
//				float d;

				//mprintf(0,"Got chunk SOBJ, len=%d\n",len);

				n = cfread_int(fp);

				Assert(n < pm->n_models );

#if defined( FREESPACE2_FORMAT )	
				pm->submodel[n].rad = cfread_float(fp);		//radius
#endif

				pm->submodel[n].parent = cfread_int(fp);

//				cfread_vector(&pm->submodel[n].norm,fp);
//				d = cfread_float(fp);				
//				cfread_vector(&pm->submodel[n].pnt,fp);
				cfread_vector(&pm->submodel[n].offset,fp);

//			mprintf(( "Subobj %d, offs = %.1f, %.1f, %.1f\n", n, pm->submodel[n].offset.x, pm->submodel[n].offset.y, pm->submodel[n].offset.z ));
	
#if defined ( FREESPACE1_FORMAT )
				pm->submodel[n].rad = cfread_float(fp);		//radius
#endif

//				pm->submodel[n].tree_offset = cfread_int(fp);	//offset
//				pm->submodel[n].data_offset = cfread_int(fp);	//offset

				cfread_vector(&pm->submodel[n].geometric_center,fp);

				cfread_vector(&pm->submodel[n].min,fp);
				cfread_vector(&pm->submodel[n].max,fp);

				model_calc_bound_box(pm->submodel[n].bounding_box,&pm->submodel[n].min,&pm->submodel[n].max);

				pm->submodel[n].name[0] = '\0';

				cfread_string_len( pm->submodel[n].name, MAX_NAME_LEN, fp);		// get the name
				cfread_string_len(props, MAX_PROP_LEN, fp);			// and the user properites

				pm->submodel[n].movement_type = cfread_int(fp);
				pm->submodel[n].movement_axis = cfread_int(fp);

				// change turret movement type to MOVEMENT_TYPE_ROT_SPECIAL
				if (pm->submodel[n].movement_type == MOVEMENT_TYPE_ROT) {
					if ( strstr(pm->submodel[n].name, "turret") || strstr(pm->submodel[n].name, "gun") || strstr(pm->submodel[n].name, "cannon")) {
						pm->submodel[n].movement_type = MOVEMENT_TYPE_ROT_SPECIAL;
					} else if (strstr(pm->submodel[n].name, "thruster")) {
						// Int3();
						pm->submodel[n].movement_type = MOVEMENT_TYPE_NONE;
						pm->submodel[n].movement_axis = MOVEMENT_AXIS_NONE;
					}
				}

				if ( pm->submodel[n].name[0] == '\0' ) {
					strcpy(pm->submodel[n].name, "unknown object name");
				}

				bool rotating_submodel_has_subsystem = !(pm->submodel[n].movement_type == MOVEMENT_TYPE_ROT);
				if ( ( p = strstr(props, "$special"))!= NULL ) {
					char type[32];

					get_user_prop_value(p+9, type);
					if ( !stricmp(type, "subsystem") ) {	// if we have a subsystem, put it into the list!
						do_new_subsystem( n_subsystems, subsystems, n, pm->submodel[n].rad, &pm->submodel[n].offset, props, pm->submodel[n].name, pm->id );
						rotating_submodel_has_subsystem = true;
					} else if ( !stricmp(type, "no_rotate") ) {
						// mark those submodels which should not rotate - ie, those with no subsystem
						pm->submodel[n].movement_type = MOVEMENT_TYPE_NONE;
						pm->submodel[n].movement_axis = MOVEMENT_AXIS_NONE;
					} else {
						// if submodel rotates (via bspgen), then there is either a subsys or special=no_rotate
						Assert( pm->submodel[n].movement_type != MOVEMENT_TYPE_ROT );
					}
				}

				if ( !rotating_submodel_has_subsystem ) {
					nprintf(("Model", "Model %s: Rotating Submodel without subsystem: %s\n", pm->filename, pm->submodel[n].name));

					// mark those submodels which should not rotate - ie, those with no subsystem
					pm->submodel[n].movement_type = MOVEMENT_TYPE_NONE;
					pm->submodel[n].movement_axis = MOVEMENT_AXIS_NONE;
				}


				pm->submodel[n].angs.p = 0.0f;
				pm->submodel[n].angs.b = 0.0f;
				pm->submodel[n].angs.h = 0.0f;

				{
					int nchunks = cfread_int( fp );		// Throw away nchunks
					if ( nchunks > 0 )	{
						Error( LOCATION, "Model '%s' is chunked.  See John or Adam!\n", pm->filename );
					}
				}
				pm->submodel[n].bsp_data_size = cfread_int(fp);
				if ( pm->submodel[n].bsp_data_size > 0 )	{
					pm->submodel[n].bsp_data = (ubyte *)malloc(pm->submodel[n].bsp_data_size);
					cfread(pm->submodel[n].bsp_data,1,pm->submodel[n].bsp_data_size,fp);
				} else {
					pm->submodel[n].bsp_data = NULL;
				}

				if ( strstr( pm->submodel[n].name, "thruster") )	
					pm->submodel[n].is_thruster=1;
				else
					pm->submodel[n].is_thruster=0;

				if ( strstr( pm->submodel[n].name, "-destroyed") )	
					pm->submodel[n].is_damaged=1;
				else
					pm->submodel[n].is_damaged=0;
					
				//mprintf(( "Submodel %d, name '%s', parent = %d\n", n, pm->submodel[n].name, pm->submodel[n].parent ));
				//key_getch();

		//mprintf(( "Submodel %d, tree offset %d\n", n, pm->submodel[n].tree_offset ));
		//mprintf(( "Submodel %d, data offset %d\n", n, pm->submodel[n].data_offset ));
		//key_getch();


				break;

			}

			case ID_SHLD:
				{
					int nverts, ntris;					

					nverts = cfread_int( fp );		// get the number of vertices in the list
					pm->shield.nverts = nverts;
					pm->shield.verts = (shield_vertex *)malloc(nverts * sizeof(shield_vertex) );
					Assert( pm->shield.verts );
					for ( i = 0; i < nverts; i++ )							// read in the vertex list
						cfread_vector( &(pm->shield.verts[i].pos), fp );

					ntris = cfread_int( fp );		// get the number of triangles that compose the shield
					pm->shield.ntris = ntris;
					pm->shield.tris = (shield_tri *)malloc(ntris * sizeof(shield_tri) );
					Assert( pm->shield.tris );
					for ( i = 0; i < ntris; i++ ) {
						cfread_vector( &(pm->shield.tris[i].norm), fp );
						for ( j = 0; j < 3; j++ ) {
							pm->shield.tris[i].verts[j] = cfread_int( fp );		// read in the indices into the shield_vertex list
							/*
#ifndef NDEBUG
							if (pm->shield.tris[i].verts[j] >= nverts)
								if (!warning_displayed) {
									warning_displayed = 1;
									Warning(LOCATION, "Ship %s has a bogus shield mesh.\nOnly %i vertices, index %i found.\n", filename, nverts, pm->shield.tris[i].verts[j]);
								}
#endif
								*/
						}
						for ( j = 0; j < 3; j++ )
							pm->shield.tris[i].neighbors[j] = cfread_int( fp );	// read in the neighbor indices -- indexes into tri list
					}
					break;


					
				}
				break;

			case ID_GPNT:
				pm->n_guns = cfread_int(fp);
				pm->gun_banks = (w_bank *)malloc(sizeof(w_bank) * pm->n_guns);
				Assert( pm->gun_banks != NULL );

				for (i = 0; i < pm->n_guns; i++ ) {
					w_bank *bank = &pm->gun_banks[i];

					bank->num_slots = cfread_int(fp);
					Assert ( bank->num_slots < MAX_SLOTS );
					for (j = 0; j < bank->num_slots; j++) {
						cfread_vector( &(bank->pnt[j]), fp );
						cfread_vector( &(bank->norm[j]), fp );
					}
				}
				break;
			
			case ID_MPNT:
				pm->n_missiles = cfread_int(fp);
				pm->missile_banks = (w_bank *)malloc(sizeof(w_bank) * pm->n_missiles);
				Assert( pm->missile_banks != NULL );

				for (i = 0; i < pm->n_missiles; i++ ) {
					w_bank *bank = &pm->missile_banks[i];

					bank->num_slots = cfread_int(fp);
					Assert ( bank->num_slots < MAX_SLOTS );
					for (j = 0; j < bank->num_slots; j++) {
						cfread_vector( &(bank->pnt[j]), fp );
						cfread_vector( &(bank->norm[j]), fp );
					}
				}
				break;

			case ID_DOCK: {
				char props[MAX_PROP_LEN];

				pm->n_docks = cfread_int(fp);
				pm->docking_bays = (dock_bay *)malloc(sizeof(dock_bay) * pm->n_docks);
				Assert( pm->docking_bays != NULL );

				for (i = 0; i < pm->n_docks; i++ ) {
					char *p;
					dock_bay *bay = &pm->docking_bays[i];

					cfread_string_len( props, MAX_PROP_LEN, fp );
					if ( (p = strstr(props, "$name"))!= NULL )
						get_user_prop_value(p+5, bay->name);
					else
						sprintf(bay->name, "<unnamed bay %c>", 'A' + i);
					bay->num_spline_paths = cfread_int( fp );
					if ( bay->num_spline_paths > 0 ) {
						bay->splines = (int *)malloc(sizeof(int) * bay->num_spline_paths);
						for ( j = 0; j < bay->num_spline_paths; j++ )
							bay->splines[j] = cfread_int(fp);
					} else {
						bay->splines = NULL;
					}

					// determine what this docking bay can be used for
					if ( !strnicmp(bay->name, "cargo", 5) )
						bay->type_flags = DOCK_TYPE_CARGO;
					else
						bay->type_flags = (DOCK_TYPE_REARM | DOCK_TYPE_GENERIC);

					bay->num_slots = cfread_int(fp);
					Assert( bay->num_slots == 2 );					// Get Allender if Asserted!
					for (j = 0; j < bay->num_slots; j++) {
						cfread_vector( &(bay->pnt[j]), fp );
						cfread_vector( &(bay->norm[j]), fp );
					}
				}
				break;
			}

			case ID_FUEL:
				char props[MAX_PROP_LEN];
				pm->n_thrusters = cfread_int(fp);
				pm->thrusters = (thruster_bank *)malloc(sizeof(thruster_bank) * pm->n_thrusters);
				Assert( pm->thrusters != NULL );

				for (i = 0; i < pm->n_thrusters; i++ ) {
					thruster_bank *bank = &pm->thrusters[i];

					bank->num_slots = cfread_int(fp);

					if (pm->version < 2117) {
						bank->wash_info_index = -1;
					} else {
						cfread_string_len( props, MAX_PROP_LEN, fp );
						// look for $engine_subsystem=xxx
						int length = strlen(props);
						if (length > 0) {
							int base_length = strlen("$engine_subsystem=");
							Assert( strstr( (const char *)&props, "$engine_subsystem=") != NULL );
							Assert( length > base_length );
							char *engine_subsys_name = props + base_length;
							if (engine_subsys_name[0] == '$') {
								engine_subsys_name++;
							}

							nprintf(("wash", "Ship %s with engine wash associated with subsys %s\n", filename, engine_subsys_name));

							// set wash_info_index to invalid
							int table_error = 1;
							bank->wash_info_index = -1;
							for (int k=0; k<n_subsystems; k++) {
								if ( 0 == stricmp(subsystems[k].subobj_name, engine_subsys_name) ) {
									bank->wash_info_index = subsystems[k].engine_wash_index;
									if (bank->wash_info_index >= 0) {
										table_error = 0;
									}
									break;
								}
							}

							if ( (bank->wash_info_index == -1) && (n_subsystems > 0) ) {
								if (table_error) {
									Warning(LOCATION, "No engine wash table entry in ships.tbl for ship model %s", filename);
								} else {
									Warning(LOCATION, "Inconsistent model: Engine wash engine subsystem does not match any ship subsytem names for ship model %s", filename);
								}
							}
						} else {
							bank->wash_info_index = -1;
						}
					}

					for (j = 0; j < bank->num_slots; j++) {

						cfread_vector( &(bank->pnt[j]), fp );
						cfread_vector( &(bank->norm[j]), fp );
						if ( pm->version > 2004 )	{
							bank->radius[j] = cfread_float( fp );
							//mprintf(( "Rad = %.2f\n", rad ));
						} else {
							bank->radius[j] = 1.0f;
						}
					}
					//mprintf(( "Num slots = %d\n", bank->num_slots ));

				}
				break;

			case ID_TGUN:
			case ID_TMIS: {
				int n_banks, n_slots, parent;
				model_subsystem *subsystemp;
				int i, j, snum=-1;
	
				n_banks = cfread_int(fp);				// number of turret points
				for ( i = 0; i < n_banks; i++ ) {
					int physical_parent;			// who are we attached to?
					parent = cfread_int( fp );			// get the turret parent of the object

					physical_parent = cfread_int(fp);	// The parent subobj that this is physically attached to

					if ( subsystems ) {
						for ( snum = 0; snum < n_subsystems; snum++ ) {
							subsystemp = &subsystems[snum];

							if ( parent == subsystemp->subobj_num ) {
								cfread_vector( &subsystemp->turret_norm, fp );
								vm_vector_2_matrix(&subsystemp->turret_matrix,&subsystemp->turret_norm,NULL,NULL);

								n_slots = cfread_int( fp );
								subsystemp->turret_gun_sobj = physical_parent;
								Assert(n_slots < MAX_TFP);		// only MAX_TFP firing points per model_subsystem
								for (j = 0; j < n_slots; j++ )	{
									cfread_vector( &subsystemp->turret_firing_point[j], fp );
								}
								Assert( n_slots > 0 );

								subsystemp->turret_num_firing_points = n_slots;

								break;
							}
						}
					}

//turret_gun_sobj

					if ( (n_subsystems == 0) || (snum == n_subsystems) ) {
						vector bogus;

						nprintf(("Warning", "Turret object not found for turret firing point in model %s\n", model_filename));
						cfread_vector( &bogus, fp );
						n_slots = cfread_int( fp );
						for (j = 0; j < n_slots; j++ )
							cfread_vector( &bogus, fp );
					}
				}
				break;
			}

			case ID_SPCL: {
				char name[MAX_NAME_LEN], props[MAX_PROP_LEN], *p;
				int n_specials;
				float radius;
				vector pnt;

				n_specials = cfread_int(fp);		// get the number of special subobjects we have
				for (i = 0; i < n_specials; i++) {

					// get the next free object of the subobject list.  Flag error if no more room

					cfread_string_len(name, MAX_NAME_LEN, fp);			// get the name of this special polygon

					cfread_string_len(props, MAX_PROP_LEN, fp);		// will definately have properties as well!
					cfread_vector( &pnt, fp );
					radius = cfread_float( fp );

					// check if $Split
					p = strstr(name, "$split");
					if (p != NULL) {
						pm->split_plane[pm->num_split_plane] = pnt.z;
						pm->num_split_plane++;
						Assert(pm->num_split_plane <= MAX_SPLIT_PLANE);
					} else if ( ( p = strstr(props, "$special"))!= NULL ) {
						char type[32];

						get_user_prop_value(p+9, type);
						if ( !stricmp(type, "subsystem") )						// if we have a subsystem, put it into the list!
							do_new_subsystem( n_subsystems, subsystems, -1, radius, &pnt, props, &name[1], pm->id );		// skip the first '$' character of the name
					} else if ( strstr(name, "$enginelarge") || strstr(name, "$enginehuge") ){
						do_new_subsystem( n_subsystems, subsystems, -1, radius, &pnt, props, &name[1], pm->id );		// skip the first '$' character of the name
					} else {
						nprintf(("Warning", "Unknown special object type %s while reading model %s\n", name, pm->filename));
					}					
				}
				break;
			}
			
			case ID_TXTR: {		//Texture filename list
				int i,n;
//				char name_buf[128];

				//mprintf(0,"Got chunk TXTR, len=%d\n",len);

				n = cfread_int(fp);
				pm->n_textures = n;
				// Dont overwrite memory!!
				Assert(n <= MAX_MODEL_TEXTURES);
				//mprintf(0,"  num textures = %d\n",n);
				for (i=0; i<n; i++ )	{
					char tmp_name[256];
					cfread_string_len(tmp_name,127,fp);

					if ( strstr(tmp_name, "thruster") || strstr(tmp_name, "invisible") )	{
						// Don't load textures for thruster animations or invisible textures
						pm->textures[i] = -1;
					} else {
						pm->textures[i] = bm_load( tmp_name );
						if (pm->textures[i]<0)	{
							Error( LOCATION, "Couldn't open texture '%s'\nreferenced by model '%s'\n", tmp_name, pm->filename );
						}
					}
					pm->original_textures[i] = pm->textures[i];
					//mprintf(0,"<%s>\n",name_buf);
				}

				break;
			}
			
/*			case ID_IDTA:		//Interpreter data
				//mprintf(0,"Got chunk IDTA, len=%d\n",len);

				pm->model_data = (ubyte *)malloc(len);
				pm->model_data_size = len;
				Assert(pm->model_data != NULL );
			
				cfread(pm->model_data,1,len,fp);
			
				break;
*/

			case ID_INFO:		// don't need to do anything with info stuff

				#ifndef NDEBUG
					pm->debug_info_size = len;
					pm->debug_info = (char *)malloc(pm->debug_info_size+1);
					Assert(pm->debug_info!=NULL);
					memset(pm->debug_info,0,len+1);
					cfread( pm->debug_info, 1, len, fp );
				#endif
				break;

			case ID_GRID:
				break;

			case ID_PATH:
				pm->n_paths = cfread_int( fp );
				pm->paths = (model_path *)malloc(sizeof(model_path)*pm->n_paths);
				Assert( pm->paths != NULL );
					
				for (i=0; i<pm->n_paths; i++ )	{
					cfread_string_len(pm->paths[i].name , MAX_NAME_LEN-1, fp);
					if ( pm->version >= 2002 ) {
						// store the sub_model name number of the parent
						cfread_string_len(pm->paths[i].parent_name , MAX_NAME_LEN-1, fp);
						// get rid of leading '$' char in name
						if ( pm->paths[i].parent_name[0] == '$' ) {
							char tmpbuf[MAX_NAME_LEN];
							strcpy(tmpbuf, pm->paths[i].parent_name+1);
							strcpy(pm->paths[i].parent_name, tmpbuf);
						}
						// store the sub_model index (ie index into pm->submodel) of the parent
						pm->paths[i].parent_submodel = -1;
						for ( j = 0; j < pm->n_models; j++ ) {
							if ( !stricmp( pm->submodel[j].name, pm->paths[i].parent_name) ) {
								pm->paths[i].parent_submodel = j;
							}
						}
					} else {
						pm->paths[i].parent_name[0] = 0;
						pm->paths[i].parent_submodel = -1;
					}
					pm->paths[i].nverts = cfread_int( fp );
					pm->paths[i].verts = (mp_vert *)malloc( sizeof(mp_vert) * pm->paths[i].nverts );
					pm->paths[i].goal = pm->paths[i].nverts - 1;
					pm->paths[i].type = MP_TYPE_UNUSED;
					pm->paths[i].value = 0;
					Assert(pm->paths[i].verts!=NULL);
					for (j=0; j<pm->paths[i].nverts; j++ )	{
						cfread_vector(&pm->paths[i].verts[j].pos,fp );
						pm->paths[i].verts[j].radius = cfread_float( fp );
						
						{					// version 1802 added turret stuff
							int nturrets, k;

							nturrets = cfread_int( fp );
							pm->paths[i].verts[j].nturrets = nturrets;
							pm->paths[i].verts[j].turret_ids = (int *)malloc( sizeof(int) * nturrets );
							for ( k = 0; k < nturrets; k++ )
								pm->paths[i].verts[j].turret_ids[k] = cfread_int( fp );
						} 
						
					}
				}
				break;

			case ID_EYE:					// an eye position(s)
				{
					int num_eyes, i;

					// all eyes points are stored simply as vectors and their normals.
					// 0th element is used as usual player view position.

					num_eyes = cfread_int( fp );
					pm->n_view_positions = num_eyes;
					Assert ( num_eyes < MAX_EYES );
					for (i = 0; i < num_eyes; i++ ) {
						pm->view_positions[i].parent = cfread_int( fp );
						cfread_vector( &pm->view_positions[i].pnt, fp );
						cfread_vector( &pm->view_positions[i].norm, fp );
					}
				}
				break;			

			case ID_INSG:				
				int num_ins, num_verts, num_faces, idx, idx2, idx3;			
				
				// get the # of insignias
				num_ins = cfread_int(fp);
				pm->num_ins = num_ins;
				
				// read in the insignias
				for(idx=0; idx<num_ins; idx++){
					// get the detail level
					pm->ins[idx].detail_level = cfread_int(fp);

					// # of faces
					num_faces = cfread_int(fp);
					pm->ins[idx].num_faces = num_faces;
					Assert(num_faces <= MAX_INS_FACES);

					// # of vertices
					num_verts = cfread_int(fp);
					Assert(num_verts <= MAX_INS_VECS);

					// read in all the vertices
					for(idx2=0; idx2<num_verts; idx2++){
						cfread_vector(&pm->ins[idx].vecs[idx2], fp);
					}

					// read in world offset
					cfread_vector(&pm->ins[idx].offset, fp);

					// read in all the faces
					for(idx2=0; idx2<pm->ins[idx].num_faces; idx2++){						
						// read in 3 vertices
						for(idx3=0; idx3<3; idx3++){
							pm->ins[idx].faces[idx2][idx3] = cfread_int(fp);
							pm->ins[idx].u[idx2][idx3] = cfread_float(fp);
							pm->ins[idx].v[idx2][idx3] = cfread_float(fp);
						}
					}
				}					
				break;

			// autocentering info
			case ID_ACEN:
				cfread_vector(&pm->autocenter, fp);
				pm->flags |= PM_FLAG_AUTOCEN;
				break;

			default:
				mprintf(("Unknown chunk <%c%c%c%c>, len = %d\n",id,id>>8,id>>16,id>>24,len));
				cfseek(fp,len,SEEK_CUR);
				break;

		}
		cfseek(fp,next_chunk,SEEK_SET);

		id = cfread_int(fp);
		len = cfread_int(fp);
		next_chunk = cftell(fp) + len;

	}

#ifndef NDEBUG
	if ( ss_fp) {
		int size;
		
		cfclose(ss_fp);
		ss_fp = cfopen(debug_name, "rb");
		if ( ss_fp )	{
			size = cfilelength(ss_fp);
			cfclose(ss_fp);
			if ( size <= 0 )	{
				_unlink(debug_name);
			}
		}
	}
#endif

	cfclose(fp);
	// mprintf(("Done processing chunks\n"));
	return 1;
}



//returns the number of this model
int model_load(char *filename, int n_subsystems, model_subsystem *subsystems)
{
	int i, num, arc_idx;

	if ( !model_initted )
		model_init();

//	int Model_ram = 0;

#ifndef NDEBUG
	int ram_before = TotalRam;
#endif

	//Assert(strlen(filename) <= 12);

	num = -1;

	for (i=0; i< MAX_POLYGON_MODELS; i++)	{
		if ( Polygon_models[i] )	{
			if (!stricmp(filename, Polygon_models[i]->filename))		{
				// Model already loaded; just return.
				return Polygon_models[i]->id;
			}
		} else if ( num == -1 )	{
			// This is the first empty slot
			num = i;
		}
	}

	// No empty slot
	if ( num == -1 )	{
		Error( LOCATION, "Too many models" );
		return -1;
	}	

	mprintf(( "Loading model '%s'\n", filename ));

	polymodel * pm = (polymodel *)malloc( sizeof(polymodel) );
	
	Polygon_models[num] = pm;
	
	memset(pm, 0, sizeof(polymodel));

	pm->n_paths = 0;
	pm->paths = NULL;

	int org_sig = Model_signature;
	Model_signature+=MAX_POLYGON_MODELS;
	if ( Model_signature < org_sig )	{
		Model_signature = 0;
	}
	Assert( (Model_signature % MAX_POLYGON_MODELS) == 0 );
	pm->id = Model_signature + num;

	if (!read_model_file(pm, filename, n_subsystems, subsystems))	{
		return -1;
	}

//mprintf(( "Loading model '%s'\n", filename ));
//key_getch();

//=============================
// Find the destroyed replacement models

	// Set up the default values
	for (i=0; i<pm->n_models; i++ )	{
		pm->submodel[i].my_replacement = -1;	// assume nothing replaces this
		pm->submodel[i].i_replace = -1;		// assume this doesn't replaces anything
	}

	// Search for models that have destroyed versions
	for (i=0; i<pm->n_models; i++ )	{
		int j;
		char destroyed_name[128];

		strcpy( destroyed_name, pm->submodel[i].name );
		strcat( destroyed_name, "-destroyed" );
		for (j=0; j<pm->n_models; j++ )	{
			if ( !stricmp( pm->submodel[j].name, destroyed_name ))	{
				// mprintf(( "Found destroyed model for '%s'\n", pm->submodel[i].name ));
				pm->submodel[i].my_replacement = j;
				pm->submodel[j].i_replace = i;
			}
		}

		// Search for models with live debris
		// This debris comes from a destroyed subsystem when ship is still alive
		char live_debris_name[128];

		strcpy( live_debris_name, "debris-" );
		strcat( live_debris_name, pm->submodel[i].name );


		pm->submodel[i].num_live_debris = 0;
		for (j=0; j<pm->n_models; j++ ) {
			// check if current model name is substring of destroyed
			if ( strstr( pm->submodel[j].name, live_debris_name ))	{
				mprintf(( "Found live debris model for '%s'\n", pm->submodel[i].name ));
				Assert(pm->submodel[i].num_live_debris < MAX_LIVE_DEBRIS);
				pm->submodel[i].live_debris[pm->submodel[i].num_live_debris++] = j;
				pm->submodel[j].is_live_debris = 1;
			}
		}

	}

	create_family_tree(pm);
//	dump_object_tree(pm);

//==============================
// Find all the lower detail versions of the hires model
	for (i=0; i<pm->n_models; i++ )	{
		int j, l1;
		bsp_info * sm1 = &pm->submodel[i];

		// set all arc types to be default 		
		for(arc_idx=0; arc_idx < MAX_ARC_EFFECTS; arc_idx++){
			sm1->arc_type[arc_idx] = MARC_TYPE_NORMAL;
		}

		sm1->num_details = 0;
		l1 = strlen(sm1->name);

		for (j=0; j<pm->num_debris_objects;j++ )	{
			if ( i == pm->debris_objects[j] )	{
				sm1->is_damaged = 1;
			} 
		}


		for (j=0; j<MAX_MODEL_DETAIL_LEVELS; j++ )	{
			sm1->details[j] = -1;
		}

		for (j=0; j<pm->n_models; j++ )	{
			int k;
			bsp_info * sm2 = &pm->submodel[j];

			if ( i==j ) continue;
			
			// set all arc types to be default 		
			for(arc_idx=0; arc_idx < MAX_ARC_EFFECTS; arc_idx++){
				sm2->arc_type[arc_idx] = MARC_TYPE_NORMAL;
			}

			// if sm2 is a detail of sm1 and sm1 is a high detail, then add it to sm1's list
			if ((int)strlen(sm2->name)!=l1) continue;
	
			int ndiff = 0;
			int first_diff = 0;
			for ( k=0; k<l1; k++)	{
				if (sm1->name[k] != sm2->name[k] )	{
					if (ndiff==0) first_diff = k;
					ndiff++;
				}
			}
			if (ndiff==1)	{		// They only differ by one character!
				int dl1, dl2;
				dl1 = tolower(sm1->name[first_diff]) - 'a';
				dl2 = tolower(sm2->name[first_diff]) - 'a';

				if ( (dl1<0) || (dl2<0) || (dl1>=MAX_MODEL_DETAIL_LEVELS) || (dl2>=MAX_MODEL_DETAIL_LEVELS) ) continue;	// invalid detail levels

				if ( dl1 == 0 )	{
					dl2--;	// Start from 1 up...
					if (dl2 >= sm1->num_details ) sm1->num_details = dl2+1;
					sm1->details[dl2] = j;
					//mprintf(( "Submodel '%s' is detail level %d of '%s'\n", sm2->name, dl2, sm1->name ));
				}
			}
		}

		for (j=0; j<sm1->num_details; j++ )	{
			if ( sm1->details[j] == -1 )	{
				Warning( LOCATION, "Model '%s' could find all detail levels for submodel '%s'", pm->filename, sm1->name );
				sm1->num_details = 0;
			}
		}

	}


	model_octant_create( pm );

	// Find the core_radius... the minimum of 
	float rx, ry, rz;
	rx = fl_abs( pm->submodel[pm->detail[0]].max.x - pm->submodel[pm->detail[0]].min.x );
	ry = fl_abs( pm->submodel[pm->detail[0]].max.y - pm->submodel[pm->detail[0]].min.y );
	rz = fl_abs( pm->submodel[pm->detail[0]].max.z - pm->submodel[pm->detail[0]].min.z );

	pm->core_radius = min( rx, min(ry, rz) ) / 2.0f;

	for (i=0; i<pm->n_view_positions; i++ )	{
		if ( pm->view_positions[i].parent == pm->detail[0] )	{
			float d = vm_vec_mag( &pm->view_positions[i].pnt );

			d += 0.1f;		// Make the eye 1/10th of a meter inside the sphere.

			if ( d > pm->core_radius )	{
				//mprintf(( "Model %s core radius increased from %.1f to %.1f to fit eye\n", pm->filename, pm->core_radius, d ));
				pm->core_radius = d;
			}		
		}
	}

#ifndef NDEBUG
	int ram_after = TotalRam;

	pm->ram_used = ram_after - ram_before;
	Model_ram += pm->ram_used;
	//mprintf(( "Model RAM = %d KB\n", Model_ram ));
#endif

	const int detail_level = pm->n_detail_levels;
	const int detail_index = pm->detail[detail_level];

	createOgreMesh(filename, pm, detail_index, false);

	// Now do the submodels (force each as a submesh as we may need to rotate them
	int lSubmodelIndex = pm->submodel[detail_index].first_child;

	while( lSubmodelIndex > -1 )	
	{
		createOgreMesh(filename, pm, lSubmodelIndex, true);		
		lSubmodelIndex = pm->submodel[lSubmodelIndex].next_sibling;
	}

	return pm->id;
}

// Get "parent" submodel for live debris submodel
int model_get_parent_submodel_for_live_debris( int model_num, int live_debris_model_num )
{
	polymodel *pm = model_get(model_num);

	Assert(pm->submodel[live_debris_model_num].is_live_debris == 1);

	int mn;
	bsp_info *child;

	// Start with the high level of detail hull 
	// Check all its children until we find the submodel to which the live debris belongs
	child = &pm->submodel[pm->detail[0]];
	mn = child->first_child;

	while (mn > 0) {
		child = &pm->submodel[mn];

		if (child->num_live_debris > 0) {
			// check all live debris submodels for the current child
			for (int idx=0; idx<child->num_live_debris; idx++) {
				if (child->live_debris[idx] == live_debris_model_num) {
					return mn;
				}
			}
			// DKA 5/26/99: can multiple live debris subsystems with each ship
			// NO LONGER TRUE Can only be 1 submodel with live debris
			// Error( LOCATION, "Could not find parent submodel for live debris.  Possible model error");
		}

		// get next child
		mn = child->next_sibling;
	}
	Error( LOCATION, "Could not find parent submodel for live debris");
	return -1;
}


float model_get_radius( int modelnum )
{
	polymodel *pm;

	pm = model_get(modelnum);

	return pm->rad;
}

float model_get_core_radius( int modelnum )
{
	polymodel *pm;

	pm = model_get(modelnum);

	return pm->core_radius;
}

float submodel_get_radius( int modelnum, int submodelnum )
{
	polymodel *pm;

	pm = model_get(modelnum);

	return pm->submodel[submodelnum].rad;
}



polymodel * model_get(int model_num)
{
	Assert( model_num > -1 );

	int num = model_num % MAX_POLYGON_MODELS;
	
	Assert( num > -1 );
	Assert( num < MAX_POLYGON_MODELS );
	Assert( Polygon_models[num]->id == model_num );

	return Polygon_models[num];
}


/*
// Finds the 3d bounding box of a model.  If submodel_num is -1,
// then it starts from the root object.   If inc_children is non-zero, 
// then this will recurse and find the bounding box for all children
// also.
void model_find_bound_box_3d(int model_num,int submodel_num, int inc_children, matrix *orient, vector * pos, vector * box )
{
	polymodel * pm;
	vector to_root_xlat;
	matrix to_root_rotate;
	int n_steps, steps[16];
	int tmp_sobj;
	
	if ( (model_num < 0) || (model_num >= N_polygon_models) ) return;

	pm = &Polygon_models[model_num];

	if ( submodel_num < 0 ) submodel_num = pm->detail[0];

	// traverse up the model tree to a root object.
	// Store this path in n_steps,
	n_steps = 0;
	tmp_sobj = submodel_num;
	while( tmp_sobj > -1 )	{
		steps[n_steps++] = tmp_sobj;
		tmp_sobj = pm->submodel[tmp_sobj].parent;
	}
	
	

//	vm_copy_transpose_matrix(&to_world_rotate, orient );
//	to_world_xlat = *pos;

}
*/



// Returns zero is x1,y1,x2,y2 are valid
// returns 1 for invalid model, 2 for point offscreen.
// note that x1,y1,x2,y2 aren't clipped to 2d screen coordinates!
int model_find_2d_bound_min(int model_num,matrix *orient, vector * pos,int *x1, int *y1, int *x2, int *y2 )
{
	polymodel * po;
	int n_valid_pts;
	int i, x,y,min_x, min_y, max_x, max_y;
	int rval = 0;

	po = model_get(model_num);

	g3_start_instance_matrix(pos,orient);
	
	n_valid_pts = 0;

	int hull = po->detail[0];

	min_x = min_y = max_x = max_y = 0;

	for (i=0; i<8; i++ )	{
		vertex pt;
		ubyte flags;

		flags = g3_rotate_vertex(&pt,&po->submodel[hull].bounding_box[i]);
		if ( !(flags&CC_BEHIND) ) {
			g3_project_vertex(&pt);

			if (!(pt.flags & PF_OVERFLOW)) {
				x = fl2i(pt.sx);
				y = fl2i(pt.sy);
				if ( n_valid_pts == 0 )	{
					min_x = x;
					min_y = y;
					max_x = x;
					max_y = y;
				} else {
					if ( x < min_x ) min_x = x;
					if ( y < min_y ) min_y = y;

					if ( x > max_x ) max_x = x;
					if ( y > max_y ) max_y = y;
				}
				n_valid_pts++;
			}
		}
	}

	if ( n_valid_pts < 8 )	{
		rval = 2;
	}

	if (x1) *x1 = min_x;
	if (y1) *y1 = min_y;

	if (x2) *x2 = max_x;
	if (y2) *y2 = max_y;

	g3_done_instance();

	return rval;
}


// Returns zero is x1,y1,x2,y2 are valid
// returns 1 for invalid model, 2 for point offscreen.
// note that x1,y1,x2,y2 aren't clipped to 2d screen coordinates!
int submodel_find_2d_bound_min(int model_num,int submodel, matrix *orient, vector * pos,int *x1, int *y1, int *x2, int *y2 )
{
	polymodel * po;
	int n_valid_pts;
	int i, x,y,min_x, min_y, max_x, max_y;
	bsp_info * sm;

	po = model_get(model_num);
	if ( (submodel < 0) || (submodel >= po->n_models ) ) return 1;
	sm = &po->submodel[submodel];
	
	g3_start_instance_matrix(pos,orient);
	
	n_valid_pts = 0;

	min_x = min_y = max_x = max_y = 0;

	for (i=0; i<8; i++ )	{
		vertex pt;
		ubyte flags;

		flags = g3_rotate_vertex(&pt,&sm->bounding_box[i]);
		if ( !(flags&CC_BEHIND) ) {
			g3_project_vertex(&pt);

			if (!(pt.flags & PF_OVERFLOW)) {
				x = fl2i(pt.sx);
				y = fl2i(pt.sy);
				if ( n_valid_pts == 0 )	{
					min_x = x;
					min_y = y;
					max_x = x;
					max_y = y;
				} else {
					if ( x < min_x ) min_x = x;
					if ( y < min_y ) min_y = y;

					if ( x > max_x ) max_x = x;
					if ( y > max_y ) max_y = y;
				}
				n_valid_pts++;
			}
		}
	}

	if ( n_valid_pts == 0 )	{
		return 2;
	}

	if (x1) *x1 = min_x;
	if (y1) *y1 = min_y;

	if (x2) *x2 = max_x;
	if (y2) *y2 = max_y;

	g3_done_instance();

	return 0;
}


// Returns zero is x1,y1,x2,y2 are valid
// returns 1 for invalid model, 2 for point offscreen.
// note that x1,y1,x2,y2 aren't clipped to 2d screen coordinates!
int model_find_2d_bound(int model_num,matrix *orient, vector * pos,int *x1, int *y1, int *x2, int *y2 )
{
	float t,w,h;
	vertex pnt;
	ubyte flags;
	polymodel * po;

	po = model_get(model_num);
	float width = po->rad;
	float height = po->rad;

	flags = g3_rotate_vertex(&pnt,pos);

	if ( pnt.flags & CC_BEHIND ) 
		return 2;

	if (!(pnt.flags&PF_PROJECTED))
		g3_project_vertex(&pnt);

	if (pnt.flags & PF_OVERFLOW)
		return 2;

	t = (width * Canv_w2)/pnt.z;
	w = t*Matrix_scale.x;

	t = (height*Canv_h2)/pnt.z;
	h = t*Matrix_scale.y;

	if (x1) *x1 = fl2i(pnt.sx - w);
	if (y1) *y1 = fl2i(pnt.sy - h);

	if (x2) *x2 = fl2i(pnt.sx + w);
	if (y2) *y2 = fl2i(pnt.sy + h);

	return 0;
}

// Returns zero is x1,y1,x2,y2 are valid
// returns 2 for point offscreen.
// note that x1,y1,x2,y2 aren't clipped to 2d screen coordinates!
int subobj_find_2d_bound(float radius ,matrix *orient, vector * pos,int *x1, int *y1, int *x2, int *y2 )
{
	float t,w,h;
	vertex pnt;
	ubyte flags;

	float width = radius;
	float height = radius;

	flags = g3_rotate_vertex(&pnt,pos);

	if ( pnt.flags & CC_BEHIND ) 
		return 2;

	if (!(pnt.flags&PF_PROJECTED))
		g3_project_vertex(&pnt);

	if (pnt.flags & PF_OVERFLOW)
		return 2;

	t = (width * Canv_w2)/pnt.z;
	w = t*Matrix_scale.x;

	t = (height*Canv_h2)/pnt.z;
	h = t*Matrix_scale.y;

	if (x1) *x1 = fl2i(pnt.sx - w);
	if (y1) *y1 = fl2i(pnt.sy - h);

	if (x2) *x2 = fl2i(pnt.sx + w);
	if (y2) *y2 = fl2i(pnt.sy + h);

	return 0;
}


// Given a vector that is in sub_model_num's frame of
// reference, and given the object's orient and position,
// return the vector in the model's frame of reference.
void model_find_obj_dir(vector *w_vec, vector *m_vec, object *ship_obj, int sub_model_num)
{
	vector tvec, vec;
	matrix m;
	int mn;

	Assert(ship_obj->type == OBJ_SHIP);

	polymodel *pm = model_get(Ships[ship_obj->instance].modelnum);
	vec = *m_vec;
	mn = sub_model_num;

	// instance up the tree for this point
	while ((mn>-1) && (pm->submodel[mn].parent > -1)) {
		
		vm_angles_2_matrix(&m, &pm->submodel[mn].angs);
		vm_vec_unrotate(&tvec, &vec, &m);
		vec = tvec;

		mn = pm->submodel[mn].parent;
	}

	// now instance for the entire object
	vm_vec_unrotate(w_vec, &vec, &ship_obj->orient);
}


// Given a point (pnt) that is in sub_model_num's frame of
// reference, return the point in in the object's frame of reference
void model_rot_sub_into_obj(vector * outpnt, vector *mpnt,polymodel *pm, int sub_model_num)
{
	vector pnt;
	vector tpnt;
	matrix m;
	int mn;

	pnt = *mpnt;
	mn = sub_model_num;

	//instance up the tree for this point
	while ((mn>-1) && (pm->submodel[mn].parent > -1) ) {
		
		vm_angles_2_matrix(&m,&pm->submodel[mn].angs );
		vm_transpose_matrix(&m);
		vm_vec_rotate(&tpnt,&pnt,&m);

		vm_vec_add(&pnt,&tpnt,&pm->submodel[mn].offset );

		mn = pm->submodel[mn].parent;
	}

	//now instance for the entire object
	*outpnt = pnt;
}


// Given a rotating submodel, find the ship and world axes or rotatation.
void model_get_rotating_submodel_axis(vector *model_axis, vector *world_axis, int modelnum, int submodel_num, object *obj)
{
	polymodel *pm = model_get(modelnum);

	bsp_info *sm = &pm->submodel[submodel_num];
	Assert(sm->movement_type == MOVEMENT_TYPE_ROT);

	if (sm->movement_axis == MOVEMENT_AXIS_X) {
		vm_vec_make(model_axis, 1.0f, 0.0f, 0.0f);
	} else if (sm->movement_axis == MOVEMENT_AXIS_Y) {
		vm_vec_make(model_axis, 0.0f, 1.0f, 0.0f);
	} else {
		Assert(sm->movement_axis == MOVEMENT_AXIS_Z);
		vm_vec_make(model_axis, 0.0f, 0.0f, 1.0f);
	}

	model_find_obj_dir(world_axis, model_axis, obj, submodel_num);
}


// Does stepped rotation of a submodel
#pragma warning ( push )
#pragma warning (disable : 4701)
void submodel_stepped_rotate(model_subsystem *psub, submodel_instance_info *sii)
{
	Assert(psub->flags & MSS_FLAG_STEPPED_ROTATE);

	if ( psub->subobj_num < 0 ) return;

	polymodel *pm = model_get(psub->model_num);
	bsp_info *sm = &pm->submodel[psub->subobj_num];

	if ( sm->movement_type != MOVEMENT_TYPE_ROT ) return;

	// get active rotation time this frame
	int end_stamp = timestamp();
	float rotation_time = 0.001f * (end_stamp - sii->step_zero_timestamp);
	Assert(rotation_time >= 0);

	// save last angles
	sii->prev_angs = sii->angs;

	// float pointer into struct to get angle (either p,b,h)
	float *ang_prev, *ang_next = 0;
	switch( sm->movement_axis ) {
	case MOVEMENT_AXIS_X:
		ang_prev = &sii->prev_angs.p;
		ang_next = &sii->angs.p;
		break;

	case MOVEMENT_AXIS_Y:	
		ang_prev = &sii->prev_angs.h;
		ang_next = &sii->angs.h;
		break;

	case MOVEMENT_AXIS_Z:	
		ang_prev = &sii->prev_angs.b;
		ang_next = &sii->angs.b;
		break;
	}
	
	// angular displacement of one step
	float step_size = (PI2 / psub->stepped_rotation->num_steps);

	// get time to complete one step, including pause
	float step_time = psub->stepped_rotation->t_transit + psub->stepped_rotation->t_pause;

	// cur_step is step number relative to zero (0 - num_steps)
	// step_offset_time is TIME into current step
	float step_offset_time = (float)fmod(rotation_time, step_time);
	// subtract off fractional step part, round up  (ie, 1.999999 -> 2)
	int cur_step = int( ((rotation_time - step_offset_time) / step_time) + 0.5f);
	// mprintf(("cur step %d\n", cur_step));
	// Assert(step_offset_time >= 0);

	if (cur_step >= psub->stepped_rotation->num_steps) {
		// I don;t know why, but removing this line makes it all good.
		// sii->step_zero_timestamp += int(1000.0f * (psub->stepped_rotation->num_steps * step_time) + 0.5f);

		// reset cur_step (use mod to handle physics/ai pause)
		cur_step = cur_step % psub->stepped_rotation->num_steps;
	}

	// get base angle
	*ang_next = cur_step * step_size;

	// determine which phase of rotation we're in
	float coast_start_time = psub->stepped_rotation->fraction * psub->stepped_rotation->t_transit;
	float decel_start_time = psub->stepped_rotation->t_transit * (1.0f - psub->stepped_rotation->fraction);
	float pause_start_time = psub->stepped_rotation->t_transit;

	float start_coast_angle = 0.5f * psub->stepped_rotation->max_turn_accel * coast_start_time * coast_start_time;

	if (step_offset_time < coast_start_time) {
		// do accel
		float accel_time = step_offset_time;
		*ang_next += 0.5f * psub->stepped_rotation->max_turn_accel * accel_time * accel_time;
		sii->cur_turn_rate = psub->stepped_rotation->max_turn_accel * accel_time;
	} else if (step_offset_time < decel_start_time) {
		// do coast
		float coast_time = step_offset_time - coast_start_time;
		*ang_next += start_coast_angle + psub->stepped_rotation->max_turn_rate * coast_time;
		sii->cur_turn_rate = psub->stepped_rotation->max_turn_rate;
	} else if (step_offset_time < pause_start_time) {
		// do decel
		float time_to_pause = psub->stepped_rotation->t_transit - step_offset_time;
		*ang_next += (step_size - 0.5f * psub->stepped_rotation->max_turn_accel * time_to_pause * time_to_pause);
		sii->cur_turn_rate = psub->stepped_rotation->max_turn_rate * time_to_pause;
	} else {
		// do pause
		*ang_next += step_size;
		sii->cur_turn_rate = 0.0f;
	}
}
#pragma warning ( pop )


// Rotates the angle of a submodel.  Use this so the right unlocked axis
// gets stuffed.
void submodel_rotate(model_subsystem *psub, submodel_instance_info *sii)
{
	bsp_info * sm;

	if ( psub->subobj_num < 0 ) return;

	polymodel *pm = model_get(psub->model_num);
	sm = &pm->submodel[psub->subobj_num];

	if ( sm->movement_type != MOVEMENT_TYPE_ROT ) return;

	// save last angles
	sii->prev_angs = sii->angs;

	// probably send in a calculated desired turn rate
	float diff = sii->desired_turn_rate - sii->cur_turn_rate;

	float final_turn_rate;
	if (diff > 0) {
		final_turn_rate = sii->cur_turn_rate + sii->turn_accel * flFrametime;
		if (final_turn_rate > sii->desired_turn_rate) {
			final_turn_rate = sii->desired_turn_rate;
		}
	} else if (diff < 0) {
		final_turn_rate = sii->cur_turn_rate - sii->turn_accel * flFrametime;
		if (final_turn_rate < sii->desired_turn_rate) {
			final_turn_rate = sii->desired_turn_rate;
		}
	} else {
		final_turn_rate = sii->desired_turn_rate;
	}

	float delta = (sii->cur_turn_rate + final_turn_rate) * 0.5f * flFrametime;
	sii->cur_turn_rate = final_turn_rate;


	//float delta = psub->turn_rate * flFrametime;

	switch( sm->movement_axis )	{
	case MOVEMENT_AXIS_X:	
		sii->angs.p += delta;
		if (sii->angs.p > PI2 )
			sii->angs.p -= PI2;
		else if (sii->angs.p < 0.0f )
			sii->angs.p += PI2;
		break;
	case MOVEMENT_AXIS_Y:	
		sii->angs.h += delta;
		if (sii->angs.h > PI2 )
			sii->angs.h -= PI2;
		else if (sii->angs.h < 0.0f )
			sii->angs.h += PI2;
		break;
	case MOVEMENT_AXIS_Z:	
		sii->angs.b += delta;
		if (sii->angs.b > PI2 )
			sii->angs.b -= PI2;
		else if (sii->angs.b < 0.0f )
			sii->angs.b += PI2;
		break;
	}
}


//=========================================================================
// Make a turret's correct orientation matrix.   This should be done when 
// the model is read, but I wasn't sure at what point all the data that I
// needed was read, so I just check a flag and call this routine when
// I determine I need the correct matrix.   In this code, you can't use
// vm_vec_2_matrix or anything, since these turrets could be either 
// right handed or left handed.
void model_make_turrent_matrix(int model_num, model_subsystem * turret )
{
	polymodel * pm;
	vector fvec, uvec, rvec;

	pm = model_get(model_num);
	bsp_info * sm = &pm->submodel[turret->turret_gun_sobj];
	bsp_info * sm_parent = &pm->submodel[turret->subobj_num];


	model_clear_instance(model_num);
	model_find_world_dir(&fvec, &turret->turret_norm, model_num, turret->turret_gun_sobj, &vmd_identity_matrix, NULL );

	sm_parent->angs.h = -PI/2.0f;
	sm->angs.p = -PI/2.0f;
	model_find_world_dir(&rvec, &turret->turret_norm, model_num, turret->turret_gun_sobj, &vmd_identity_matrix, NULL );

	sm_parent->angs.h = 0.0f;
	sm->angs.p = -PI/2.0f;
	model_find_world_dir(&uvec, &turret->turret_norm, model_num, turret->turret_gun_sobj, &vmd_identity_matrix, NULL );
									
	vm_vec_normalize(&fvec);
	vm_vec_normalize(&rvec);
	vm_vec_normalize(&uvec);

	turret->turret_matrix.fvec = fvec;
	turret->turret_matrix.rvec = rvec;
	turret->turret_matrix.uvec = uvec;

//	vm_vector_2_matrix(&turret->turret_matrix,&turret->turret_norm,NULL,NULL);

	// HACK!! WARNING!!!
	// I'm doing nothing to verify that this matrix is orthogonal!!
	// In other words, there's no guarantee that the vectors are 90 degrees
	// from each other.
	// I'm not doing this because I don't know how to do it without ruining
	// the handedness of the matrix... however, I'm not too worried about	
	// this because I am creating these 3 vectors by making them 90 degrees
	// apart, so this should be close enough.  I think this will start 
	// causing weird errors when we view from turrets. -John
	turret->flags |= MSS_FLAG_TURRET_MATRIX;
}

// Tries to move joints so that the turrent points to the point dst.
// turret1 is the angles of the turret, turret2 is the angles of the gun from turret
//	Returns 1 if rotated gun, 0 if no gun to rotate (rotation handled by AI)
int model_rotate_gun(int model_num, model_subsystem *turret, matrix *orient, angles *turret1, angles *turret2, vector *pos, vector *dst)
{
	polymodel * pm;

	pm = model_get(model_num);
	bsp_info * sm = &pm->submodel[turret->turret_gun_sobj];
	bsp_info * sm_parent = &pm->submodel[turret->subobj_num];

	// Check for a valid turret
	Assert( turret->turret_num_firing_points > 0 );


	if ( sm_parent == sm ) {
		return 0;
	}

	// Build the correct turret matrix if there isn't already one
	if ( !(turret->flags & MSS_FLAG_TURRET_MATRIX) )
		model_make_turrent_matrix(model_num, turret );

	Assert( turret->flags & MSS_FLAG_TURRET_MATRIX);
//	Assert( sm->movement_axis == MOVEMENT_AXIS_X );				// Gun must be able to change pitch
//	Assert( sm_parent->movement_axis == MOVEMENT_AXIS_Z );	// Parent must be able to change heading

//======================================================
// DEBUG code to draw the normal out of this gun and a circle
// at the gun point.
#if 0
	{
		vector tmp;
		vector tmp1;
		vertex dpnt1, dpnt2;

		model_clear_instance(model_num);
		sm->angs.p = turret2->p;
		sm_parent->angs.h = turret1->h;

		model_find_world_point(&tmp, &vmd_zero_vector, model_num, turret->turret_gun_sobj, orient, pos );
		gr_set_color(255,0,0);
		g3_rotate_vertex( &dpnt1, &tmp );

		gr_set_color(255,0,0);
		g3_draw_sphere(&dpnt1,1.0f);

		vm_vec_copy_scale( &tmp1, &turret->turret_matrix.fvec, 10.0f );
		model_find_world_point(&tmp, &tmp1, model_num, turret->turret_gun_sobj, orient, pos );
		g3_rotate_vertex( &dpnt2, &tmp );

		gr_set_color(0,255,0);
		g3_draw_line(&dpnt1,&dpnt2);
		gr_set_color(0,128,0);
		g3_draw_sphere(&dpnt2,0.2f);

		vm_vec_copy_scale( &tmp1, &turret->turret_matrix.rvec, 10.0f );
		model_find_world_point(&tmp, &tmp1, model_num, turret->turret_gun_sobj, orient, pos );
		g3_rotate_vertex( &dpnt2, &tmp );

		gr_set_color(0,0,255);
		g3_draw_line(&dpnt1,&dpnt2);

		vm_vec_copy_scale( &tmp1, &turret->turret_matrix.uvec, 10.0f );
		model_find_world_point(&tmp, &tmp1, model_num, turret->turret_gun_sobj, orient, pos );
		g3_rotate_vertex( &dpnt2, &tmp );

		gr_set_color(255,0,0);
		g3_draw_line(&dpnt1,&dpnt2);
	}
#endif

	//------------	
	// rotate the dest point into the turret gun normal's frame of
	// reference, but not using the turret's angles.
	// Call this vector of_dst
	vector of_dst;							
	matrix world_to_turret_matrix;		// converts world coordinates to turret's FOR
	vector world_to_turret_translate;	// converts world coordinates to turret's FOR
	vector tempv;

	vm_vec_unrotate( &tempv, &sm_parent->offset, orient);
	vm_vec_add( &world_to_turret_translate, pos, &tempv );

	vm_matrix_x_matrix( &world_to_turret_matrix, orient, &turret->turret_matrix );

	vm_vec_sub( &tempv, dst, &world_to_turret_translate );
	vm_vec_rotate( &of_dst, &tempv, &world_to_turret_matrix );

	vm_vec_normalize(&of_dst);	

	//------------	
	// Find the heading and pitch that the gun needs to turn to
	// by extracting them from the of_dst vector.
	// Call this the desired_angles
	angles desired_angles;

	desired_angles.p = (float)acos(of_dst.z);
	desired_angles.h = PI - atan2_safe(of_dst.x, of_dst.y);
	desired_angles.b = 0.0f;

	//	mprintf(( "Z = %.1f, atan= %.1f\n", of_dst.z, desired_angles.p ));

	//------------	
	// Gradually turn the turret towards the desired angles
	float step_size = turret->turret_turning_rate * flFrametime;

	vm_interp_angle(&turret1->h,desired_angles.h,step_size);
	vm_interp_angle(&turret2->p,desired_angles.p,step_size);

//	turret1->h -= step_size*(key_down_timef(KEY_1)-key_down_timef(KEY_2) );
//	turret2->p += step_size*(key_down_timef(KEY_3)-key_down_timef(KEY_4) );

	return 1;

}


// Given a point (pnt) that is in sub_model_num's frame of
// reference, and given the object's orient and position, 
// return the point in 3-space in outpnt.
void model_find_world_point(vector * outpnt, vector *mpnt,int model_num,int sub_model_num, matrix * objorient, vector * objpos )
{
	vector pnt;
	vector tpnt;
	matrix m;
	int mn;
	polymodel *pm = model_get(model_num);

	pnt = *mpnt;
	mn = sub_model_num;

	//instance up the tree for this point
	while ((mn>-1) && (pm->submodel[mn].parent > -1) ) {
		
		vm_angles_2_matrix(&m,&pm->submodel[mn].angs );
		vm_vec_unrotate(&tpnt,&pnt,&m);

		vm_vec_add(&pnt,&tpnt,&pm->submodel[mn].offset );

		mn = pm->submodel[mn].parent;
	}

	//now instance for the entire object
	vm_vec_unrotate(outpnt,&pnt,objorient);
	vm_vec_add2(outpnt,objpos);
}

// Given a point in the world RF, find the corresponding point in the model RF.
// This is special purpose code, specific for model collision.
// NOTE - this code ASSUMES submodel is 1 level down from hull (detail[0])
//
// out - point in model RF
// world_pt - point in world RF
// pm - polygon model
// submodel_num - submodel in whose RF we're trying to find the corresponding world point
// orient - orient matrix of ship
// pos - pos vector of ship
void world_find_model_point(vector *out, vector *world_pt, polymodel *pm, int submodel_num, matrix *orient, vector *pos)
{
	Assert( (pm->submodel[submodel_num].parent == pm->detail[0]) || (pm->submodel[submodel_num].parent == -1) );

	vector tempv1, tempv2;
	matrix m;

	// get into ship RF
	vm_vec_sub(&tempv1, world_pt, pos);
	vm_vec_rotate(&tempv2, &tempv1, orient);

	if (pm->submodel[submodel_num].parent == -1) {
		*out  = tempv2;
		return;
	}

	// put into submodel RF
	vm_vec_sub2(&tempv2, &pm->submodel[submodel_num].offset);
	vm_angles_2_matrix(&m, &pm->submodel[submodel_num].angs);
	vm_vec_rotate(out, &tempv2, &m);
}

// Verify rotating submodel has corresponding ship subsystem -- info in which to store rotation angle
int rotating_submodel_has_ship_subsys(int submodel, ship *shipp)
{
	model_subsystem	*psub;
	ship_subsys			*pss;

	int found = 0;

	// Go through all subsystems and look for submodel
	// the subsystems that need it.
	for ( pss = GET_FIRST(&shipp->subsys_list); pss != END_OF_LIST(&shipp->subsys_list); pss = GET_NEXT(pss) ) {
		psub = pss->system_info;
		if (psub->subobj_num == submodel) {
			found = 1;
			break;
		}
	}
	
	return found;
}

void model_get_rotating_submodel_list(int *submodel_list, int *num_rotating_submodels, object *objp)
{
	Assert(objp->type == OBJ_SHIP);

	// Check if not currently rotating - then treat as part of superstructure.
	int modelnum = Ships[objp->instance].modelnum;
	polymodel *pm = model_get(modelnum);
	bsp_info *child_submodel;

	*num_rotating_submodels = 0;
	child_submodel = &pm->submodel[pm->detail[0]];

	int i = child_submodel->first_child;
	while ( i>-1 )	{
		child_submodel = &pm->submodel[i];

		// Don't check it or its children if it is destroyed or it is a replacement (non-moving)
		if ( !child_submodel->blown_off && (child_submodel->i_replace == -1) )	{

			// Only look for submodels that rotate
			if (child_submodel->movement_type == MOVEMENT_TYPE_ROT) {

				// find ship subsys and check submodel rotation is less than max allowed.
				ship *pship = &Ships[objp->instance];
				ship_subsys *subsys;

				for ( subsys = GET_FIRST(&pship->subsys_list); subsys !=END_OF_LIST(&pship->subsys_list); subsys = GET_NEXT(subsys) ) {
					Assert(subsys->system_info->model_num == modelnum);
					if (i == subsys->system_info->subobj_num) {
						// found the correct subsystem - now check delta rotation angle not too large
						float delta_angle = get_submodel_delta_angle(&subsys->submodel_info_1);
						if (delta_angle < MAX_SUBMODEL_COLLISION_ROT_ANGLE) {
							Assert(*num_rotating_submodels < MAX_ROTATING_SUBMODELS-1);
							submodel_list[(*num_rotating_submodels)++] = i;
						}
						break;
					}
				}
			}
		}
		i = child_submodel->next_sibling;
	}

	// error checking
//#define MODEL_CHECK
#ifdef MODEL_CHECK
	ship *pship = &Ships[objp->instance];
	for (int idx=0; idx<*num_rotating_submodels; idx++) {
		int valid = rotating_submodel_has_ship_subsys(submodel_list[idx], pship);
//		Assert( valid );
		if ( !valid ) {

			Warning( LOCATION, "Ship %s has rotating submodel [%s] without ship subsystem\n", pship->ship_name, pm->submodel[submodel_list[idx]].name );
			pm->submodel[submodel_list[idx]].movement_type &= ~MOVEMENT_TYPE_ROT;
			*num_rotating_submodels = 0;
		}
	}
#endif

}


// Given a direction (pnt) that is in sub_model_num's frame of
// reference, and given the object's orient and position, 
// return the point in 3-space in outpnt.
void model_find_world_dir(vector * out_dir, vector *in_dir,int model_num, int sub_model_num, matrix * objorient, vector * objpos )
{
	vector pnt;
	vector tpnt;
	matrix m;
	int mn;
	polymodel *pm = model_get(model_num);

	pnt = *in_dir;
	mn = sub_model_num;

	//instance up the tree for this point
	while ((mn>-1) && (pm->submodel[mn].parent > -1) ) {
		
		vm_angles_2_matrix(&m,&pm->submodel[mn].angs );
		vm_vec_unrotate(&tpnt,&pnt,&m);
		pnt = tpnt;

		mn = pm->submodel[mn].parent;
	}

	//now instance for the entire object
	vm_vec_unrotate(out_dir,&pnt,objorient);
}



// Clears all the submodel instances stored in a model to their defaults.
void model_clear_instance(int model_num)
{
	polymodel * pm;
	int i;

	pm = model_get(model_num);

	// reset textures to original ones
	for (i=0; i<pm->n_textures; i++ )	{
		pm->textures[i] = pm->original_textures[i];
	}
	
	for (i=0; i<pm->n_models; i++ )	{
		bsp_info *sm = &pm->submodel[i];
		
		if ( pm->submodel[i].is_damaged )	{
			sm->blown_off = 1;
		} else {
			sm->blown_off = 0;
		}
		sm->angs.p = 0.0f;
		sm->angs.b = 0.0f;
		sm->angs.h = 0.0f;

		// set pointer to other ship subsystem info [turn rate, accel, moment, axis, ...]
		sm->sii = NULL;

		sm->num_arcs = 0;		// Turn off any electric arcing effects
	}

	for (i=0; i<pm->num_lights; i++ )	{
		pm->lights[i].value = 0.0f;
	}

	interp_clear_instance();

//	if ( keyd_pressed[KEY_1] ) pm->lights[0].value = 1.0f/255.0f;
//	if ( keyd_pressed[KEY_2] ) pm->lights[1].value = 1.0f/255.0f;
//	if ( keyd_pressed[KEY_3] ) pm->lights[2].value = 1.0f/255.0f;
//	if ( keyd_pressed[KEY_4] ) pm->lights[3].value = 1.0f/255.0f;
//	if ( keyd_pressed[KEY_5] ) pm->lights[4].value = 1.0f/255.0f;
//	if ( keyd_pressed[KEY_6] ) pm->lights[5].value = 1.0f/255.0f;


}

// initialization during ship set
void model_clear_instance_info( submodel_instance_info * sii )
{
	sii->blown_off = 0;
	sii->angs.p = 0.0f;
	sii->angs.b = 0.0f;
	sii->angs.h = 0.0f;
	sii->prev_angs.p = 0.0f;
	sii->prev_angs.b = 0.0f;
	sii->prev_angs.h = 0.0f;

	sii->cur_turn_rate = 0.0f;
	sii->desired_turn_rate = 0.0f;
	sii->turn_accel = 0.0f;
}

// initialization during ship set
void model_set_instance_info(submodel_instance_info *sii, float turn_rate, float turn_accel)
{
	sii->blown_off = 0;
	sii->angs.p = 0.0f;
	sii->angs.b = 0.0f;
	sii->angs.h = 0.0f;
	sii->prev_angs.p = 0.0f;
	sii->prev_angs.b = 0.0f;
	sii->prev_angs.h = 0.0f;

	sii->cur_turn_rate = turn_rate * 0.0f;
	sii->desired_turn_rate = turn_rate;
	sii->turn_accel = turn_accel;
	sii->axis_set = 0;
	sii->step_zero_timestamp = timestamp();
}



// Sets the submodel instance data in a submodel (for all detail levels)
void model_set_instance(int model_num, int sub_model_num, submodel_instance_info * sii)
{
	int i;
	polymodel * pm;

	pm = model_get(model_num);

	Assert( sub_model_num >= 0 );
	Assert( sub_model_num < pm->n_models );

	if ( sub_model_num < 0 ) return;
	if ( sub_model_num >= pm->n_models ) return;
	bsp_info *sm = &pm->submodel[sub_model_num];

	// Set the "blown out" flags	
	sm->blown_off = sii->blown_off;

	if ( sm->blown_off )	{
		if ( sm->my_replacement > -1 )	{
			pm->submodel[sm->my_replacement].blown_off = 0;
			pm->submodel[sm->my_replacement].angs = sii->angs;
			pm->submodel[sm->my_replacement].sii = sii;
		}
	} else {
		if ( sm->my_replacement > -1 )	{
			pm->submodel[sm->my_replacement].blown_off = 1;
		}
	}

	// Set the angles
	sm->angs = sii->angs;
	sm->sii = sii;

	// For all the detail levels of this submodel, set them also.
	for (i=0; i<sm->num_details; i++ )	{
		model_set_instance(model_num, sm->details[i], sii );
	}
}


// Finds a point on the rotation axis of a submodel, used in collision, generally find rotational velocity
void model_init_submodel_axis_pt(submodel_instance_info *sii, int model_num, int submodel_num)
{
	vector axis;
	vector *mpoint1, *mpoint2;
	vector p1, v1, p2, v2, int1;

	polymodel *pm = model_get(model_num);
	Assert(pm->submodel[submodel_num].movement_type == MOVEMENT_TYPE_ROT);
	Assert(sii);

	mpoint1 = NULL;
	mpoint2 = NULL;

	// find 2 fixed points in submodel RF
	// these will be rotated to about the axis an angle of 0 and PI and we'll find the intersection of the
	// two lines to find a point on the axis
	if (pm->submodel[submodel_num].movement_axis == MOVEMENT_AXIS_X) {
		axis = vmd_x_vector;
		mpoint1 = &vmd_y_vector;
		mpoint2 = &vmd_z_vector;
	} else if (pm->submodel[submodel_num].movement_axis == MOVEMENT_AXIS_Y) {
		mpoint1 = &vmd_x_vector;
		axis = vmd_z_vector;		// rotation about y is a change in heading (p,b,h), so we need z
		mpoint2 = &vmd_z_vector;
	} else if (pm->submodel[submodel_num].movement_axis == MOVEMENT_AXIS_Z) {
		mpoint1 = &vmd_x_vector;
		mpoint2 = &vmd_y_vector;
		axis = vmd_y_vector;		// rotation about z is a change in bank (p,b,h), so we need y
	} else {
		// must be one of these axes or submodel_rot_hit is incorrectly set
		Int3();
	}

	// copy submodel angs
	angles copy_angs = pm->submodel[submodel_num].angs;

	// find two points rotated into model RF when angs set to 0
	vm_vec_copy_scale((vector*)&pm->submodel[submodel_num].angs, &axis, 0.0f);
	model_find_world_point(&p1, mpoint1, model_num, submodel_num, &vmd_identity_matrix, &vmd_zero_vector);
	model_find_world_point(&p2, mpoint2, model_num, submodel_num, &vmd_identity_matrix, &vmd_zero_vector);

	// find two points rotated into model RF when angs set to PI
	vm_vec_copy_scale((vector*)&pm->submodel[submodel_num].angs, &axis, PI);
	model_find_world_point(&v1, mpoint1, model_num, submodel_num, &vmd_identity_matrix, &vmd_zero_vector);
	model_find_world_point(&v2, mpoint2, model_num, submodel_num, &vmd_identity_matrix, &vmd_zero_vector);

	// reset submodel angs
	pm->submodel[submodel_num].angs = copy_angs;

	// find direction vectors of the two lines
	vm_vec_sub2(&v1, &p1);
	vm_vec_sub2(&v2, &p2);

	// find the intersection of the two lines
	float s, t;
	fvi_two_lines_in_3space(&p1, &v1, &p2, &v2, &s, &t);

	// find the actual intersection points
	vm_vec_scale_add(&int1, &p1, &v1, s);

	// set flag to init
	sii->pt_on_axis = int1;
	sii->axis_set = 1;
}


// Adds an electrical arcing effect to a submodel
void model_add_arc(int model_num, int sub_model_num, vector *v1, vector *v2, int arc_type )
{
	polymodel * pm;

	pm = model_get(model_num);

	if ( sub_model_num == -1 )	{
		sub_model_num = pm->detail[0];
	}

	Assert( sub_model_num >= 0 );
	Assert( sub_model_num < pm->n_models );

	if ( sub_model_num < 0 ) return;
	if ( sub_model_num >= pm->n_models ) return;
	bsp_info *sm = &pm->submodel[sub_model_num];

	if ( sm->num_arcs < MAX_ARC_EFFECTS )	{
		sm->arc_type[sm->num_arcs] = (ubyte)arc_type;
		sm->arc_pts[sm->num_arcs][0] = *v1;
		sm->arc_pts[sm->num_arcs][1] = *v2;
		sm->num_arcs++;
	}
}

// function to return an index into the docking_bays array which matches the criteria passed
// to this function.  dock_type is one of the DOCK_TYPE_XXX defines in model.h
int model_find_dock_index(int modelnum, int dock_type)
{
	int i;

	polymodel *pm;

	pm = model_get(modelnum);

	// no docking points -- return -1
	if ( pm->n_docks <= 0 )
		return -1;

	for (i = 0; i < pm->n_docks; i++ ) {
		if ( dock_type & pm->docking_bays[i].type_flags )
			return i;
	}

	return -1;
}

int model_get_dock_index_type(int modelnum, int index)
{
	polymodel *pm = model_get(modelnum);				

	return pm->docking_bays[index].type_flags;
}

// get all the different docking point types on a model
int model_get_dock_types(int modelnum)
{
	int i, type = 0;
	polymodel *pm;

	pm = model_get(modelnum);
	for (i=0; i<pm->n_docks; i++)
		type |= pm->docking_bays[i].type_flags;

	return type;
}

// function to return an index into the docking_bays array which matches the string passed
// Fred uses strings to identify docking positions.  This functin also accepts generic strings
// so that a desginer doesn't have to know exact names if building a mission from hand.
int model_find_dock_name_index( int modelnum, char *name )
{
	int i;
	polymodel *pm;

	pm = model_get(modelnum);
	if ( pm->n_docks <= 0 )
		return -1;

	// check the generic names and call previous function to find first dock point of
	// the specified type
	if ( !stricmp(name, "cargo") )
		return model_find_dock_index( modelnum, DOCK_TYPE_CARGO );
	else if (!stricmp( name, "rearm") )
		return model_find_dock_index( modelnum, DOCK_TYPE_REARM );
	else if (!stricmp( name, "generic") )
		return model_find_dock_index( modelnum, DOCK_TYPE_GENERIC );

	for (i = 0; i < pm->n_docks; i++ ) {
		if ( !stricmp(pm->docking_bays[i].name, name) )
			return i;
	}

	// if we get here, name wasn't found -- return -1 and hope for the best
	return -1;
}

// returns the actual name of a docking point on a model, needed by Fred.
char *model_get_dock_name(int modelnum, int index)
{
	polymodel *pm;

	pm = model_get(modelnum);
	Assert((index >= 0) && (index < pm->n_docks));
	return pm->docking_bays[index].name;
}

int model_get_num_dock_points(int modelnum)
{
	polymodel *pm;

	pm = model_get(modelnum);
	return pm->n_docks;
}
