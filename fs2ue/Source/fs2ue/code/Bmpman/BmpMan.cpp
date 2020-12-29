/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#ifdef FS2_UE
#include "bmpman.h"
#endif
#ifndef UNITY_BUILD
#include "PcxUtils.h"
#include "BmpMan.h"
#include "PalMan.h"
#include "2d.h"
#include "AnimPlay.h"
#include "Timer.h"
#include "SystemVars.h"
#include "Key.h"
#include "PackUnPack.h"
#include "cfile.h"
#include "GrInternal.h"
#include "TgaUtils.h"
#include "Ship.h"

#include <ctype.h>

#include "bmpman.h"
#endif

// keep this defined to use per-ship nondarkening pixels
#define BMPMAN_SPECIAL_NONDARK

int bm_inited = 0;

bool bitmap_entry::ms_bitmapLockingOn = false;

uint Bm_next_signature = 0x1234;

bitmap_entry bm_bitmaps[MAX_BITMAPS];

int bm_texture_ram = 0;

// Bm_max_ram - How much RAM bmpman can use for textures.
// Set to <1 to make it use all it wants.
int Bm_max_ram = 0;		//16*1024*1024;			// Only use 16 MB for textures

int bm_next_handle = 1;

int Bm_paging = 0;

// get and put functions for 16 bit pixels - neat bit slinging, huh?
#define BM_SET_R_ARGB(p, r)	{ p[1] &= ~(0x7c); p[1] |= ((r & 0x1f) << 2); }
#define BM_SET_G_ARGB(p, g)	{ p[0] &= ~(0xe0); p[1] &= ~(0x03); p[0] |= ((g & 0x07) << 5); p[1] |= ((g & 0x18) >> 3); }
#define BM_SET_B_ARGB(p, b)	{ p[0] &= ~(0x1f); p[0] |= b & 0x1f; }
#define BM_SET_A_ARGB(p, a)	{ p[1] &= ~(0x80); p[1] |= ((a & 0x01) << 7); }

#define BM_SET_R_D3D(p, r)		{ *p |= (ushort)(( (int)r / Gr_current_red->scale ) << Gr_current_red->shift); }
#define BM_SET_G_D3D(p, g)		{ *p |= (ushort)(( (int)g / Gr_current_green->scale ) << Gr_current_green->shift); }
#define BM_SET_B_D3D(p, b)		{ *p |= (ushort)(( (int)b / Gr_current_blue->scale ) << Gr_current_blue->shift); }
#define BM_SET_A_D3D(p, a)		{ if(a == 0){ *p = (ushort)Gr_current_green->mask; } }

#define BM_SET_R(p, r)	BM_SET_R_D3D(p, r)
#define BM_SET_G(p, g)	BM_SET_G_D3D(p, g)
#define BM_SET_B(p, b)	BM_SET_B_D3D(p, b) 
#define BM_SET_A(p, a)	BM_SET_A_D3D(p, a)

static int bm_get_next_handle()
{
	int n = bm_next_handle;
	bm_next_handle++;
	if ( bm_next_handle > 30000 )	{
		bm_next_handle = 1;
	}
	return n;
}

void bm_free_data_without_unlock(int handle)
{
	int n = handle % MAX_BITMAPS;

	bitmap_entry	*be  = &bm_bitmaps[n];
	bitmap			*bmp = &be->bm;

	Assert( n >= 0 && n < MAX_BITMAPS );

	// If there isn't a bitmap in this structure, don't
	// do anything but clear out the bitmap info
	if ( be->type==BM_TYPE_NONE) 
		goto SkipFree;

	// If this bitmap doesn't have any data to free, skip
	// the freeing it part of this.
	if ( bmp->data == 0 ) 
		goto SkipFree;

	// Don't free up memory for user defined bitmaps, since
	// BmpMan isn't the one in charge of allocating/deallocing them.
	if ( ( be->type==BM_TYPE_USER ) )	
		goto SkipFree;

	// Free up the data now!

	//	mprintf(( "Bitmap %d freed %d bytes\n", n, bm_bitmaps[n].data_size ));
	#ifdef BMPMAN_NDEBUG
		bm_texture_ram -= be->data_size;
	#endif
	free((void *)bmp->data);

SkipFree:

	// Clear out & reset the bitmap data structure
	bmp->flags = 0;
	bmp->bpp = 0;
	bmp->data = 0;
	bmp->palette = NULL;
	#ifdef BMPMAN_NDEBUG
		be->data_size = 0;
	#endif
	be->signature = Bm_next_signature++; 
}

// Frees a bitmaps data if it should, and
// Returns true if bitmap n can free it's data.
static void bm_free_data(int n)
{
	bitmap_entry	*be  = &bm_bitmaps[n];

	Assert( n >= 0 && n < MAX_BITMAPS );

	if(be->m_locked)
	{
		be->m_locked = false;
	}
	bm_free_data_without_unlock(n);
}

#ifdef BMPMAN_NDEBUG

int Bm_ram_freed = 0;

static void bm_free_some_ram( int n, int size )
{
/*
	if ( Bm_max_ram < 1 ) return;
	if ( bm_texture_ram + size < Bm_max_ram ) return;

	int current_time = timer_get_milliseconds();

	while( bm_texture_ram + size > Bm_max_ram )	{
		Bm_ram_freed++;

		// Need to free some RAM up!
		int i, oldest=-1, best_val=0;
		for (i = 0; i < MAX_BITMAPS; i++)	{
			if ( (bm_bitmaps[i].type != BM_TYPE_NONE) && (bm_bitmaps[i].first_frame!=bm_bitmaps[n].first_frame) && (bm_bitmaps[i].ref_count==0) && (bm_bitmaps[i].data_size>0) )	{
				int page_func = ( current_time-bm_bitmaps[i].last_used)*bm_bitmaps[i].data_size;
				if ( (oldest==-1) || (page_func>best_val) )	{
					oldest=i;
					best_val = page_func;
				}
			}
		}

		if ( oldest > -1 )	{
			//mprintf(( "Freeing bitmap '%s'\n", bm_bitmaps[oldest].filename ));
			for (i=0; i<bm_bitmaps[oldest].num_frames; i++ )	{
				bm_free_data(bm_bitmaps[oldest].first_frame+i);
			}
		} else {
			//mprintf(( "Couldn't free enough! %d\n", bm_texture_ram ));
			break;
		}
	}	
*/
}

#endif

static void *bm_malloc( int n, int size )
{
	Assert( n >= 0 && n < MAX_BITMAPS );
//	mprintf(( "Bitmap %d allocated %d bytes\n", n, size ));
	#ifdef BMPMAN_NDEBUG
	bm_free_some_ram( n, size );
	Assert( bm_bitmaps[n].data_size == 0 );
	bm_bitmaps[n].data_size += size;
	bm_texture_ram += size;
	#endif
	return malloc(size);
}

void bm_close()
{
	int i;
	if ( bm_inited )	{
		for (i=0; i<MAX_BITMAPS; i++ )	{
			bm_free_data(i);			// clears flags, bbp, data, etc
		}
		bm_inited = 0;
	}
}


