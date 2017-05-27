
#ifndef UNITY_BUILD
#include <windows.h>

#include <D3d9.h>
#include <d3d9types.h>
#include <D3dx9math.h>
#include <Tmapper.h>
#include <assert.h>

#include "Graphics/GrD3D9.h"
#include "Graphics/GrD3D9Render.h"
#include "Graphics/GrD3D9Texture.h"
#include "Graphics/GrD3D9Shader.h"

#include "Graphics/Font.h"
#include "osapi.h"
#include "2d.h"
#include "GrInternal.h"
#include "bmpman.h"
#include "3d.h"
#include "Math/vector.h"

#include "nebula/neb.h" // for neb2_get_pixel
#include "Palman/PalMan.h" // for gr_palette
#endif

enum
{
	BITMAP_FONT,
	BITMAP_NORMAL,
};

const int NEBULA_COLORS = 20;
int D3D9_last_state = -1;
int D3D9_zbuffering = 0;
int D3D9_global_zbuffering = 0;

float D3D9_z_mult = 30000.0f;
int D3D9_fog_mode = 2;

void gr_d3d9_aabitmap_ex_internal(int type, int x,int y,int w,int h,int sx,int sy);
bool SetMatrices();

void SetupRenderSystem()
{
	ResetRenderSystem();
}

void ResetRenderSystem()
{
	D3D9_last_state = -1;

	SetRenderState(D3DRS_LIGHTING, FALSE); 
}

void CleanupRenderSystem()
{
	ResetRenderSystem();
}

HRESULT DrawPrimitive2D(D3DPRIMITIVETYPE primType, const int primCount, void *data, int flags)
{
	HRESULT hr = S_OK;

	SetMatrices();

#ifdef FIXED_PIPELINE
	const DWORD vertexFVF = (D3DFVF_XYZRHW | D3DFVF_DIFFUSE 
#ifdef FIXED_PIPELINE_SPECULAR
		| D3DFVF_SPECULAR 
#endif
		| D3DFVF_TEX1 );

	DWORD currentFVF;
	hr = GetDevice()->GetFVF(&currentFVF);
	assert(SUCCEEDED(hr));
	if(currentFVF != vertexFVF)
	{
		hr = GetDevice()->SetFVF(vertexFVF);
		assert(SUCCEEDED(hr));
	}

#endif

	VertexShaderType vertexShader = VertexShader_Passthrough;
	PixelShaderType pixelShader = PixelShader_Passthrough;
	if(flags & TMAP_FLAG_NO_TEXTURE)
	{
		vertexShader = VertexShader_PassthroughNoTexture;
		pixelShader = PixelShader_PassthroughNoTexture;
	}
#ifdef FIXED_PIPELINE
	vertexShader = VertexShader_None;
	pixelShader  = PixelShader_None;
#endif

	SetShader(vertexShader, pixelShader, 
		gr_screen.offset_x, gr_screen.offset_x + gr_screen.clip_right, 
		gr_screen.offset_y, gr_screen.offset_y + gr_screen.clip_bottom);

	hr = GetDevice()->DrawPrimitiveUP(primType, primCount, data, sizeof(Vertex));
	if(FAILED(hr))
	{
		mprintf(("Failed DrawPrimitiveUP\n"));
	}
#if defined(_WIN64)
	return S_OK;
#else
	return hr == S_OK;
#endif
}

HRESULT SetRenderState( D3DRENDERSTATETYPE dwRenderStateType,  DWORD dwRenderState )
{
	DWORD currentState;
	GetDevice()->GetRenderState(dwRenderStateType, &currentState);
	if(currentState != dwRenderState)
	{
		return GetDevice()->SetRenderState(dwRenderStateType, dwRenderState );
	}

	return S_OK;
}

HRESULT SetTextureStageState ( D3DTEXTURESTAGESTATETYPE type,  DWORD value )
{
	// Not support on 360

	const int stage = 0;
	DWORD currentState;
	GetDevice()->GetTextureStageState(stage, type, &currentState);
	if(currentState != value)
	{
		return GetDevice()->SetTextureStageState(stage, type, value );
	}

	return S_OK;
}

HRESULT SetSamplerState(D3DSAMPLERSTATETYPE type, DWORD value)
{
	const int sampler = 0;
	DWORD currentState;
	GetDevice()->GetSamplerState(sampler, type, &currentState);
	if(currentState != value)
	{
		return GetDevice()->SetSamplerState(sampler, type, value );
	}

	return S_OK;
}

