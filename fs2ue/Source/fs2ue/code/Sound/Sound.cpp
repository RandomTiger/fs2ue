/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/

#ifndef UNITY_BUILD
#include "Sound.h"

#include <windows.h>
#include <mmreg.h>

#include "3dInternal.h"
#include "AudioStr.h"
#include "FSobject.h"
#include "cmdline.h"
#include "osapi.h"
#include "gamesnd.h"
#include "alphacolors.h"
#include "vdsound.h"
#include "ds.h"
#include "ds3d.h"
#include "acm.h"
#include "dscap.h"
#endif

#define SND_F_USED			(1<<0)		// Sounds[] element is used

typedef struct sound	{
	int				sid;			// software id
	int				hid;			// hardware id, -1 if sound is not in hardware
	char				filename[MAX_FILENAME_LEN];
	int				sig;
	int				flags;
	sound_info		info;
	int				uncompressed_size;		// size (in bytes) of sound (uncompressed)
	int				duration;
} sound;

sound	Sounds[MAX_SOUNDS];

int Sound_enabled = TRUE;				// global flag to turn sound on/off
int Snd_sram;								// mem (in bytes) used up by storing sounds in system memory
int Snd_hram;								// mem (in bytes) used up by storing sounds in soundcard memory
float Master_sound_volume = 1.0f;	// range is 0 -> 1, used for non-music sound fx
float Master_voice_volume = 0.7f;	// range is 0 -> 1, used for all voice playback

// min volume to play a sound after all volume processing (range is 0.0 -> 1.0)
#define	MIN_SOUND_VOLUME				0.10f

static int snd_next_sig	= 1;

// convert the game level sound priorities to the DirectSound priority descriptions
int ds_priority(int priority)
{
	switch(priority){
		case SND_PRIORITY_MUST_PLAY:
			return DS_MUST_PLAY;
		case SND_PRIORITY_SINGLE_INSTANCE:
			return DS_LIMIT_ONE;
		case SND_PRIORITY_DOUBLE_INSTANCE:
			return DS_LIMIT_TWO;
		case SND_PRIORITY_TRIPLE_INSTANCE:
			return DS_LIMIT_THREE;
		default:
			Int3();
			return DS_MUST_PLAY;
	};
}

void snd_clear()
{
#if defined(PREPROC_ENABLED_SOUND)
	int i;

	// flag all Sounds[] as free
	for (i=0; i<MAX_SOUNDS; i++ )	{
		Sounds[i].flags &=  ~SND_F_USED;
		Sounds[i].sid = -1;
		Sounds[i].hid = -1;
	}

	// reset how much storage sounds are taking up in memory
	Snd_sram = 0;
	Snd_hram = 0;
#endif
}

// ---------------------------------------------------------------------------------------
// Initialize the game sound system
// 
// Initialize the game sound system.  Depending on what sound library is being used,
// call the appropriate low-level initiailizations
//
// returns:     1		=> init success
//              0		=> init failed
//
int snd_init(int use_a3d, int use_eax)
{
#if defined(PREPROC_ENABLED_SOUND)
	int rval;

	if ( Cmdline_freespace_no_sound )
		return 0;

	if (ds_initialized)	{
		nprintf(( "Sound", "SOUND => Direct Sound is already initialized!\n" ));
		return 1;
	}

	snd_clear();

	// Init DirectSound 

	// Connect to DirectSound
	int num_tries=0;
	int gave_warning = 0;
	while(1) {
		rval = ds_init(use_a3d, use_eax);

		if( rval != 0 ) {
			nprintf(( "Sound", "SOUND ==> Error initializing DirectSound, trying again in 1 second.\n"));
			Sleep(1000);
		} else {
			break;
		}

		if ( num_tries++ > 5 ) {
			if ( !gave_warning ) {
				MessageBoxA(NULL, XSTR("DirectSound could not be initialized.  If you are running any applications playing sound in the background, you should stop them before continuing.",971), NULL, MB_OK);
				gave_warning = 1;
			} else {
				goto Failure;
			}
		}
	}

	// Init the Audio Compression Manager
	if ( ACM_init() == -1 ) {
		HWND hwnd = (HWND)os_get_window();
		MessageBoxA(hwnd, XSTR("Could not properly initialize the Microsoft ADPCM codec.\n\nPlease see the readme.txt file for detailed instructions on installing the Microsoft ADPCM codec.",972), NULL, MB_OK);
//		Warning(LOCATION, "Could not properly initialize the Microsoft ADPCM codec.\nPlease see the readme.txt file for detailed instructions on installing the Microsoft ADPCM codec.");
	}

	// Init the audio streaming stuff
	audiostream_init();
			
	ds_initialized = 1;
#endif
	return 1;

#if defined(PREPROC_ENABLED_SOUND)
	Failure:
//	Warning(LOCATION, "Sound system was unable to be initialized.  If you continue, sound will be disabled.\n");
	nprintf(( "Sound", "SOUND => Direct Sound init unsuccessful, continuing without sound.\n" ));
	return 0;
#endif
}


