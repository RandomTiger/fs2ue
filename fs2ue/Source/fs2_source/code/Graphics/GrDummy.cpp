#ifndef UNITY_BUILD
#include "osapi.h"
#include "2d.h"
#include "GrInternal.h"
#include "bmpman.h"
#endif

typedef uint32 UE4COLOR;

struct Vertex
{
	Vertex() : sx(0.0f)
		, sy(0.0f)
		, sz(0.0f)
		, m_rhw(1.0f)
		, color(0)
	{
	}

	float sx, sy, sz;
private:
	float m_rhw;
public:
	UE4COLOR color;
	float tu, tv;
};

void gr_dummy_pixel(int x, int y)
{
}

void gr_dummy_clear()
{
}

void gr_dummy_flip()
{
}

void gr_dummy_set_clip(int x,int y,int w,int h)
{
}

void gr_dummy_reset_clip()
{
}

void gr_dummy_set_font(int fontnum)
{
}

void gr_dummy_set_color( int r, int g, int b )
{
}

void gr_dummy_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha )
{
	gr_screen.current_alpha = alpha;
	gr_screen.current_alphablend_mode = alphablend_mode;
	gr_screen.current_bitblt_mode = bitblt_mode;
	gr_screen.current_bitmap = bitmap_num;
}

void gr_dummy_create_shader(shader * shade, float r, float g, float b, float c )
{
	shade->screen_sig = gr_screen.signature;
}

void gr_dummy_set_shader( shader * shade )
{	
	if ( shade )	{
		if (shade->screen_sig != gr_screen.signature)	{
			gr_create_shader( shade, shade->r, shade->g, shade->b, shade->c );
		}
		gr_screen.current_shader = *shade;
	} else {
		gr_create_shader( &gr_screen.current_shader, 0.0f, 0.0f, 0.0f, 0.0f );
	}
}

void gr_dummy_bitmap(int x, int y)
{
}

void gr_dummy_rect(int x,int y,int w,int h)
{
}


void gr_dummy_shade(int x,int y,int w,int h)
{
}

void gr_dummy_string(int x,int y, const char *text)
{
}




void gr_dummy_circle( int xc, int yc, int d )
{
}


void gr_dummy_line(int x1,int y1,int x2,int y2)
{
}

void gr_dummy_scaler(vertex *va, vertex *vb )
{
}

const int VERT_ARRAY_SIZE = 64;
Vertex d3d_verts[VERT_ARRAY_SIZE];
VertexNorm lVertices[VERT_ARRAY_SIZE];

void gr_dummy_tmapper( int nverts, vertex * verts[], uint flags )
{
#if defined(FS2_UE)
	for (int i = 0; i < (nverts - 1); i++)
	{
		vertex * va1 = verts[i];
		vertex * va2 = verts[i+1];

		FVector start(va1->x, va1->y, va1->z);
		FVector end(va2->x, va2->y, va2->z);

		/*
		DrawDebugLine(
			AFS2GameMode::Instance->GetWorld(),
			start,
			end,
			FColor(255, 0, 0),
			false, -1, 0,
			1.0f
		);
		*/
	}
#endif

	int i;
	float u_scale = 1.0f, v_scale = 1.0f;

	if (isModelCacheInProgress())
	{
		extern void model_cache_set_texture(int texture);
		model_cache_set_texture(gr_screen.current_bitmap);
	}

	Vertex *src_v = d3d_verts;

	assert(nverts < VERT_ARRAY_SIZE);
	for (i = 0; i<nverts; i++) {
		vertex * va = verts[i];

		if (isModelCacheInProgress())
		{
			src_v->sx = va->x;
			src_v->sy = va->y;
			src_v->sz = va->z;
		}
		else
		{
			src_v->sx = va->sx + gr_screen.offset_x;
			src_v->sy = va->sy + gr_screen.offset_y;
			src_v->sz = 0.99f;
		}

		src_v->color = 0xffffffff;

		if (!isModelCacheInProgress())
		{
			src_v->sx = va->sx + gr_screen.offset_x;
			src_v->sy = va->sy + gr_screen.offset_y;
		}

		if (flags & TMAP_FLAG_TEXTURED) {
			static float changer = 0.000288f;
			src_v->tu = (va->u*u_scale) + changer;
			src_v->tv = (va->v*v_scale) + changer;
		}
		else {
			src_v->tu = 0.0f;
			src_v->tv = 0.0f;
		}

		src_v++;
	}


	assert(nverts < VERT_ARRAY_SIZE);

	if (isModelCacheInProgress())
	{
		for (i = 0; i<nverts; i++)
		{
			extern void model_cache_add_vertex(VertexNorm *src_v);

#if defined(FS2_UE)
			lVertices[i].nx = verts[i]->normal.X;
			lVertices[i].ny = verts[i]->normal.Y;
			lVertices[i].nz = verts[i]->normal.Z;
#endif
			lVertices[i].sx = d3d_verts[i].sx;
			lVertices[i].sy = d3d_verts[i].sy;
			lVertices[i].sz = d3d_verts[i].sz;
			lVertices[i].tu = d3d_verts[i].tu;
			lVertices[i].tv = d3d_verts[i].tv;

			if (i >= 2)
			{
				model_cache_add_vertex(&lVertices[0]);
				model_cache_add_vertex(&lVertices[i - 1]);
				model_cache_add_vertex(&lVertices[i]);
			}
		}
	}
}

