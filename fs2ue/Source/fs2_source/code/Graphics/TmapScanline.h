/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 




#ifndef _TMAPSCANLINE_H
#define _TMAPSCANLINE_H


typedef struct tmapper_vertex {
	float sx, sy, sw, u, v, b;
} tmapper_vertex;

typedef struct tmapper_data {
	// These are filled once for each texture map being drawn.
	// The inner loop cannot change any of these!!
	int		nv;						// number of vertices
	ubyte		*pixptr;
	bitmap	*bp;
	int		src_offset;
	uint		flags;
	float		FixedScale;				// constant for asm inner loop
	float		FixedScale8;			// constant for asm inner loop
	float		One;						// constant for asm inner loop

	// This are filled in by the outer loop before each scan line.
	int		loop_count;
	tmapper_vertex	l, r, dl, dr, deltas;
	int		lx, rx, y;
	float		clipped_left;			// how many pixels were clipped off the left in 2d.

	// These are used internally by the assembly texture mapper.
	fix		fx_l, fx_l_right, fx_dl_dx;
	fix		fx_u, fx_v, fx_du_dx, fx_dv_dx;
	float		fl_dudx_wide;
	float		fl_dvdx_wide;
	float		fl_dwdx_wide;
	uint		dest_row_data;
	int		num_big_steps;
	uint		uv_delta[2];
	float		FloatTemp;
	uint		Subdivisions;
	uint		WidthModLength;
	uint		BlendLookup;
	uint		FadeLookup;
	uint		DeltaU;
	uint		DeltaV;
	uint		DeltaUFrac, DeltaVFrac;
   uint		UFixed;
	uint		VFixed;
   ushort	FPUCW;
	ushort	OldFPUCW;
	int		InnerLooper;
	uint		pScreenBits;
	int		fx_w;
	int		fx_dwdx;

	uint		saved_esp;
	uint		lookup;

} tmapper_data;

extern tmapper_data Tmap;

extern void tmapscan_generic();
extern void tmapscan_generic8();
//extern void tmapscan_pnn();
extern void tmapscan_pln();
extern void tmapscan_lln();
extern void tmapscan_flat();
extern void tmapscan_nebula8();
extern void tmapscan_flat_z();

extern void tmapscan_flat8();
extern void tmapscan_lln8();
extern void tmapscan_lnt8();
extern void tmapscan_lnn8();
extern void tmapscan_lnt8();
extern void tmapscan_lln8_tiled();
extern void tmapscan_llt8_tiled();
extern void tmapscan_pln8();
extern void tmapscan_plt8();

extern void tmapscan_lnaa8();

extern void tmapscan_pln8_tiled();

extern void tmapscan_lnn8_tiled_256x256();
extern void tmapscan_pnn8_tiled_256x256_subspace();

extern void tmapscan_flat_gouraud();

extern void tmapscan_nebula8();

#endif