void gr_d3d9_set_state( gr_texture_source ts, gr_alpha_blend ab, gr_zbuffer_type zt )
{
	int current_state = 0;

	current_state = current_state | (ts<<0);
	current_state = current_state | (ab<<5);
	current_state = current_state | (zt<<10);

	if ( current_state == D3D9_last_state ) {
		return;
	}

	D3D9_last_state = current_state;

	switch( ts )	{
	case TEXTURE_SOURCE_NONE:
		SetTexutre(0, 0);
		// Let the texture cache system know whe set the handle to NULL
		gr_tcache_set(-1, -1, NULL, NULL );
		break;

	case TEXTURE_SOURCE_DECAL:
		SetSamplerState(D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC );
		SetSamplerState(D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC );

		SetTextureStageState(D3DTSS_COLOROP, D3DTOP_MODULATE);
	
		SetSamplerState(D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		SetSamplerState(D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		break;

	case TEXTURE_SOURCE_NO_FILTERING:
		SetSamplerState(D3DSAMP_MINFILTER, D3DTEXF_NONE );
		SetSamplerState(D3DSAMP_MAGFILTER, D3DTEXF_NONE );

		SetTextureStageState(D3DTSS_COLOROP, D3DTOP_MODULATE);

		SetSamplerState(D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		SetSamplerState(D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		break;

	default:
		Int3();
	}

	switch( ab )	{
	case ALPHA_BLEND_NONE:							// 1*SrcPixel + 0*DestPixel
		SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE );
		break;

	case ALPHA_BLEND_ALPHA_ADDITIVE:				// Alpha*SrcPixel + 1*DestPixel
	case ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR:	// Alpha*SrcPixel + (1-SrcPixel)*DestPixel

		SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE );
		SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE );
		//d3d_SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA );
		SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE );
		break;

	case ALPHA_BLEND_ALPHA_BLEND_ALPHA:			// Alpha*SrcPixel + (1-Alpha)*DestPixel
		SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE );
		SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		break;


	default:
		Int3();
	}

	switch( zt )	{

	case ZBUFFER_TYPE_NONE:
		SetRenderState(D3DRS_ZENABLE,FALSE);
		SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
		break;

	case ZBUFFER_TYPE_READ:
		SetRenderState(D3DRS_ZENABLE,TRUE);
		SetRenderState(D3DRS_ZWRITEENABLE,FALSE);
		break;

	case ZBUFFER_TYPE_WRITE:
		SetRenderState(D3DRS_ZENABLE,FALSE);
		SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
		break;

	case ZBUFFER_TYPE_FULL:
		SetRenderState(D3DRS_ZENABLE,TRUE);
		SetRenderState(D3DRS_ZWRITEENABLE,TRUE);
		break;

	default:
		Int3();
	}

}

void gr_d3d9_set_clear_color(int r, int g, int b)
{
	gr_init_color(&gr_screen.current_clear_color, r, g, b);
}

void gr_d3d9_clear()
{
	// Turn off zbuffering so this doesn't clear the zbuffer
	// gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE );

	D3DRECT dst;

	D3DCOLOR colour = D3DCOLOR_RGBA(
		gr_screen.current_clear_color.red, 
		gr_screen.current_clear_color.green, 
		gr_screen.current_clear_color.blue,
		255);

	dst.x1 = gr_screen.clip_left+gr_screen.offset_x;
	dst.y1 = gr_screen.clip_top+gr_screen.offset_y;
	dst.x2 = gr_screen.clip_right+1+gr_screen.offset_x;
	dst.y2 = gr_screen.clip_bottom+1+gr_screen.offset_y;	

	GetDevice()->Clear(1, &dst, D3DCLEAR_TARGET, colour, 0, 0);
}

// sets the clipping region & offset
void gr_d3d9_set_clip(int x,int y,int w,int h)
{
	gr_screen.offset_x = x;
	gr_screen.offset_y = y;

	gr_screen.clip_left = 0;
	gr_screen.clip_right = w;

	gr_screen.clip_top = 0;
	gr_screen.clip_bottom = h;

	// check for sanity of parameters
	if ( gr_screen.clip_left+x < 0 ) {
		gr_screen.clip_left = -x;
	} else if ( gr_screen.clip_left+x > gr_screen.max_w)	{
		gr_screen.clip_left = gr_screen.max_w-x;
	}
	if ( gr_screen.clip_right+x < 0 ) {
		gr_screen.clip_right = -x;
	} else if ( gr_screen.clip_right+x >= gr_screen.max_w )	{
		gr_screen.clip_right = gr_screen.max_w-x;
	}

	if ( gr_screen.clip_top+y < 0 ) {
		gr_screen.clip_top = -y;
	} else if ( gr_screen.clip_top+y > gr_screen.max_h )	{
		gr_screen.clip_top = gr_screen.max_h-y;
	}

	if ( gr_screen.clip_bottom+y < 0 ) {
		gr_screen.clip_bottom = -y;
	} else if ( gr_screen.clip_bottom+y > gr_screen.max_h )	{
		gr_screen.clip_bottom = gr_screen.max_h-y;
	}

	gr_screen.clip_width = gr_screen.clip_right - gr_screen.clip_left;
	gr_screen.clip_height = gr_screen.clip_bottom - gr_screen.clip_top;

	// Setup the viewport for a reasonable viewing area
	D3DVIEWPORT9 viewdata;

	viewdata.X = gr_screen.clip_left+x;
	viewdata.Y = gr_screen.clip_top+y;
	viewdata.Width = gr_screen.clip_width;
	viewdata.Height = gr_screen.clip_height;
	viewdata.MinZ = 0.0F;
	viewdata.MaxZ = 0.0F; // choose something appropriate here!

	const HRESULT hr = GetDevice()->SetViewport(&viewdata);
	Assert(SUCCEEDED(hr));
}

void gr_d3d9_reset_clip()
{
	gr_d3d9_set_clip(0, 0, gr_screen.max_w, gr_screen.max_h);
}

void gr_d3d9_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha )
{
	gr_screen.current_alpha = alpha;
	gr_screen.current_alphablend_mode = alphablend_mode;
	gr_screen.current_bitblt_mode = bitblt_mode;
	gr_screen.current_bitmap = bitmap_num;
}

void gr_d3d9_create_shader(shader * shade, float r, float g, float b, float c )
{
	shade->screen_sig = gr_screen.signature;
}

void gr_d3d9_set_shader( shader * shade )
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