#undef UpdateResource

/*
const uint32 COLOR_ARGB(uint8 a, uint8 r, uint8 g, uint8 b)
{
	return ((((a) & 0xff) << 24) | (((r) & 0xff) << 16) | (((g) & 0xff) << 8) | ((b) & 0xff));
}*/

// get the final texture size (the one which will get allocated as a surface)
void Dummy_d3d9_tcache_get_adjusted_texture_size(int w_in, int h_in, int *w_out, int *h_out)
{
	// starting size
	int tex_w = w_in;
	int tex_h = h_in;

	// bogus
	if ((w_out == NULL) || (h_out == NULL)) {
		return;
	}

	//if ( D3D_pow2_textures )	
	{
		int i;
		const int kCheckPower = 16;
		for (i = 0; i<kCheckPower; i++) {
			const int checkSize = 1 << i;
			const int nextSize = 1 << (i + 1);

			if (tex_w > checkSize && tex_w <= nextSize) {
				tex_w = nextSize;
				break;
			}
		}

		for (i = 0; i<kCheckPower; i++) {
			const int checkSize = 1 << i;
			const int nextSize = 1 << (i + 1);

			if (tex_h > checkSize && tex_h <= nextSize) {
				tex_h = nextSize;
				break;
			}
		}
	}

	int unrealTextureLimit = 8192;
	int D3D_min_texture_width = 16;
	int D3D_min_texture_height = 16;

	if (tex_w < D3D_min_texture_width) {
		tex_w = D3D_min_texture_width;
	}
	else if (tex_w > unrealTextureLimit) {
		tex_w = unrealTextureLimit;
	}

	if (tex_h < D3D_min_texture_height) {
		tex_h = D3D_min_texture_height;
	}
	else if (tex_h > unrealTextureLimit) {
		tex_h = unrealTextureLimit;
	}

	// store the outgoing size
	*w_out = tex_w;
	*h_out = tex_h;
}

int gr_dummy_tcache_set(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full, const bool justCacheNoSet)
{
	/*
	const int n = bm_get_cache_slot(bitmap_id, 1);
	bitmap_entry	*be = &bm_bitmaps[n];
	be->Texture

	bool result = true;
	if ((bitmap_id < 0) || (bitmap_id != t->m_bitmapID)) {
		result = d3d_create_texture(bitmap_id, bitmap_type, t, fail_on_full);
	}

	// everything went ok
	if (result && t->m_handle)
	{
		if (u_scale)
			*u_scale = t->m_uScale;
		if (v_scale)
			*v_scale = t->m_vScale;

		if (justCacheNoSet == false)
		{
			// set texture
			t->m_usedThisFrameCount++;
		}

		return true;
	}
	*/
	return false;
}


void gr_dummy_gradient(int x1,int y1,int x2,int y2)
{
}

void gr_dummy_set_palette(ubyte *new_palette, int is_alphacolor)
{
}

void gr_dummy_init_color(color *c, int r, int g, int b)
{
}

void gr_dummy_set_color_fast(color *dst)
{
}

void gr_dummy_print_screen(char *filename)
{

}

void gr_dummy_cleanup()
{
}

void gr_dummy_fog_set(int fog_mode, int r, int g, int b, float near, float far)
{
}

void gr_dummy_get_region(int front, int w, int g, ubyte *data)
{
}

void gr_dummy_set_cull(int cull)
{
}

// cross fade
void gr_dummy_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct)
{
}

void gr_dummy_set_clear_color(int r, int g, int b)
{
}

void gr_dummy_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type )
{
}

uint gr_dummy_lock()
{
	return 1;
}

void gr_dummy_unlock()
{
}

void gr_dummy_aabitmap(int x, int y) {}
void gr_dummy_aabitmap_ex(int x,int y,int w,int h,int sx,int sy) {}

void gr_dummy_dump_frame() {}
void gr_dummy_dump_frame_start( int first_frame_number, int nframes_between_dumps ) {}
void gr_dummy_dump_frame_stop() {}
void gr_dummy_set_gamma(float gamma) {}