void snd_spew_info()
{
#if !defined(PREPROC_ENABLED_SOUND)
	int idx;
	char txt[512] = "";
	CFILE *out = cfopen("sounds.txt", "wt", CFILE_NORMAL, CF_TYPE_DATA);
	if(out == NULL){
		return;
	}
	
	cfwrite_string("Sounds loaded :\n", out);

	// spew info for all sounds
	for(idx=0; idx<MAX_SOUNDS; idx++){
		if(!(Sounds[idx].flags & SND_F_USED)){
			continue;
		}
		
		sprintf(txt, "%s (%ds)\n", Sounds[idx].filename, Sounds[idx].info.duration); 
		cfwrite_string(txt, out);
	}

	// close the outfile
	if(out != NULL){
		cfclose(out);
		out = NULL;
	}
#endif
}

#if defined(PREPROC_ENABLED_SOUND)
int Sound_spew = 0;
DCF(show_sounds, "")
{
	Sound_spew = !Sound_spew;
	if(Sound_spew){
		dc_printf("Sound debug info ON");
	} else {
		dc_printf("Sound debug info OFF");
	}
}
#endif
void snd_spew_debug_info()
{
#if defined(PREPROC_ENABLED_SOUND)
	int game_sounds = 0;
	int message_sounds = 0;
	int interface_sounds = 0;
	int done = 0;
	int s_idx;

	if(!Sound_spew){
		return;
	}

	// count up game, interface and message sounds
	for(int idx=0; idx<MAX_SOUNDS; idx++){
		if(!(Sounds[idx].flags & SND_F_USED)){
			continue;
		}

		done = 0;

		// what kind of sound is this
		for(s_idx=0; s_idx<MAX_GAME_SOUNDS; s_idx++){
			if(!stricmp(Snds[s_idx].filename, Sounds[idx].filename)){
				game_sounds++;
				done = 1;
			}
		}

		if(!done){
			for(s_idx=0; s_idx<MAX_GAME_SOUNDS; s_idx++){
				if(!stricmp(Snds_iface[s_idx].filename, Sounds[idx].filename)){
					interface_sounds++;
					done = 1;
				}
			}
		}

		if(!done){
			message_sounds++;
		}		
	}

	// spew info
	gr_set_color_fast(&Color_normal);
	gr_printf(30, 100, "Game sounds : %d\n", game_sounds);
	gr_printf(30, 110, "Interface sounds : %d\n", interface_sounds);
	gr_printf(30, 120, "Message sounds : %d\n", message_sounds);
	gr_printf(30, 130, "Total sounds : %d\n", game_sounds + interface_sounds + message_sounds);
#endif
}

// ---------------------------------------------------------------------------------------
// snd_load() 
//
// Load a sound into memory and prepare it for playback.  The sound will reside in memory as
// a single instance, and can be played multiple times simultaneously.  Through the magic of
// DirectSound, only 1 copy of the sound is used.
//
// parameters:		gs							=> file of sound to load
//						allow_hardware_load	=> whether to try to allocate in hardware
//
// returns:			success => index of sound in Sounds[] array
//						failure => -1
//
//int snd_load( char *filename, int hardware, int use_ds3d, int *sig)
int snd_load( game_snd *gs, int allow_hardware_load )
{
#if defined(PREPROC_ENABLED_DS)
	int				n, rc, type;
	sound_info		*si;
	sound				*snd;
	WAVEFORMATEX	*header = NULL;

	if ( gs->filename == NULL || gs->filename[0] == 0 )
		return -1;

	for (n=0; n<MAX_SOUNDS; n++ )	{
		if (!(Sounds[n].flags & SND_F_USED))
			break;
		else if ( !stricmp( Sounds[n].filename, gs->filename )) {
			gs->sig = Sounds[n].sig;
			return n;
		}
	}

	if ( n == MAX_SOUNDS ) {
#ifndef NDEBUG
		// spew sound info
		snd_spew_info();
#endif

		Int3();
		return -1;
	}

	snd = &Sounds[n];

	if ( !ds_initialized )
		return -1;

	si = &snd->info;

	if ( ds_parse_wave(
		"FS2_AArv_A01.wav",
		//gs->filename, 
		&si->data, &si->size, &header) == -1 )
		return -1;

	si->format					= header->wFormatTag;		// 16-bit flag (wFormatTag)
	si->n_channels				= header->nChannels;			// 16-bit channel count (nChannels)
	si->sample_rate			= header->nSamplesPerSec;	// 32-bit sample rate (nSamplesPerSec)
	si->avg_bytes_per_sec	= header->nAvgBytesPerSec;	// 32-bit average bytes per second (nAvgBytesPerSec)
	si->n_block_align			= header->nBlockAlign;		// 16-bit block alignment (nBlockAlign)
	si->bits						= header->wBitsPerSample;	// Read 16-bit bits per sample			

	snd->duration = fl2i(1000.0f * (si->size / (si->bits/8.0f)) / si->sample_rate);
	type = 0;

	if ( allow_hardware_load ) {
		if ( gs->preload ) {
			type |= DS_HARDWARE;
		}
	}

	if ( (gs->flags&GAME_SND_USE_DS3D)  ) {
		type |= DS_USE_DS3D;
	}
	
	rc = ds_load_buffer(&snd->sid, &snd->hid, &snd->uncompressed_size, header, si, type);

	free(header);
	free(si->data);	// don't want to keep this around

	if ( rc == -1 )
		return -1;

	strncpy( snd->filename, gs->filename, MAX_FILENAME_LEN );
	snd->flags = SND_F_USED;

	snd->sig = snd_next_sig++;
	if (snd_next_sig < 0) snd_next_sig = 1;
	gs->id_sig = snd->sig;
	gs->id = n;

	nprintf(("Sound", "Loaded %s\n", gs->filename));
	return n;
#else
		return 0;
#endif
}