void gr_d3d9_bitmap(int x, int y)
{
	int saved_zbuffer_mode;
	vertex v[4];
	vertex *vertlist[4] = { &v[0], &v[1], &v[2], &v[3] };

	g3_start_frame(1);

	// turn off zbuffering	
	saved_zbuffer_mode = gr_zbuffer_get();
	gr_zbuffer_set(GR_ZBUFF_NONE);	

	int w, h;
	bm_get_info(gr_screen.current_bitmap, &w, &h);

	// stuff coords	
	v[0].sx = (float)x;
	v[0].sy = (float)y;	
	v[0].sw = 0.0f;
	v[0].u = 0.0f;
	v[0].v = 0.0f;
	v[0].flags = PF_PROJECTED;
	v[0].codes = 0;

	v[1].sx = (float)(x + w);
	v[1].sy = (float)y;	
	v[1].sw = 0.0f;
	v[1].u = 1.0f;
	v[1].v = 0.0f;
	v[1].flags = PF_PROJECTED;
	v[1].codes = 0;

	v[2].sx = (float)(x + w);
	v[2].sy = (float)(y + h);	
	v[2].sw = 0.0f;
	v[2].u = 1.0f;
	v[2].v = 1.0f;
	v[2].flags = PF_PROJECTED;
	v[2].codes = 0;

	v[3].sx = (float)x;
	v[3].sy = (float)(y + h);	
	v[3].sw = 0.0f;
	v[3].u = 0.0f;
	v[3].v = 1.0f;
	v[3].flags = PF_PROJECTED;
	v[3].codes = 0;		
		
	// set debrief	
	g3_draw_poly_constant_sw(4, vertlist, TMAP_FLAG_TEXTURED | TMAP_FLAG_NO_FILTERING, 0.1f);

	g3_end_frame();
	
	gr_zbuffer_set(saved_zbuffer_mode);	
}

// basically just fills in the alpha component of the specular color. Hardware does the rest
// when rendering the poly
D3DCOLOR d3d9_stuff_fog_value(float z)
{
	// linear fog formula
	float f_float = (gr_screen.fog_far - z) / (gr_screen.fog_far - gr_screen.fog_near);
	if(f_float < 0.0f){
		f_float = 0.0f;
	} else if(f_float > 1.0f){
		f_float = 1.0f;
	}
	
	return D3DCOLOR_COLORVALUE(0.0f, 0.0f, 0.0f, f_float);
}

