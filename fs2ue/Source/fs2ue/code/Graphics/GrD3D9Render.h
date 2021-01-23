#pragma once

struct shader;
struct color;

#ifndef FS2_UE
#ifndef UNITY_BUILD
#include "GlobalIncs/PsTypes.h"
#include <windows.h> // ifndef FS2_UE
#include <d3d9types.h>
#include "Graphics/GrD3D9Render.h"
#endif

//#define FIXED_PIPELINE 1

#ifdef FIXED_PIPELINE
#define FIXED_PIPELINE_SPECULAR 1
#endif

struct Vertex
{
	Vertex() : sx(0.0f)
		, sy(0.0f)
		, sz(0.0f)
		, m_rhw(1.0f)
		, color(0)
#ifdef FIXED_PIPELINE_SPECULAR
		, m_specular(0)
#endif
	{
	}

	void SetRhw(float rhw)
	{
#ifdef FIXED_PIPELINE
		m_rhw = rhw;
#endif
	}

	void SetSpecular(float specular)
	{
#ifdef FIXED_PIPELINE_SPECULAR
		m_specular = specular;
#endif
	}

	float sx, sy, sz;
private:
	float m_rhw;
public:
	DWORD color;
#ifdef FIXED_PIPELINE_SPECULAR
private:
	DWORD m_specular;
public:
#endif
	float tu, tv;
};

typedef enum gr_texture_source {
	TEXTURE_SOURCE_NONE,
	TEXTURE_SOURCE_DECAL,
	TEXTURE_SOURCE_NO_FILTERING,
} gr_texture_source;

typedef enum gr_alpha_blend {
	ALPHA_BLEND_NONE,							// 1*SrcPixel + 0*DestPixel
	ALPHA_BLEND_ALPHA_ADDITIVE,			// Alpha*SrcPixel + 1*DestPixel
	ALPHA_BLEND_ALPHA_BLEND_ALPHA,		// Alpha*SrcPixel + (1-Alpha)*DestPixel
	ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR,	// Alpha*SrcPixel + (1-SrcPixel)*DestPixel
} gr_alpha_blend;

typedef enum gr_zbuffer_type {
	ZBUFFER_TYPE_NONE,
	ZBUFFER_TYPE_READ,
	ZBUFFER_TYPE_WRITE,
	ZBUFFER_TYPE_FULL,
} gr_zbuffer_type;

void SetupRenderSystem();
void ResetRenderSystem();
void CleanupRenderSystem();

HRESULT DrawPrimitive2D(D3DPRIMITIVETYPE primType, const int primCount, void *data, int flags);
HRESULT SetRenderState( D3DRENDERSTATETYPE dwRenderStateType,  DWORD dwRenderState );

void gr_d3d9_set_state( gr_texture_source ts, gr_alpha_blend ab, gr_zbuffer_type zt );

void gr_d3d9_pixel(int x, int y);
void gr_d3d9_clear();
void gr_d3d9_set_clip(int x,int y,int w,int h);
void gr_d3d9_reset_clip();
void gr_d3d9_set_font(int fontnum);
void gr_d3d9_set_color( int r, int g, int b );
void gr_d3d9_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha );
void gr_d3d9_create_shader(shader * shade, float r, float g, float b, float c );
void gr_d3d9_set_shader( shader * shade );
void gr_d3d9_bitmap(int x, int y);
void gr_d3d9_rect(int x,int y,int w,int h);
void gr_d3d9_shade(int x,int y,int w,int h);
void gr_d3d9_string(int x,int y, const char * const text);
void gr_d3d9_scaler(vertex *va, vertex *vb );
void gr_d3d9_tmapper( int nv, vertex * verts[], uint flags );
void gr_d3d9_set_palette(ubyte *new_palette, int is_alphacolor);
void gr_d3d9_init_color(color *c, int r, int g, int b);
void gr_d3d9_set_color_fast(color *dst);
void gr_d3d9_print_screen(char *filename);
void gr_d3d9_fog_set(int fog_mode, int r, int g, int b, float near, float far);
void gr_d3d9_get_pixel(int x, int y, int *r, int *g, int *b);
void gr_d3d9_get_region(int front, int w, int g, ubyte *data);
void gr_d3d9_set_cull(int cull);
void gr_d3d9_filter_set(int filter);
void gr_d3d9_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct);
int gr_d3d9_tcache_set(int bitmap_id, int bitmap_type, float *u_ratio, float *v_ratio, int fail_on_full = 0, const bool justCacheNoSet = false);
void gr_d3d9_set_clear_color(int r, int g, int b);
void gr_d3d9_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type );
uint gr_d3d9_lock();
void gr_d3d9_unlock();
void gr_d3d9_aabitmap(int x, int y);
void gr_d3d9_aabitmap_ex(int x,int y,int w,int h,int sx,int sy);
void gr_d3d9_dump_frame();
void gr_d3d9_dump_frame_start( int first_frame_number, int nframes_between_dumps );
void gr_d3d9_dump_frame_stop();
void gr_d3d9_set_gamma(float gamma);
void gr_d3d9_aascaler(vertex *va, vertex *vb );
void gr_d3d9_flash( int r, int g, int b );
int gr_d3d9_zbuffer_get();
int gr_d3d9_zbuffer_set(int mode);
void gr_d3d9_zbuffer_clear(int use_zbuffer);
int gr_d3d9_save_screen();
void gr_d3d9_restore_screen(int id);
void gr_d3d9_free_screen(int id);
void gr_d3d9_zbias(int zbias);

#endif