// ---------------------------------------------------------------------------------------
// snd_unload() 
//
// Unload a sound from memory.  This will release the storage, and the sound must be re-loaded via
// sound_load() before it can be played again.
//
int snd_unload( int n )
{
#if defined(PREPROC_ENABLED_DS)

	if (!ds_initialized)
		return 0;

	if ( (n < 0) || ( n >= MAX_SOUNDS) )
		return 0;

	if ( !(Sounds[n].flags & SND_F_USED) )
		return 0;
	
	ds_unload_buffer(Sounds[n].sid, Sounds[n].hid);
	if ( Sounds[n].sid != -1 ) {
		Snd_sram -= Sounds[n].uncompressed_size;
	}
	if ( Sounds[n].hid != -1 ) {
		Snd_hram -= Sounds[n].uncompressed_size;
	}

	Sounds[n].flags &= ~SND_F_USED;
#endif
	return 1;
}

// ---------------------------------------------------------------------------------------
// snd_unload_all() 
//
// Unload all sounds from memory.  This will release the storage, and the sound must be re-loaded via
// sound_load() before it can be played again.
//
void snd_unload_all()
{
#if defined(PREPROC_ENABLED_SOUND)
	int i;
	for (i=0; i<MAX_SOUNDS; i++ )	{
		if ( Sounds[i].flags & SND_F_USED )
			snd_unload(i);
	}
#endif
}

// ---------------------------------------------------------------------------------------
// snd_close()
//
// This is the companion function to snd_init()... it closes down the game sound system.
//
void snd_close(void)
{
#if defined(PREPROC_ENABLED_SOUND)
	snd_stop_all();
	if (!ds_initialized) return;
	snd_unload_all();		// free the sound data stored in DirectSound secondary buffers
	ACM_close();	// Close the Audio Compression Manager (ACM)
	ds3d_close();	// Close DirectSound3D
	dscap_close();	// Close DirectSoundCapture
	ds_close();		// Close DirectSound off
#endif
}

// ---------------------------------------------------------------------------------------
//	snd_play_raw()
//
// Allow a sound to be played directly from the index in Sounds[].  This bypasses the 
// normal game sound management system.
//
// returns:		-1		=>		sound could not be played
//					n		=>		handle for instance of sound
//
int snd_play_raw( int soundnum, float pan, float vol_scale, int priority )
{
#if defined(PREPROC_ENABLED_SOUND)
	game_snd gs;
	int		rval;

	gs.id = soundnum;
	gs.id_sig = Sounds[soundnum].sig;
	gs.filename[0] = 0;
	gs.default_volume = 1.0f;
//	gs.flags = GAME_SND_VOICE | GAME_SND_USE_DS3D;
	gs.flags = GAME_SND_VOICE;

	rval = snd_play(&gs, 0.0f, vol_scale, priority, true);
	return rval;
#endif
	return 0;
}

#if defined(PREPROC_ENABLED_SOUND)
MONITOR( NumSoundsStarted );
MONITOR( NumSoundsLoaded );
#endif