void gr_d3d9_tmapper_internal( int nverts, vertex **verts, uint flags, int is_scaler )	
{
	int i;
	float u_scale = 1.0f, v_scale = 1.0f;	

	// Make nebula use the texture mapper... this blends the colors better.
	if ( flags & TMAP_FLAG_NEBULA ){
		Int3();
	}

	gr_texture_source texture_source = (gr_texture_source)-1;
	gr_alpha_blend alpha_blend = (gr_alpha_blend)-1;
	gr_zbuffer_type zbuffer_type = (gr_zbuffer_type)-1;


	if ( D3D9_zbuffering )	{
		if ( is_scaler || (gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER)	)	{
			zbuffer_type = ZBUFFER_TYPE_READ;
		} else {
			zbuffer_type = ZBUFFER_TYPE_FULL;
		}
	} else {
		zbuffer_type = ZBUFFER_TYPE_NONE;
	}

	int alpha;

	int tmap_type = TCACHE_TYPE_NORMAL;

	int r, g, b;

	if ( flags & TMAP_FLAG_TEXTURED )	{
		r = 255;
		g = 255;
		b = 255;
	} else {
		r = gr_screen.current_color.red;
		g = gr_screen.current_color.green;
		b = gr_screen.current_color.blue;
	}

	if ( gr_screen.current_alphablend_mode == GR_ALPHABLEND_FILTER )	{

		//if ( lpDevDesc->dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_ONE  )	
		{
			tmap_type = TCACHE_TYPE_NORMAL;
			alpha_blend = ALPHA_BLEND_ALPHA_ADDITIVE;

			// Blend with screen pixel using src*alpha+dst
			float factor = gr_screen.current_alpha;

			alpha = 255;

			if ( factor <= 1.0f )	{
				int tmp_alpha = fl2i(gr_screen.current_alpha*255.0f);
				r = (r*tmp_alpha)/255;
				g = (g*tmp_alpha)/255;
				b = (b*tmp_alpha)/255;
			}
		} 
/* else {

			tmap_type = TCACHE_TYPE_XPARENT;

			alpha_blend = ALPHA_BLEND_ALPHA_BLEND_ALPHA;

			// Blend with screen pixel using src*alpha+dst
			float factor = gr_screen.current_alpha;

			if ( factor > 1.0f )	{
				alpha = 255;
			} else {
				alpha = fl2i(gr_screen.current_alpha*255.0f);
			}
		}
*/
	} else {
		alpha_blend = ALPHA_BLEND_ALPHA_BLEND_ALPHA;
		alpha = 255;
	}

	if(flags & TMAP_FLAG_NO_FILTERING){
		tmap_type = TCACHE_TYPE_INTERFACE;
	}

	texture_source = TEXTURE_SOURCE_NONE;
 
	if ( flags & TMAP_FLAG_TEXTURED )	{
		if ( !gr_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0))	
		{
			mprintf(( "Not rendering a texture because it didn't fit in VRAM!\n" ));
			return;
		}

		// use nonfiltered textures for bitmap sections
		if(flags & TMAP_FLAG_NO_FILTERING){
			texture_source = TEXTURE_SOURCE_NO_FILTERING;
		} else {
			texture_source = TEXTURE_SOURCE_DECAL;
		}
	}
	
	gr_d3d9_set_state( texture_source, alpha_blend, zbuffer_type );
	
	const int VERT_ARRAY_SIZE = 64;
	Vertex d3d_verts[VERT_ARRAY_SIZE];
	Vertex *src_v = d3d_verts;

	assert(nverts < VERT_ARRAY_SIZE);
	for (i=0; i<nverts; i++ )	{
		vertex * va = verts[i];		
				
		// store in case we're doing vertex fog.		
		if ( D3D9_zbuffering || (flags & TMAP_FLAG_NEBULA) )	{
			src_v->sz = va->z / D3D9_z_mult;	// For zbuffering and fogging
			if ( src_v->sz > 0.98f )	{
				src_v->sz = 0.98f;
			}		
		} else {
			src_v->sz = 0.99f;
		}			

		// http://www.mvps.org/directx/articles/linear_z/linearz.htm
		if ( flags & TMAP_FLAG_CORRECT )	{
			src_v->SetRhw(va->sw);				// For texture correction 						
		}

		int a;

		if ( flags & TMAP_FLAG_ALPHA )	{
			a = verts[i]->a;
		} else {
			a = alpha;
		}

		if ( flags & TMAP_FLAG_NEBULA )	{
			int pal = (verts[i]->b*(NEBULA_COLORS-1))/255;
			r = gr_palette[pal*3+0];
			g = gr_palette[pal*3+1];
			b = gr_palette[pal*3+2];
		} else if ( (flags & TMAP_FLAG_RAMP) && (flags & TMAP_FLAG_GOURAUD) )	{
			r = verts[i]->b;
			g = verts[i]->b;
			b = verts[i]->b;
		} else if ( (flags & TMAP_FLAG_RGB)  && (flags & TMAP_FLAG_GOURAUD) )	{
			// Make 0.75 be 256.0f
			r = verts[i]->r;
			g = verts[i]->g;
			b = verts[i]->b;
		} else {
			// use constant RGB values...
		}

		src_v->color = D3DCOLOR_RGBA(r, g, b, a);

		// if we're fogging and we're doing vertex fog
		if((gr_screen.current_fog_mode != GR_FOGMODE_NONE) && (D3D9_fog_mode == 1))
		{
			src_v->SetSpecular(d3d9_stuff_fog_value(va->z));
		}
		
		src_v->sx = va->sx + gr_screen.offset_x;
		src_v->sy = va->sy + gr_screen.offset_y;

		if ( flags & TMAP_FLAG_TEXTURED )	{
			static float changer = 0.000288f;
			src_v->tu = (va->u*u_scale) + changer;
			src_v->tv = (va->v*v_scale) + changer;						
		} else {
			src_v->tu = 0.0f;
			src_v->tv = 0.0f;
		}
		src_v++;
	}

	// if we're rendering against a fullneb background
	if(flags & TMAP_FLAG_PIXEL_FOG){	
		int r, g, b;
		int ra, ga, ba;		
		ra = ga = ba = 0;		

		// get the average pixel color behind the vertices
		for(i=0; i<nverts; i++){			
			neb2_get_pixel((int)d3d_verts[i].sx, (int)d3d_verts[i].sy, &r, &g, &b);
			ra += r;
			ga += g;
			ba += b;
		}				
		ra /= nverts;
		ga /= nverts;
		ba /= nverts;		

		// set fog
		gr_fog_set(GR_FOGMODE_FOG, ra, ga, ba);
	}					

	const static bool batch = true;
/*
	// Cant batch in fog, causes vertex corruption!!!!
	if(gr_screen.current_fog_mode == GR_FOGMODE_NONE)
	{
		// Convert from triangle fan to list
		int triCount = 0;
		// A fan will always create two triangles less than the number of vertices
		for(int i = 0, srcCnt = 1, dstCnt =0; i < nverts - 2; i++, srcCnt++,dstCnt+=3)
		{
			batchConvertData.MemCpy(dstCnt,   &d3d_verts[0],        1);
			batchConvertData.MemCpy(dstCnt+1, &d3d_verts[srcCnt],   1);
			batchConvertData.MemCpy(dstCnt+2, &d3d_verts[srcCnt+1], 1);

			Assert(dstCnt+2 < VERT_ARRAY_SIZE_LIST);
			Assert(srcCnt+1 < VERT_ARRAY_SIZE);
			triCount++;
		}

		Assert(triCount == nverts - 2);

		//d3d_DrawPrimitive
		d3d_batch(D3DPT_TRIANGLELIST, D3DVT_TLVERTEX, batchConvertData, triCount * 3, NULL);
	}
	else
*/
	{

		DrawPrimitive2D(D3DPT_TRIANGLEFAN, nverts - 2, d3d_verts, flags);
	}
	// turn off fog
	// if(flags & TMAP_FLAG_PIXEL_FOG){
		// gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	// }
}

void gr_d3d9_tmapper( int nverts, vertex **verts, uint flags )	
{
	gr_d3d9_tmapper_internal( nverts, verts, flags, 0 );
}

