/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __FREESPACE_KEYCONTROL_H__
#define __FREESPACE_KEYCONTROL_H__

#ifndef UNITY_BUILD
#include "ControlsConfig.h"
#endif


#define WITHIN_BBOX()	do { \
	float scale = 2.0f; \
	polymodel *pm = model_get(s_check->modelnum); \
	collided = 0; \
	if(pm != NULL){ \
		vector temp = new_obj->pos; \
		vector gpos; \
		vm_vec_sub2(&temp, &hit_check->pos); \
		vm_vec_rotate(&gpos, &temp, &hit_check->orient); \
		if((gpos.x >= pm->mins.x * scale) && (gpos.y >= pm->mins.y * scale) && (gpos.z >= pm->mins.z * scale) && (gpos.x <= pm->maxs.x * scale) && (gpos.y <= pm->maxs.y * scale) && (gpos.z <= pm->maxs.z * scale)) { \
			collided = 1; \
		} \
	} \
} while(0)

#define MOVE_AWAY_BBOX() do { \
	polymodel *pm = model_get(s_check->modelnum); \
	if(pm != NULL){ \
		switch((int)frand_range(0.0f, 3.9f)){ \
		case 0: \
			new_obj->pos.x += 200.0f; \
			break; \
		case 1: \
			new_obj->pos.x -= 200.0f; \
			break; \
		case 2: \
			new_obj->pos.y += 200.0f; \
			break; \
		case 3: \
			new_obj->pos.y -= 200.0f; \
			break; \
		default : \
			new_obj->pos.z -= 200.0f; \
			break; \
		} \
	} \
} while(0)

// Holds the bit arrays that indicate which action is to be executed.
#define NUM_BUTTON_FIELDS	((CCFG_MAX + 31) / 32)

extern int Dead_key_set[];
extern int Dead_key_set_size;

typedef struct button_info
{
	int status[NUM_BUTTON_FIELDS];
} button_info;

void button_info_set(button_info *bi, int n);
void button_info_unset(button_info *bi, int n);
int button_info_query(button_info *bi, int n);
void button_info_do(button_info *bi);
void button_info_clear(button_info *bi);
void process_set_of_keys(int key, int count, int *list);
void game_process_pause_key();
void button_strip_noncritical_keys(button_info *bi);


#endif