// ---------------------------------------------------------------------------------------
//	snd_play()
//
//	NOTE: vol_scale parameter is the multiplicative scaling applied to the default volume
//       (vol_scale is a default parameter with a default value of 1.0f)
//
// input:	gs				=>	game-level sound description
//				pan			=>	-1 (full left) to 1.0 (full right), this is a default parm
//				vol_scale	=>	factor to scale default volume by (applied before global sound volume applied)
//				priority		=> SND_PRIORITY_MUST_PLAY
//									SND_PRIORITY_SINGLE_INSTANCE		(default value)
//									SND_PRIORITY_DOUBLE_INSTANCE
//									SND_PRIORITY_TRIPLE_INSTANCE
//
// returns:		-1		=>		sound could not be played
//					n		=>		handle for instance of sound
//
int snd_play( game_snd *gs, float pan, float vol_scale, int priority, bool is_voice_msg )
{
#if defined(PREPROC_ENABLED_SOUND)
	float volume;
	sound	*snd;

	int handle = -1;

	if (!Sound_enabled)
		return -1;

	Assert( gs != NULL );

	MONITOR_INC( NumSoundsStarted, 1 );

	if ( gs->id == -1 ) {
		gs->id = snd_load(gs);
		MONITOR_INC( NumSoundsLoaded, 1);
	} else if ( gs->id_sig != Sounds[gs->id].sig ) {
		gs->id = snd_load(gs);
	}

	if ( gs->id == -1 )
		return -1;

	volume = gs->default_volume * vol_scale;
	if ( gs->flags&GAME_SND_VOICE ) {
		volume *= Master_voice_volume;
	} else {
		volume *= Master_sound_volume;
	}
	if ( volume > 1.0f )
		volume = 1.0f;

	snd = &Sounds[gs->id];

	if ( !(snd->flags & SND_F_USED) )
		return -1;

	if (!ds_initialized)
		return -1;

	if ( volume > MIN_SOUND_VOLUME ) {
		handle = ds_play( snd->sid, snd->hid, gs->id_sig, ds_priority(priority), ds_convert_volume(volume), fl2i(pan*MAX_PAN), 0, is_voice_msg);
	}

	return handle;
#else
	return -1;
#endif
}

#if defined(PREPROC_ENABLED_SOUND)
MONITOR( Num3DSoundsStarted );
MONITOR( Num3DSoundsLoaded );
#endif

// ---------------------------------------------------------------------------------------
// snd_play_3d()
//
//	NOTE: vol_scale parameter is the multiplicative scaling applied to the default volume
//       (vol_scale is a default parameter with a default value of 1.0f)
//
// input:	gs				=>	game-level sound description
//				source_pos	=>	global pos of where the sound is
//				listen_pos	=>	global pos of where listener is
//				radius		=>	optional parameter, this specifes distance at which to apply min/max distances
//				source_vel	=>	velocity of the source playing the sound (used for DirectSound3D only)
//				looping		=>	flag to indicate the sound should loop (default value 0)
//				vol_scale	=>	factor to scale the static volume by (applied before attenuation)
//				priority		=> SND_PRIORITY_MUST_PLAY
//									SND_PRIORITY_SINGLE_INSTANCE	(default value)
//									SND_PRIORITY_DOUBLE_INSTANCE
//									SND_PRIORITY_TRIPLE_INSTANCE
//				sound_fvec		=> forward vector of where sound is emitting from (RSX use only)
//				range_factor	=>	factor N, which increases distance sound is heard by N times (default value 1)
//
// returns:		-1		=>		sound could not be played
//					n		=>		handle for instance of sound
//
int snd_play_3d(game_snd *gs, vector *source_pos, vector *listen_pos, float radius, vector *source_vel, int looping, float vol_scale, int priority, vector *sound_fvec, float range_factor, int force )
{
#if defined(PREPROC_ENABLED_SOUND)
	int		handle, min_range, max_range;
	vector	vector_to_sound;
	sound		*snd;
	float		volume, distance, pan, max_volume;

	if ( !Sound_enabled )
		return -1;

	Assert(gs != NULL);

	MONITOR_INC( Num3DSoundsStarted, 1 );

	if ( gs->id == -1 ) {
		gs->id = snd_load(gs);
		MONITOR_INC( Num3DSoundsLoaded, 1 );
	}else if ( gs->id_sig != Sounds[gs->id].sig ) {
		gs->id = snd_load(gs);
	}

	if ( gs->id == -1 )
		return -1;

	snd = &Sounds[gs->id];

	if ( !(snd->flags & SND_F_USED) )
		return -1;

	handle = -1;

	min_range = fl2i( (gs->min + radius) * range_factor);
	max_range = fl2i( (gs->max + radius) * range_factor + 0.5f);

	if (!ds_initialized)
		return -1;
	
	// DirectSound3D will not cut off sounds, no matter how quite they become.. so manually
	// prevent sounds from playing past the max distance.
	distance = vm_vec_normalized_dir_quick( &vector_to_sound, source_pos, listen_pos );
	max_volume = gs->default_volume * vol_scale;
	if ( (distance > max_range) && !force){
		return -1;
	}

	if ( distance <= min_range ) {
		volume = max_volume;
	}
	else {
		volume = max_volume - max_volume*(distance/max_range);
	}

	if ( volume > 1.0f ){
		volume = 1.0f;
	}

	if ( priority == SND_PRIORITY_MUST_PLAY ) {
		if ( volume < 0.3 ) {
			priority = SND_PRIORITY_DOUBLE_INSTANCE;
		} 
	}

	volume *= Master_sound_volume;
	if ( (volume < MIN_SOUND_VOLUME) && !force) {
		return -1;
	}

	int play_using_ds3d = 0;

	if (ds_using_ds3d()) {
		if ( ds_is_3d_buffer(snd->sid) ) {
			play_using_ds3d = 1;
		}
	}

	if ( play_using_ds3d ) {
		// play through DirectSound3D
		handle = ds3d_play( snd->sid, snd->hid, gs->id_sig, source_pos, source_vel, min_range, max_range, looping, ds_convert_volume(max_volume*Master_sound_volume), ds_convert_volume(volume), ds_priority(priority));
	}
	else {
		// play sound as a fake 3D sound
		if ( distance <= 0 ) {
			pan = 0.0f;
		}
		else {
			pan = vm_vec_dot(&View_matrix.rvec,&vector_to_sound);
		}
		if(looping){
			handle = snd_play_looping( gs, pan, -1, -1, volume/gs->default_volume, priority, force );
		} else {
			handle = snd_play( gs, pan, volume/gs->default_volume, priority);
		}
	}

	return handle;
#else
	return -1;
#endif
}