void bm_init()
{
	int i;

	mprintf(( "Size of bitmap info = %d KB\n", sizeof( bm_bitmaps )/1024 ));
	mprintf(( "Size of bitmap extra info = %d bytes\n", sizeof( bm_extra_info ) ));
	
	if (!bm_inited)	{
		bm_inited = 1;
		atexit(bm_close);
	}
	
	for (i=0; i<MAX_BITMAPS; i++ ) {
		bm_bitmaps[i].filename[0] = '\0';
		bm_bitmaps[i].type = BM_TYPE_NONE;
		bm_bitmaps[i].info.user.data = NULL;
		bm_bitmaps[i].bm.data = 0;
		bm_bitmaps[i].bm.palette = NULL;
		#ifdef BMPMAN_NDEBUG
			bm_bitmaps[i].data_size = 0;
			bm_bitmaps[i].used_count = 0;
			bm_bitmaps[i].used_last_frame = 0;
			bm_bitmaps[i].used_this_frame = 0;
		#endif
		bm_free_data(i);  	// clears flags, bbp, data, etc
	}


}

#ifdef BMPMAN_NDEBUG

// Returns number of bytes of bitmaps locked this frame
// ntotal = number of bytes of bitmaps locked this frame
// nnew = number of bytes of bitmaps locked this frame that weren't locked last frame
void bm_get_frame_usage(int *ntotal, int *nnew)
{
	int i;
	
	*ntotal = 0;
	*nnew = 0;

	for (i=0; i<MAX_BITMAPS; i++ ) {
		if ( (bm_bitmaps[i].type != BM_TYPE_NONE) && (bm_bitmaps[i].used_this_frame))	{
			if ( !bm_bitmaps[i].used_last_frame )	{
				*nnew += bm_bitmaps[i].bm.w*bm_bitmaps[i].bm.h; 
			}
			*ntotal += bm_bitmaps[i].bm.w*bm_bitmaps[i].bm.h;
		}
		bm_bitmaps[i].used_last_frame = bm_bitmaps[i].used_this_frame;
		bm_bitmaps[i].used_this_frame = 0;
	}

}
#else
void bm_get_frame_usage(int *ntotal, int *nnew)
{
}
#endif

// Creates a bitmap that exists in RAM somewhere, instead
// of coming from a disk file.  You pass in a pointer to a
// block of 32 (or 8)-bit-per-pixel data.  Right now, the only
// bpp you can pass in is 32 or 8.  On success, it returns the
// bitmap number.  You cannot free that RAM until bm_release
// is called on that bitmap.
int bm_create( int bpp, int w, int h, void * data, int flags )
{
	int i, n, first_slot = MAX_BITMAPS;

	// Assert((bpp==32)||(bpp==8));
	if(bpp == 8){
		Assert(flags & BMP_AABITMAP);
	} else {
		Assert(bpp == 32 || bpp == 16);
	}

	if ( !bm_inited ) bm_init();

	for (i = MAX_BITMAPS-1; i >= 0; i-- ) {
		if ( bm_bitmaps[i].type == BM_TYPE_NONE )	{
			first_slot = i;
			break;
		}
	}

	n = first_slot;
	Assert( n > -1 );

	// Out of bitmap slots
	if ( n == -1 ) return -1;

	memset( &bm_bitmaps[n], 0, sizeof(bitmap_entry) );

	sprintf( bm_bitmaps[n].filename, "BM_TYPE_USER %dx%d", w, h );
	bm_bitmaps[n].type = BM_TYPE_USER;
	bm_bitmaps[n].palette_checksum = 0;

	bm_bitmaps[n].bm.w = short(w);
	bm_bitmaps[n].bm.h = short(h);
	bm_bitmaps[n].bm.rowsize = short(w);
	bm_bitmaps[n].bm.bpp = unsigned char(bpp);
	bm_bitmaps[n].bm.flags = flags;
	bm_bitmaps[n].bm.data = 0;
	bm_bitmaps[n].bm.palette = NULL;

	bm_bitmaps[n].info.user.bpp = ubyte(bpp);
	bm_bitmaps[n].info.user.data = data;
	bm_bitmaps[n].info.user.flags = ubyte(flags);

	if(bitmap_entry::ms_bitmapLockingOn)
	{
		bm_bitmaps[n].m_locked = true;
	}

	bm_bitmaps[n].signature = Bm_next_signature++;

	bm_bitmaps[n].handle = bm_get_next_handle()*MAX_BITMAPS + n;
	bm_bitmaps[n].last_used = -1;

	return bm_bitmaps[n].handle;
}

// sub helper function. Given a raw filename and an extension, try and find the bitmap
// returns -1 if it could not be found
//          0 if it was found as a file
//          1 if it already exists, fills in handle
int Bm_ignore_duplicates = 0;
int bm_load_sub(char *real_filename, char *ext, int *handle)
{	
	int i;
	char filename[MAX_FILENAME_LEN] = "";
	
	strcpy( filename, real_filename );
	strcat( filename, ext );	
	for (i=0; i<(int)strlen(filename); i++ ){
		filename[i] = char(tolower(filename[i]));
	}		

	// try to find given filename to see if it has been loaded before
	if(!Bm_ignore_duplicates){
		for (i = 0; i < MAX_BITMAPS; i++) {
			if ( (bm_bitmaps[i].type != BM_TYPE_NONE) && !stricmp(filename, bm_bitmaps[i].filename) ) {
				nprintf (("BmpMan", "Found bitmap %s -- number %d\n", filename, i));
				*handle = bm_bitmaps[i].handle;
				return 1;
			}
		}	
	}

	// try and find the file
	/*
	CFILE *test = cfopen(filename, "rb");
	if(test != NULL){
		cfclose(test);
		return 0;
	}
	*/

	// could not be found
	return 0;
}

// This loads a bitmap so we can draw with it later.
// It returns a negative number if it couldn't load
// the bitmap.   On success, it returns the bitmap
// number.  Function doesn't acutally load the data, only
// width, height, and possibly flags.
int bm_load( char * real_filename )
{
	int i, n, first_slot = MAX_BITMAPS;
	int w, h, bpp;
	char filename[MAX_FILENAME_LEN];
	int tga = 0;
	int handle;
	int found = 0;

	if ( !bm_inited ) bm_init();

	// nice little trick for keeping standalone memory usage way low - always return a bogus bitmap 
	if(Game_mode & GM_STANDALONE_SERVER){
		strcpy(filename,"test128");
	}

	// make sure no one passed an extension
	strcpy( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) {
		mprintf(( "Someone passed an extension to bm_load for file '%s'\n", real_filename ));
		//Int3();
		*p = 0;
	}
	 
	// try and find the pcx file		
	switch(bm_load_sub(filename, ".pcx", &handle)){
	// error
	case -1:
		break;

	// found as a file
	case 0:
		found = 1;
		strcat(filename, ".pcx");
		break;

	// found as pre-existing
	case 1:
		found = 1;
		return handle;		
	}

	if(!found){
		// try and find the tga file
		switch(bm_load_sub(filename, ".tga", &handle)){
		// error
		case -1:			
			return -1;
			break;

		// found as a file
		case 0:			
			strcat(filename, ".tga");
			tga = 1;
			break;

		// found as pre-existing
		case 1:						
			return handle;					
		}
	}

	// if its a tga file
	if(tga){
		int tga_error=targa_read_header( filename, &w, &h, &bpp, NULL );
		if ( tga_error != TARGA_ERROR_NONE )	{
			mprintf(( "Couldn't open '%s'\n", filename ));
			return -1;
		}
	}
	// if its a pcx file
	else {
		int pcx_error=pcx_read_header( filename, &w, &h, NULL );		
		if ( pcx_error != PCX_ERROR_NONE )	{
			mprintf(( "Couldn't open '%s'\n", filename ));
			return -1;
		}
	}

	// Error( LOCATION, "Unknown bitmap type %s\n", filename );
		
	// Find an open slot
	for (i = 0; i < MAX_BITMAPS; i++) {
		if ( (bm_bitmaps[i].type == BM_TYPE_NONE) && (first_slot == MAX_BITMAPS) ){
			first_slot = i;
		}
	}

	n = first_slot;
	Assert( n < MAX_BITMAPS );	

	if ( n == MAX_BITMAPS ) return -1;	

	// ensure fields are cleared out from previous bitmap
//	Assert(bm_bitmaps[n].bm.data == 0);
//	Assert(bm_bitmaps[n].bm.palette == NULL);
//	Assert(bm_bitmaps[n].ref_count == 0 );
//	Assert(bm_bitmaps[n].user_data == NULL);
	memset( &bm_bitmaps[n], 0, sizeof(bitmap_entry) );
	
	// Mark the slot as filled, because cf_read might load a new bitmap
	// into this slot.
	bm_bitmaps[n].type = tga ? (ubyte)BM_TYPE_TGA : (ubyte)BM_TYPE_PCX;
	bm_bitmaps[n].signature = Bm_next_signature++;
	Assert ( strlen(filename) < MAX_FILENAME_LEN );
	strncpy(bm_bitmaps[n].filename, filename, MAX_FILENAME_LEN-1 );
	bm_bitmaps[n].bm.w = short(w);
	bm_bitmaps[n].bm.rowsize = short(w);
	bm_bitmaps[n].bm.h = short(h);
	bm_bitmaps[n].bm.bpp = 0;
	bm_bitmaps[n].bm.flags = 0;
	bm_bitmaps[n].bm.data = 0;
	bm_bitmaps[n].bm.palette = NULL;

	if(bitmap_entry::ms_bitmapLockingOn)
	{
		bm_bitmaps[n].m_locked = true;
	}

	bm_bitmaps[n].palette_checksum = 0;
	bm_bitmaps[n].handle = bm_get_next_handle()*MAX_BITMAPS + n;
	bm_bitmaps[n].last_used = -1;

	return bm_bitmaps[n].handle;
}

