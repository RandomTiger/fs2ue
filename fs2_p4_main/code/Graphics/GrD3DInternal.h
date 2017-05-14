/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 

#ifndef _GRD3DINTERNAL_H
#define _GRD3DINTERNAL_H

#ifndef UNITY_BUILD
#include "DirectX\vD3d.h"
#include "DirectX\vD3dcaps.h"
#include "DirectX\vd3drm.h"
#include "DirectX\vd3drmdef.h"
#include "DirectX\vd3drmobj.h"
#include "DirectX\vd3drmwin.h"
#include "DirectX\vD3dtypes.h"
#include "DirectX\vDdraw.h"
#include "Graphics\GrInternal.h"
#include "GlobalIncs\SafeArray.h"

#include <windows.h>
#include <windowsx.h>

#define D3D_OVERLOADS
#include "vddraw.h"

#include "vd3d.h"
#include "2d.h"
#include "GrInternal.h"
#endif

struct color;
struct shader;

extern LPDIRECTDRAW			lpDD1;
extern LPDIRECTDRAW2			lpDD;
extern LPDIRECT3D2			lpD3D;
extern LPDIRECT3DDEVICE		lpD3DDeviceEB; 
extern LPDIRECT3DDEVICE2	lpD3DDevice; 
extern LPDIRECTDRAWSURFACE	lpBackBuffer;
extern LPDIRECTDRAWSURFACE	lpFrontBuffer;
extern LPDIRECTDRAWSURFACE	lpZBuffer;

extern LPDIRECT3DVIEWPORT2	lpViewport;
extern LPDIRECTDRAWPALETTE	lpPalette;

extern DDPIXELFORMAT			AlphaTextureFormat32;
extern DDPIXELFORMAT			NonAlphaTextureFormat32;
extern DDPIXELFORMAT			AlphaTextureFormat;
extern DDPIXELFORMAT			NonAlphaTextureFormat;
extern DDPIXELFORMAT			ScreenFormat;

extern D3DDEVICEDESC D3DHWDevDesc, D3DHELDevDesc;
extern LPD3DDEVICEDESC lpDevDesc;
extern DDCAPS DD_driver_caps;
extern DDCAPS DD_hel_caps;

extern int D3D_texture_divider;

extern char* d3d_error_string(HRESULT error);

void d3d_tcache_init();
void d3d_tcache_cleanup();
void d3d_tcache_flush();
void d3d_tcache_frame();

int d3d_tcache_set(int bitmap_id, int bitmap_type, float *u_ratio, float *v_ratio, int fail_on_full=0, const bool justCacheNoSet = false);

// Functions in GrD3DRender.cpp stuffed into gr_screen structure

void gr_d3d_flash(int r, int g, int b);
void gr_d3d_zbuffer_clear(int mode);
int gr_d3d_zbuffer_get();
int gr_d3d_zbuffer_set(int mode);
void gr_d3d_tmapper( int nverts, vertex **verts, uint flags );
void gr_d3d_scaler(vertex *va, vertex *vb );
void gr_d3d_aascaler(vertex *va, vertex *vb );
void gr_d3d_pixel(int x, int y);
void gr_d3d_clear();
void gr_d3d_set_clip(int x,int y,int w,int h);
void gr_d3d_reset_clip();
void gr_d3d_init_color(color *c, int r, int g, int b);
void gr_d3d_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type );
void gr_d3d_set_color( int r, int g, int b );
void gr_d3d_set_color_fast(color *dst);
void gr_d3d_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha);
void gr_d3d_bitmap(int x, int y);
void gr_d3d_aabitmap_ex(int x,int y,int w,int h,int sx,int sy);
void gr_d3d_aabitmap(int x, int y);
void gr_d3d_create_shader(shader * shade, float r, float g, float b, float c );
void gr_d3d_set_shader( shader * shade );
void gr_d3d_create_font_bitmap();
void gr_d3d_char(int x,int y,int letter);
void gr_d3d_string( int sx, int sy, const char *s );
void gr_d3d_circle( int xc, int yc, int d );
void gr_d3d_line(int x1,int y1,int x2,int y2);
void gr_d3d_aaline(vertex *v1, vertex *v2);
void gr_d3d_gradient(int x1,int y1,int x2,int y2);
void gr_d3d_set_palette(ubyte *new_palette, int restrict_alphacolor);
void gr_d3d_diamond(int x, int y, int width, int height);
void gr_d3d_print_screen(char *filename);


// Functions used to render.  Calls either DrawPrim or Execute buffer code
HRESULT d3d_SetRenderState( D3DRENDERSTATETYPE dwRenderStateType,  DWORD dwRenderState );
HRESULT d3d_DrawPrimitive( D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags );
void d3d_batch(D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, SafeArray<D3DTLVERTEX> &lpvVertices, DWORD dwVertexCount, DWORD dwFlags );
void d3d_batch_render();

void d3d_zbias(int bias);

#endif //_GRD3DINTERNAL_H