// update the given 3d sound with a new position
void snd_update_3d_pos(int soundnum, game_snd *gs, vector *new_pos)
{
#if defined(PREPROC_ENABLED_SOUND)
	float vol, pan;
	
	// get new volume and pan vals
	snd_get_3d_vol_and_pan(gs, new_pos, &vol, &pan);

	// set volume
	snd_set_volume(soundnum, vol);

	// set pan
	snd_set_pan(soundnum, pan);
#endif
}

// ---------------------------------------------------------------------------------------
// snd_get_3d_vol_and_pan()
//
// Based on the 3D position the player and the object, calculate
// the correct volume and pan.
//
// parameters:		gs			=> pointer to sound description
//						pos		=> 3D position used to calc volume and pan
//						vol		=> output parameter for the volume
//						pan		=> output parameter for the pan
//						radius	=>	optional parameter (default value 0) which indicates sound attenuation
//										should occur from this radius
//
// returns:			-1			=> could not determine vol or pan
//						0			=> success 
//
//	NOTE: the volume is not scaled by the Master_sound_volume, since this always occurs
//			when snd_play() or snd_play_looping() is called
//
int snd_get_3d_vol_and_pan(game_snd *gs, vector *pos, float* vol, float *pan, float radius)
{
#if defined(PREPROC_ENABLED_SOUND)
	vector	vector_to_sound;
	float		distance, max_volume;
	sound		*snd;

	*vol = 0.0f;
	*pan = 0.0f;

	if (!ds_initialized)
		return -1;

	Assert(gs != NULL);

	if ( gs->id == -1 ) {
		gs->id = snd_load(gs);
	}
	
	snd = &Sounds[gs->id];
	if ( !(snd->flags & SND_F_USED) )
		return -1;

	distance = vm_vec_normalized_dir_quick( &vector_to_sound, pos, &View_position );
	distance -= radius;

	max_volume = gs->default_volume;
	if ( distance <= gs->min ) {
		*vol = max_volume;
	}
	else {
		*vol = max_volume - (distance - gs->min) * max_volume / (gs->max - gs->min);
	}

	if ( *vol > 1.0f )
		*vol = 1.0f;

	if ( *vol > MIN_SOUND_VOLUME ) {
		if ( distance <= 0 )
			*pan = 0.0f;
		else
			*pan = vm_vec_dot(&View_matrix.rvec,&vector_to_sound);
	}
#endif
	return 0;
}

// ---------------------------------------------------------------------------------------
// volume 0 to 1.0.  Returns the handle of the sound. -1 if failed.
// If startloop or stoploop are not -1, then then are used.
//
//	NOTE: vol_scale parameter is the multiplicative scaling applied to the default volume
//       (vol_scale is a default parameter with a default value of 1.0f)
//
// input:	gs				=>	game-level sound description
//				source_pos	=>	global pos of where the sound is
//				listen_pos	=>	global pos of where listener is
//				source_vel	=>	velocity of the source playing the sound (used for DirectSound3D only)
//				looping		=>	flag to indicate the sound should loop (default value 0)
//				vol_scale	=>	factor to scale the static volume by (applied before attenuation)
//				priority		=> SND_PRIORITY_MUST_PLAY			(default value)
//									SND_PRIORITY_SINGLE_INSTANCE
//									SND_PRIORITY_DOUBLE_INSTANCE
//									SND_PRIORITY_TRIPLE_INSTANCE
//
// returns:		-1		=>		sound could not be played
//					n		=>		handle for instance of sound
//
int snd_play_looping( game_snd *gs, float pan, int start_loop, int stop_loop, float vol_scale, int priority, int force )
{	
#if defined(PREPROC_ENABLED_SOUND)
	float volume;
	int	handle = -1;
	sound	*snd;	

	if (!Sound_enabled)
		return -1;

	Assert( gs != NULL );

	if (!ds_initialized)
		return -1;

	if ( gs->id == -1 ) {
		gs->id = snd_load(gs);
	}
	else if ( gs->id_sig != Sounds[gs->id].sig ) {
		gs->id = snd_load(gs);
	}

	if ( gs->id == -1 )
		return -1;

	snd = &Sounds[gs->id];

	if ( !(snd->flags & SND_F_USED) )
		return -1;

	volume = gs->default_volume * vol_scale;
	volume *= Master_sound_volume;
	if ( volume > 1.0f )
		volume = 1.0f;

	if ( (volume > MIN_SOUND_VOLUME) || force) {
		handle = ds_play( snd->sid, snd->hid, gs->id_sig, ds_priority(priority), ds_convert_volume(volume), fl2i(pan*MAX_PAN), 1);
	}

	return handle;
#else
	return -1;
#endif
}

