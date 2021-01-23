/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/


#ifndef UNITY_BUILD
#include "ds.h"
#include "vdsound.h"
#include "cfile.h"
#include "channel.h"
#include "ds3d.h"
#include "acm.h"
#include "OsApi.h"
#include "dscap.h"

#include <objbase.h>
#include <initguid.h>
#ifndef FS2_UE
#include <mmreg.h>
#endif
#include <mmeapi.h>
#endif

void	ds_get_primary_format(WAVEFORMATEX *wfx);

// Pointers to functions contained in DSOUND.dll
HRESULT (__stdcall *pfn_DirectSoundCreate)(LPGUID lpGuid, LPDIRECTSOUND *ppDS, IUnknown FAR *pUnkOuter) = NULL;
HRESULT (__stdcall *pfn_DirectSoundCaptureCreate)(LPGUID lpGUID, LPDIRECTSOUNDCAPTURE *lplpDSC, LPUNKNOWN pUnkOuter) = NULL;
int Ds_dll_loaded=0;
HINSTANCE Ds_dll_handle=NULL;

LPDIRECTSOUND				pDirectSound = NULL;
LPDIRECTSOUNDBUFFER		pPrimaryBuffer = NULL;

static LPKSPROPERTYSET			pPropertySet;		// pointer to sound card property set
static LPDIRECTSOUNDBUFFER		Ds_property_set_pdsb = NULL;
static LPDIRECTSOUND3DBUFFER	Ds_property_set_pds3db = NULL;

static int Ds_must_call_couninitialize = 0;

channel* Channels;		//[MAX_CHANNELS];
static int channel_next_sig = 1;

#define MAX_DS_SOFTWARE_BUFFERS	256
typedef struct ds_sound_buffer
{
	LPDIRECTSOUNDBUFFER	pdsb;
	DSBUFFERDESC			desc;
	WAVEFORMATEX			wfx;

} ds_sound_buffer;

ds_sound_buffer ds_software_buffers[MAX_DS_SOFTWARE_BUFFERS];

#define MAX_DS_HARDWARE_BUFFERS	32
ds_sound_buffer ds_hardware_buffers[MAX_DS_HARDWARE_BUFFERS];

int ds_vol_lookup[101];						// lookup table for direct sound volumes
int ds_initialized = FALSE;

static DSCAPS Soundcard_caps;					// current soundcard capabilities

extern int Snd_sram;					// mem (in bytes) used up by storing sounds in system memory
extern int Snd_hram;					// mem (in bytes) used up by storing sounds in soundcard memory

static int Ds_use_ds3d = 0;
static int Ds_use_eax = 0;

static bool Stop_logging_sounds = false;

static int MAX_CHANNELS = 0;		// initialized properly in ds_init_channels()

///////////////////////////
//
// EAX
//
///////////////////////////

// presets
//#define EAX_PRESET_GENERIC         EAX_ENVIRONMENT_GENERIC,0.5F,1.493F,0.5F
#define EAX_PRESET_GENERIC         EAX_ENVIRONMENT_GENERIC,0.2F,0.2F,1.0F
#define EAX_PRESET_PADDEDCELL      EAX_ENVIRONMENT_PADDEDCELL,0.25F,0.1F,0.0F
#define EAX_PRESET_ROOM            EAX_ENVIRONMENT_ROOM,0.417F,0.4F,0.666F
#define EAX_PRESET_BATHROOM        EAX_ENVIRONMENT_BATHROOM,0.653F,1.499F,0.166F
#define EAX_PRESET_LIVINGROOM      EAX_ENVIRONMENT_LIVINGROOM,0.208F,0.478F,0.0F
#define EAX_PRESET_STONEROOM       EAX_ENVIRONMENT_STONEROOM,0.5F,2.309F,0.888F
#define EAX_PRESET_AUDITORIUM      EAX_ENVIRONMENT_AUDITORIUM,0.403F,4.279F,0.5F
#define EAX_PRESET_CONCERTHALL     EAX_ENVIRONMENT_CONCERTHALL,0.5F,3.961F,0.5F
#define EAX_PRESET_CAVE            EAX_ENVIRONMENT_CAVE,0.5F,2.886F,1.304F
#define EAX_PRESET_ARENA           EAX_ENVIRONMENT_ARENA,0.361F,7.284F,0.332F
#define EAX_PRESET_HANGAR          EAX_ENVIRONMENT_HANGAR,0.5F,10.0F,0.3F
#define EAX_PRESET_CARPETEDHALLWAY EAX_ENVIRONMENT_CARPETEDHALLWAY,0.153F,0.259F,2.0F
#define EAX_PRESET_HALLWAY         EAX_ENVIRONMENT_HALLWAY,0.361F,1.493F,0.0F
#define EAX_PRESET_STONECORRIDOR   EAX_ENVIRONMENT_STONECORRIDOR,0.444F,2.697F,0.638F
#define EAX_PRESET_ALLEY           EAX_ENVIRONMENT_ALLEY,0.25F,1.752F,0.776F
#define EAX_PRESET_FOREST          EAX_ENVIRONMENT_FOREST,0.111F,3.145F,0.472F
#define EAX_PRESET_CITY            EAX_ENVIRONMENT_CITY,0.111F,2.767F,0.224F
#define EAX_PRESET_MOUNTAINS       EAX_ENVIRONMENT_MOUNTAINS,0.194F,7.841F,0.472F
#define EAX_PRESET_QUARRY          EAX_ENVIRONMENT_QUARRY,1.0F,1.499F,0.5F
#define EAX_PRESET_PLAIN           EAX_ENVIRONMENT_PLAIN,0.097F,2.767F,0.224F
#define EAX_PRESET_PARKINGLOT      EAX_ENVIRONMENT_PARKINGLOT,0.208F,1.652F,1.5F
#define EAX_PRESET_SEWERPIPE       EAX_ENVIRONMENT_SEWERPIPE,0.652F,2.886F,0.25F
#define EAX_PRESET_UNDERWATER      EAX_ENVIRONMENT_UNDERWATER,1.0F,1.499F,0.0F
#define EAX_PRESET_DRUGGED         EAX_ENVIRONMENT_DRUGGED,0.875F,8.392F,1.388F
#define EAX_PRESET_DIZZY           EAX_ENVIRONMENT_DIZZY,0.139F,17.234F,0.666F
#define EAX_PRESET_PSYCHOTIC       EAX_ENVIRONMENT_PSYCHOTIC,0.486F,7.563F,0.806F

static LPKSPROPERTYSET Ds_eax_reverb = NULL;

static int Ds_eax_inited = 0;

EAX_REVERBPROPERTIES Ds_eax_presets[] = 
{
	{EAX_PRESET_GENERIC},
	{EAX_PRESET_PADDEDCELL},
	{EAX_PRESET_ROOM},
	{EAX_PRESET_BATHROOM},
	{EAX_PRESET_LIVINGROOM},
	{EAX_PRESET_STONEROOM},
	{EAX_PRESET_AUDITORIUM},
	{EAX_PRESET_CONCERTHALL},
	{EAX_PRESET_CAVE},
	{EAX_PRESET_ARENA},
	{EAX_PRESET_HANGAR},
	{EAX_PRESET_CARPETEDHALLWAY},
	{EAX_PRESET_HALLWAY},
	{EAX_PRESET_STONECORRIDOR},
	{EAX_PRESET_ALLEY},
	{EAX_PRESET_FOREST},
	{EAX_PRESET_CITY},
	{EAX_PRESET_MOUNTAINS},
	{EAX_PRESET_QUARRY},
	{EAX_PRESET_PLAIN},
	{EAX_PRESET_PARKINGLOT},
	{EAX_PRESET_SEWERPIPE},
	{EAX_PRESET_UNDERWATER},
	{EAX_PRESET_DRUGGED},
	{EAX_PRESET_DIZZY},
	{EAX_PRESET_PSYCHOTIC},
};


// EAX buffer reverb property set {4a4e6fc0-c341-11d1-b73a-444553540000}
DEFINE_GUID(DSPROPSETID_EAXBUFFER_ReverbProperties,
	0x4a4e6fc0,
	0xc341,
	0x11d1,
	0xb7, 0x3a, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00);

// EAX (listener) reverb property set {4a4e6fc1-c341-11d1-b73a-444553540000}
DEFINE_GUID(DSPROPSETID_EAX_ReverbProperties,
	0x4a4e6fc1,
	0xc341,
	0x11d1,
	0xb7, 0x3a, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00);

