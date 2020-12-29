/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef __ANIMPLAY_H__
#define __ANIMPLAY_H__

class anim;
struct anim_info;
struct anim_instance;
struct CFILE;
class vector;

// structure passed in when playing an anim.  Talk about overkill..
typedef struct {
	anim *anim_info;
	int x;
	int y;
	int start_at;
	int stop_at;
	int screen_id;
	vector *world_pos;
	float radius;
	int framerate_independent;
	void *color;
	int skip_frames;
	int looped;
	int ping_pong;
} anim_play_struct;

extern int Anim_paused;

void				anim_init();
void				anim_level_init();
void				anim_level_close();
void				anim_render_all(int screen_id, float frametime);
void				anim_render_one(int screen_id, anim_instance *ani, float frametime);
void				anim_play_init(anim_play_struct *aps, anim *a_info, int x, int y);
anim_instance *anim_play(anim_play_struct *aps);
void				anim_ignore_next_frametime();
int				anim_stop_playing(anim_instance* anim_instance);
int				anim_show_next_frame(anim_instance *instance, float frametime);
void				anim_release_all_instances(int screen_id = 0);
void				anim_release_render_instance(anim_instance* instance);
anim			  *anim_load(const char * const name, int file_mapped = 0);
int				anim_free(anim *ptr);
int				anim_playing(anim_instance *ai);
int				anim_write_frames_out(char *filename);
void				anim_display_info(char *filename);
void				anim_read_header(anim *ptr, CFILE *fp);
void				anim_reverse_direction(anim_instance *ai);						// called automatically for ping-ponging, and can also be called externally
void				anim_pause(anim_instance *ai);
void				anim_unpause(anim_instance *ai);

int	anim_instance_is_streamed(anim_instance *ai);
unsigned char anim_instance_get_byte(anim_instance *ai, int offset);
#endif /* __ANIMPLAY_H__ */