// ---------------------------------------------------------------------------------------
// snd_stop()
//
// Stop a sound from playing.
//
// parameters:		sig => handle to sound, what is returned from snd_play()
//
void snd_stop( int sig )
{
#if defined(PREPROC_ENABLED_SOUND)
	int channel;

	if (!ds_initialized) return;
	if ( sig < 0 ) return;

	channel = ds_get_channel(sig);
	if ( channel == -1 )
		return;
	
	ds_stop_channel(channel);
#endif
}

// ---------------------------------------------------------------------------------------
// snd_set_volume()
//
// Set the volume of a currently playing sound
//
// parameters:		sig		=> handle to sound, what is returned from snd_play()
//						volume	=> volume of sound (range: 0.0 -> 1.0)
//
void snd_set_volume( int sig, float volume )
{
#if defined(PREPROC_ENABLED_SOUND)
	int	channel;
	float	new_volume;

	if (!ds_initialized)
		return;

	if ( sig < 0 )
		return;

	channel = ds_get_channel(sig);
	if ( channel == -1 ) {
		nprintf(( "Sound", "WARNING: Trying to set volume for a non-playing sound.\n" ));
		return;
	}

	new_volume = volume * Master_sound_volume;
	ds_set_volume( channel, ds_convert_volume(new_volume) );
#endif
}

// ---------------------------------------------------------------------------------------
// snd_set_pan()
//
// Set the pan of a currently playing sound
//
// parameters:		sig	=> handle to sound, what is returned from snd_play()
//						pan	=> pan of sound (range: -1.0 -> 1.0)
//
void snd_set_pan( int sig, float pan )
{
#if defined(PREPROC_ENABLED_SOUND)

	int channel;

	if (!ds_initialized)
		return;

	if ( sig < 0 )
		return;
	
	channel = ds_get_channel(sig);
	if ( channel == -1 ) {
		nprintf(( "Sound", "WARNING: Trying to set pan for a non-playing sound.\n" ));
		return;
	}

	ds_set_pan( channel, fl2i(pan*MAX_PAN) );
#endif
}

// ---------------------------------------------------------------------------------------
// snd_get_pitch()
//
// Return the pitch of a currently playing sound
//
// returns:			pitch of sound ( range: 100 to 100000)
//
// parameters:		sig	=> handle to sound, what is returned from snd_play()
//
int snd_get_pitch(int sig)
{
#if defined(PREPROC_ENABLED_SOUND)

	int channel, pitch=10000;

	if (!ds_initialized)
		return -1;

	if ( sig < 0 )
		return -1;

	channel = ds_get_channel(sig);
	if ( channel == -1 ) {
		nprintf(( "Sound", "WARNING: Trying to get pitch for a non-playing sound.\n" ));
		return -1;
	}

	pitch = ds_get_pitch(channel);

	return pitch;
#else
	return -1;
#endif
}

// ---------------------------------------------------------------------------------------
// snd_set_pitch()
//
// Set the pitch of a currently playing sound
//
// parameters:		sig		=> handle to sound, what is returned from snd_play()
//						pan		=> pitch of sound (range: 100 to 100000)
//
void snd_set_pitch( int sig, int pitch )
{
#if defined(PREPROC_ENABLED_SOUND)

	int channel;

	if (!ds_initialized) return;
	if ( sig < 0 ) return;

	channel = ds_get_channel(sig);
	if ( channel == -1 ) {
		nprintf(( "Sound", "WARNING: Trying to set pitch for a non-playing sound.\n" ));
		return;
	}

	ds_set_pitch(channel, pitch);
#endif
}

