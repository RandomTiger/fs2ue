/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _GRAPHICS_H
#define _GRAPHICS_H

/* ========================= pixel plotters =========================
In the 2d/texture mapper, bitmaps to be drawn will be passed by number.
The 2d function will call a bmpman function to get the bitmap into whatever
format it needs.  Then it will render.   The only pixels that will ever 
get drawn go thru the 2d/texture mapper libraries only.   This will make
supporting accelerators and psx easier.   Colors will always be set with
the color set functions.

gr_surface_flip()	switch onscreen, offscreen

gr_set_clip(x,y,w,h)	// sets the clipping region
gr_reset_clip(x,y,w,h)	// sets the clipping region
gr_set_color --? 8bpp, 15bpp?
gr_set_font(int fontnum)
// see GR_ALPHABLEND defines for values for alphablend_mode
// see GR_BITBLT_MODE defines for bitblt_mode.
// Alpha = scaler for intensity
gr_set_bitmap( int bitmap_num, int alphblend_mode, int bitblt_mode, float alpha )	
gr_set_shader( int value )  0=normal -256=darken, 256=brighten
gr_set_palette( ubyte * palette ) 

gr_clear()	// clears entire clipping region
gr_bitmap(x,y)
gr_bitmap_ex(x,y,w,h,sx,sy)
gr_rect(x,y,w,h)
gr_shade(x,y,w,h)
gr_string(x,y,char * text)
gr_line(x1,y1,x2,y2)

 
*/

#ifndef UNITY_BUILD
#include "GlobalIncs/PsTypes.h"
#include "Tmapper.h"
#endif

struct vertex;
class vector;
struct shader;

// This is a structure used by the shader to keep track
// of the values you want to use in the shade primitive.
typedef struct shader {
	uint	screen_sig;					// current mode this is in
	float	r,g,b,c;						// factors and constant
	ubyte	lookup[256];
} shader;

#define AC_TYPE_NONE		0		// Not an alphacolor
#define AC_TYPE_HUD		1		// Doesn't change hue depending on background.  Used for HUD stuff.
#define AC_TYPE_BLEND	2		// Changes hue depending on background.  Used for stars, etc.

#define GR_ALPHABLEND_NONE			0		// no blending
#define GR_ALPHABLEND_FILTER		1		// 50/50 mix of foreground, background, using intensity as alpha

#define GR_BITBLT_MODE_NORMAL		0		// Normal bitblting
#define GR_BITBLT_MODE_RLE			1		// RLE would be faster

// fog modes
#define GR_FOGMODE_NONE				0		// set this to turn off fog
#define GR_FOGMODE_FOG				1		// linear fog

