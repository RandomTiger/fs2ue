/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef _PCXUTILS_H
#define _PCXUTILS_H

#ifndef UNITY_BUILD
#include "GlobalIncs/PsTypes.h"
#endif


/*
#ifdef __cplusplus
extern "C" {
#endif
*/

#define PCX_ERROR_NONE 				0
#define PCX_ERROR_OPENING			1
#define PCX_ERROR_NO_HEADER		2
#define PCX_ERROR_WRONG_VERSION	3
#define PCX_ERROR_READING			4
#define PCX_ERROR_NO_PALETTE		5
#define PCX_ERROR_WRITING			6
#define PCX_ERROR_MEMORY			7

extern int pcx_read_header(char *filename, int *w, int *h, ubyte *pal );
extern int pcx_read_bitmap_8bpp( char * filename, ubyte *org_data, ubyte *palette );
extern int pcx_read_bitmap_16bpp( char * filename, ubyte *org_data );
extern int pcx_read_bitmap_16bpp_aabitmap( char *filename, ubyte *org_data );
extern int pcx_read_bitmap_32bpp( char * real_filename, ubyte *org_data );


// Dumps a 8bpp bitmap to a file.
// Set rowoff to -w for upside down bitmaps.
extern int pcx_write_bitmap( char * filename, int w, int h, ubyte ** row_ptrs, ubyte * palette );


/*
#ifdef __cplusplus
}
#endif
*/

#endif