// ---------------------------------------------------------------------------------------
// snd_is_playing()
//
// Determine if a sound is playing
//
// returns:			1				=> sound is currently playing
//						0				=> sound is not playing
//
// parameters:		sig	=> signature of sound, what is returned from snd_play()
//
int snd_is_playing( int sig )
{
#if defined(PREPROC_ENABLED_SOUND)

	int	channel, is_playing;

	if (!ds_initialized)
		return 0;

	if ( sig < 0 )
		return 0;

	channel = ds_get_channel(sig);
	if ( channel == -1 )
		return 0;

	is_playing = ds_is_channel_playing(channel);
	if ( is_playing == TRUE ) {
		return 1;
	}
#endif
	return 0;
}


// ---------------------------------------------------------------------------------------
// snd_chg_loop_status()
//
// Change whether a currently playing song is looping or not
//
// parameters:		sig			=> handle to sound, what is returned from snd_play()
//						loop			=> whether to start (1) or stop (0) looping
//
void snd_chg_loop_status(int sig, int loop)
{
#if defined(PREPROC_ENABLED_SOUND)
	int channel;

	if (!ds_initialized)
		return;

	if ( sig < 0 )
		return;

	channel = ds_get_channel(sig);
	if ( channel == -1 ) {
		nprintf(( "Sound", "WARNING: Trying to change loop status of a non-playing sound!\n" ));
		return;
	}

	ds_chg_loop_status(channel, loop);
#endif
}

// ---------------------------------------------------------------------------------------
// snd_stop_all()
//
// Stop all playing sound channels (including looping sounds)
//
// NOTE: This stops all sounds that are playing from Channels[] sound buffers.  It doesn't
//			stop every secondary sound buffer in existance
//
void snd_stop_all()
{
#if defined(PREPROC_ENABLED_SOUND)
	if (!ds_initialized)
		return;

	ds_stop_channel_all();
#endif
}

// ---------------------------------------------------------------------------------------
// sound_get_ds()
//
// Return the pointer to the DirectSound interface
//
//
uint sound_get_ds()
{
#if defined(PREPROC_ENABLED_SOUND)

	return (uint)pDirectSound;
#else 
	return 0;
#endif
}

// ---------------------------------------------------------------------------------------
// snd_is_inited()
//
// 
int snd_is_inited()
{
#if defined(PREPROC_ENABLED_SOUND)
	if ( !ds_initialized )
		return FALSE;
#endif
	return TRUE;
}

// return the time in ms for the duration of the sound
int snd_get_duration(int snd_id)
{
	if ( snd_id < 0 )
		return 0;

	return Sounds[snd_id].duration;
}


#if defined(PREPROC_ENABLED_SOUND)
MONITOR( SoundChannels );
#endif

// update the position of the listener for the specific 3D sound API we're 
// using
void snd_update_listener(vector *pos, vector *vel, matrix *orient)
{
#if defined(PREPROC_ENABLED_SOUND)

	MONITOR_INC( SoundChannels, ds_get_number_channels() );
	ds3d_update_listener(pos, vel, orient);
#endif
}

// this could probably be optimized a bit
void snd_rewind(int snd_handle, game_snd *gs, float seconds)
{			
#if defined(PREPROC_ENABLED_SOUND)

	float current_time,desired_time;
	float bps;
	DWORD current_offset,desired_offset;
	sound_info *snd;

	if(!snd_is_playing(snd_handle))
		return;

	snd = &Sounds[gs->id].info;
	
	current_offset = ds_get_play_position(ds_get_channel(snd_handle));	// current offset into the sound
	bps = (float)snd->sample_rate * (float)snd->bits;							// data rate
	current_time = (float)current_offset/bps;										// how many seconds we're into the sound

	// don't rewind if it'll put us before the beginning of the sound
	if(current_time - seconds < 0.0f)
		return;

	desired_time = current_time - seconds;											// where we want to be
	desired_offset = (DWORD)(desired_time * bps);								// the target
			
	ds_set_position(ds_get_channel(snd_handle),desired_offset);
#endif
}

// this could probably be optimized a bit
void snd_ffwd(int snd_handle, game_snd *gs, float seconds)
{
#if defined(PREPROC_ENABLED_SOUND)

	if(!snd_is_playing(snd_handle))
		return;

	float current_time,desired_time;
	float bps;
	DWORD current_offset,desired_offset;
	sound_info *snd;

	if(!snd_is_playing(snd_handle))
		return;

	snd = &Sounds[gs->id].info;

	current_offset = ds_get_play_position(ds_get_channel(snd_handle));	// current offset into the sound
	bps = (float)snd->sample_rate * (float)snd->bits;							// data rate
	current_time = (float)current_offset/bps;										// how many seconds we're into the sound

	// don't rewind if it'll put us past the end of the sound
	if(current_time + seconds > (float)snd->duration)
		return;

	desired_time = current_time + seconds;											// where we want to be
	desired_offset = (DWORD)(desired_time * bps);								// the target
			
	ds_set_position(ds_get_channel(snd_handle),desired_offset);
#endif
}