GUID DSPROPSETID_EAX_ReverbProperties_Def = {0x4a4e6fc1, 0xc341, 0x11d1, {0xb7, 0x3a, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};
GUID DSPROPSETID_EAXBUFFER_ReverbProperties_Def = {0x4a4e6fc0, 0xc341, 0x11d1, {0xb7, 0x3a, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}};

//----------------------------------------------------------------
// prototypes 
void ds_get_soundcard_caps(DSCAPS *dscaps);




//--------------------------------------------------------------------------
// ds_is_3d_buffer()
//
// Determine if a secondary buffer is a 3d secondary buffer.
//
int ds_is_3d_buffer(LPDIRECTSOUNDBUFFER pdsb)
{
	DSBCAPS			dsbc;
	HRESULT			hr;

	dsbc.dwSize = sizeof(dsbc);
	hr = pdsb->GetCaps(&dsbc);
	if ( hr == DS_OK && dsbc.dwFlags & DSBCAPS_CTRL3D ) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

//--------------------------------------------------------------------------
// ds_is_3d_buffer()
//
// Determine if a secondary buffer is a 3d secondary buffer.
//
int ds_is_3d_buffer(int sid)
{
	if ( sid >= 0 ) {
		return ds_is_3d_buffer(ds_software_buffers[sid].pdsb);
	}

	return 0;
}

//--------------------------------------------------------------------------
//  ds_build_vol_lookup()
//
//  Fills up the ds_vol_lookup[] tables that converts from a volume in the form
//  0.0 -> 1.0 to -10000 -> 0 (this is the DirectSound method, where units are
//  hundredths of decibls)
//
void ds_build_vol_lookup()
{
	int	i;
	float	vol;

	ds_vol_lookup[0] = -10000;
	for ( i = 1; i <= 100; i++ ) {
		vol = i / 100.0f;
		ds_vol_lookup[i] = fl2i( (log(vol) / log(2.0f)) * 1000.0f);
	}
}


//--------------------------------------------------------------------------
// ds_convert_volume()
//
// Takes volume between 0.0f and 1.0f and converts into
// DirectSound style volumes between -10000 and 0.
int ds_convert_volume(float volume)
{
	int index;

	index = fl2i(volume * 100.0f);
	if ( index > 100 )
		index = 100;
	if ( index < 0 )
		index = 0;

	return ds_vol_lookup[index];
}

//--------------------------------------------------------------------------
// ds_get_percentage_vol()
//
// Converts -10000 -> 0 range volume to 0 -> 1
float ds_get_percentage_vol(int ds_vol)
{
	double vol;
	vol = pow(2.0, ds_vol/1000.0);
	return (float)vol;
}

// ---------------------------------------------------------------------------------------
// ds_parse_wave() 
//
// Parse a wave file.
//
// parameters:		filename			=> file of sound to parse
//						dest				=> address of pointer of where to store raw sound data (output parm)
//						dest_size		=> number of bytes of sound data stored (output parm)
//						header			=> address of pointer to a WAVEFORMATEX struct (output parm)
//
// returns:			0					=> wave file successfully parsed
//						-1					=> error
//
//	NOTE: memory is malloced for the header and dest in this function.  It is the responsibility
//			of the caller to free this memory later.
//
int ds_parse_wave(char *filename, ubyte **dest, uint *dest_size, WAVEFORMATEX **header)
{
	CFILE				*fp;
	PCMWAVEFORMAT	PCM_header;
	int				cbExtra = 0;
	unsigned int	tag, size, next_chunk;

	fp = cfopen( filename, "rb" );
	if ( fp == NULL )	{
		nprintf(("Error", "Couldn't open '%s'\n", filename ));
		return -1;
	}
	
	// Skip the "RIFF" tag and file size (8 bytes)
	// Skip the "WAVE" tag (4 bytes)
	cfseek( fp, 12, CF_SEEK_SET );

	// Now read RIFF tags until the end of file

	while(1)	{
		if ( cfread( &tag, sizeof(uint), 1, fp ) != 1 )
			break;

		if ( cfread( &size, sizeof(uint), 1, fp ) != 1 )
			break;

		next_chunk = cftell(fp) + size;

		switch( tag )	{
		case 0x20746d66:		// The 'fmt ' tag
			//nprintf(("Sound", "SOUND => size of fmt block: %d\n", size));
			cfread( &PCM_header, sizeof(PCMWAVEFORMAT), 1, fp );
			if ( PCM_header.wf.wFormatTag != WAVE_FORMAT_PCM ) {
				cbExtra = cfread_short(fp);
			}

			// Allocate memory for WAVEFORMATEX structure + extra bytes
			if ( (*header = (WAVEFORMATEX *) malloc ( sizeof(WAVEFORMATEX)+cbExtra )) != NULL ){
				// Copy bytes from temporary format structure
				memcpy (*header, &PCM_header, sizeof(PCM_header));
				(*header)->cbSize = (unsigned short)cbExtra;

				// Read those extra bytes, append to WAVEFORMATEX structure
				if (cbExtra != 0) {
					cfread( ((ubyte *)(*header) + sizeof(WAVEFORMATEX)), cbExtra, 1, fp);
				}
			}
			else {
				Assert(0);		// malloc failed
			}
	
			break;
		case 0x61746164:		// the 'data' tag
			*dest_size = size;
			(*dest) = (ubyte *)malloc(size);
			Assert( *dest != NULL );
			cfread( *dest, size, 1, fp );
			break;
		default:	// unknown, skip it
			break;
		}
		cfseek( fp, next_chunk, CF_SEEK_SET );
	}
	cfclose(fp);

	return 0;
}


// ---------------------------------------------------------------------------------------
// ds_get_sid()
//
//	
int ds_get_sid()
{
	int i;

	for ( i = 0; i < MAX_DS_SOFTWARE_BUFFERS; i++ ) {
		if ( ds_software_buffers[i].pdsb == NULL )
		break;
	}

	if ( i == MAX_DS_SOFTWARE_BUFFERS )	{
		return -1;
	}

	return i;
}

// ---------------------------------------------------------------------------------------
// ds_get_hid()
//
//	
int ds_get_hid()
{
	int i;

	for ( i = 0; i < MAX_DS_HARDWARE_BUFFERS; i++ ) {
		if ( ds_hardware_buffers[i].pdsb == NULL )
		break;
	}

	if ( i == MAX_DS_HARDWARE_BUFFERS )	{
		return -1;
	}

	return i;
}

// ---------------------------------------------------------------------------------------
// Load a DirectSound secondary buffer with sound data.  The sounds data for
// game sounds are stored in the DirectSound secondary buffers, and are 
// duplicated as needed and placed in the Channels[] array to be played.
// 
//
// parameters:  
//					 sid				  => pointer to software id for sound ( output parm)
//					 hid				  => pointer to hardware id for sound ( output parm)
//					 final_size		  => pointer to storage to receive uncompressed sound size (output parm)
//              header          => pointer to a WAVEFORMATEX structure
//					 si				  => sound_info structure, contains details on the sound format
//					 flags			  => buffer properties ( DS_HARDWARE , DS_3D )
//
// returns:     -1           => sound effect could not loaded into a secondary buffer
//               0           => sound effect successfully loaded into a secondary buffer
//
//
// NOTE: this function is slow, especially when sounds are loaded into hardware.  Don't call this
// function from within gameplay.
//
int ds_load_buffer(int *sid, int *hid, int *final_size, void *header, sound_info *si, int flags)
{
#if defined(PREPROC_ENABLED_SOUND)
	Assert( final_size != NULL );
	Assert( header != NULL );
	Assert( si != NULL );
	Assert( si->data != NULL );
	Assert( si->size > 0 );
	Assert( si->sample_rate > 0);
	Assert( si->bits > 0 );
	Assert( si->n_channels > 0 );
	Assert( si->n_block_align >= 0 );
	Assert( si->avg_bytes_per_sec > 0 );

	WAVEFORMATEX	*pwfx = (WAVEFORMATEX *)header;
	DSBUFFERDESC	BufferDesc;
	WAVEFORMATEX	WaveFormat;
	HRESULT			DSReturn;
	int				final_sound_size, DSOUND_load_buffer_result = 0;
	BYTE				*pData, *pData2;
	DWORD				DataSize, DataSize2;

	// the below two covnert_ variables are only used when the wav format is not 
	// PCM.  DirectSound only takes PCM sound data, so we must convert to PCM if required
	ubyte *convert_buffer = NULL;		// storage for converted wav file 
	
	// Ensure DirectSound initialized
	if (!ds_initialized) {
		DSOUND_load_buffer_result = -1;
		goto DSOUND_load_buffer_done;
	}

	// Set up buffer information
	WaveFormat.wFormatTag		= (unsigned short)si->format;
	WaveFormat.nChannels			= (unsigned short)si->n_channels;
	WaveFormat.nSamplesPerSec	= si->sample_rate;
	WaveFormat.wBitsPerSample	= (unsigned short)si->bits;
	WaveFormat.cbSize				= 0;
	WaveFormat.nBlockAlign		= (unsigned short)si->n_block_align;
	WaveFormat.nAvgBytesPerSec = si->avg_bytes_per_sec;

	final_sound_size = si->size;	// assume this format will be used, may be over-ridded by convert_len

//	Assert(WaveFormat.nChannels == 1);

	switch (si->format) {
	case WAVE_FORMAT_PCM:
		break;

#if defined(PREPROC_ENABLED_SOUND)
		case WAVE_FORMAT_ADPCM: // convert!
		{
			int	convert_len;					// num bytes of converted wav file
			uint	src_bytes_used;				// number of source bytes actually converted (should always be equal to original size)
			int				rc;

			nprintf(("Sound", "SOUND ==> converting sound from ADPCM to PCM\n"));
			rc = ACM_convert_ADPCM_to_PCM(pwfx, si->data, si->size, &convert_buffer, 0, &convert_len, &src_bytes_used, 8);
			if (rc == -1) {
				DSOUND_load_buffer_result = -1;
				goto DSOUND_load_buffer_done;
			}
			if (src_bytes_used != si->size) {
				Int3();	// ACM conversion failed?
				DSOUND_load_buffer_result = -1;
				goto DSOUND_load_buffer_done;
			}

			final_sound_size = convert_len;

			// Set up the WAVEFORMATEX structure to have the right PCM characteristics
			WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
			WaveFormat.nChannels = (unsigned short)si->n_channels;
			WaveFormat.nSamplesPerSec = si->sample_rate;
			WaveFormat.wBitsPerSample = 8;
			WaveFormat.cbSize = 0;
			WaveFormat.nBlockAlign = (unsigned short)((WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8);
			WaveFormat.nAvgBytesPerSec = WaveFormat.nBlockAlign * WaveFormat.nSamplesPerSec;

			nprintf(("Sound", "SOUND ==> Coverted sound from ADPCM to PCM successfully\n"));
			break;
		}
#endif

		default:
			nprintf(( "Sound", "Unsupported sound encoding\n" ));
			DSOUND_load_buffer_result = -1;
			goto DSOUND_load_buffer_done;
			break;
	}

	WaveFormat.wFormatTag = WAVE_FORMAT_PCM;		// DirectSound only used PCM wave files

	// Set up a DirectSound buffer
	ZeroMemory(&BufferDesc, sizeof(BufferDesc));
	BufferDesc.dwSize = sizeof(BufferDesc);
	BufferDesc.dwBufferBytes = final_sound_size;
	BufferDesc.lpwfxFormat = &WaveFormat;

	// check if DirectSound3D is enabled and the sound is flagged for 3D
	if ((ds_using_ds3d()) && (flags & DS_USE_DS3D)) {
//	if (ds_using_ds3d()) {
		BufferDesc.dwFlags = DSBCAPS_STATIC | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
	} else {
		BufferDesc.dwFlags = DSBCAPS_STATIC | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_LOCSOFTWARE;
	}

	// Create a new software buffer using the settings for this wave
	// All sounds are required to have a software buffer
	*sid = ds_get_sid();
	if ( *sid == -1 ) {
		nprintf(("Sound","SOUND ==> No more software secondary buffers available\n"));
		return -1;
	}
	DSReturn = pDirectSound->CreateSoundBuffer(&BufferDesc, &ds_software_buffers[*sid].pdsb, NULL );

	if ( DSReturn == DS_OK && ds_software_buffers[*sid].pdsb != NULL )	{

		ds_software_buffers[*sid].desc = BufferDesc;
		ds_software_buffers[*sid].wfx =	*BufferDesc.lpwfxFormat;

		// Lock the buffer and copy in the data
		if ((ds_software_buffers[*sid].pdsb)->Lock(0, final_sound_size, (void**)(&pData), &DataSize, (void**)(&pData2), &DataSize2, 0) == DS_OK)	{

			if ( convert_buffer )
				memcpy(pData, convert_buffer, final_sound_size); // use converted data (PCM format)
			else
				memcpy(pData, si->data, final_sound_size);

			(ds_software_buffers[*sid].pdsb)->Unlock(pData, DataSize, 0, 0);
		}
		DSOUND_load_buffer_result = 0;

		// update ram used for sound
		Snd_sram += final_sound_size;
		*final_size = final_sound_size;
	}
	else {
		nprintf(("Sound","SOUND => fatal error in DSOUND_load_buffer\n"));
		*sid = -1;
		DSOUND_load_buffer_result = -1;
	}

	DSOUND_load_buffer_done:
	if ( convert_buffer )
		free( convert_buffer );

	return DSOUND_load_buffer_result;
#else
	return 0;
#endif
}

// ---------------------------------------------------------------------------------------
// ds_init_channels()
//
// init the Channels[] array
//
void ds_init_channels()
{
	int i;

	// detect how many channels we can support
	DSCAPS caps;
	ds_get_soundcard_caps(&caps);

//	caps.dwSize = sizeof(DSCAPS);
//	pDirectSound->GetCaps(&caps);
	
	// minimum 16 channels
	MAX_CHANNELS = caps.dwMaxHwMixingStaticBuffers;
	if (MAX_CHANNELS < 16) {
		MAX_CHANNELS = 16;
	}

	// allocate the channels array
	Channels = (channel*) malloc(sizeof(channel) * MAX_CHANNELS);
	if (Channels == NULL) {
		Error(LOCATION, "Unable to allocate %d bytes for %d audio channels.", sizeof(channel) * MAX_CHANNELS, MAX_CHANNELS);
	}

	// init the channels
	for ( i = 0; i < MAX_CHANNELS; i++ ) {
		Channels[i].pdsb = NULL;
		Channels[i].pds3db = NULL;
		Channels[i].vol = 0;
	}

	mprintf(("** MAX_CHANNELS set to %d\n", MAX_CHANNELS));
}

// ---------------------------------------------------------------------------------------
// ds_init_software_buffers()
//
// init the software buffers
//
void ds_init_software_buffers()
{
	int i;

	for ( i = 0; i < MAX_DS_SOFTWARE_BUFFERS; i++ ) {
		ds_software_buffers[i].pdsb = NULL;
	}
}

// ---------------------------------------------------------------------------------------
// ds_init_hardware_buffers()
//
// init the hardware buffers
//
void ds_init_hardware_buffers()
{
	int i;

	for ( i = 0; i < MAX_DS_HARDWARE_BUFFERS; i++ ) {
		ds_hardware_buffers[i].pdsb = NULL;
	}
}

// ---------------------------------------------------------------------------------------
// ds_init_buffers()
//
// init the both the software and hardware buffers
//
void ds_init_buffers()
{
	ds_init_software_buffers();
	ds_init_hardware_buffers();
}

// Get the current soundcard capabilities
void ds_get_soundcard_caps(DSCAPS *dscaps)
{
	HRESULT	hr;
	int		n_hbuffers, hram;

	dscaps->dwSize = sizeof(DSCAPS);

	hr = pDirectSound->GetCaps(dscaps); 
	if (hr != DS_OK )	{
		nprintf(("Sound","SOUND ==> DirectSound GetCaps() failed with code %s\n.",get_DSERR_text(hr) ));
		return;
	}
	
	n_hbuffers = dscaps->dwMaxHwMixingStaticBuffers;
	hram = dscaps->dwTotalHwMemBytes;
	
	if ( !(dscaps->dwFlags & DSCAPS_CERTIFIED) ) {
		nprintf(("Sound","SOUND ==> Warning: audio driver is not Microsoft certified.\n"));
	}
}

// ---------------------------------------------------------------------------------------
// ds_show_caps()
//
// init the both the software and hardware buffers
//
void ds_show_caps(DSCAPS *dscaps)
{
	nprintf(("Sound", "SOUND => Soundcard Capabilities:\n"));
	nprintf(("Sound", "================================\n"));
	nprintf(("Sound", "Number of primary buffers: %d\n", dscaps->dwPrimaryBuffers ));
	nprintf(("Sound", "Number of total hw mixing buffers: %d\n", dscaps->dwMaxHwMixingAllBuffers ));
	nprintf(("Sound", "Number of total hw mixing static buffers: %d\n", dscaps->dwMaxHwMixingStaticBuffers ));
	nprintf(("Sound", "Number of total hw mixing streaming buffers: %d\n", dscaps->dwMaxHwMixingStreamingBuffers ));
	nprintf(("Sound", "Number of free hw mixing buffers: %d\n", dscaps->dwFreeHwMixingAllBuffers ));
	nprintf(("Sound", "Number of free hw mixing static buffers: %d\n", dscaps->dwFreeHwMixingStaticBuffers ));
	nprintf(("Sound", "Number of free hw mixing streaming buffers: %d\n", dscaps->dwFreeHwMixingStreamingBuffers ));
	nprintf(("Sound", "Number of hw 3D buffers: %d\n", dscaps->dwMaxHw3DAllBuffers ));
	nprintf(("Sound", "Number of hw 3D static buffers: %d\n", dscaps->dwMaxHw3DStaticBuffers ));
	nprintf(("Sound", "Number of hw 3D streaming buffers: %d\n", dscaps->dwMaxHw3DStreamingBuffers ));
	nprintf(("Sound", "Number of free hw 3D buffers: %d\n", dscaps->dwFreeHw3DAllBuffers ));
	nprintf(("Sound", "Number of free hw static 3D buffers: %d\n", dscaps->dwFreeHw3DStaticBuffers ));
	nprintf(("Sound", "Number of free hw streaming 3D buffers: %d\n", dscaps->dwFreeHw3DStreamingBuffers ));
	nprintf(("Sound", "Number of total hw bytes: %d\n", dscaps->dwTotalHwMemBytes ));
	nprintf(("Sound", "Number of free hw bytes: %d\n", dscaps->dwFreeHwMemBytes ));
	nprintf(("Sound", "================================\n"));
}

// Fill in the waveformat struct with the primary buffer characteristics.
void ds_get_primary_format(WAVEFORMATEX *wfx)
{
	// Set 16 bit / 22KHz / mono
	wfx->wFormatTag = WAVE_FORMAT_PCM;
	wfx->nChannels = 2;
	wfx->nSamplesPerSec = 22050;
	wfx->wBitsPerSample = 16;
	wfx->cbSize = 0;
	wfx->nBlockAlign = (unsigned short)(wfx->nChannels * (wfx->wBitsPerSample / 8));
	wfx->nAvgBytesPerSec = wfx->nBlockAlign * wfx->nSamplesPerSec;
}
//XSTR:OFF
// obtain the function pointers from the dsound.dll
void ds_dll_get_functions()
{
	extern HRESULT(__stdcall *pfn_DirectSoundCaptureCreate)(LPGUID lpGUID, LPDIRECTSOUNDCAPTURE *lplpDSC, LPUNKNOWN pUnkOuter);

	pfn_DirectSoundCreate = (HRESULT(__stdcall *)(LPGUID lpGuid, LPDIRECTSOUND *ppDS, IUnknown FAR *pUnkOuter))GetProcAddress(Ds_dll_handle,"DirectSoundCreate");
	pfn_DirectSoundCaptureCreate = (HRESULT(__stdcall *)(LPGUID lpGuid, LPDIRECTSOUNDCAPTURE *lplpDSC, IUnknown FAR *pUnkOuter))GetProcAddress(Ds_dll_handle,"DirectSoundCaptureCreate");
}

// Load the dsound.dll, and get funtion pointers
// exit:	0	->	dll loaded successfully
//			!0	->	dll could not be loaded
int ds_dll_load()
{
	if ( !Ds_dll_loaded ) {
		Ds_dll_handle = LoadLibraryA("dsound.dll");
		if ( !Ds_dll_handle ) {
			return -1;
		}
		ds_dll_get_functions();
		Ds_dll_loaded=1;
	}
	return 0;
}


// Initialize the property set interface.
//
// returns: 0 if successful, otherwise -1.  If successful, the global pPropertySet will
//          set to a non-NULL value.
//
int ds_init_property_set()
{
#if defined(PREPROC_ENABLED_DS)
	HRESULT hr;

	// Create the secondary buffer required for EAX initialization
	WAVEFORMATEX wf;
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = 1;
	wf.nSamplesPerSec = 22050;
	wf.wBitsPerSample = 16;
	wf.cbSize = 0;
	wf.nBlockAlign = (unsigned short)(wf.nChannels * (wf.wBitsPerSample / 8));
	wf.nAvgBytesPerSec = wf.nBlockAlign * wf.nSamplesPerSec;

	DSBUFFERDESC dsbd;
	ZeroMemory(&dsbd, sizeof(dsbd));
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_CTRLDEFAULT | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_STATIC | DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
	dsbd.dwBufferBytes = 3 * wf.nAvgBytesPerSec;
	dsbd.lpwfxFormat = &wf;

	// Create a new buffer using the settings for this wave
	hr = pDirectSound->CreateSoundBuffer(&dsbd, &Ds_property_set_pdsb, NULL);
	if (FAILED(hr)) {
		pPropertySet = NULL;
		return -1;
	}

	// Get the 3D interface from the secondary buffer, which is used to query the EAX interface
	hr = Ds_property_set_pdsb->QueryInterface(IID_IDirectSound3DBuffer, (void**)&Ds_property_set_pds3db);
	if (FAILED(hr)) {
		Ds_property_set_pds3db = NULL;
		return -1;
	}

	Assert(Ds_property_set_pds3db != NULL);
	hr = Ds_property_set_pds3db->QueryInterface(IID_IKsPropertySet, (void**)&pPropertySet);
	if ((FAILED(hr)) || (pPropertySet == NULL)) {
		return -1;
	}
#endif
	return 0;
}

// ---------------------------------------------------------------------------------------
// ds_init()
//
// returns:     -1           => init failed
//               0           => init success
int ds_init(int use_a3d, int use_eax)
{
#if defined(PREPROC_ENABLED_DS)
	HRESULT			hr;
	HWND				hwnd;
	WAVEFORMATEX	wave_format;
	DSBUFFERDESC	BufferDesc;

	nprintf(( "Sound", "SOUND ==> Initializing DirectSound...\n" ));

	hwnd = (HWND)os_get_window();
	if ( hwnd == NULL )	{
		nprintf(( "Sound", "SOUND ==> No window handle, so no sound...\n" ));
		return -1;
	}

	if ( ds_dll_load() == -1 ) {
		return -1;
	}

	pDirectSound = NULL;

	Ds_use_eax = use_eax;

	if (Ds_use_eax) {
		Ds_use_ds3d = 1;
	}

	/*
	if (Ds_use_eax) {
		Ds_use_eax = 0;
	}
	*/	
	
	if (!pfn_DirectSoundCreate) {
		nprintf(( "Sound", "SOUND ==> Could not get DirectSoundCreate function pointer\n" ));
		return -1;
	}

	hr = pfn_DirectSoundCreate(NULL, &pDirectSound, NULL);
	if (FAILED(hr)) {
		return -1;
	}

	// Set up DirectSound for exclusive mode, so we can change the primary buffer if we want to.	
	hr = pDirectSound->SetCooperativeLevel(hwnd, DSSCL_EXCLUSIVE);
	if (hr != DS_OK) {
		nprintf(("Sound","SOUND ==> DirectSound pDirectSound->SetCooperativeLevel failed with code %s\n.",get_DSERR_text(hr) ));
		pDirectSound = NULL;	
		return -1;
	}

	// Create the primary buffer
	ZeroMemory(&BufferDesc, sizeof(BufferDesc));
	BufferDesc.dwSize = sizeof(BufferDesc);

	ds_get_soundcard_caps(&Soundcard_caps);

	if (Ds_use_ds3d) {
		BufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D;

		hr = pDirectSound->CreateSoundBuffer(&BufferDesc, &pPrimaryBuffer, 0);
		if (hr != DS_OK) {
			nprintf(("Sound","SOUND ==> Primary Buffer create failed with DSBCAPS_CTRL3D property... disabling DirectSound3D\n"));
			Ds_use_ds3d = 0;
			Ds_use_eax = 0;			
		} else {
			nprintf(("Sound","SOUND ==> Primary Buffer created with DirectSound3D enabled\n"));
		}
	}

	// If not using DirectSound3D, then create a normal primary buffer
	if (Ds_use_ds3d == 0) {
		BufferDesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
		hr = pDirectSound->CreateSoundBuffer(&BufferDesc, &pPrimaryBuffer, 0);
		if (hr != DS_OK) {
			nprintf(("Sound","SOUND ==> Primary Buffer create failed with error: %s\n",get_DSERR_text(hr) ));
			pDirectSound = NULL;	
			return -1;
		}
		else {
			nprintf(("Sound","SOUND ==> Primary Buffer created with without DirectSound3D enabled\n"));
		}
	}

	// Get the primary buffer format
	ds_get_primary_format(&wave_format);

	hr = pPrimaryBuffer->SetFormat(&wave_format);
	if (hr != DS_OK) {
		nprintf(("Sound","SOUND ==> pPrimaryBuffer->SetFormat() failed with code %s\n",get_DSERR_text(hr) ));
	}

	pPrimaryBuffer->GetFormat(&wave_format, sizeof(wave_format), NULL);
	nprintf(("Sound","SOUND ==> Primary Buffer forced to: rate: %d Hz bits: %d n_channels: %d\n",
			wave_format.nSamplesPerSec, wave_format.wBitsPerSample, wave_format.nChannels));

	// start the primary buffer playing.  This will reduce sound latency when playing a sound
	// if no other sounds are playing.
	hr = pPrimaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
	if (hr != DS_OK) {
		nprintf(("Sound","SOUND ==> pPrimaryBuffer->Play() failed with code %s\n",get_DSERR_text(hr) ));
	}

	// Initialize DirectSound3D.  Since software performance of DirectSound3D is unacceptably
	// slow, we require the voice manger (a DirectSound extension) to be present.  The 
	// exception is when A3D is being used, since A3D has a resource manager built in.
	if (Ds_use_ds3d) {
		int vm_required = 1;	// voice manager		

		if (ds3d_init(vm_required) != 0) {
			Ds_use_ds3d = 0;
			//Ds_use_eax = 0;
		}
	}

	if (Ds_use_eax == 1) {
		ds_init_property_set();
		if (ds_eax_init() != 0) {
			Ds_use_eax = 0;
		}
	}

	ds_build_vol_lookup();
	ds_init_channels();
	ds_init_buffers();

	ds_show_caps(&Soundcard_caps);
#endif
	return 0;
}

// ---------------------------------------------------------------------------------------
// get_DSERR_text()
//
// returns the text equivalent for the a DirectSound DSERR_ code
//
char *get_DSERR_text(int DSResult)
{
	switch( DSResult ) {

		case DS_OK:
			return "DS_OK";
			break;

		case DSERR_ALLOCATED:
			return "DSERR_ALLOCATED";
			break;

		case DSERR_ALREADYINITIALIZED:
			return "DSERR_ALREADYINITIALIZED";
			break;

		case DSERR_BADFORMAT:
			return "DSERR_BADFORMAT";
			break;

		case DSERR_BUFFERLOST:
			return "DSERR_BUFFERLOST";
			break;

		case DSERR_CONTROLUNAVAIL:
			return "DSERR_CONTROLUNAVAIL";
			break;

		case DSERR_GENERIC:
			return "DSERR_GENERIC";
			break;

		case DSERR_INVALIDCALL:
			return "DSERR_INVALIDCALL";
			break;

		case DSERR_INVALIDPARAM:
			return "DSERR_INVALIDPARAM";
			break;

		case DSERR_NOAGGREGATION:
			return "DSERR_NOAGGREGATION";
			break;

		case DSERR_NODRIVER:
			return "DSERR_NODRIVER";
			break;

		case DSERR_OUTOFMEMORY:
			return "DSERR_OUTOFMEMORY";
			break;

		case DSERR_OTHERAPPHASPRIO:
			return "DSERR_OTHERAPPHASPRIO";
			break;

		case DSERR_PRIOLEVELNEEDED:
			return "DSERR_PRIOLEVELNEEDED";
			break;

		case DSERR_UNINITIALIZED:
			return "DSERR_UNINITIALIZED";
			break;

		case DSERR_UNSUPPORTED:
			return "DSERR_UNSUPPORTED";
			break;

		default:
			return "unknown";
			break;
	}
}


// ---------------------------------------------------------------------------------------
// ds_close_channel()
//
// Free a single channel
//
void ds_close_channel(int i)
{
	HRESULT	hr;

	// If a 3D interface exists, free it
	if ( Channels[i].pds3db != NULL ) {

		{
			int attempts = 0;
			while(++attempts < 10) {
				hr = Channels[i].pds3db->Release();
				if ( hr == DS_OK ) {
					break;
				} else {
					// nprintf(("Sound", "SOUND ==> Channels[channel].pds3db->Release() failed with return value %s\n", get_DSERR_text(second_hr) ));
				}
			}

			Channels[i].pds3db = NULL;
		}
	}

	if ( Channels[i].pdsb != NULL ) {
		// If a 2D interface exists, free it
		if ( Channels[i].pdsb != NULL ) {
			int attempts = 0;
			while(++attempts < 10) {
				hr = Channels[i].pdsb->Release();
				if ( hr == DS_OK ) {
					break;
				} else {
					nprintf(("Sound", "SOUND ==> Channels[channel].pdsb->Release() failed with return value %s\n", get_DSERR_text(hr) ));
				}
			}
		}

		Channels[i].pdsb = NULL;
	}
}



// ---------------------------------------------------------------------------------------
// ds_close_all_channels()
//
// Free all the channel buffers
//
void ds_close_all_channels()
{
	int		i;

	for (i = 0; i < MAX_CHANNELS; i++)	{
		ds_close_channel(i);
	}
}

// ---------------------------------------------------------------------------------------
// ds_unload_buffer()
//
//
void ds_unload_buffer(int sid, int hid)
{
	HRESULT	hr;

	if ( sid != -1 ) {
		if ( ds_software_buffers[sid].pdsb != NULL ) {
			hr = ds_software_buffers[sid].pdsb->Release();
			if ( hr != DS_OK ) {
				Int3();
				nprintf(("Sound", "SOUND ==> ds_software_buffers[sid]->Release() failed with return value %s\n", get_DSERR_text(hr) ));
			}
			ds_software_buffers[sid].pdsb = NULL;
		}
	}

	if ( hid != -1 ) {
		if ( ds_hardware_buffers[hid].pdsb != NULL ) {
			hr = ds_hardware_buffers[hid].pdsb->Release();
			if ( hr != DS_OK ) {
				Int3();
				nprintf(("Sound", "SOUND ==> ds_hardware_buffers[hid]->Release() failed with return value %s\n", get_DSERR_text(hr) ));
			}
			ds_hardware_buffers[hid].pdsb = NULL;
		}
	}
}

// ---------------------------------------------------------------------------------------
// ds_close_software_buffers()
//
//
void ds_close_software_buffers()
{
	int		i;
	HRESULT	hr;

	for (i = 0; i < MAX_DS_SOFTWARE_BUFFERS; i++)	{
		if ( ds_software_buffers[i].pdsb != NULL ) {
			hr = ds_software_buffers[i].pdsb->Release();
			if ( hr != DS_OK ) {
				Int3();
				nprintf(("Sound", "SOUND ==> ds_software_buffers[i]->Release() failed with return value %s\n", get_DSERR_text(hr) ));
			}
			ds_software_buffers[i].pdsb = NULL;
		}
	}
}

// ---------------------------------------------------------------------------------------
// ds_close_hardware_buffers()
//
//
void ds_close_hardware_buffers()
{
	int		i;
	HRESULT	hr;

	for (i = 0; i < MAX_DS_HARDWARE_BUFFERS; i++)	{
		if ( ds_hardware_buffers[i].pdsb != NULL ) {
			hr = ds_hardware_buffers[i].pdsb->Release();
			if ( hr != DS_OK ) {
				Int3();
				nprintf(("Sound", "SOUND ==> ds_hardware_buffers[i]->Release() failed with return value %s\n", get_DSERR_text(hr) ));
			}
			ds_hardware_buffers[i].pdsb = NULL;
		}
	}
}

// ---------------------------------------------------------------------------------------
// ds_close_buffers()
//
// Free the channel buffers
//
void ds_close_buffers()
{
	ds_close_software_buffers();
	ds_close_hardware_buffers();
}

// ---------------------------------------------------------------------------------------
// ds_close()
//
// Close the DirectSound system
//
void ds_close()
{
	ds_close_all_channels();
	ds_close_buffers();

	if (pPropertySet != NULL) {
		pPropertySet->Release();
		pPropertySet = NULL;
	}

	if (Ds_property_set_pdsb != NULL) {
		Ds_property_set_pdsb->Release();
		Ds_property_set_pdsb = NULL;
	}

	if (Ds_property_set_pds3db != NULL) {
		Ds_property_set_pds3db->Release();
		Ds_property_set_pds3db = NULL;
	}

	if (pPrimaryBuffer)	{
		pPrimaryBuffer->Release();
		pPrimaryBuffer = NULL;
	}

	if (pDirectSound)	{
		pDirectSound->Release();
		pDirectSound = NULL;
	}

	if ( Ds_dll_loaded ) {
		FreeLibrary(Ds_dll_handle);
		Ds_dll_loaded=0;
	}

	if (Ds_must_call_couninitialize == 1) {
		CoUninitialize();
	}

	// free the Channels[] array, since it was dynamically allocated
	free(Channels);
	Channels = NULL;
}

// ---------------------------------------------------------------------------------------
// ds_get_3d_interface()
// 
// Get the 3d interface for a secondary buffer. 
//
// If the secondary buffer wasn't created with a DSBCAPS_CTRL3D flag, then no 3d interface
// exists
//
void ds_get_3d_interface(LPDIRECTSOUNDBUFFER pdsb, LPDIRECTSOUND3DBUFFER *ppds3db)
{
#if defined(PREPROC_ENABLED_DS)

	DSBCAPS			dsbc;
	HRESULT			DSResult;

	dsbc.dwSize = sizeof(dsbc);
	DSResult = pdsb->GetCaps(&dsbc);
	if ( DSResult == DS_OK && dsbc.dwFlags & DSBCAPS_CTRL3D ) {
		DSResult = pdsb->QueryInterface( IID_IDirectSound3DBuffer, (void**)ppds3db );
		if ( DSResult != DS_OK ) {
			nprintf(("SOUND","Could not obtain 3D interface for hardware buffer: %s\n", get_DSERR_text(DSResult) ));
		}
	}
#endif
}


// ---------------------------------------------------------------------------------------
// ds_get_free_channel()
// 
// Find a free channel to play a sound on.  If no free channels exists, free up one based
// on volume levels.
//
//	input:		new_volume	=>		volume in DS units for sound to play at
//					snd_id		=>		which kind of sound to play
//					priority		=>		DS_MUST_PLAY
//											DS_LIMIT_ONE
//											DS_LIMIT_TWO
//											DS_LIMIT_THREE
//
//	returns:		channel number to play sound on
//					-1 if no channel could be found
//
// NOTE:	snd_id is needed since we limit the number of concurrent samples
//
//
#define DS_MAX_SOUND_INSTANCES 2

int ds_get_free_channel(int new_volume, int snd_id, int priority)
{
	int				i, first_free_channel, limit;
	int				lowest_vol = 0, lowest_vol_index = -1;
	int				instance_count;	// number of instances of sound already playing
	int				lowest_instance_vol, lowest_instance_vol_index;
	unsigned long	status;
	HRESULT			hr;
	channel			*chp;

	instance_count = 0;
	lowest_instance_vol = 99;
	lowest_instance_vol_index = -1;
	first_free_channel = -1;

	// Look for a channel to use to play this sample
	for ( i = 0; i < MAX_CHANNELS; i++ )	{
		chp = &Channels[i];
		if ( chp->pdsb == NULL ) {
			if ( first_free_channel == -1 )
				first_free_channel = i;
			continue;
		}

		hr = chp->pdsb->GetStatus(&status);
		if ( hr != DS_OK ) {
			nprintf(("Sound", "SOUND ==> GetStatus failed with return value %s\n", get_DSERR_text(hr) ));
			return -1;
		}
		if ( !(status & DSBSTATUS_PLAYING) ) {
			if ( first_free_channel == -1 )
				first_free_channel = i;
			ds_close_channel(i);
			continue;
		}
		else {
			if ( chp->snd_id == snd_id ) {
				instance_count++;
				if ( chp->vol < lowest_instance_vol && chp->looping == FALSE ) {
					lowest_instance_vol = chp->vol;
					lowest_instance_vol_index = i;
				}
			}

			if ( chp->vol < lowest_vol && chp->looping == FALSE ) {
				lowest_vol_index = i;
				lowest_vol = chp->vol;
			}
		}
	}

	// determine the limit of concurrent instances of this sound
	switch(priority) {
		case DS_MUST_PLAY:
			limit = 100;
			break;
		case DS_LIMIT_ONE:
			limit = 1;
			break;
		case DS_LIMIT_TWO:
			limit = 2;
			break;
		case DS_LIMIT_THREE:
			limit = 3;
			break;
		default:
			Int3();			// get Alan
			limit = 100;
			break;
	}


	// If we've exceeded the limit, then maybe stop the duplicate if it is lower volume
	if ( instance_count >= limit ) {
		// If there is a lower volume duplicate, stop it.... otherwise, don't play the sound
		if ( lowest_instance_vol_index >= 0 && (Channels[lowest_instance_vol_index].vol <= new_volume) ) {
			ds_close_channel(lowest_instance_vol_index);
			first_free_channel = lowest_instance_vol_index;
		} else {
			first_free_channel = -1;
		}
	} else {
		// there is no limit barrier to play the sound, so see if we've ran out of channels
		if ( first_free_channel == -1 ) {
			// stop the lowest volume instance to play our sound if priority demands it
			if ( lowest_vol_index != -1 && priority == DS_MUST_PLAY ) {
				// Check if the lowest volume playing is less than the volume of the requested sound.
				// If so, then we are going to trash the lowest volume sound.
				if ( Channels[lowest_vol_index].vol <= new_volume ) {
					ds_close_channel(lowest_vol_index);
					first_free_channel = lowest_vol_index;
				}
			}
		}
	}

	return first_free_channel;
}


// ---------------------------------------------------------------------------------------
// ds_channel_dup()
// 
// Find a free channel to play a sound on.  If no free channels exists, free up one based
// on volume levels.
//
// returns:		0		=>		dup was successful
//					-1		=>		dup failed (Channels[channel].pdsb will be NULL)
//
int ds_channel_dup(LPDIRECTSOUNDBUFFER pdsb, int channel, int use_ds3d)
{
	HRESULT DSResult;

	// Duplicate the master buffer into a channel buffer.
	DSResult = pDirectSound->DuplicateSoundBuffer(pdsb, &Channels[channel].pdsb );
	if ( DSResult != DS_OK ) {
		nprintf(("Sound", "SOUND ==> DuplicateSoundBuffer failed with return value %s\n", get_DSERR_text(DSResult) ));
		Channels[channel].pdsb = NULL;
		return -1;
	}

	// get the 3d interface for the buffer if it exists
	if ( use_ds3d ) {
		if (Channels[channel].pds3db == NULL) {
			ds_get_3d_interface(Channels[channel].pdsb, &Channels[channel].pds3db);
		}
	}
	
	return 0;
}


// ---------------------------------------------------------------------------------------
// ds_restore_buffer()
// 
//
void ds_restore_buffer(LPDIRECTSOUNDBUFFER pdsb)
{
	HRESULT hr;
	
	Int3();	// get Alan, he wants to see this
	hr = pdsb->Restore();
	if ( hr != DS_OK ) {
		nprintf(("Sound", "Sound ==> Lost a buffer, tried restoring but got %s\n", get_DSERR_text(hr) ));
	}
}

// Create a direct sound buffer in software, without locking any data in
int ds_create_buffer(int frequency, int bits_per_sample, int nchannels, int nseconds)
{
	HRESULT			dsrval;
	DSBUFFERDESC	dsbd;
	WAVEFORMATEX	wfx;
	int				sid;

	if (!ds_initialized) {
		return -1;
	}

	sid = ds_get_sid();
	if ( sid == -1 ) {
		nprintf(("Sound","SOUND ==> No more software secondary buffers available\n"));
		return -1;
	}

	// Set up buffer format
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = (unsigned short)nchannels;
	wfx.nSamplesPerSec = frequency;
	wfx.wBitsPerSample = (unsigned short)bits_per_sample;
	wfx.cbSize = 0;
	wfx.nBlockAlign = (unsigned short)(wfx.nChannels * (wfx.wBitsPerSample / 8));
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

	memset(&dsbd, 0, sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwBufferBytes = wfx.nAvgBytesPerSec * nseconds;
	dsbd.lpwfxFormat = &wfx;
	dsbd.dwFlags = DSBCAPS_STATIC | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLDEFAULT | DSBCAPS_LOCSOFTWARE;

	dsrval = pDirectSound->CreateSoundBuffer(&dsbd, &ds_software_buffers[sid].pdsb, NULL);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	ds_software_buffers[sid].desc = dsbd;
	return sid;
}

// Lock data into an existing buffer
int ds_lock_data(int sid, unsigned char *data, int size)
{
	HRESULT					dsrval;
	LPDIRECTSOUNDBUFFER	pdsb;
	DSBCAPS					caps;
	void						*buffer_data, *buffer_data2;
	DWORD						buffer_size, buffer_size2;

	Assert(sid >= 0);
	pdsb = ds_software_buffers[sid].pdsb;

	memset(&caps, 0, sizeof(DSBCAPS));
	caps.dwSize = sizeof(DSBCAPS);
	dsrval = pdsb->GetCaps(&caps);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	pdsb->SetCurrentPosition(0);

	// lock the entire buffer
	dsrval = pdsb->Lock(0, caps.dwBufferBytes, &buffer_data, &buffer_size, &buffer_data2, &buffer_size2, 0 );
	if ( dsrval != DS_OK ) {
		return -1;
	}

	// first clear it out with silence
	memset(buffer_data, 0x80, buffer_size);
	memcpy(buffer_data, data, size);

	dsrval = pdsb->Unlock(buffer_data, buffer_size, 0, 0);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	return 0;
}

// Stop a buffer from playing directly
void ds_stop_easy(int sid)
{
	HRESULT					dsrval;
	LPDIRECTSOUNDBUFFER	pdsb;

	Assert(sid >= 0);
	pdsb = ds_software_buffers[sid].pdsb;
	dsrval = pdsb->Stop();
}

//	Play a sound without the usual baggage (used for playing back real-time voice)
//
// parameters:  
//					sid			=> software id of sound
//					volume      => volume of sound effect in DirectSound units
int ds_play_easy(int sid, int volume)
{
	HRESULT					dsrval;
	LPDIRECTSOUNDBUFFER	pdsb;

	Assert(sid >= 0);
	pdsb = ds_software_buffers[sid].pdsb;

	pdsb->SetVolume(volume);
	dsrval=pdsb->Play(0, 0, 0);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	return 0;
}

//extern void HUD_add_to_scrollback(char *text, int source);
//extern void HUD_printf(char *format, ...);

// ---------------------------------------------------------------------------------------
// Play a DirectSound secondary buffer.  
// 
//
// parameters:  
//					sid			=> software id of sound
//					hid			=> hardware id of sound ( -1 if not in hardware )
//					snd_id		=>	what kind of sound this is
//					priority		=>		DS_MUST_PLAY
//											DS_LIMIT_ONE
//											DS_LIMIT_TWO
//											DS_LIMIT_THREE
//					volume      => volume of sound effect in DirectSound units
//					pan         => pan of sound in DirectSound units
//             looping     => whether the sound effect is looping or not
//
// returns:    -1          => sound effect could not be started
//              >=0        => sig for sound effect successfully started
//
int ds_play(int sid, int hid, int snd_id, int priority, int volume, int pan, int looping, bool is_voice_msg)
{
	int				channel;
	HRESULT			DSResult;

	if (!ds_initialized)
		return -1;

	channel = ds_get_free_channel(volume, snd_id, priority);

	if (channel > -1)	{
		if ( Channels[channel].pdsb != NULL ) {
			return -1;
		}

		// First check if the sound is in hardware, and try to duplicate from there
		if ( hid != -1 ) {
			Int3();
			if ( ds_channel_dup(ds_hardware_buffers[hid].pdsb, channel, 0) == 0 ) {
//				nprintf(("Sound", "SOUND ==> Played sound in hardware..\n"));
			}
		}

		// Channel will be NULL if hardware dup failed, or there was no hardware dup attempted
		if ( Channels[channel].pdsb == NULL ) {
			if ( ds_channel_dup(ds_software_buffers[sid].pdsb, channel, 0) == 0 ) {
//				nprintf(("Sound", "SOUND ==> Played sound in software..\n"));
			}
		}
	
		if ( Channels[channel].pdsb == NULL ) {
			return -1;
		}

		if ( ds_using_ds3d() ) {
			if ( ds_is_3d_buffer(Channels[channel].pdsb) ) {
				if (Channels[channel].pds3db == NULL) {
					ds_get_3d_interface(Channels[channel].pdsb, &Channels[channel].pds3db);
				}
				if ( Channels[channel].pds3db ) {
					Channels[channel].pds3db->SetMode(DS3DMODE_DISABLE,DS3D_IMMEDIATE);
				}
			}
		}

		// Actually play it
		Channels[channel].vol = volume;
		Channels[channel].looping = looping;
		Channels[channel].priority = priority;
    	Channels[channel].pdsb->SetPan(pan);
		Channels[channel].pdsb->SetVolume(volume);
		Channels[channel].is_voice_msg = is_voice_msg;

		int ds_flags = 0;
		if ( looping )
			ds_flags |= DSBPLAY_LOOPING;
		
		DSResult = Channels[channel].pdsb->Play(0, 0, ds_flags );

		/*
		if (Stop_logging_sounds == false) {
			char buf[256];
			sprintf(buf, "channel %d, address: %x, ds_flags: %d", channel, Channels[channel].pdsb, ds_flags);
			HUD_add_to_scrollback(buf, 3);
		}
		*/

		if ( DSResult == DSERR_BUFFERLOST ) {
			ds_restore_buffer(Channels[channel].pdsb);
			DSResult = Channels[channel].pdsb->Play(0, 0, ds_flags );
		}

		if ( DSResult != DS_OK ) {
			nprintf(("Sound", "Sound ==> Play failed with return value %s\n", get_DSERR_text(DSResult) ));
			return -1;
		}
	}
	else {
//		nprintf(( "Sound", "SOUND ==> Not playing sound requested at volume %.2f\n", ds_get_percentage_vol(volume) ));
		return -1;
	}

	Channels[channel].snd_id = snd_id;
	Channels[channel].sig = channel_next_sig++;
	if (channel_next_sig < 0 ) {
		channel_next_sig = 1;
	}

	/*
	if (Stop_logging_sounds == false) {
		if (is_voice_msg) {
			char buf[256];
			sprintf(buf, "VOICE sig: %d, sid: %d, snd_id: %d, ch: %d", Channels[channel].sig, sid, snd_id, channel);
			HUD_add_to_scrollback(buf, 3);
		}
	}
	*/

	Channels[channel].last_position = 0;

	// make sure there aren't any looping voice messages
	for (int i=0; i<MAX_CHANNELS; i++) {
		if (Channels[i].is_voice_msg == true) {
			if (Channels[i].pdsb == NULL) {
				continue;
			}

			DWORD current_position = ds_get_play_position(i);
			if (current_position != 0) {
				if (current_position < Channels[i].last_position) {
					ds_close_channel(i);
				} else {
					Channels[i].last_position = current_position;
				}
			}
		}
	}

	return Channels[channel].sig;
}


// ---------------------------------------------------------------------------------------
// ds_get_channel()
//
// Return the channel number that is playing the sound identified by sig.  If that sound is
// not playing, return -1.
//
int ds_get_channel(int sig)
{
	int i;

	for ( i = 0; i < MAX_CHANNELS; i++ ) {
		if ( Channels[i].pdsb && Channels[i].sig == sig ) {
			if ( ds_is_channel_playing(i) == TRUE ) {
				return i;
			}
		}
	}
	return -1;
}

// ---------------------------------------------------------------------------------------
// ds_is_channel_playing()
//
//
int ds_is_channel_playing(int channel)
{
	HRESULT			hr;
	unsigned long	status;		

	if ( !Channels[channel].pdsb ) {
		return 0;
	}

	hr = Channels[channel].pdsb->GetStatus(&status);
	if ( hr != DS_OK ) {
		nprintf(("Sound", "SOUND ==> GetStatus failed with return value %s\n", get_DSERR_text(hr) ));
		return 0;
	}

	if ( status & DSBSTATUS_PLAYING )
		return TRUE;
	else
		return FALSE;
}

// ---------------------------------------------------------------------------------------
// ds_stop_channel()
//
//
void ds_stop_channel(int channel)
{
	ds_close_channel(channel);
}

// ---------------------------------------------------------------------------------------
// ds_stop_channel_all()
//
//	
void ds_stop_channel_all()
{
	int i;

	for ( i=0; i<MAX_CHANNELS; i++ )	{
		if ( Channels[i].pdsb != NULL ) {
			ds_stop_channel(i);
		}
	}
}

// ---------------------------------------------------------------------------------------
// ds_set_volume()
//
//	Set the volume for a channel.  The volume is expected to be in DirectSound units
//
//	If the sound is a 3D sound buffer, this is like re-establishing the maximum 
// volume.
//
void ds_set_volume( int channel, int vol )
{
	HRESULT			hr;
	unsigned long	status;		

	hr = Channels[channel].pdsb->GetStatus(&status);
	if ( hr != DS_OK ) {
		nprintf(("Sound", "SOUND ==> GetStatus failed with return value %s\n", get_DSERR_text(hr) ));
		return;
	}

	if ( status & DSBSTATUS_PLAYING ) {
		Channels[channel].pdsb->SetVolume(vol);
	}
}

// ---------------------------------------------------------------------------------------
// ds_set_pan()
//
//	Set the pan for a channel.  The pan is expected to be in DirectSound units
//
void ds_set_pan( int channel, int pan )
{
	HRESULT			hr;
	unsigned long	status;		

	hr = Channels[channel].pdsb->GetStatus(&status);
	if ( hr != DS_OK ) {
		nprintf(("Sound", "SOUND ==> GetStatus failed with return value %s\n", get_DSERR_text(hr) ));
		return;
	}

	if ( status & DSBSTATUS_PLAYING ) {
		Channels[channel].pdsb->SetPan(pan);
	}
}

// ---------------------------------------------------------------------------------------
// ds_get_pitch()
//
//	Get the pitch of a channel
//
int ds_get_pitch(int channel)
{
	unsigned long	status, pitch = 0;
	HRESULT			hr;

	hr = Channels[channel].pdsb->GetStatus(&status);

	if ( hr != DS_OK ) {
		nprintf(("Sound", "SOUND ==> GetStatus failed with return value %s\n", get_DSERR_text(hr) ));
		return -1;
	}

	if ( status & DSBSTATUS_PLAYING )	{
		hr = Channels[channel].pdsb->GetFrequency(&pitch);
		if ( hr != DS_OK ) {
			nprintf(("Sound", "SOUND ==> GetFrequency failed with return value %s\n", get_DSERR_text(hr) ));
			return -1;
		}
	}

	return (int)pitch;
}

// ---------------------------------------------------------------------------------------
// ds_set_pitch()
//
//	Set the pitch of a channel
//
void ds_set_pitch(int channel, int pitch)
{
	unsigned long	status;
	HRESULT			hr;

	hr = Channels[channel].pdsb->GetStatus(&status);
	if ( hr != DS_OK ) {
		nprintf(("Sound", "SOUND ==> GetStatus failed with return value %s\n", get_DSERR_text(hr) ));
		return;
	}

	if ( pitch < DS_MIN_PITCH )
		pitch = DS_MIN_PITCH;

	if ( pitch > DS_MAX_PITCH)
		pitch = DS_MAX_PITCH;

	if ( status & DSBSTATUS_PLAYING )	{
		Channels[channel].pdsb->SetFrequency((unsigned long)pitch);
	}
}

// ---------------------------------------------------------------------------------------
// ds_chg_loop_status()
//
//	
void ds_chg_loop_status(int channel, int loop)
{
	unsigned long	status;
	HRESULT			hr;

	hr = Channels[channel].pdsb->GetStatus(&status);
	if ( hr != DS_OK ) {
		nprintf(("Sound", "SOUND ==> GetStatus failed with return value %s\n", get_DSERR_text(hr) ));
		return;
	}
	
	if ( !(status & DSBSTATUS_PLAYING) )
		return;		// sound is not playing anymore

	if ( status & DSBSTATUS_LOOPING ) {
		if ( loop )
			return;	// we are already looping
		else {
			// stop the sound from looping
			hr = Channels[channel].pdsb->Play(0,0,0);
		}
	}
	else {
		if ( !loop )
			return;	// the sound is already not looping
		else {
			// start the sound looping
			hr = Channels[channel].pdsb->Play(0,0,DSBPLAY_LOOPING);
		}
	}
}

// ---------------------------------------------------------------------------------------
// ds3d_play()
//
// Starts a ds3d sound playing
// 
//	input:
//
//					sid				=>	software id for sound to play
//					hid				=>	hardware id for sound to play (-1 if not in hardware)
//					snd_id			=> identifies what type of sound is playing
//					pos				=>	world pos of sound
//					vel				=>	velocity of object emitting sound
//					min				=>	distance at which sound doesn't get any louder
//					max				=>	distance at which sound becomes inaudible
//					looping			=>	boolean, whether to loop the sound or not
//					max_volume		=>	volume (-10000 to 0) for 3d sound at maximum
//					estimated_vol	=>	manual estimated volume
//					priority		=>		DS_MUST_PLAY
//											DS_LIMIT_ONE
//											DS_LIMIT_TWO
//											DS_LIMIT_THREE
//
//	returns:			0				=> sound started successfully
//						-1				=> sound could not be played
//
int ds3d_play(int sid, int hid, int snd_id, vector *pos, vector *vel, int min, int max, int looping, int max_volume, int estimated_vol, int priority )
{
	int				channel;
	HRESULT			hr;

	if (!ds_initialized)
		return -1;

	channel = ds_get_free_channel(estimated_vol, snd_id, priority);

	if (channel > -1)	{
		Assert(Channels[channel].pdsb == NULL);

		// First check if the sound is in hardware, and try to duplicate from there
		if ( hid != -1 ) {
			Int3();
			if ( ds_is_3d_buffer(ds_hardware_buffers[hid].pdsb) == FALSE ) {
				nprintf(("Sound", "SOUND ==> Tried to play non-3d buffer in ds3d_play()..\n"));
				return -1;
			}

			if ( ds_channel_dup(ds_hardware_buffers[hid].pdsb, channel, 1) == 0 ) {
				nprintf(("Sound", "SOUND ==> Played sound using DirectSound3D in hardware..\n"));
			}
		}

		// Channel will be NULL if hardware dup failed, or there was no hardware dup attempted
		if ( Channels[channel].pdsb == NULL ) {

/*
			if ( ds_is_3d_buffer(ds_software_buffers[sid].pdsb) == FALSE ) {
				nprintf(("Sound", "SOUND ==> Tried to play non-3d buffer in ds3d_play()..\n"));
				return -1;
			}
*/

			if ( ds_channel_dup(ds_software_buffers[sid].pdsb, channel, 1) == 0 ) {
//				nprintf(("Sound", "SOUND ==> Played sound using DirectSound3D \n"));
			}
		}

		if ( Channels[channel].pdsb == NULL ) {
			return -1;
/*
			DSBUFFERDESC desc;

			desc = ds_software_buffers[sid].desc;
			desc.lpwfxFormat = &ds_software_buffers[sid].wfx;

			// duplicate buffer failed, so call CreateBuffer instead

			hr = pDirectSound->CreateSoundBuffer(&desc, &Channels[channel].pdsb, NULL );
			// lock the data in
			if ( (hr == DS_OK) && (Channels[channel].pdsb) ) {
				BYTE	*pdest, *pdest2;
				BYTE	*psrc, *psrc2;
				DWORD	src_ds_size, dest_ds_size, not_used;
				int	src_size;
			
				if ( ds_get_size(sid, &src_size) != 0 ) {
					Int3();
					Channels[channel].pdsb->Release();
					return -1;
				}

				// lock the src buffer
				hr = ds_software_buffers[sid].pdsb->Lock(0, src_size, (void**)&psrc, &src_ds_size, (void**)&psrc2, &not_used, 0);
				if ( hr != DS_OK ) {
					mprintf(("err: %s\n", get_DSERR_text(hr)));
					Int3();
					Channels[channel].pdsb->Release();
					return -1;
				}

				if ( Channels[channel].pdsb->Lock(0, src_ds_size, (void**)(&pdest), &dest_ds_size, (void**)&pdest2, &not_used, 0) == DS_OK)	{
					memcpy(pdest, psrc, src_ds_size);
					Channels[channel].pdsb->Unlock(pdest, dest_ds_size, 0, 0);
					ds_get_3d_interface(Channels[channel].pdsb, &Channels[channel].pds3db);
				} else {
					Channels[channel].pdsb->Release();
					return -1;
				}
			}
*/
		}

		Assert(Channels[channel].pds3db );
		Channels[channel].pds3db->SetMode(DS3DMODE_NORMAL,DS3D_IMMEDIATE);

		// set up 3D sound data here
		ds3d_update_buffer(channel, i2fl(min), i2fl(max), pos, vel);

		Channels[channel].vol = estimated_vol;
		Channels[channel].looping = looping;

		// sets the maximum "inner cone" volume
		Channels[channel].pdsb->SetVolume(max_volume);

		int ds_flags = 0;
		if ( looping )
			ds_flags |= DSBPLAY_LOOPING;

		// Actually play it
		hr = Channels[channel].pdsb->Play(0, 0, ds_flags );

		if ( hr == DSERR_BUFFERLOST ) {
			ds_restore_buffer(Channels[channel].pdsb);
			hr = Channels[channel].pdsb->Play(0, 0, ds_flags );
		}

		if ( hr != DS_OK ) {
			nprintf(("Sound", "Sound ==> Play failed with return value %s\n", get_DSERR_text(hr) ));
			if ( Channels[channel].pdsb ) {
				int attempts = 0;
				while(++attempts < 10) {
					hr = Channels[channel].pdsb->Release();
					if ( hr == DS_OK ) {
						break;
					} else {
						nprintf(("Sound","SOUND ==> DirectSound Release() failed with code %s\n.",get_DSERR_text(hr) ));
						continue;
					}
				}
				Channels[channel].pdsb = NULL;
			}
			return -1;
		}
	}
	else {
		nprintf(( "Sound", "SOUND ==> Not playing requested 3D sound\n"));
		return -1;
	}

	Channels[channel].snd_id = snd_id;
	Channels[channel].sig = channel_next_sig++;
	if (channel_next_sig < 0 ) {
		channel_next_sig = 1;
	}
	return Channels[channel].sig;
}

void ds_set_position(int channel, DWORD offset)
{
	// set the position of the sound buffer
	Channels[channel].pdsb->SetCurrentPosition(offset);
}

DWORD ds_get_play_position(int channel)
{
	DWORD play,write;	
	if ( Channels[channel].pdsb ) {
		Channels[channel].pdsb->GetCurrentPosition((LPDWORD)&play,(LPDWORD)&write);
	} else {
		play = 0;
	}

	return play;
}

DWORD ds_get_write_position(int channel)
{
	DWORD play,write;	
	if ( Channels[channel].pdsb ) {
		Channels[channel].pdsb->GetCurrentPosition((LPDWORD)&play,(LPDWORD)&write);
	} else {
		write = 0;
	}

	return write;
}

int ds_get_channel_size(int channel)
{
	int		size;
	DSBCAPS	caps;
	HRESULT	dsrval;

	if ( Channels[channel].pdsb ) {
		memset(&caps, 0, sizeof(DSBCAPS));
		caps.dwSize = sizeof(DSBCAPS);
		dsrval = Channels[channel].pdsb->GetCaps(&caps);
		if ( dsrval != DS_OK ) {
			return 0;
		}
		size = caps.dwBufferBytes;
	} else {
		size = 0;
	}

	return size;
}

// Returns the number of channels that are actually playing
int ds_get_number_channels()
{
	int i,n;

	n = 0;
	for ( i = 0; i < MAX_CHANNELS; i++ ) {
		if ( Channels[i].pdsb ) {
			if ( ds_is_channel_playing(i) == TRUE ) {
				n++;
			}
		}
	}

	return n;
}

// retreive raw data from a sound buffer
int ds_get_data(int sid, char *data)
{
	HRESULT					dsrval;
	LPDIRECTSOUNDBUFFER	pdsb;
	DSBCAPS					caps;
	void						*buffer_data;
	DWORD						buffer_size;

	Assert(sid >= 0);
	pdsb = ds_software_buffers[sid].pdsb;

	memset(&caps, 0, sizeof(DSBCAPS));
	caps.dwSize = sizeof(DSBCAPS);
	dsrval = pdsb->GetCaps(&caps);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	// lock the entire buffer
	dsrval = pdsb->Lock(0, caps.dwBufferBytes, &buffer_data, &buffer_size, 0, 0, 0);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	memcpy(data, buffer_data, buffer_size);

	dsrval = pdsb->Unlock(buffer_data, buffer_size, 0, 0);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	return 0;
}

// return the size of the raw sound data
int ds_get_size(int sid, int *size)
{
	HRESULT					dsrval;
	LPDIRECTSOUNDBUFFER	pdsb;
	DSBCAPS					caps;

	Assert(sid >= 0);
	pdsb = ds_software_buffers[sid].pdsb;

	memset(&caps, 0, sizeof(DSBCAPS));
	caps.dwSize = sizeof(DSBCAPS);
	dsrval = pdsb->GetCaps(&caps);
	if ( dsrval != DS_OK ) {
		return -1;
	}

	*size = caps.dwBufferBytes;
	return 0;
}

int ds_using_ds3d()
{
	return Ds_use_ds3d;
}

// Return the primary buffer interface.  Note that we cast to a uint to avoid
// having to include dsound.h (and thus windows.h) in ds.h.
//
void *ds_get_primary_buffer_interface()
{
	return pPrimaryBuffer;
}

// Return the DirectSound Interface.
//
void *ds_get_dsound_interface()
{
	return pDirectSound;
}

void *ds_get_property_set_interface()
{
	return pPropertySet;
}

// --------------------
//
// EAX Functions below
//
// --------------------

// Set the master volume for the reverb added to all sound sources.
//
// volume: volume, range from 0 to 1.0
//
// returns: 0 if the volume is set successfully, otherwise return -1
//
int ds_eax_set_volume(float volume)
{
	HRESULT hr;

	if (Ds_eax_inited == 0) {
		return -1;
	}

	Assert(Ds_eax_reverb);

	CAP(volume, 0.0f, 1.0f);

	hr = Ds_eax_reverb->Set(DSPROPSETID_EAX_ReverbProperties_Def, DSPROPERTY_EAX_VOLUME, NULL, 0, &volume, sizeof(float));
	if (SUCCEEDED(hr)) {
		return 0;
	} else {
		return -1;
	}
}

// Set the decay time for the EAX environment (ie all sound sources)
//
// seconds: decay time in seconds
//
// returns: 0 if decay time is successfully set, otherwise return -1
//
int ds_eax_set_decay_time(float seconds)
{
	HRESULT hr;

	if (Ds_eax_inited == 0) {
		return -1;
	}

	Assert(Ds_eax_reverb);

	CAP(seconds, 0.1f, 20.0f);

	hr = Ds_eax_reverb->Set(DSPROPSETID_EAX_ReverbProperties_Def, DSPROPERTY_EAX_DECAYTIME, NULL, 0, &seconds, sizeof(float));
	if (SUCCEEDED(hr)) {
		return 0;
	} else {
		return -1;
	}
}

// Set the damping value for the EAX environment (ie all sound sources)
//
// damp: damp value from 0 to 2.0
//
// returns: 0 if the damp value is successfully set, otherwise return -1
//
int ds_eax_set_damping(float damp)
{
	HRESULT hr;

	if (Ds_eax_inited == 0) {
		return -1;
	}

	Assert(Ds_eax_reverb);

	CAP(damp, 0.0f, 2.0f);

	hr = Ds_eax_reverb->Set(DSPROPSETID_EAX_ReverbProperties_Def, DSPROPERTY_EAX_DAMPING, NULL, 0, &damp, sizeof(float));
	if (SUCCEEDED(hr)) {
		return 0;
	} else {
		return -1;
	}
}

// Set up the environment type for all sound sources.
//
// envid: value from the EAX_ENVIRONMENT_* enumeration in ds_eax.h
//
// returns: 0 if the environment is set successfully, otherwise return -1
//
int ds_eax_set_environment(unsigned long envid)
{
	HRESULT hr;

	if (Ds_eax_inited == 0) {
		return -1;
	}

	Assert(Ds_eax_reverb);

	hr = Ds_eax_reverb->Set(DSPROPSETID_EAX_ReverbProperties_Def, DSPROPERTY_EAX_ENVIRONMENT, NULL, 0, &envid, sizeof(unsigned long));
	if (SUCCEEDED(hr)) {
		return 0;
	} else {
		return -1;
	}
}

// Set up a predefined environment for EAX
//
// envid: value from teh EAX_ENVIRONMENT_* enumeration
//
// returns: 0 if successful, otherwise return -1
//
int ds_eax_set_preset(unsigned long envid)
{
	HRESULT hr;

	if (Ds_eax_inited == 0) {
		return -1;
	}

	Assert(Ds_eax_reverb);
	Assert(envid < EAX_ENVIRONMENT_COUNT);

	hr = Ds_eax_reverb->Set(DSPROPSETID_EAX_ReverbProperties_Def, DSPROPERTY_EAX_ALL, NULL, 0, &Ds_eax_presets[envid], sizeof(EAX_REVERBPROPERTIES));
	if (SUCCEEDED(hr)) {
		return 0;
	} else {
		return -1;
	}
}


// Set up all the parameters for an environment
//
// id: value from teh EAX_ENVIRONMENT_* enumeration
// volume: volume for the environment (0 to 1.0)
// damping: damp value for the environment (0 to 2.0)
// decay: decay time in seconds (0.1 to 20.0)
//
// returns: 0 if successful, otherwise return -1
//
int ds_eax_set_all(unsigned long id, float vol, float damping, float decay)
{
	HRESULT hr;

	if (Ds_eax_inited == 0) {
		return -1;
	}

	Assert(Ds_eax_reverb);
	Assert(id < EAX_ENVIRONMENT_COUNT);

	EAX_REVERBPROPERTIES er;

	er.environment = id;
	er.fVolume = vol;
	er.fDecayTime_sec = decay;
	er.fDamping = damping;

	hr = Ds_eax_reverb->Set(DSPROPSETID_EAX_ReverbProperties_Def, DSPROPERTY_EAX_ALL, NULL, 0, &er, sizeof(EAX_REVERBPROPERTIES));
	if (SUCCEEDED(hr)) {
		return 0;
	} else {
		return -1;
	}
}

// Get up the parameters for the current environment
//
// er: (output) hold environment parameters
//
// returns: 0 if successful, otherwise return -1
//
int ds_eax_get_all(EAX_REVERBPROPERTIES *er)
{
	HRESULT hr;
	unsigned long outsize;

	if (Ds_eax_inited == 0) {
		return -1;
	}

	Assert(Ds_eax_reverb);

	hr = Ds_eax_reverb->Get(DSPROPSETID_EAX_ReverbProperties_Def, DSPROPERTY_EAX_ALL, NULL, 0, er, sizeof(EAX_REVERBPROPERTIES), &outsize);
	if (SUCCEEDED(hr)) {
		return 0;
	} else {
		return -1;
	}

}

// Close down EAX, freeing any allocated resources
//
void ds_eax_close()
{
	if (Ds_eax_inited == 0) {
		return;
	}

	Ds_eax_inited = 0;
}

// Initialize EAX
//
// returns: 0 if initialization is successful, otherwise return -1
//
int ds_eax_init()
{
#if defined(PREPROC_ENABLED_DS)
	HRESULT hr;
	unsigned long driver_support = 0;

	if (Ds_eax_inited) {
		return 0;
	}

	Assert(Ds_eax_reverb == NULL);

	Ds_eax_reverb = (LPKSPROPERTYSET)ds_get_property_set_interface();
	if (Ds_eax_reverb == NULL) {
		return -1;
	}

	// check if the listener property is supported by the audio driver
	hr = Ds_eax_reverb->QuerySupport(DSPROPSETID_EAX_ReverbProperties_Def, DSPROPERTY_EAX_ALL, &driver_support);
	if (FAILED(hr)) {
		nprintf(("Sound", "QuerySupport for the EAX Listener property set failed.. disabling EAX\n"));
		goto ds_eax_init_failed;
	}

	if ((driver_support & (KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET)) != (KSPROPERTY_SUPPORT_GET | KSPROPERTY_SUPPORT_SET)) {
		goto ds_eax_init_failed;
	}

	ds_eax_set_all(EAX_ENVIRONMENT_GENERIC, 0.0f, 0.0f, 0.0f);

	Ds_eax_inited = 1;
	return 0;

ds_eax_init_failed:
	if (Ds_eax_reverb != NULL) {
		Ds_eax_reverb->Release();
		Ds_eax_reverb = NULL;
	}

	Ds_eax_inited = 0;

	return -1;
#else
	return 0;
#endif
}

int ds_eax_is_inited()
{
	return Ds_eax_inited;
}

bool ds_using_a3d()
{	
	return false;
}

// Called once per game frame to make sure voice messages aren't looping
//
void ds_do_frame()
{
	channel *cp;

	for (int i=0; i<MAX_CHANNELS; i++) {
		cp = &Channels[i];
		if (cp->is_voice_msg == true) {
			if (cp->pdsb == NULL) {
				continue;
			}

			DWORD current_position = ds_get_play_position(i);
			if (current_position != 0) {
				if (current_position < cp->last_position) {
					ds_close_channel(i);
				} else {
					cp->last_position = current_position;
				}
			}
		}
	}
}