// Renders text and HUD
void gr_d3d9_aabitmap_ex_internal(int type, int x,int y,int w,int h,int sx,int sy)
{
	if ( w < 1 ) return;
	if ( h < 1 ) return;

	float u_scale = 1.0f;
	float v_scale = 1.0f;

	if ( !gr_screen.current_color.is_alphacolor )	return;

	gr_d3d9_set_state( TEXTURE_SOURCE_NO_FILTERING, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	if ( !gr_tcache_set( gr_screen.current_bitmap, TCACHE_TYPE_AABITMAP, &u_scale, &v_scale ) )	{
		// Couldn't set texture
		return;
	}

	const int kNumVerts = 6;
	Vertex d3d_verts[kNumVerts];
	Vertex *src_v = &d3d_verts[0];

	float u0, u1, v0, v1;
	float x1, x2, y1, y2;
	int bw, bh;

	bm_get_info( gr_screen.current_bitmap, &bw, &bh );

	u0 = u_scale*(i2fl(sx)+0.5f)/i2fl(bw);
	v0 = v_scale*(i2fl(sy)+0.5f)/i2fl(bh);

	u1 = u_scale*(i2fl(sx+w)+0.5f)/i2fl(bw);
	v1 = v_scale*(i2fl(sy+h)+0.5f)/i2fl(bh);

	x1 = i2fl(x+gr_screen.offset_x);
	y1 = i2fl(y+gr_screen.offset_y);
	x2 = i2fl(x+w+gr_screen.offset_x);
	y2 = i2fl(y+h+gr_screen.offset_y);

	const DWORD alpha = ( gr_screen.current_color.is_alphacolor ) ? gr_screen.current_color.alpha : 255;
	const DWORD color = D3DCOLOR_RGBA(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, alpha);

	for(int i = 0; i < kNumVerts; i++)
	{
		d3d_verts[i].sz    = 0.99f;
		d3d_verts[i].color = color;	
	}

	src_v->sx = x1;
	src_v->sy = y1;
	src_v->tu = u0;
	src_v->tv = v0;
	src_v++;

	src_v->sx = x2;
	src_v->sy = y1;
	src_v->tu = u1;
	src_v->tv = v0;
	src_v++;

	src_v->sx = x2;
	src_v->sy = y2;
	src_v->tu = u1;
	src_v->tv = v1;
	src_v++;
 
	*src_v = d3d_verts[0];
	src_v++;
	*src_v = d3d_verts[2];
	src_v++;

	src_v->sx = x1;
	src_v->sy = y2;
	src_v->tu = u0;
	src_v->tv = v1;
	src_v++;

	DrawPrimitive2D(D3DPT_TRIANGLELIST, 2, d3d_verts, 0);
}

void gr_d3d9_string(int sx,int sy, const char *text)
{
	int width, spacing, letter;

	if ( !Current_font )	{
		return;
	}

	gr_set_bitmap(Current_font->bitmap_id);

	int x = sx;
	int y = sy;

	if (sx==0x8000) {			//centered
		x = get_centered_x(text);
	} else {
		x = sx;
	}
	
	spacing = 0;

	while (*text)	{
		x += spacing;

		while (*text== '\n' )	{
			text++;
			y += Current_font->h;
			if (sx==0x8000) {			//centered
				x = get_centered_x(text);
			} else {
				x = sx;
			}
		}
		if (*text == 0 ) break;

		letter = get_char_width(text[0],text[1],&width,&spacing);
		text++;

		//not in font, draw as space
		if (letter<0)	{
			continue;
		}

		int xd, yd, xc, yc;
		int wc, hc;

		// Check if this character is totally clipped
		if ( x + width < gr_screen.clip_left ) continue;
		if ( y + Current_font->h < gr_screen.clip_top ) continue;
		if ( x > gr_screen.clip_right ) continue;
		if ( y > gr_screen.clip_bottom ) continue;

		xd = yd = 0;

		xc = x+xd;
		yc = y+yd;

		wc = width - xd; hc = Current_font->h - yd;

		if ( wc < 1 ) continue;
		if ( hc < 1 ) continue;

		font_char *ch;
	
		ch = &Current_font->char_data[letter];

		int u = Current_font->bm_u[letter];
		int v = Current_font->bm_v[letter];

		gr_d3d9_aabitmap_ex_internal(BITMAP_FONT, xc, yc, wc, hc, u+xd, v+yd );
	}
}

float FIND_SCALED_NUM9(float x, float x0, float x1, float y0, float y1)
{
	return (((x-x0)*(y1-y0))/(x1-x0))+y0;
}

void gr_d3d9_scaler(vertex *va, vertex *vb )
{
	float x0, y0, x1, y1;
	float u0, v0, u1, v1;
	float clipped_x0, clipped_y0, clipped_x1, clipped_y1;
	float clipped_u0, clipped_v0, clipped_u1, clipped_v1;
	float xmin, xmax, ymin, ymax;
	int dx0, dy0, dx1, dy1;

	//============= CLIP IT =====================

	x0 = va->sx; y0 = va->sy;
	x1 = vb->sx; y1 = vb->sy;

	xmin = i2fl(gr_screen.clip_left); ymin = i2fl(gr_screen.clip_top);
	xmax = i2fl(gr_screen.clip_right); ymax = i2fl(gr_screen.clip_bottom);

	u0 = va->u; v0 = va->v;
	u1 = vb->u; v1 = vb->v;

	// Check for obviously offscreen bitmaps...
	if ( (y1<=y0) || (x1<=x0) ) return;
	if ( (x1<xmin ) || (x0>xmax) ) return;
	if ( (y1<ymin ) || (y0>ymax) ) return;

	clipped_u0 = u0; clipped_v0 = v0;
	clipped_u1 = u1; clipped_v1 = v1;

	clipped_x0 = x0; clipped_y0 = y0;
	clipped_x1 = x1; clipped_y1 = y1;

	// Clip the left, moving u0 right as necessary
	if ( x0 < xmin ) 	{
		clipped_u0 = FIND_SCALED_NUM9(xmin,x0,x1,u0,u1);
		clipped_x0 = xmin;
	}

	// Clip the right, moving u1 left as necessary
	if ( x1 > xmax )	{
		clipped_u1 = FIND_SCALED_NUM9(xmax,x0,x1,u0,u1);
		clipped_x1 = xmax;
	}

	// Clip the top, moving v0 down as necessary
	if ( y0 < ymin ) 	{
		clipped_v0 = FIND_SCALED_NUM9(ymin,y0,y1,v0,v1);
		clipped_y0 = ymin;
	}

	// Clip the bottom, moving v1 up as necessary
	if ( y1 > ymax ) 	{
		clipped_v1 = FIND_SCALED_NUM9(ymax,y0,y1,v0,v1);
		clipped_y1 = ymax;
	}
	
	dx0 = fl2i(clipped_x0); dx1 = fl2i(clipped_x1);
	dy0 = fl2i(clipped_y0); dy1 = fl2i(clipped_y1);

	if (dx1<=dx0) return;
	if (dy1<=dy0) return;

	//============= DRAW IT =====================

	vertex v[4];
	vertex *vl[4];

	vl[0] = &v[0];	
	v->sx = clipped_x0;
	v->sy = clipped_y0;
	v->sw = va->sw;
	v->z = va->z;
	v->u = clipped_u0;
	v->v = clipped_v0;

	vl[1] = &v[1];	
	v[1].sx = clipped_x1;
	v[1].sy = clipped_y0;
	v[1].sw = va->sw;
	v[1].z = va->z;
	v[1].u = clipped_u1;
	v[1].v = clipped_v0;

	vl[2] = &v[2];	
	v[2].sx = clipped_x1;
	v[2].sy = clipped_y1;
	v[2].sw = va->sw;
	v[2].z = va->z;
	v[2].u = clipped_u1;
	v[2].v = clipped_v1;

	vl[3] = &v[3];	
	v[3].sx = clipped_x0;
	v[3].sy = clipped_y1;
	v[3].sw = va->sw;
	v[3].z = va->z;
	v[3].u = clipped_u0;
	v[3].v = clipped_v1;

	gr_d3d9_tmapper_internal( 4, vl, TMAP_FLAG_TEXTURED, 1 );
}


void gr_d3d9_set_palette(ubyte *new_palette, int is_alphacolor)
{
}

void gr_d3d9_init_color(color *c, int r, int g, int b)
{
	c->screen_sig = gr_screen.signature;
	c->red = unsigned char(r);
	c->green = unsigned char(g);
	c->blue = unsigned char(b);
	c->alpha = 255;
	c->ac_type = AC_TYPE_NONE;
	c->alphacolor = -1;
	c->is_alphacolor = 0;
	c->magic = 0xAC01;
}

void gr_d3d9_set_color_fast(color *dst)
{
	if ( dst->screen_sig != gr_screen.signature )	{
		if ( dst->is_alphacolor )	{
			gr_d3d9_init_alphacolor( dst, dst->red, dst->green, dst->blue, dst->alpha, dst->ac_type );
		} else {
			gr_d3d9_init_color( dst, dst->red, dst->green, dst->blue );
		}
	}
	gr_screen.current_color = *dst;
}

void gr_d3d9_set_cull(int cull)
{
// switch culling on or off
	if(cull){
		SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );		
	} else {
		SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );				
	}
}