// special load function. basically allows you to load a bitmap which already exists (by filename). 
// this is useful because in some cases we need to have a bitmap which is locked in screen format
// _and_ texture format, such as pilot pics and squad logos
int bm_load_duplicate(char *filename)
{
	int ret;

	// ignore duplicates
	Bm_ignore_duplicates = 1;
	
	// load
	ret = bm_load(filename);

	// back to normal
	Bm_ignore_duplicates = 0;

	return ret;
}

DCF(bm_frag,"Shows BmpMan fragmentation")
{
	if ( Dc_command )	{

		gr_clear();

		int x=0, y=0;
		int xs=2, ys=2;
		int w=4, h=4;

		for (int i=0; i<MAX_BITMAPS; i++ )	{
			switch( bm_bitmaps[i].type )	{
			case BM_TYPE_NONE:
				gr_set_color(128,128,128);
				break;
			case BM_TYPE_PCX:
				gr_set_color(255,0,0);
				break;
			case BM_TYPE_USER:
				gr_set_color(0,255,0);
				break;
			case BM_TYPE_ANI:
				gr_set_color(0,0,255);
				break;
			}

			gr_rect( x+xs, y+ys, w, h );
			x += w+xs+xs;
			if ( x > 639 )	{
				x = 0;
				y += h + ys + ys;
			}

		}

		gr_flip();
		key_getch();
	}
}

static int find_block_of(int n)
{
	int i, cnt, nstart;

	cnt=0;
	nstart = 0;
	for (i=0; i<MAX_BITMAPS; i++ )	{
		if ( bm_bitmaps[i].type == BM_TYPE_NONE )	{
			if (cnt==0) nstart = i;
			cnt++;
		} else
			cnt=0;
		if ( cnt == n ) return nstart;
	}

	// Error( LOCATION, "Couldn't find block of %d frames\n", n );
	return -1;
}

// ------------------------------------------------------------------
// bm_load_animation()
//
//	input:		filename		=>		filename of animation
//					nframes		=>		OUTPUT parameter:	number of frames in the animation
//					fps			=>		OUTPUT/OPTIONAL parameter: intended fps for the animation
//
// returns:		bitmap number of first frame in the animation
//
int bm_load_animation( char *real_filename, int *nframes, int *fps, int can_drop_frames)
{
	int	i, n;
	anim	the_anim;
	CFILE	*fp;
	char filename[MAX_FILENAME_LEN];

	if ( !bm_inited ) bm_init();

	strcpy( filename, real_filename );
	char *p = strchr( filename, '.' );
	if ( p ) {
		mprintf(( "Someone passed an extension to bm_load_animation for file '%s'\n", real_filename ));
		//Int3();
		*p = 0;
	}
	strcat( filename, ".ani" );

	if ( (fp = cfopen(filename, "rb")) == NULL ) {
//		Error(LOCATION,"Could not open filename %s in bm_load_ani()\n", filename);
		return -1;
	}

#ifndef NDEBUG
	// for debug of ANI sizes
	strcpy(the_anim.name, real_filename);
#endif
	anim_read_header(&the_anim, fp);
	if ( can_drop_frames )	{
	}
	cfclose(fp);

	*nframes = the_anim.total_frames;
	if ( fps != NULL )	{
		*fps = the_anim.fps;
	}

	// first check to see if this ani already has it's frames loaded
	for (i = 0; i < MAX_BITMAPS; i++) {
		if ( (bm_bitmaps[i].type == BM_TYPE_ANI) && !stricmp(filename, bm_bitmaps[i].filename) ) {
			break;
		}
	}
	
	if ( i < MAX_BITMAPS ) {
		Assert(bm_bitmaps[i].info.ani.num_frames == *nframes);
		return bm_bitmaps[i].handle;
	}

	n = find_block_of(*nframes);
	if(n < 0){
		return -1;
	}
	// Assert( n >= 0 );

	int first_handle = bm_get_next_handle();

	Assert ( strlen(filename) < MAX_FILENAME_LEN );
	for ( i = 0; i < *nframes; i++ ) {
		memset( &bm_bitmaps[n+i], 0, sizeof(bitmap_entry) );
		bm_bitmaps[n+i].info.ani.first_frame = n;
		bm_bitmaps[n+i].info.ani.num_frames = ubyte(the_anim.total_frames);
		bm_bitmaps[n+i].info.ani.fps = ubyte(the_anim.fps);
		bm_bitmaps[n+i].bm.w = short(the_anim.width);
		bm_bitmaps[n+i].bm.rowsize = short(the_anim.width);
		bm_bitmaps[n+i].bm.h = short(the_anim.height);
		bm_bitmaps[n+i].bm.flags = 0;
		bm_bitmaps[n+i].bm.bpp = 0;
		bm_bitmaps[n+i].bm.data = 0;
		bm_bitmaps[n+i].bm.palette = NULL;
		bm_bitmaps[n+i].type = BM_TYPE_ANI;
		bm_bitmaps[n+i].palette_checksum = 0;
		bm_bitmaps[n+i].signature = Bm_next_signature++;
		bm_bitmaps[n+i].handle = first_handle*MAX_BITMAPS + n+i;
		bm_bitmaps[n+i].last_used = -1;

		if(bitmap_entry::ms_bitmapLockingOn)
		{
			bm_bitmaps[n].m_locked = true;
		}

		if ( i == 0 )	{
			sprintf( bm_bitmaps[n+i].filename, "%s", filename );
		} else {
			sprintf( bm_bitmaps[n+i].filename, "%s[%d]", filename, i );
		}
	}

	return bm_bitmaps[n].handle;
}

