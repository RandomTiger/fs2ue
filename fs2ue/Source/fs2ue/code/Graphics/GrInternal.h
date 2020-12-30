/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _GRINTERNAL_H
#define _GRINTERNAL_H

#ifndef UNITY_BUILD
//#include "Font.h"
#include "2d.h"
#endif

inline ubyte *GR_SCREEN_PTR(int x, int y)
{
	return ((ubyte *) gr_screen.offscreen_buffer) + 
		(x+gr_screen.offset_x)*sizeof(ubyte) + 
		(y+gr_screen.offset_y)*gr_screen.rowsize;
}
// #define GR_SCREEN_PTR_SIZE(bpp,x,y) ((uint)(uint(gr_screen.offscreen_buffer) + uint(((x)+gr_screen.offset_x)*(bpp)) + uint(((y)+gr_screen.offset_y)*gr_screen.rowsize)))

extern ubyte Gr_original_palette[768];		// The palette 
extern ubyte Gr_current_palette[768];

typedef struct alphacolor {
	int	used;
	int	r,g,b,alpha;
	int	type;						// See AC_TYPE_??? define
	color	*clr;
	/*
	union {
		ubyte		lookup[16][256];		// For 8-bpp rendering modes
	} table;
	*/
} alphacolor;

// for backwards fred aabitmap compatibility
typedef struct alphacolor_old {
	int	used;
	int	r,g,b,alpha;
	int	type;						// See AC_TYPE_??? define
	color	*clr;	
	union {
		ubyte		lookup[16][256];		// For 8-bpp rendering modes
	} table;	
} alphacolor_old;

extern alphacolor * Current_alphacolor;
void gr_init_alphacolors();

extern char Gr_current_palette_name[128];

typedef struct color_gun {
	int	bits;
	int	shift;
	int	scale;
	int	mask;
} color_gun;

// screen format
extern color_gun Gr_red, Gr_green, Gr_blue, Gr_alpha;

// texture format
extern color_gun Gr_t_red, Gr_t_green, Gr_t_blue, Gr_t_alpha;

// alpha texture format
extern color_gun Gr_ta_red, Gr_ta_green, Gr_ta_blue, Gr_ta_alpha;

// CURRENT FORMAT - note - this is what bmpman uses when fiddling with pixels/colors. so be sure its properly set to one
// of the above values
extern color_gun *Gr_current_red, *Gr_current_green, *Gr_current_blue, *Gr_current_alpha;


// Translate the 768 byte 'src' palette into 
// the current screen format's palette.
// The size of the dst array is assumed to be gr_screen.bpp
// bytes per element.
void gr_xlat_palette( void *dst, bitmap *bmp );

extern float Gr_gamma;
extern int Gr_gamma_int;				// int(Gr_gamma*100)
extern int Gr_gamma_lookup[256];

enum
{
	TCACHE_TYPE_AABITMAP,						// HUD bitmap.  All Alpha.
	TCACHE_TYPE_NORMAL,							// Normal bitmap. Alpha = 0.
	TCACHE_TYPE_XPARENT,						// Bitmap with 0,255,0 = transparent.  Alpha=0 if transparent, 1 if not.
	TCACHE_TYPE_INTERFACE,							
};

#endif