void gr_d3d9_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type )
{
	if ( r < 0 ) r = 0; else if ( r > 255 ) r = 255;
	if ( g < 0 ) g = 0; else if ( g > 255 ) g = 255;
	if ( b < 0 ) b = 0; else if ( b > 255 ) b = 255;
	if ( alpha < 0 ) alpha = 0; else if ( alpha > 255 ) alpha = 255;

	gr_d3d9_init_color( clr, r, g, b );

	clr->alpha = unsigned char(alpha);
	clr->ac_type = (ubyte)type;
	clr->alphacolor = -1;
	clr->is_alphacolor = 1;
}

void gr_d3d9_aabitmap_ex(int x,int y,int w,int h,int sx,int sy)
{
	int reclip;
	#ifndef NDEBUG
	int count = 0;
	#endif

	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;

	int bw, bh;
	bm_get_info( gr_screen.current_bitmap, &bw, &bh, NULL );

	do {
		reclip = 0;
		#ifndef NDEBUG
			if ( count > 1 ) Int3();
			count++;
		#endif
	
		if ((dx1 > gr_screen.clip_right ) || (dx2 < gr_screen.clip_left)) return;
		if ((dy1 > gr_screen.clip_bottom ) || (dy2 < gr_screen.clip_top)) return;
		if ( dx1 < gr_screen.clip_left ) { sx += gr_screen.clip_left-dx1; dx1 = gr_screen.clip_left; }
		if ( dy1 < gr_screen.clip_top ) { sy += gr_screen.clip_top-dy1; dy1 = gr_screen.clip_top; }
		if ( dx2 > gr_screen.clip_right )	{ dx2 = gr_screen.clip_right; }
		if ( dy2 > gr_screen.clip_bottom )	{ dy2 = gr_screen.clip_bottom; }

		if ( sx < 0 ) {
			dx1 -= sx;
			sx = 0;
			reclip = 1;
		}

		if ( sy < 0 ) {
			dy1 -= sy;
			sy = 0;
			reclip = 1;
		}

		w = dx2-dx1+1;
		h = dy2-dy1+1;

		if ( sx + w > bw ) {
			w = bw - sx;
			dx2 = dx1 + w - 1;
		}

		if ( sy + h > bh ) {
			h = bh - sy;
			dy2 = dy1 + h - 1;
		}

		if ( w < 1 ) return;		// clipped away!
		if ( h < 1 ) return;		// clipped away!

	} while (reclip);

	// Make sure clipping algorithm works
	#ifndef NDEBUG
		Assert( w > 0 );
		Assert( h > 0 );
		Assert( w == (dx2-dx1+1) );
		Assert( h == (dy2-dy1+1) );
		Assert( sx >= 0 );
		Assert( sy >= 0 );
		Assert( sx+w <= bw );
		Assert( sy+h <= bh );
		Assert( dx2 >= dx1 );
		Assert( dy2 >= dy1 );
		Assert( (dx1 >= gr_screen.clip_left ) && (dx1 <= gr_screen.clip_right) );
		Assert( (dx2 >= gr_screen.clip_left ) && (dx2 <= gr_screen.clip_right) );
		Assert( (dy1 >= gr_screen.clip_top ) && (dy1 <= gr_screen.clip_bottom) );
		Assert( (dy2 >= gr_screen.clip_top ) && (dy2 <= gr_screen.clip_bottom) );
	#endif

	// We now have dx1,dy1 and dx2,dy2 and sx, sy all set validly within clip regions.
	gr_d3d9_aabitmap_ex_internal(BITMAP_NORMAL, dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy);
}