// Gets info.   w,h,or flags,nframes or fps can be NULL if you don't care.
void bm_get_info( int handle, int *w, int * h, ubyte * flags, int *nframes, int *fps)
{
	bitmap * bmp;

	if ( !bm_inited ) return;

	int bitmapnum = handle % MAX_BITMAPS;
	Assert( bm_bitmaps[bitmapnum].handle == handle );		// INVALID BITMAP HANDLE!	
	
	if ( (bm_bitmaps[bitmapnum].type == BM_TYPE_NONE) || (bm_bitmaps[bitmapnum].handle != handle) ) {
		if (w) *w = 0;
		if (h) *h = 0;
		if (flags) *flags = 0;
		if (nframes) *nframes=0;
		if (fps) *fps=0;
		return;
	}

	bmp = &(bm_bitmaps[bitmapnum].bm);

	if (w) *w = bmp->w;
	if (h) *h = bmp->h;
	if (flags) *flags = bmp->flags;
	if ( bm_bitmaps[bitmapnum].type == BM_TYPE_ANI )	{
		if (nframes) {
			*nframes = bm_bitmaps[bitmapnum].info.ani.num_frames;
		} 
		if (fps) {
			*fps= bm_bitmaps[bitmapnum].info.ani.fps;
		}
	} else {
		if (nframes) {
			*nframes = 1;
		} 
		if (fps) {
			*fps= 0;
		}
	}
}

uint bm_get_signature( int handle )
{
	if ( !bm_inited ) bm_init();

	int bitmapnum = handle % MAX_BITMAPS;
	Assert( bm_bitmaps[bitmapnum].handle == handle );		// INVALID BITMAP HANDLE

	return bm_bitmaps[bitmapnum].signature;
}

extern int palman_is_nondarkening(int r,int g, int b);
static void bm_convert_format( int bitmapnum, bitmap *bmp, ubyte bpp, ubyte flags )
{	
	int idx;	

	if(Fred_running || Pofview_running || Is_standalone){
		Assert(bmp->bpp == 8);

		return;
	} else {
		if(flags & BMP_AABITMAP){
			Assert(bmp->bpp == 8);
		}
	}

	// maybe swizzle to be an xparent texture
	if(!(bmp->flags & BMP_TEX_XPARENT) && (flags & BMP_TEX_XPARENT))
	{
		if(bpp == 16)
		{
			for(idx=0; idx<bmp->w*bmp->h; idx++){			
				
				// if the pixel is transparent
				if ( ((ushort*)bmp->data)[idx] == Gr_t_green.mask)	{
					((int*)bmp->data)[idx] = 0;
				}
			}
		}
		
		bmp->flags |= BMP_TEX_XPARENT;
	}	
}

// basically, map the bitmap into the current palette. used to be done for all pcx's, now just for
// Fred, since its the only thing that uses the software tmapper
void bm_swizzle_8bit_for_fred(bitmap_entry *be, bitmap *bmp, ubyte *data, ubyte *palette)
{		
	int pcx_xparent_index = -1;
	int i;
	int r, g, b;
	ubyte palxlat[256];

	for (i=0; i<256; i++ ) {
		r = palette[i*3];
		g = palette[i*3+1];
		b = palette[i*3+2];
		if ( g == 255 && r == 0 && b == 0 ) {
			palxlat[i] = 255;
			pcx_xparent_index = i;
		} else {			
			palxlat[i] = (ubyte)(palette_find( r, g, b ));			
		}
	}		
	for (i=0; i<bmp->w * bmp->h; i++ ) {		
		ubyte c = palxlat[data[i]];			
		data[i] = c;		
	}			
	be->palette_checksum = gr_palette_checksum;	
}

int bm_get_type(int bitmap_handle)
{
	int bitmapnum = bitmap_handle % MAX_BITMAPS;
	Assert( bm_bitmaps[bitmapnum].handle == bitmap_handle );
	return bm_bitmaps[bitmapnum].type;
}

int bm_get_bpp(int bitmap_handle)
{
	int bitmapnum = bitmap_handle % MAX_BITMAPS;
	Assert( bm_bitmaps[bitmapnum].handle == bitmap_handle );
	return bm_bitmaps[bitmapnum].bm.bpp;
}