typedef struct screen {
	uint	signature;			// changes when mode or palette or width or height changes
	int	max_w, max_h;		// Width and height
	int	res;					// GR_640 or GR_1024
	int	mode;					// What mode gr_init was called with.
	float	aspect;				// Aspect ratio
	int	rowsize;				// What you need to add to go to next row (includes bytes_per_pixel)
	int	bits_per_pixel;	// How many bits per pixel it is. (7,8,15,16,24,32)
	int	bytes_per_pixel;	// How many bytes per pixel (1,2,3,4)
	int	offset_x, offset_y;		// The offsets into the screen
	int	clip_width, clip_height;

	float fog_near, fog_far;

	// the clip_l,r,t,b are used internally.  left and top are
	// actually always 0, but it's nice to have the code work with
	// arbitrary clipping regions.
	int		clip_left, clip_right, clip_top, clip_bottom;	

	int		current_alphablend_mode;		// See GR_ALPHABLEND defines above
	int		current_bitblt_mode;				// See GR_BITBLT_MODE defines above
	int		current_fog_mode;					// See GR_FOGMODE_* defines above
	int		current_bitmap;
	int		current_bitmap_sx;				// bitmap x section
	int		current_bitmap_sy;				// bitmap y section
	color		current_color;
	color		current_fog_color;				// current fog color
	color		current_clear_color;				// current clear color
	shader	current_shader;
	float		current_alpha;
	void		*offscreen_buffer;				// NEVER ACCESS!  This+rowsize*y = screen offset
	void		*offscreen_buffer_base;			// Pointer to lowest address of offscreen buffer

	//switch onscreen, offscreen
	void (*gf_flip)();

	// Sets the current palette
 	void gf_set_palette(ubyte * new_pal, int restrict_alphacolor = 0)
	{
		gf_set_palette_ptr( new_pal, restrict_alphacolor);
	}
	void (*gf_set_palette_ptr)(ubyte * new_pal, int restrict_alphacolor);

	// Flash the screen
	void (*gf_flash)( int r, int g, int b );

	// sets the clipping region
	void (*gf_set_clip)(int x, int y, int w, int h);

	// resets the clipping region to entire screen
	void (*gf_reset_clip)();

	void (*gf_set_color)( int r, int g, int b );
	void (*gf_init_color)( color * dst, int r, int g, int b );

 	void gf_init_alphacolor( color * dst, int r, int g, int b, int alpha, int type=AC_TYPE_HUD )
	{
		gf_init_alphacolor_ptr( dst, r, g, b, alpha, type);
	}
	void (*gf_init_alphacolor_ptr)( color * dst, int r, int g, int b, int alpha, int type);
	void (*gf_set_color_fast)( color * dst );

	void (*gf_set_font)(int fontnum);

	// Sets the current bitmap
	void gf_set_bitmap( int bitmap_num, int alphablend=GR_ALPHABLEND_NONE, int bitbltmode=GR_BITBLT_MODE_NORMAL, float alpha=1.0f ) 
	{
		gf_set_bitmap_ptr( bitmap_num, alphablend, bitbltmode, alpha);
	}
	void (*gf_set_bitmap_ptr)( int bitmap_num, int alphablend, int bitbltmode, float alpha);

	// Call this to create a shader.   
	// This function takes a while, so don't call it once a frame!
	// r,g,b, and c should be between -1.0 and 1.0f

	// The matrix is used as follows:
	// Dest(r) = Src(r)*r + Src(g)*r + Src(b)*r + c;
	// Dest(g) = Src(r)*g + Src(g)*g + Src(b)*g + c;
	// Dest(b) = Src(r)*b + Src(g)*b + Src(b)*b + c;
	// For instance, to convert to greyscale, use
	// .3 .3 .3  0
	// To turn everything green, use:
	//  0 .3  0  0
	void (*gf_create_shader)(shader * shade, float r, float g, float b, float c );

	// Initialize the "shader" by calling gr_create_shader()
	// Passing a NULL makes a shader that turns everything black.
	void (*gf_set_shader)( shader * shade );

	// clears entire clipping region to current color
	void (*gf_clear)();

	void (*gf_bitmap)(int x,int y);

	void (*gf_aabitmap)(int x, int y);
	void (*gf_aabitmap_ex)(int x, int y, int w, int h, int sx, int sy);

	void (*gf_string)(int x, int y, const char *text);

	// Draw a gradient line... x1,y1 is bright, x2,y2 is transparent.
	void (*gf_gradient)(int x1, int y1, int x2, int y2);
 
	void (*gf_circle)(int x, int y, int r);

	// Integer line. Used to draw a fast but pixely line.  
	void (*gf_line)(int x1, int y1, int x2, int y2);

	// Draws an antialiased line is the current color is an 
	// alphacolor, otherwise just draws a fast line.  This
	// gets called internally by g3_draw_line.   This assumes
	// the vertex's are already clipped, so call g3_draw_line
	// not this if you have two 3d points.
	void (*gf_aaline)(vertex *v1, vertex *v2);

	void (*gf_pixel)( int x, int y );

	// Scales current bitmap between va and vb with clipping
	void (*gf_scaler)(vertex *va, vertex *vb );

	// Scales current bitmap between va and vb with clipping, draws an aabitmap
	void (*gf_aascaler)(vertex *va, vertex *vb );

	// Texture maps the current bitmap.  See TMAP_FLAG_?? defines for flag values
	void (*gf_tmapper)(int nv, vertex *verts[], uint flags );

	// dumps the current screen to a file
	void (*gf_print_screen)(char * filename);

	// Call once before rendering anything.
	void (*gf_start_frame)();

	// Call after rendering is over.
	void (*gf_stop_frame)();

	// Retrieves the zbuffer mode.
	int (*gf_zbuffer_get)();

	// Sets mode.  Returns previous mode.
	int (*gf_zbuffer_set)(int mode);

	// Clears the zbuffer.  If use_zbuffer is FALSE, then zbuffering mode is ignored and zbuffer is always off.
	void (*gf_zbuffer_clear)(int use_zbuffer);
	
	// Saves screen. Returns an id you pass to restore and free.
	int (*gf_save_screen)();
	
	// Resets clip region and copies entire saved screen to the screen.
	void (*gf_restore_screen)(int id);

	// Frees up a saved screen.
	void (*gf_free_screen)(int id);

	// CODE FOR DUMPING FRAMES TO A FILE
	// Begin frame dumping
	void (*gf_dump_frame_start)( int first_frame_number, int nframes_between_dumps );

	// Dump the current frame to file
	void (*gf_dump_frame)();

	// Dump the current frame to file
	void (*gf_dump_frame_stop)();

	// Sets the gamma
	void (*gf_set_gamma)(float gamma);

	// Lock/unlock the screen
	// Returns non-zero if sucessful (memory pointer)
	uint (*gf_lock)();
	void (*gf_unlock)();

	// grab a region of the screen. assumes data is large enough
	void (*gf_get_region)(int front, int w, int h, ubyte *data);

	// set fog attributes
 	void gf_fog_set(int fog_mode, int r, int g, int b, float _fog_near = -1.0f, float _fog_far = -1.0f)
	{
		gf_fog_set_ptr(fog_mode, r, g, b, _fog_near, _fog_far);	
	}
	void (*gf_fog_set_ptr)(int fog_mode, int r, int g, int b, float _fog_near, float _fog_far);	

	// poly culling
	void (*gf_set_cull)(int cull);

	// cross fade
	void (*gf_cross_fade)(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct);

	// set a texture into cache. for sectioned bitmaps, pass in sx and sy to set that particular section of the bitmap
	int gf_tcache_set(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full = 0, const bool justCacheNoSet = false)
	{
		return gf_tcache_set_ptr(bitmap_id, bitmap_type, u_scale, v_scale, fail_on_full, justCacheNoSet);	
	}
	int (*gf_tcache_set_ptr)(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full, const bool justCacheNoSet);	

	// set the color to be used when clearing the background
	void (*gf_set_clear_color)(int r, int g, int b);

	void (*gf_clean)();

	void (*gf_zbias)(int zbias);
	void (*gf_tcache_flush)();

} screen;