void gr_d3d9_aabitmap(int x, int y)
{
	int w, h;

	bm_get_info( gr_screen.current_bitmap, &w, &h, NULL );
	int dx1=x, dx2=x+w-1;
	int dy1=y, dy2=y+h-1;
	int sx=0, sy=0;

	if ((dx1 > gr_screen.clip_right ) || (dx2 < gr_screen.clip_left)) return;
	if ((dy1 > gr_screen.clip_bottom ) || (dy2 < gr_screen.clip_top)) return;
	if ( dx1 < gr_screen.clip_left ) { sx = gr_screen.clip_left-dx1; dx1 = gr_screen.clip_left; }
	if ( dy1 < gr_screen.clip_top ) { sy = gr_screen.clip_top-dy1; dy1 = gr_screen.clip_top; }
	if ( dx2 > gr_screen.clip_right )	{ dx2 = gr_screen.clip_right; }
	if ( dy2 > gr_screen.clip_bottom )	{ dy2 = gr_screen.clip_bottom; }

	if ( sx < 0 ) return;
	if ( sy < 0 ) return;
	if ( sx >= w ) return;
	if ( sy >= h ) return;

	// Draw bitmap bm[sx,sy] into (dx1,dy1)-(dx2,dy2)
	gr_aabitmap_ex(dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy);
}

void gr_d3d9_zbias(int zbias)
{
	SetRenderState(D3DRS_DEPTHBIAS, zbias);
}

int gr_d3d9_zbuffer_get()
{
	if ( !D3D9_global_zbuffering )	{
		return GR_ZBUFF_NONE;
	}
	
	return gr_zbuffering_mode;
}

int gr_d3d9_zbuffer_set(int mode)
{
	const int last = gr_zbuffering_mode;
	gr_zbuffering_mode = mode;
	D3D9_zbuffering = gr_zbuffering_mode != GR_ZBUFF_NONE;
	return last;
}

// If mode is FALSE, turn zbuffer off the entire frame,
// no matter what people pass to gr_zbuffer_set.
void gr_d3d9_zbuffer_clear(int mode)
{
	if ( mode )	{
		D3D9_zbuffering = 1;
		gr_zbuffering_mode = GR_ZBUFF_FULL;
		D3D9_global_zbuffering = 1;

		// Make sure zbuffering is on
		gr_d3d9_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_FULL );

		D3DRECT rect;

		rect.x1 = gr_screen.clip_left + gr_screen.offset_x;
		rect.y1 = gr_screen.clip_top + gr_screen.offset_y;
		rect.x2 = gr_screen.clip_right + gr_screen.offset_x;
		rect.y2 = gr_screen.clip_bottom + gr_screen.offset_y;

		HRESULT hr = GetDevice()->Clear(1, &rect, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
			
		if(FAILED(hr))
		{
			mprintf(( "Failed to clear zbuffer!\n" ));
			return;
		}


	} else {
		D3D9_zbuffering = 0;
		gr_zbuffering_mode = GR_ZBUFF_NONE;
		D3D9_global_zbuffering = 0;
	}
}

// Replace screenshot code
void gr_d3d9_dump_frame() {}
void gr_d3d9_dump_frame_start( int first_frame_number, int nframes_between_dumps ) {}
void gr_d3d9_dump_frame_stop() {}

// Do we need gamma?
void gr_d3d9_set_gamma(float gamma) {}

void gr_d3d9_aascaler(vertex *va, vertex *vb )
{
	// not required
}