void gr_dummy_aaline(vertex *v1, vertex *v2){}
void gr_dummy_aascaler(vertex *va, vertex *vb ){}
void gr_dummy_flash( int r, int g, int b ){}
int gr_dummy_zbuffer_get(){return GR_ZBUFF_NONE;}
int gr_dummy_zbuffer_set(int mode){return GR_ZBUFF_NONE;}
void gr_dummy_zbuffer_clear(int use_zbuffer){}
int gr_dummy_save_screen(){ return 0;}
void gr_dummy_restore_screen(int id){}
void gr_dummy_free_screen(int id){}

void gr_dummy_zbias(int zbias){}
void gr_dummy_tcache_flush(){}

void gr_dummy_start_frame(){}
void gr_dummy_stop_frame(){}

bool gr_dummy_init()
{
	mprintf(( "Initializing DUMMY graphics device...\n" ));

	gr_screen.gf_flip = gr_dummy_flip;
	gr_screen.gf_set_clip = gr_dummy_set_clip;
	gr_screen.gf_reset_clip = gr_dummy_reset_clip;
	gr_screen.gf_set_font = grx_set_font;

	gr_screen.gf_init_color = gr_dummy_init_color;
	gr_screen.gf_set_color_fast = gr_dummy_set_color_fast;
	gr_screen.gf_set_color = gr_dummy_set_color;
	gr_screen.gf_init_color = gr_dummy_init_color;
	gr_screen.gf_init_alphacolor_ptr = gr_dummy_init_alphacolor;

	gr_screen.gf_set_bitmap_ptr = gr_dummy_set_bitmap;
	gr_screen.gf_create_shader = gr_dummy_create_shader;
	gr_screen.gf_set_shader = gr_dummy_set_shader;
	gr_screen.gf_clear = gr_dummy_clear;
	gr_screen.gf_bitmap = gr_dummy_bitmap;
	gr_screen.gf_aabitmap = gr_dummy_aabitmap;
	gr_screen.gf_aabitmap_ex = gr_dummy_aabitmap_ex;

	gr_screen.gf_string = gr_dummy_string;
	gr_screen.gf_circle = gr_dummy_circle;

	gr_screen.gf_line = gr_dummy_line;
	gr_screen.gf_aaline = gr_dummy_aaline;
	gr_screen.gf_pixel = gr_dummy_pixel;
	gr_screen.gf_scaler = gr_dummy_scaler;
	gr_screen.gf_aascaler = gr_dummy_aascaler;
	gr_screen.gf_tmapper = gr_dummy_tmapper;

	gr_screen.gf_gradient = gr_dummy_gradient;

	gr_screen.gf_set_palette_ptr = gr_dummy_set_palette;
	gr_screen.gf_print_screen = gr_dummy_print_screen;

	gr_screen.gf_flash = gr_dummy_flash;

	gr_screen.gf_zbuffer_get = gr_dummy_zbuffer_get;
	gr_screen.gf_zbuffer_set = gr_dummy_zbuffer_set;
	gr_screen.gf_zbuffer_clear = gr_dummy_zbuffer_clear;

	gr_screen.gf_save_screen = gr_dummy_save_screen;
	gr_screen.gf_restore_screen = gr_dummy_restore_screen;
	gr_screen.gf_free_screen = gr_dummy_free_screen;

	// Screen dumping stuff
	gr_screen.gf_dump_frame_start = gr_dummy_dump_frame_start;
	gr_screen.gf_dump_frame_stop = gr_dummy_dump_frame_stop;
	gr_screen.gf_dump_frame = gr_dummy_dump_frame;

	gr_screen.gf_set_gamma = gr_dummy_set_gamma; 

	// Lock/unlock stuff
	gr_screen.gf_lock = gr_dummy_lock;
	gr_screen.gf_unlock = gr_dummy_unlock;

	// screen region
	gr_screen.gf_get_region = gr_dummy_get_region;

	// fog stuff
	gr_screen.gf_fog_set_ptr = gr_dummy_fog_set;

	// poly culling
	gr_screen.gf_set_cull = gr_dummy_set_cull;

	// cross fade
	gr_screen.gf_cross_fade = gr_dummy_cross_fade;

	// texture cache
	gr_screen.gf_tcache_set_ptr = gr_dummy_tcache_set;

	// set clear color
	gr_screen.gf_set_clear_color = gr_dummy_set_clear_color;

	gr_screen.gf_clean = gr_dummy_cleanup;

	gr_screen.gf_zbias		  = gr_dummy_zbias;

	gr_screen.gf_start_frame  = gr_dummy_start_frame;
	gr_screen.gf_stop_frame   = gr_dummy_stop_frame;


	gr_screen.gf_tcache_flush = gr_dummy_tcache_flush;
	return true;
}