// this could probably be optimized a bit
void snd_set_pos(int snd_handle, game_snd *gs, float val,int as_pct)
{
#if defined(PREPROC_ENABLED_SOUND)

	if(!snd_is_playing(snd_handle))
		return;

	sound_info *snd;

	snd = &Sounds[gs->id].info;		
	// set position as an absolute from 0 to 1
	if(as_pct){
		Assert((val >= 0.0) && (val <= 1.0));
		ds_set_position(ds_get_channel(snd_handle),(DWORD)((float)snd->size * val));
	} 
	// set the position as an absolute # of seconds from the beginning of the sound
	else {
		float bps;
		Assert(val <= (float)snd->duration/1000.0f);
		bps = (float)snd->sample_rate * (float)snd->bits;							// data rate			
		ds_set_position(ds_get_channel(snd_handle),(DWORD)(bps * val));
	}
#endif
}

// Return the number of sounds currently playing
int snd_num_playing()
{
	return ds_get_number_channels();
}

// Stop the first channel found that is playing a sound
void snd_stop_any_sound()
{
	int i;

	for ( i = 0; i < 16; i++ ) {
		if ( ds_is_channel_playing(i) ) {
			ds_stop_channel(i);
			break;
		}
	}
}

// Return the raw sound data for a loaded sound
//
// input:	handle	=>	index into Sounds[] array
//				data		=>	allocated mem to hold sound
//
// exit:		0	=>	success
//				!0	=>	fail
int snd_get_data(int handle, char *data)
{
#if defined(PREPROC_ENABLED_SOUND)

	Assert(handle >= 0 && handle < MAX_SOUNDS);
	if ( ds_get_data(Sounds[handle].sid, data) ) {
		return -1;
	}
#endif
	return 0;
}

// return the size of the sound data associated with the sound handle
int snd_size(int handle, int *size)
{
#if defined(PREPROC_ENABLED_SOUND)

	Assert(handle >= 0 && handle < MAX_SOUNDS);
	if ( ds_get_size(Sounds[handle].sid, size) ) {
		return -1;
	}
#endif
	return 0;
}

// retrieve the bits per sample and frequency for a given sound
void snd_get_format(int handle, int *bits_per_sample, int *frequency)
{
#if defined(PREPROC_ENABLED_SOUND)

	Assert(handle >= 0 && handle < MAX_SOUNDS);
	*bits_per_sample = Sounds[handle].info.bits;
	*frequency = Sounds[handle].info.sample_rate;
#endif
}

// return the time for the sound to play in milliseconds
int snd_time_remaining(int handle, int bits_per_sample, int frequency)
{
#if defined(PREPROC_ENABLED_SOUND)
	int channel, is_playing, time_remaining = 0;

	if (!ds_initialized)
		return 0;

	if ( handle < 0 )
		return 0;

	channel = ds_get_channel(handle);
	if ( channel == -1 )
		return 0;

	is_playing = ds_is_channel_playing(channel);
	if ( !is_playing ) {
		return 0;
	}

	int current_offset, max_offset;

	current_offset = ds_get_play_position(channel);
	max_offset = ds_get_channel_size(channel);

	if ( current_offset < max_offset ) {
		int bytes_remaining = max_offset - current_offset;
		int samples_remaining = bytes_remaining / fl2i(bits_per_sample/8.0f);
		time_remaining = fl2i(1000 * samples_remaining/frequency + 0.5f);
	}	

//	mprintf(("time_remaining: %d\n", time_remaining));	
	return time_remaining;
#else
	return 0;
#endif
}


// snd_env_ interface

static unsigned long Sound_env_id;
static float Sound_env_volume;
static float Sound_env_damping;
static float Sound_env_decay;

// Set the sound environment
//
int sound_env_set(sound_env *se)
{
	if (ds_eax_set_all(se->id, se->volume, se->damping, se->decay) == 0) {
		Sound_env_id = se->id;
		Sound_env_volume = se->volume;
		Sound_env_damping = se->damping;
		Sound_env_decay = se->decay;
		return 0;
	} else {
		return -1;
	}
}

// Get the sound environment
//
int sound_env_get(sound_env *se)
{
	EAX_REVERBPROPERTIES er;

	if (ds_eax_get_all(&er) == 0) {
		se->id = er.environment;
		se->volume = er.fVolume;
		se->decay = er.fDecayTime_sec;
		se->damping = er.fDamping;
		return 0;
	} else {
		return -1;
	}
}

// Turn off the sound environment
//
int sound_env_disable()
{
	sound_env se;
	se.id = SND_ENV_GENERIC;
	se.volume = 0.0f;
	se.damping = 0.0f;
	se.decay = 0.0f;
	sound_env_set(&se);
	return 0;
}

// Return 1 if EAX can used to set the sound environment, otherwise return 0
//
int sound_env_supported()
{
	return ds_eax_is_inited();
}

// Called once per game frame
//
void snd_do_frame()
{
	ds_do_frame();
}