void gr_d3d9_flash( int r, int g, int b )
{
	CAP(r,0,255);
	CAP(g,0,255);
	CAP(b,0,255);

	if ( !(r || g || b) )	
	{
		return;
	}

	uint color;
	// if ( lpDevDesc->dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_ONE  )	{
		gr_d3d9_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_NONE );
		color = D3DCOLOR_RGBA(r, g, b, 255);
	/*
	} else {
		gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

		int a = (r+g+b)/3;
		color = D3DCOLOR_RGBA(r,g,b,a);
	} */

	float x1 = i2fl(gr_screen.clip_left+gr_screen.offset_x);
	float y1 = i2fl(gr_screen.clip_top+gr_screen.offset_y);
	float x2 = i2fl(gr_screen.clip_right+gr_screen.offset_x);
	float y2 = i2fl(gr_screen.clip_bottom+gr_screen.offset_y);

	const int kNumVerts = 4;
	Vertex *src_v;
	Vertex d3d_verts[kNumVerts];
	for(int i = 0; i < kNumVerts; i++)
	{
		d3d_verts[i].sz = 0.99f;
		d3d_verts[i].color = color;
	}

	src_v = d3d_verts;

	src_v->sx = x1;
	src_v->sy = y1;
	src_v++;

	src_v->sx = x2;
	src_v->sy = y1;
	src_v++;

	src_v->sx = x2;
	src_v->sy = y2;
	src_v++;

	src_v->sx = x1;
	src_v->sy = y2;

	DrawPrimitive2D(D3DPT_TRIANGLEFAN, 2, d3d_verts, TMAP_FLAG_NO_TEXTURE);
}

// Used for rendering background to messagebox
int gr_d3d9_save_screen()
{ 
	return 0;
}
void gr_d3d9_restore_screen(int id)
{
	gr_clear();
}

void gr_d3d9_free_screen(int id){}

void gr_d3d9_set_color( int r, int g, int b )
{
	assert((r >= 0) && (r < 256));
	assert((g >= 0) && (g < 256));
	assert((b >= 0) && (b < 256));

	gr_d3d9_init_color( &gr_screen.current_color, r, g, b );
}

void gr_d3d9_print_screen(char *filename)
{

}

void gr_d3d9_fog_set(int fog_mode, int r, int g, int b, float fog_near, float fog_far)
{
	D3DCOLOR color = 0;	
	  
	Assert((r >= 0) && (r < 256));
	Assert((g >= 0) && (g < 256));
	Assert((b >= 0) && (b < 256));	

	// turning fog off
	if(fog_mode == GR_FOGMODE_NONE){
		// only change state if we need to
		if(gr_screen.current_fog_mode != fog_mode){
			GetDevice()->SetRenderState(D3DRS_FOGENABLE, FALSE );		
		}
		gr_screen.current_fog_mode = fog_mode;

		// to prevent further state changes
		return;
	}

	// maybe switch fogging on
	if(gr_screen.current_fog_mode != fog_mode){		
		GetDevice()->SetRenderState(D3DRS_FOGENABLE, TRUE);	

		// if we're using table fog, enable table fogging
		if(D3D9_fog_mode == 2){
			GetDevice()->SetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_LINEAR );			
		}

		gr_screen.current_fog_mode = fog_mode;	
	}	

	// is color changing?
	if( (gr_screen.current_fog_color.red != r) || (gr_screen.current_fog_color.green != g) || (gr_screen.current_fog_color.blue != b) ){
		// store the values
		gr_init_color( &gr_screen.current_fog_color, r, g, b );

		color = D3DCOLOR_RGBA(r, g, b, 255);
		GetDevice()->SetRenderState(D3DRS_FOGCOLOR, color);	
	}		

	// planes changing?
	if( (fog_near >= 0.0f) && (fog_far >= 0.0f) && ((fog_near != gr_screen.fog_near) || (fog_far != gr_screen.fog_far)) ){		
		gr_screen.fog_near = fog_near;		
		gr_screen.fog_far = fog_far;					

		// only generate a new fog table if we have to (wfog/table fog mode)
		if(D3D9_fog_mode == 2){	
			GetDevice()->SetRenderState( D3DRS_FOGSTART, *((DWORD *)(&fog_near)));		
			GetDevice()->SetRenderState( D3DRS_FOGEND, *((DWORD *)(&fog_far)));
		}				
	}	
}

void gr_d3d9_get_region(int front, int w, int g, ubyte *data)
{
}

// cross fade (used in credits)
void gr_d3d9_cross_fade(int bmap1, int bmap2, int x1, int y1, int x2, int y2, float pct)
{
	gr_set_bitmap(bmap1, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, 1.0 - pct );
	gr_bitmap(x1, y1);
	gr_set_bitmap(bmap2, GR_ALPHABLEND_FILTER, GR_BITBLT_MODE_NORMAL, pct );
	gr_bitmap(x2, y2);
}

uint gr_d3d9_lock()
{
	return 1;
}

void gr_d3d9_unlock()
{
}

bool SetMatrices()
{
					
	float dvWNear = 0.0f;
	float dvWFar  = D3D9_z_mult;

	HRESULT result = S_OK;
	D3DXMATRIX matWorld;
	D3DXMATRIX matView;
	D3DXMATRIX matProj;

	D3DXMatrixIdentity(&matWorld);
	D3DXMatrixIdentity(&matView);
	D3DXMatrixIdentity(&matProj);

	//result = lpDev->SetTransform( D3DTRANSFORMSTATE_WORLD,      &matWorld );
	//assert(result);
	result = GetDevice()->SetTransform(D3DTS_VIEW,       &matView );
	assert( SUCCEEDED(result));

	matProj._43 = 0;
	matProj._34 = 1;
	matProj._44 = dvWNear; // not used
	matProj._33 = dvWNear / (dvWFar - dvWNear) + 1;  

	result = GetDevice()->SetTransform(D3DTS_PROJECTION, &matProj );
	assert( SUCCEEDED(result));
	return SUCCEEDED(result);
}