/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _TMAPPER_H
#define _TMAPPER_H

#define TMAP_MAX_VERTS	25		// Max number of vertices per polygon

enum TMAP_FLAGS
{
// Flags to pass to g3_draw_??? routines
	TMAP_FLAG_TEXTURED		=	(1<<0),	// Uses texturing (Interpolate uv's)
	TMAP_FLAG_CORRECT		=	(1<<1),	// Perspective correct (Interpolate sw)
	TMAP_FLAG_RAMP			=	(1<<2),	// Use RAMP lighting (interpolate L)
	TMAP_FLAG_RGB			=	(1<<3),	// Use RGB lighting (interpolate RGB)
	TMAP_FLAG_GOURAUD		=	(1<<4),	// Lighting values differ on each vertex. 
														// If this is not set, then the texture mapper will use
														// the lighting parameters in each vertex, otherwise it
														// will use the ones specified in tmapper_set_??
	TMAP_FLAG_XPARENT		=	(1<<5),	// texture could have transparency
	TMAP_FLAG_TILED			=	(1<<6),	// This means uv's can be > 1.0
	TMAP_FLAG_NEBULA		=	(1<<7),	// Must be used with RAMP and GOURAUD.  Means l 0-1 is 0-31 palette entries

	TMAP_MAX_SCANLINES		=	(1<<8),

// Add any entries that don't work for software under here:
// Make sure to disable them at top of grx_tmapper
	TMAP_FLAG_ALPHA			=	(1<<9),	// Has an alpha component
	TMAP_FLAG_NONDARKENING	=   (1<<10),	// RGB=255,255,255 doesn't darken

// flags for full nebula effect
	TMAP_FLAG_PIXEL_FOG	    =	(1<<11),	// fog the polygon based upon the average pixel colors of the backbuffer behind it
	TMAP_FLAG_SIMPLE_FOG     =  (1<<12),
	TMAP_FLAG_NO_FILTERING   =  (1<<13), 
	TMAP_FLAG_NO_TEXTURE     =  (1<<14),
};

#endif