void bm_lock_pcx( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{	
	ubyte *data = NULL, *palette;
	ubyte pal[768];
	palette = NULL;

	// Unload any existing data
	bm_free_data( bitmapnum );	

	// allocate bitmap data
	if(bpp == 8){
		// Assert(Fred_running || Pofview_running || Is_standalone);		
			data = (ubyte *)bm_malloc(bitmapnum, bmp->w * bmp->h );
		#ifdef BMPMAN_NDEBUG
			Assert( be->data_size == bmp->w * bmp->h );
		#endif
		palette = pal;
		bmp->data = data;
		bmp->bpp = 8;
		bmp->palette = gr_palette;
		memset( data, 0, bmp->w * bmp->h);
	} 
	else if(bpp == 32 || bpp == 16) {
		data = (ubyte*)bm_malloc(bitmapnum, bmp->w * bmp->h * bpp / 8);	
		bmp->bpp = bpp;
		bmp->data = data;
		bmp->palette = NULL;
		memset( data, 0, bmp->w * bmp->h * bpp / 8);
	}
	else
	{
		Assert(0);
	}

	Assert( &be->bm == bmp );
	#ifdef BMPMAN_NDEBUG
		Assert( be->data_size > 0 );
	#endif

	// some sanity checks on flags
	Assert(!((flags & BMP_AABITMAP) && (flags & BMP_TEX_ANY)));						// no aabitmap textures

	if(bpp == 32)
	{
		bmp->flags = 0;
		//int pcx_error = 
		pcx_read_bitmap_32bpp( be->filename, data );

	}
	else 
	{
		if(bpp == 8)
		{
			int pcx_error=pcx_read_bitmap_8bpp( be->filename, data, palette );
			if ( pcx_error != PCX_ERROR_NONE )	{
				// Error( LOCATION, "Couldn't open '%s'\n", be->filename );
				//Error( LOCATION, "Couldn't open '%s'\n", filename );
				//return -1;
			}

			// now swizzle the thing into the proper format
			if(Fred_running || Pofview_running){
				bm_swizzle_8bit_for_fred(be, bmp, data, palette);
			}
		} else {	
			int pcx_error;

			// load types
			if(flags & BMP_AABITMAP){
				pcx_error = pcx_read_bitmap_16bpp_aabitmap( be->filename, data );
			} else {
				pcx_error = pcx_read_bitmap_16bpp( be->filename, data );
			}
			if ( pcx_error != PCX_ERROR_NONE )	{
				// Error( LOCATION, "Couldn't open '%s'\n", be->filename );
				//Error( LOCATION, "Couldn't open '%s'\n", filename );
				//return -1;
			}
		}
	}
	#ifdef BMPMAN_NDEBUG
	Assert( be->data_size > 0 );
	#endif	

	bmp->flags = 0;	
	bm_convert_format( bitmapnum, bmp, bpp, flags );
}

void bm_lock_ani( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{	
	anim				*the_anim;
	anim_instance	*the_anim_instance;
	bitmap			*bm;
	ubyte				*frame_data;
	int				size, i;
	int				first_frame, nframes;	

	first_frame = be->info.ani.first_frame;
	nframes = bm_bitmaps[first_frame].info.ani.num_frames;

	if ( (the_anim = anim_load(bm_bitmaps[first_frame].filename)) == NULL ) {
		// Error(LOCATION, "Error opening %s in bm_lock\n", be->filename);
	}

	if ( (the_anim_instance = init_anim_instance(the_anim, bpp)) == NULL ) {
		// Error(LOCATION, "Error opening %s in bm_lock\n", be->filename);
		anim_free(the_anim);
	}

	int can_drop_frames = 0;

	if ( the_anim->total_frames != bm_bitmaps[first_frame].info.ani.num_frames )	{
		can_drop_frames = 1;
	}
	bm = &bm_bitmaps[first_frame].bm;

	size = bm->w * bm->h * bpp / 8;
		
	for ( i=0; i<nframes; i++ )	{
		be = &bm_bitmaps[first_frame+i];
		bm = &bm_bitmaps[first_frame+i].bm;

		// Unload any existing data
		bm_free_data( first_frame+i );

		bm->flags = 0;
		// briefing editor in Fred2 uses aabitmaps (ani's) - force to 8 bit
		if(Fred_running || Is_standalone){
			bm->bpp = 8;
		} else {
			bm->bpp = bpp;
		}
		bm->data = bm_malloc(first_frame + i, size);

		frame_data = anim_get_next_raw_buffer(the_anim_instance,flags & BMP_AABITMAP ? 1 : 0, bm->bpp);

		if ( frame_data == NULL ) {
			// Error(LOCATION,"Fatal error locking .ani file: %s\n", be->filename);
		}		
		
		ubyte *dptr, *sptr;

		sptr = frame_data;
		dptr = (ubyte *)bm->data;

		if ( (bm->w!=the_anim->width) || (bm->h!=the_anim->height) )	{
			// Scale it down
			// Int3();			// not ready yet - should only be ingame
	
			// 8 bit
			if(bpp == 8){
				int w,h;
				fix u, utmp, v, du, dv;

				u = v = 0;

				du = ( the_anim->width*F1_0 ) / bm->w;
				dv = ( the_anim->height*F1_0 ) / bm->h;
												
				for (h = 0; h < bm->h; h++) {
					ubyte *drow = &dptr[bm->w * h];
					ubyte *srow = &sptr[f2i(v)*the_anim->width];

					utmp = u;

					for (w = 0; w < bm->w; w++) {
						*drow++ = srow[f2i(utmp)];
						utmp += du;
					}
					v += dv;
				}			
			}
			// 16 bpp
			else if (bpp == 16){
				int w,h;
				fix u, utmp, v, du, dv;

				u = v = 0;

				du = ( the_anim->width*F1_0 ) / bm->w;
				dv = ( the_anim->height*F1_0 ) / bm->h;
												
				for (h = 0; h < bm->h; h++) {
					ushort *drow = &((ushort*)dptr)[bm->w * h];
					ushort *srow = &((ushort*)sptr)[f2i(v)*the_anim->width];

					utmp = u;

					for (w = 0; w < bm->w; w++) {
						*drow++ = srow[f2i(utmp)];
						utmp += du;
					}
					v += dv;
				}			
			}	
			else
			{
				Assert(0);
			}
		} else {
			// 1-to-1 mapping
			memcpy(dptr, sptr, size);
		}		

		bm_convert_format( first_frame+i, bm, bpp, flags );

		// Skip a frame
		if ( (i < nframes-1)  && can_drop_frames )	{
			frame_data = anim_get_next_raw_buffer(the_anim_instance, flags & BMP_AABITMAP ? 1 : 0, bm->bpp);
		}

		//mprintf(( "Checksum = %d\n", be->palette_checksum ));
	}

	free_anim_instance(the_anim_instance);
	anim_free(the_anim);
}


void bm_lock_user( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{
	// int idx;	
	// ushort bit_16;

	// Unload any existing data
	bm_free_data( bitmapnum );	

	switch( be->info.user.bpp )	{
	case 32:
	case 16:			// user 16 bit bitmap
		bmp->bpp = bpp;
		bmp->flags = be->info.user.flags;		
		bmp->data = be->info.user.data;								
		break;	
	
	case 8:			// Going from 8 bpp to something (probably only for aabitmaps)
		/*
		Assert(flags & BMP_AABITMAP);
		bmp->bpp = 16;
		bmp->data = (uint)malloc(bmp->w * bmp->h * 2);
		bmp->flags = be->info.user.flags;
		bmp->palette = NULL;

		// go through and map the pixels
		for(idx=0; idx<bmp->w * bmp->h; idx++){			
			bit_16 = (ushort)((ubyte*)be->info.user.data)[idx];			
			Assert(bit_16 <= 255);

			// stuff the final result
			memcpy((char*)bmp->data + (idx * 2), &bit_16, sizeof(ushort));
		}
		*/		
		Assert(flags & BMP_AABITMAP);
		bmp->bpp = bpp;
		bmp->flags = be->info.user.flags;		
		bmp->data = be->info.user.data;								
		break;
		
	// default:
		// Error( LOCATION, "Unhandled user bitmap conversion from %d to %d bpp", be->info.user.bpp, bmp->bpp );
	}

	bm_convert_format( bitmapnum, bmp, bpp, flags );
}

void bm_lock_tga( int handle, int bitmapnum, bitmap_entry *be, bitmap *bmp, ubyte bpp, ubyte flags )
{
	ubyte *data;	

	// Unload any existing data
	bm_free_data( bitmapnum );	

	if(Fred_running || Is_standalone){
		Assert(bpp == 8);
	} else {
		Assert(bpp == 16 || bpp == 8);
	}

	// should never try to make an aabitmap out of a targa
	Assert(!(flags & BMP_AABITMAP));

	// allocate bitmap data	
	data = (ubyte*)bm_malloc(bitmapnum, bmp->w * bmp->h * bpp / 8);	
	
	bmp->bpp = bpp;
	bmp->data = data;
	bmp->palette = NULL;
	memset( data, 0, bmp->w * bmp->h * bpp / 8);	
	

	Assert( &be->bm == bmp );
	#ifdef BMPMAN_NDEBUG
	Assert( be->data_size > 0 );
	#endif
	
	int tga_error=targa_read_bitmap( be->filename, data, NULL, bpp / 8);
	if ( tga_error != TARGA_ERROR_NONE )	{
		// Error( LOCATION, "Couldn't open '%s'\n", be->filename );
		//Error( LOCATION, "Couldn't open '%s'\n", filename );
		//return -1;
	}

	#ifdef BMPMAN_NDEBUG
	Assert( be->data_size > 0 );
	#endif		
	
	bmp->flags = 0;	
	bm_convert_format( bitmapnum, bmp, bpp, flags );
}

MONITOR( NumBitmapPage );
MONITOR( SizeBitmapPage );

// This locks down a bitmap and returns a pointer to a bitmap
// that can be accessed until you call bm_unlock.   Only lock
// a bitmap when you need it!  This will convert it into the 
// appropriate format also.
bitmap * bm_lock( int handle, ubyte bpp, ubyte flags )
{
	bitmap			*bmp;
	bitmap_entry	*be;


	if ( !bm_inited ) bm_init();

	int bitmapnum = handle % MAX_BITMAPS;
	Assert( bm_bitmaps[bitmapnum].handle == handle );		// INVALID BITMAP HANDLE

//	flags &= (~BMP_RLE);

	// if we're on a standalone server, aways for it to lock to 8 bits
	if(Is_standalone){
		bpp = 8;
		flags = 0;
	} 
	// otherwise do it as normal
	else {
		if(Fred_running || Pofview_running){
			Assert( bpp == 8 );
			Assert( (bm_bitmaps[bitmapnum].type == BM_TYPE_PCX) || (bm_bitmaps[bitmapnum].type == BM_TYPE_ANI) || (bm_bitmaps[bitmapnum].type == BM_TYPE_TGA));
		} else {
			if(flags & BMP_AABITMAP){
				Assert( bpp == 8 );
			} else {
				Assert( bpp == 16 || bpp == 32);
			}
		}
	}

	be = &bm_bitmaps[bitmapnum];
	bmp = &be->bm;

	// If you hit this assert, chances are that someone freed the
	// wrong bitmap and now someone is trying to use that bitmap.
	// See John.
	Assert( be->type != BM_TYPE_NONE );		

	// Increment ref count for bitmap since lock was made on it.
	Assert(be->ref_count >= 0);
	be->ref_count++;					// Lock it before we page in data; this prevents a callback from freeing this
											// as it gets read in

	// Mark this bitmap as used this frame
	#ifdef BMPMAN_NDEBUG
	if ( be->used_this_frame < 255 )	{
		be->used_this_frame++;
	}
	#endif

	// if bitmap hasn't been loaded yet, then load it from disk
	// reread the bitmap from disk under certain conditions
	int pal_changed = 0;
	int rle_changed = 0;
	int fake_xparent_changed = 0;	
	if ( (bmp->data == NULL) || (bpp != bmp->bpp) || pal_changed || rle_changed || fake_xparent_changed ) {
		Assert(be->ref_count == 1);

		if ( be->type != BM_TYPE_USER ) {
			if ( bmp->data == NULL ) {
				nprintf (("BmpMan","Loading %s for the first time.\n", be->filename));
			} else if ( bpp != bmp->bpp ) {
				nprintf (("BmpMan","Reloading %s from bitdepth %d to bitdepth %d\n", be->filename, bmp->bpp, bpp));
			} else if ( pal_changed ) {
				nprintf (("BmpMan","Reloading %s to remap palette\n", be->filename));
			} else if ( rle_changed )	{
				nprintf (("BmpMan","Reloading %s to change RLE.\n", be->filename));
			} else if ( fake_xparent_changed )	{
				nprintf (("BmpMan","Reloading %s to change fake xparency.\n", be->filename));
			}
		}

		MONITOR_INC( NumBitmapPage, 1 );
		MONITOR_INC( SizeBitmapPage, bmp->w*bmp->h );

		if ( !Bm_paging )	{
			if ( be->type != BM_TYPE_USER ) {
				char flag_text[64];
				strcpy( flag_text, "--" );							
				nprintf(( "Paging", "Loading %s (%dx%dx%dx%s)\n", be->filename, bmp->w, bmp->h, bpp, flag_text ));
			}
		}

		// select proper format
		if((flags & BMP_AABITMAP) || (flags & BMP_TEX_ANY)){
			BM_SELECT_TEX_FORMAT();					
		} else {
			BM_SELECT_SCREEN_FORMAT();
		}

		switch ( be->type ) {
		case BM_TYPE_PCX:
			Assert(bpp != 16);
			bm_lock_pcx( handle, bitmapnum, be, bmp, bpp, flags );
			break;

		case BM_TYPE_ANI: 
			Assert(bpp != 16);
			bm_lock_ani( handle, bitmapnum, be, bmp, bpp, flags );
			break;

		case BM_TYPE_USER:	
			Assert(bpp != 16);
			bm_lock_user( handle, bitmapnum, be, bmp, bpp, flags );
			break;

		case BM_TYPE_TGA:
			bm_lock_tga( handle, bitmapnum, be, bmp, bpp, flags );
			break;

		default:
			Warning(LOCATION, "Unsupported type in bm_lock -- %d\n", be->type );
			return NULL;
		}		

		// always go back to screen format
		BM_SELECT_SCREEN_FORMAT();
	}

	if ( be->type == BM_TYPE_ANI ) {
		int i,first = bm_bitmaps[bitmapnum].info.ani.first_frame;

		for ( i=0; i< bm_bitmaps[first].info.ani.num_frames; i++ )	{
			// Mark all the bitmaps in this bitmap or animation as recently used
			bm_bitmaps[first+i].last_used = timer_get_milliseconds();

			// Mark all the bitmaps in this bitmap or animation as used for the usage tracker.
			#ifdef BMPMAN_NDEBUG
				bm_bitmaps[first+i].used_count++;
			#endif
			bm_bitmaps[first+i].used_flags = flags;
		}
	} else {
		// Mark all the bitmaps in this bitmap or animation as recently used
		be->last_used = timer_get_milliseconds();

		// Mark all the bitmaps in this bitmap or animation as used for the usage tracker.
		#ifdef BMPMAN_NDEBUG
			be->used_count++;
		#endif
		be->used_flags = flags;
	}

	return bmp;
}

// Unlocks a bitmap
//
// Decrements the ref_count member of the bitmap_entry struct.  A bitmap can only be unloaded
// when the ref_count is 0.
//
void bm_unlock( int handle )
{
	bitmap_entry	*be;
	bitmap			*bmp;

	int bitmapnum = handle % MAX_BITMAPS;
	Assert( bm_bitmaps[bitmapnum].handle == handle );	// INVALID BITMAP HANDLE

	Assert(bitmapnum >= 0 && bitmapnum < MAX_BITMAPS);
	if ( !bm_inited ) bm_init();

	be = &bm_bitmaps[bitmapnum];
	bmp = &be->bm;

	be->ref_count--;
	Assert(be->ref_count >= 0);		// Trying to unlock data more times than lock was called!!!

}


void bm_update()
{
}

char *bm_get_filename(int handle)
{
	int n;

	n = handle % MAX_BITMAPS;
	Assert(bm_bitmaps[n].handle == handle);		// INVALID BITMAP HANDLE
	return bm_bitmaps[n].filename;
}

void bm_get_palette(int handle, ubyte *pal, char *name)
{
	char *filename;
	int w,h;

	int n= handle % MAX_BITMAPS;
	Assert( bm_bitmaps[n].handle == handle );		// INVALID BITMAP HANDLE

	filename = bm_bitmaps[n].filename;

	if (name)	{
		strcpy( name, filename );
	}

	int pcx_error=pcx_read_header( filename, &w, &h, pal );
	if ( pcx_error != PCX_ERROR_NONE ){
		// Error(LOCATION, "Couldn't open '%s'\n", filename );
	}
}

// --------------------------------------------------------------------------------------
// bm_release()  - unloads the bitmap's data and entire slot, so bitmap 'n' won't be valid anymore
//
// parameters:		n		=>		index into bm_bitmaps ( index returned from bm_load() or bm_create() )
//
// returns:			nothing

void bm_release(int handle)
{
	bitmap_entry	*be;

	int n = handle % MAX_BITMAPS;

	Assert(n >= 0 && n < MAX_BITMAPS);
	be = &bm_bitmaps[n];

	if ( bm_bitmaps[n].type == BM_TYPE_NONE ) {
		return;	// Already been released?
	}

	if ( bm_bitmaps[n].type != BM_TYPE_USER )	{
		return;
	}

	Assert( be->handle == handle );		// INVALID BITMAP HANDLE

	// If it is locked, cannot free it.
	if (be->ref_count != 0) {
		nprintf(("BmpMan", "tried to unload %s that has a lock count of %d.. not unloading\n", be->filename, be->ref_count));
		return;
	}

	bm_free_data(n);

	if ( bm_bitmaps[n].type == BM_TYPE_USER )	{
		bm_bitmaps[n].info.user.data = NULL;
		bm_bitmaps[n].info.user.bpp = 0;
	}


	bm_bitmaps[n].type = BM_TYPE_NONE;

	// Fill in bogus structures!

	// For debugging:
	strcpy( bm_bitmaps[n].filename, "IVE_BEEN_RELEASED!" );
	bm_bitmaps[n].signature = 0xDEADBEEF;									// a unique signature identifying the data
	bm_bitmaps[n].palette_checksum = 0xDEADBEEF;							// checksum used to be sure bitmap is in current palette

	// bookeeping
	#ifdef BMPMAN_NDEBUG
	bm_bitmaps[n].data_size = -1;									// How much data this bitmap uses
	#endif
	bm_bitmaps[n].ref_count = -1;									// Number of locks on bitmap.  Can't unload unless ref_count is 0.

	// Bitmap info
	bm_bitmaps[n].bm.w = bm_bitmaps[n].bm.h = -1;
	
	// Stuff needed for animations
	// Stuff needed for user bitmaps
	memset( &bm_bitmaps[n].info, 0, sizeof(bm_extra_info) );

	bm_bitmaps[n].handle = -1;
}






// --------------------------------------------------------------------------------------
// bm_unload()  - unloads the data, but not the bitmap info.
//
// parameters:		n		=>		index into bm_bitmaps ( index returned from bm_load() or bm_create() )
//
// returns:			0		=>		unload failed
//						1		=>		unload successful
//
int bm_unload( int handle )
{
	bitmap_entry	*be;
	bitmap			*bmp;

	int n = handle % MAX_BITMAPS;

	Assert(n >= 0 && n < MAX_BITMAPS);
	be = &bm_bitmaps[n];
	bmp = &be->bm;

	if ( be->type == BM_TYPE_NONE ) {
		return 0;		// Already been released
	}

	Assert( be->handle == handle );		// INVALID BITMAP HANDLE!

	// If it is locked, cannot free it.
	if (be->ref_count != 0) {
		nprintf(("BmpMan", "tried to unload %s that has a lock count of %d.. not unloading\n", be->filename, be->ref_count));
		return 0;
	}

	nprintf(("BmpMan", "unloading %s.  %dx%dx%d\n", be->filename, bmp->w, bmp->h, bmp->bpp));
	bm_free_data(n);		// clears flags, bbp, data, etc

	return 1;
}


// unload all used bitmaps
void bm_unload_all()
{
	int i;

	for (i = 0; i < MAX_BITMAPS; i++)	{
		if ( bm_bitmaps[i].type != BM_TYPE_NONE )	{
			bm_unload(bm_bitmaps[i].handle);
		}
	}
}


DCF(bmpman,"Shows/changes bitmap caching parameters and usage")
{
	if ( Dc_command )	{
		dc_get_arg(ARG_STRING);
		if ( !strcmp( Dc_arg, "flush" ))	{
			dc_printf( "Total RAM usage before flush: %d bytes\n", bm_texture_ram );
			int i;
			for (i = 0; i < MAX_BITMAPS; i++)	{
				if ( bm_bitmaps[i].type != BM_TYPE_NONE )	{
					bm_free_data(i);
				}
			}
			dc_printf( "Total RAM after flush: %d bytes\n", bm_texture_ram );
		} else if ( !strcmp( Dc_arg, "ram" ))	{
			dc_get_arg(ARG_INT);
			Bm_max_ram = Dc_arg_int*1024*1024;
		} else {
			// print usage, not stats
			Dc_help = 1;
		}
	}

	if ( Dc_help )	{
		dc_printf( "Usage: BmpMan keyword\nWhere keyword can be in the following forms:\n" );
		dc_printf( "BmpMan flush    Unloads all bitmaps.\n" );
		dc_printf( "BmpMan ram x    Sets max mem usage to x MB. (Set to 0 to have no limit.)\n" );
		dc_printf( "\nUse '? BmpMan' to see status of Bitmap manager.\n" );
		Dc_status = 0;	// don't print status if help is printed.  Too messy.
	}

	if ( Dc_status )	{
		dc_printf( "Total RAM usage: %d bytes\n", bm_texture_ram );


		if ( Bm_max_ram > 1024*1024 )
			dc_printf( "Max RAM allowed: %.1f MB\n", i2fl(Bm_max_ram)/(1024.0f*1024.0f) );
		else if ( Bm_max_ram > 1024 )
			dc_printf( "Max RAM allowed: %.1f KB\n", i2fl(Bm_max_ram)/(1024.0f) );
		else if ( Bm_max_ram > 0 )
			dc_printf( "Max RAM allowed: %d bytes\n", Bm_max_ram );
		else
			dc_printf( "No RAM limit\n" );


	}
}

// Marks a texture as being used for this level
void bm_page_in_texture( int bitmapnum, int nframes )
{
	int i;
	for (i=0; i<nframes;i++ )	{
		int n = bitmapnum % MAX_BITMAPS;

		bm_bitmaps[n+i].preloaded = 1;

		if ( D3D_enabled )	{
			bm_bitmaps[n+i].used_flags = BMP_TEX_OTHER;
		} else {			
			bm_bitmaps[n+i].used_flags = 0;
		}
	}
}

// marks a texture as being a transparent textyre used for this level
// Marks a texture as being used for this level
// If num_frames is passed, assume this is an animation
void bm_page_in_xparent_texture( int bitmapnum, int nframes)
{
	int i;
	for (i=0; i<nframes;i++ )	{
		int n = bitmapnum % MAX_BITMAPS;

		bm_bitmaps[n+i].preloaded = 3;

		if ( D3D_enabled )	{
			// bm_bitmaps[n+i].used_flags = BMP_NO_PALETTE_MAP;
			bm_bitmaps[n+i].used_flags = BMP_TEX_XPARENT;
		} else {
			bm_bitmaps[n+i].used_flags = 0;
		}
	}
}

// Marks an aabitmap as being used for this level
void bm_page_in_aabitmap( int bitmapnum, int nframes )
{
	int i;

	for (i=0; i<nframes;i++ )	{
		int n = bitmapnum % MAX_BITMAPS;

		bm_bitmaps[n+i].preloaded = 2;
	
		if ( D3D_enabled )	{
			bm_bitmaps[n+i].used_flags = BMP_AABITMAP;
		} else {
			bm_bitmaps[n+i].used_flags = 0;
		}
	}
}



// Tell the bitmap manager to start keeping track of what bitmaps are used where.
void bm_page_in_start()
{
	int i;

	Bm_paging = 1;

	// Mark all as inited
	for (i = 0; i < MAX_BITMAPS; i++)	{
		if ( bm_bitmaps[i].type != BM_TYPE_NONE)	{
			
			if(bm_bitmaps[i].m_locked == true)
			{
				continue;
			}

			bm_unload(bm_bitmaps[i].handle);
		}
		bm_bitmaps[i].preloaded = 0;
		#ifdef BMPMAN_NDEBUG
			bm_bitmaps[i].used_count = 0;
		#endif
		bm_bitmaps[i].used_flags = 0;
	}

}

void bm_page_in_stop()
{	
	int i;	
	int ship_info_index;

	nprintf(( "BmpInfo","BMPMAN: Loading all used bitmaps.\n" ));

	// Load all the ones that are supposed to be loaded for this level.
	int n = 0;

	#ifdef BMPMAN_NDEBUG
	Bm_ram_freed = 0;
	#endif

	int preloading = 1;

	gr_tcache_flush();

	for (i = 0; i < MAX_BITMAPS; i++)	{
		if ( bm_bitmaps[i].type != BM_TYPE_NONE )	{
			if ( bm_bitmaps[i].preloaded )	{
#ifdef BMPMAN_SPECIAL_NONDARK
				// if this is a texture, check to see if a ship uses it
				ship_info_index = ship_get_texture(bm_bitmaps[i].handle);
				// use the colors from this ship
				if((ship_info_index >= 0) && (Ship_info[ship_info_index].num_nondark_colors > 0)){
					// mprintf(("Using custom pixels for %s\n", Ship_info[ship_info_index].name));
					palman_set_nondarkening(Ship_info[ship_info_index].nondark_colors, Ship_info[ship_info_index].num_nondark_colors);
				}
				// use the colors from the default table
				else {
					// mprintf(("Using default pixels\n"));
					palman_set_nondarkening(Palman_non_darkening_default, Palman_num_nondarkening_default);
				}
#endif

				// if preloaded == 3, load it as an xparent texture				
				if(bm_bitmaps[i].used_flags == BMP_AABITMAP){
					bm_lock( bm_bitmaps[i].handle, 8, bm_bitmaps[i].used_flags );
				} else {
					bm_lock( bm_bitmaps[i].handle, kDefaultBpp, bm_bitmaps[i].used_flags );
				}
				bm_unlock( bm_bitmaps[i].handle );

				if ( preloading )	{

					float u_scale, v_scale;
					const bool result = gr_tcache_set(
						bm_bitmaps[i].handle, 
						( bm_bitmaps[i].preloaded==2 ) ? 
							TCACHE_TYPE_AABITMAP : TCACHE_TYPE_NORMAL, 
						&u_scale, &v_scale, 1, true) != 0;

					if ( result == false )	{
						mprintf(( "Out of VRAM.  Done preloading.\n" ));
						preloading = 0;
						assert(0);
					}
				}

				n++;
				#ifdef BMPMAN_NDEBUG
				if ( Bm_ram_freed )	{
					nprintf(( "BmpInfo","BMPMAN: Not enough cache memory to load all level bitmaps\n" ));
					break;
				}
				#endif
			} 
		}
	}
	nprintf(( "BmpInfo","BMPMAN: Loaded %d bitmaps that are marked as used for this level.\n", n ));

	int total_bitmaps = 0;
	for (i = 0; i < MAX_BITMAPS; i++)	{
		if ( bm_bitmaps[i].type != BM_TYPE_NONE )	{
			total_bitmaps++;
		}
		if ( bm_bitmaps[i].type == BM_TYPE_USER )	{
			mprintf(( "User bitmap '%s'\n", bm_bitmaps[i].filename ));
		}
	}	

	mprintf(( "Bmpman: %d/%d bitmap slots in use.\n", total_bitmaps, MAX_BITMAPS ));
	//mprintf(( "Bmpman: Usage went from %d KB to %d KB.\n", usage_before/1024, usage_after/1024 ));

	Bm_paging = 0;
}

int bm_get_cache_slot( int bitmap_id, int separate_ani_frames )
{
	int n = bitmap_id % MAX_BITMAPS;

	Assert( bm_bitmaps[n].handle == bitmap_id );		// INVALID BITMAP HANDLE

	bitmap_entry	*be = &bm_bitmaps[n];

	if ( (!separate_ani_frames) && (be->type == BM_TYPE_ANI) )	{
		return be->info.ani.first_frame;
	} 

	return n;

}

// convert a 24 bit value to a 16 bit value
void bm_24_to_16(int bit_24, ushort *bit_16)
{
	ubyte *pixel = (ubyte*)&bit_24;
	ubyte alpha = 1;

	bm_set_components((ubyte*)bit_16, (ubyte*)&pixel[0], (ubyte*)&pixel[1], (ubyte*)&pixel[2], &alpha);	
}

void (*bm_set_components)(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a) = NULL;

void bm_set_components_argb_d3d_32_screen(ubyte *pixel, ubyte *rv, ubyte *gv, ubyte *bv, ubyte *av)
{
	*((uint*)pixel) |= (uint)(( (int)*rv / Gr_current_red->scale ) << Gr_current_red->shift);
	*((uint*)pixel) |= (uint)(( (int)*gv / Gr_current_green->scale ) << Gr_current_green->shift);
	*((uint*)pixel) |= (uint)(( (int)*bv / Gr_current_blue->scale ) << Gr_current_blue->shift);
	if(*av == 0){				
		*((uint*)pixel) = (uint)Gr_current_green->mask;		
	}
}

void bm_set_components_argb_d3d_32_tex(ubyte *pixel, ubyte *rv, ubyte *gv, ubyte *bv, ubyte *av)
{
	*((ushort*)pixel) |= (ushort)(( (int)*rv / Gr_current_red->scale ) << Gr_current_red->shift);
	*((ushort*)pixel) |= (ushort)(( (int)*gv / Gr_current_green->scale ) << Gr_current_green->shift);
	*((ushort*)pixel) |= (ushort)(( (int)*bv / Gr_current_blue->scale ) << Gr_current_blue->shift);
	*((ushort*)pixel) &= ~(Gr_current_alpha->mask);
	if(*av){
		*((ushort*)pixel) |= (ushort)(Gr_current_alpha->mask);
	} else {
		*((ushort*)pixel) = 0;
	}
}

// for selecting pixel formats
void BM_SELECT_SCREEN_FORMAT()
{
	Gr_current_red = &Gr_red;
	Gr_current_green = &Gr_green;
	Gr_current_blue = &Gr_blue;
	Gr_current_alpha = &Gr_alpha;

	bm_set_components = bm_set_components_argb_d3d_32_screen;
}

void BM_SELECT_TEX_FORMAT()
{
	Gr_current_red = &Gr_t_red; 
	Gr_current_green = &Gr_t_green; 
	Gr_current_blue = &Gr_t_blue; 
	Gr_current_alpha = &Gr_t_alpha;

	bm_set_components = bm_set_components_argb_d3d_32_tex;
}

// get the rgba components of a pixel, any of the parameters can be NULL
void bm_get_components(ubyte *pixel, ubyte *r, ubyte *g, ubyte *b, ubyte *a)
{
	// pick a byte size - 32 bits only if 32 bit mode d3d and screen format
	const int bit_32 = (Gr_current_red == &Gr_red);

	if(r != NULL){
		if(bit_32){
			*r = ubyte(( (*((uint*)pixel) & Gr_current_red->mask)>>Gr_current_red->shift)*Gr_current_red->scale);
		} else {
			*r = ubyte(( ( ((ushort*)pixel)[0] & Gr_current_red->mask)>>Gr_current_red->shift)*Gr_current_red->scale);
		}
	}
	if(g != NULL){
		if(bit_32){
			*g = ubyte(( (*((uint*)pixel) & Gr_current_green->mask) >>Gr_current_green->shift)*Gr_current_green->scale);
		} else {
			*g = ubyte(( ( ((ushort*)pixel)[0] & Gr_current_green->mask) >>Gr_current_green->shift)*Gr_current_green->scale);
		}
	}
	if(b != NULL){
		if(bit_32){
			*b = ubyte(( (*((uint*)pixel) & Gr_current_blue->mask)>>Gr_current_blue->shift)*Gr_current_blue->scale);
		} else {
			*b = ubyte(( ( ((ushort*)pixel)[0] & Gr_current_blue->mask)>>Gr_current_blue->shift)*Gr_current_blue->scale);
		}
	}

	// get the alpha value
	if(a != NULL){		
		*a = 1;

		// if we're writing to a normal texture, use nice alpha bits
		if(Gr_current_red == &Gr_t_red){				
			Assert(!bit_32);

			if(!(*((ushort*)pixel) & Gr_current_alpha->mask)){
				*a = 0;
			}
		}
		// otherwise do it as normal
		else {
			if(bit_32){
				if(*((int*)pixel) == Gr_current_green->mask){ 
					*a = 0;
				}
			} else {
				if(*((ushort*)pixel) == Gr_current_green->mask){ 
					*a = 0;
				}
			}
		}
	}
}

// get filename
void bm_get_filename(int bitmapnum, char *filename)
{
	int n = bitmapnum % MAX_BITMAPS;

	// return filename
	strcpy(filename, bm_bitmaps[n].filename);
}