extern int gr_zbuffering_mode;
extern int Gr_cursor;

// handy macro
#define GR_MAYBE_CLEAR_RES(bmap)		do  { int bmw = -1; int bmh = -1; if(bmap != -1){ bm_get_info( bmap, &bmw, &bmh); if((bmw != gr_screen.max_w) || (bmh != gr_screen.max_h)){gr_clear();} } else {gr_clear();} } while(0);

//Window's interface to set up graphics:
//--------------------------------------
// Call this at application startup

enum
{
	GR_DIRECT3D5,
	GR_DIRECT3D9,
	GR_DUMMY,
};

// resolution constants   - always keep resolutions in ascending order and starting from 0  
#define GR_NUM_RESOLUTIONS			2
#define GR_640							0		// 640 x 480
#define GR_1024						1		// 1024 x 768

extern int gr_init(int res, int mode, int depth = 16, int fred_x = -1, int fred_y = -1);

// Call this when your app ends.
extern void gr_close();

extern screen gr_screen;

#define GR_ZBUFF_NONE	0
#define GR_ZBUFF_WRITE	(1<<0)
#define GR_ZBUFF_READ	(1<<1)
#define GR_ZBUFF_FULL	(GR_ZBUFF_WRITE|GR_ZBUFF_READ)

// Returns -1 if couldn't init font, otherwise returns the
// font id number.  If you call this twice with the same typeface,
// it will return the same font number both times.  This font is
// then set to be the current font, and default font if none is 
// yet specified.
int gr_init_font( char * typeface );

// Does formatted printing.  This calls gr_string after formatting,
// so if you don't need to format the string, then call gr_string
// directly.
extern void _cdecl gr_printf( int x, int y, char * format, ... );

// Returns the size of the string in pixels in w and h
extern void gr_get_string_size( int *w, int *h, char * text, int len = 9999 );

// Returns the height of the current font
extern int gr_get_font_height();

extern void gr_set_palette(char *name, ubyte *palette, int restrict_to_128 = 0);

// These two functions use a Windows mono font.  Only for use
// in the editor, please.
void gr_get_string_size_win(int *w, int *h, char *text);
void gr_string_win(int x, int y, char *s );

// set the mouse pointer to a specific bitmap, used for animating cursors
#define GR_CURSOR_LOCK		1
#define GR_CURSOR_UNLOCK	2
void gr_set_cursor_bitmap(int n, int lock = 0);
int gr_get_cursor_bitmap();
extern int Web_cursor_bitmap;

// Called by OS when application gets/looses focus
extern void gr_activate(int active);

#define GR_CALL(x)			(*x)
#define GR_CALL_NOPTR(x)	(x)

// These macros make the function indirection look like the
// old Descent-style gr_xxx calls.

#define gr_print_screen		GR_CALL(gr_screen.gf_print_screen)

#define gr_flip				GR_CALL(gr_screen.gf_flip)

#define gr_set_clip			GR_CALL(gr_screen.gf_set_clip)
#define gr_reset_clip		GR_CALL(gr_screen.gf_reset_clip)
#define gr_set_font			GR_CALL(gr_screen.gf_set_font)

#define gr_init_color		GR_CALL(gr_screen.gf_init_color)
#define gr_init_alphacolor	GR_CALL_NOPTR(gr_screen.gf_init_alphacolor)
#define gr_set_color			GR_CALL(gr_screen.gf_set_color)
#define gr_set_color_fast	GR_CALL(gr_screen.gf_set_color_fast)

#define gr_set_bitmap		GR_CALL_NOPTR(gr_screen.gf_set_bitmap)

#define gr_create_shader	GR_CALL(gr_screen.gf_create_shader)
#define gr_set_shader		GR_CALL(gr_screen.gf_set_shader)
#define gr_clear				GR_CALL(gr_screen.gf_clear)
#define gr_bitmap				GR_CALL(gr_screen.gf_bitmap)
#define gr_aabitmap			GR_CALL(gr_screen.gf_aabitmap)
#define gr_aabitmap_ex		GR_CALL(gr_screen.gf_aabitmap_ex)
#define gr_string				GR_CALL(gr_screen.gf_string)

#define gr_circle				GR_CALL(gr_screen.gf_circle)

#define gr_line				GR_CALL(gr_screen.gf_line)
#define gr_aaline				GR_CALL(gr_screen.gf_aaline)
#define gr_pixel				GR_CALL(gr_screen.gf_pixel)
#define gr_scaler				GR_CALL(gr_screen.gf_scaler)
#define gr_aascaler			GR_CALL(gr_screen.gf_aascaler)
#define gr_tmapper			GR_CALL(gr_screen.gf_tmapper)

#define gr_gradient			GR_CALL(gr_screen.gf_gradient)
#define gr_flash				GR_CALL(gr_screen.gf_flash)

#define gr_zbuffer_get		GR_CALL(gr_screen.gf_zbuffer_get)
#define gr_zbuffer_set		GR_CALL(gr_screen.gf_zbuffer_set)
#define gr_zbuffer_clear	GR_CALL(gr_screen.gf_zbuffer_clear)

#define gr_save_screen		GR_CALL(gr_screen.gf_save_screen)
#define gr_restore_screen	GR_CALL(gr_screen.gf_restore_screen)
#define gr_free_screen		GR_CALL(gr_screen.gf_free_screen)

#define gr_dump_frame_start	GR_CALL(gr_screen.gf_dump_frame_start)
#define gr_dump_frame_stop		GR_CALL(gr_screen.gf_dump_frame_stop)
#define gr_dump_frame			GR_CALL(gr_screen.gf_dump_frame)

#define gr_set_gamma			GR_CALL(gr_screen.gf_set_gamma)

#define gr_lock				GR_CALL(gr_screen.gf_lock)
#define gr_unlock				GR_CALL(gr_screen.gf_unlock)

#define gr_get_region		GR_CALL(gr_screen.gf_get_region)

#define gr_fog_set			GR_CALL_NOPTR(gr_screen.gf_fog_set)

#define gr_set_cull			GR_CALL(gr_screen.gf_set_cull)

#define gr_cross_fade		GR_CALL(gr_screen.gf_cross_fade)

#define gr_tcache_set		GR_CALL_NOPTR(gr_screen.gf_tcache_set)

#define gr_set_clear_color	GR_CALL(gr_screen.gf_set_clear_color)

#define gr_clean			GR_CALL(gr_screen.gf_clean)

#define gr_zbias			GR_CALL(gr_screen.gf_zbias)

#define gr_start_frame		GR_CALL(gr_screen.gf_start_frame)
#define gr_stop_frame		GR_CALL(gr_screen.gf_stop_frame)

#define gr_tcache_flush		GR_CALL(gr_screen.gf_tcache_flush)

// special function for drawing polylines. this function is specifically intended for
// polylines where each section is no more than 90 degrees away from a previous section.
// Moreover, it is _really_ intended for use with 45 degree angles. 
void gr_pline_special(vector **pts, int num_pts, int thickness);

// Now generic functions
void gr_rect_internal(int x, int y, int w, int h, int r, int g, int b, int a);
void gr_rect(int x,int y,int w,int h);
void gr_shade(int x,int y,int w,int h);


void gr_reset_clip_set_player_num(int lPlayerNum);
void gr_reset_clip_splitscreen_safe(int hx = 0, int hy = 0);

#endif