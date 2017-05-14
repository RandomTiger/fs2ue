/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#ifndef UNITY_BUILD
#include "GrD3DInternal.h"
#include "2d.H"
#include "BmpMan.h"
#include "PalMan.h"
#include "Line.h"
#include "Cfile.h"
#include "3d.h"
#include "Math/vector.h"
#include "Neb.h"

#include <assert.h>
#include "font.h"
#include "tmapper.h"
#endif

const int kBatchLimit = 10240; 

SafeArray<D3DTLVERTEX> batchData(kBatchLimit);
int batchVertexCount = 0;
int batchCount = 0;
int largestVertexCount = 0;

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

int D3d_last_state = -1;
int gr_global_zbuffering = 0;
int gr_zbuffering = 0;
int gr_zbuffering_mode = 0;

// Hack! move to another file!
extern int D3D_fog_mode;

void gr_d3d_set_state( gr_texture_source ts, gr_alpha_blend ab, gr_zbuffer_type zt )
{
	int current_state = 0;

	current_state = current_state | (ts<<0);
	current_state = current_state | (ab<<5);
	current_state = current_state | (zt<<10);

	if ( current_state == D3d_last_state ) {
		return;
	}

	D3d_last_state = current_state;

	switch( ts )	{
	case TEXTURE_SOURCE_NONE:
		d3d_SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, NULL );
		// Let the texture cache system know whe set the handle to NULL
		gr_tcache_set(-1, -1, NULL, NULL );
		break;

	case TEXTURE_SOURCE_DECAL:
		d3d_SetRenderState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_LINEAR );
		d3d_SetRenderState(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_LINEAR );

		if ( lpDevDesc->dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEALPHA )	{
			d3d_SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA );
		} else {
			d3d_SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE );
		}
		d3d_SetRenderState(D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_WRAP);
		break;

	case TEXTURE_SOURCE_NO_FILTERING:
		d3d_SetRenderState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_NEAREST );
		d3d_SetRenderState(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_NEAREST );

		if ( lpDevDesc->dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEALPHA )	{
			d3d_SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA );
		} else {
			d3d_SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATE );
		}
		d3d_SetRenderState(D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_CLAMP);
		break;

	default:
		Int3();
	}

	switch( ab )	{
	case ALPHA_BLEND_NONE:							// 1*SrcPixel + 0*DestPixel
		d3d_SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
		break;

	case ALPHA_BLEND_ALPHA_ADDITIVE:				// Alpha*SrcPixel + 1*DestPixel
	case ALPHA_BLEND_ALPHA_BLEND_SRC_COLOR:	// Alpha*SrcPixel + (1-SrcPixel)*DestPixel
		if ( lpDevDesc->dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_ONE  )	{
			d3d_SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			// Must use ONE:ONE as the Permedia2 can't do SRCALPHA:ONE.
			// But I lower RGB values so we don't loose anything.
			d3d_SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE );
			//d3d_SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA );
			d3d_SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE );
			break;
		}
		// Fall through to normal alpha blending mode...

	case ALPHA_BLEND_ALPHA_BLEND_ALPHA:			// Alpha*SrcPixel + (1-Alpha)*DestPixel
		if ( lpDevDesc->dpcTriCaps.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA  )	{
			d3d_SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			d3d_SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA );
			d3d_SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );
		} else {
			d3d_SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			d3d_SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_BOTHSRCALPHA );
		}
		break;


	default:
		Int3();
	}

	switch( zt )	{

	case ZBUFFER_TYPE_NONE:
		d3d_SetRenderState(D3DRENDERSTATE_ZENABLE,FALSE);
		d3d_SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,FALSE);
		break;

	case ZBUFFER_TYPE_READ:
		d3d_SetRenderState(D3DRENDERSTATE_ZENABLE,TRUE);
		d3d_SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,FALSE);
		break;

	case ZBUFFER_TYPE_WRITE:
		d3d_SetRenderState(D3DRENDERSTATE_ZENABLE,FALSE);
		d3d_SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,TRUE);
		break;

	case ZBUFFER_TYPE_FULL:
		d3d_SetRenderState(D3DRENDERSTATE_ZENABLE,TRUE);
		d3d_SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,TRUE);
		break;

	default:
		Int3();
	}

}

void d3d_zbias(int bias)
{
	d3d_SetRenderState(D3DRENDERSTATE_ZBIAS, bias);
}

// If mode is FALSE, turn zbuffer off the entire frame,
// no matter what people pass to gr_zbuffer_set.
void gr_d3d_zbuffer_clear(int mode)
{
	if ( mode )	{
		gr_zbuffering = 1;
		gr_zbuffering_mode = GR_ZBUFF_FULL;
		gr_global_zbuffering = 1;

		// Make sure zbuffering is on
		gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_FULL );


		// An application can clear z-buffers by using the IDirectDrawSurface2::Blt method. 
		// The DDBLT_DEPTHFILL flag indicates that the blit clears z-buffers. If this flag 
		// is specified, the DDBLTFX structure passed to the IDirectDrawSurface2::Blt method 
		// should have its dwFillDepth member set to the required z-depth. If the DirectDraw device 
		// driver for a 3D-accelerated display card is designed to provide support for z-buffer 
		// clearing in hardware, it should export the DDCAPS_BLTDEPTHFILL flag and should 
		// handle DDBLT_DEPTHFILL blits. The destination surface of a depth-fill blit must 
		// be a z-buffer.
		// Note The actual interpretation of a depth value is specific to the 3D renderer.

		D3DRECT rect;

		rect.x1 = gr_screen.clip_left + gr_screen.offset_x;
		rect.y1 = gr_screen.clip_top + gr_screen.offset_y;
		rect.x2 = gr_screen.clip_right + gr_screen.offset_x;
		rect.y2 = gr_screen.clip_bottom + gr_screen.offset_y;

		if (lpViewport->Clear( 1, &rect, D3DCLEAR_ZBUFFER ) != D3D_OK )	{
			mprintf(( "Failed to clear zbuffer!\n" ));
			return;
		}


	} else {
		gr_zbuffering = 0;
		gr_zbuffering_mode = GR_ZBUFF_NONE;
		gr_global_zbuffering = 0;
	}
}

int gr_d3d_zbuffer_get()
{
	if ( !gr_global_zbuffering )	{
		return GR_ZBUFF_NONE;
	}
	return gr_zbuffering_mode;
}

int gr_d3d_zbuffer_set(int mode)
{
	/*
	if ( !gr_global_zbuffering )	{
		gr_zbuffering = 0;
		return GR_ZBUFF_NONE;
	}
	*/

	int tmp = gr_zbuffering_mode;

	gr_zbuffering_mode = mode;

	if ( gr_zbuffering_mode == GR_ZBUFF_NONE )	{
		gr_zbuffering = 0;
	} else {
		gr_zbuffering = 1;
	}
	return tmp;
}

void d3d_make_rect( D3DTLVERTEX *a, D3DTLVERTEX *b, int x1, int y1, int x2, int y2 )
{
	// Alan's nvidia riva128 PCI screws up targetting brackets if 
	// rhw are uninitialized.
	a->rhw = 1.0f;
	b->rhw = 1.0f;

	// just for completeness, initialize specular and sz.
	a->specular = 0;
	b->specular = 0;

	a->sz = 0.99f;
	b->sz = 0.99f;

	a->sx = i2fl(x1 + gr_screen.offset_x);
	a->sy = i2fl(y1 + gr_screen.offset_y);

	b->sx = i2fl(x2 + gr_screen.offset_x);
	b->sy = i2fl(y2 + gr_screen.offset_y);

	if ( x1 == x2 )	{
		// Verticle line
		if ( a->sy < b->sy )	{
			b->sy += 0.5f;
		} else {
			a->sy += 0.5f;
		}
	} else if ( y1 == y2 )	{
		// Horizontal line
		if ( a->sx < b->sx )	{
			b->sx += 0.5f;
		} else {
			a->sx += 0.5f;
		}
	}

}

// basically just fills in the alpha component of the specular color. Hardware does the rest
// when rendering the poly
void gr_d3d_stuff_fog_value(float z, D3DCOLOR *spec)
{
	float f_float;	
	*spec = 0;

	// linear fog formula
	f_float = (gr_screen.fog_far - z) / (gr_screen.fog_far - gr_screen.fog_near);
	if(f_float < 0.0f){
		f_float = 0.0f;
	} else if(f_float > 1.0f){
		f_float = 1.0f;
	}
	*spec = D3DRGBA(0.0f, 0.0f, 0.0f, f_float);
}

float z_mult = 30000.0f;
DCF(zmult, "")
{
	dc_get_arg(ARG_FLOAT);
	z_mult = Dc_arg_float;
}

float flCAP( float x, float minx, float maxx)
{
	if ( x < minx )	{
		return minx;
	} else if ( x > maxx )	{
		return maxx;
	}
	return x;
}

const int NEBULA_COLORS = 20;
const int VERT_ARRAY_SIZE = 64;
const int VERT_ARRAY_SIZE_LIST = VERT_ARRAY_SIZE * 3;
SafeArray<D3DTLVERTEX> batchConvertData(VERT_ARRAY_SIZE_LIST);

void gr_d3d_tmapper_internal( int nverts, vertex **verts, uint flags, int is_scaler )	
{
	int i;
	float u_scale = 1.0f, v_scale = 1.0f;	

	// Make nebula use the texture mapper... this blends the colors better.
	if ( flags & TMAP_FLAG_NEBULA ){
		Int3();
		/*
		flags |= TMAP_FLAG_TEXTURED | TMAP_FLAG_CORRECT;

		static int test_bmp = -1;
		static ushort data[16];
		if ( test_bmp == -1 ){
			ushort pix;
			ubyte a, r, g, b;
			int idx;

			// stuff the fake bitmap
			a = 1; r = 255; g = 255; b = 255;
			pix = 0;
			bm_set_components((ubyte*)&pix, &r, &g, &b, &a);			
			for(idx=0; idx<16; idx++){
				data[idx] = pix;
			}			
			test_bmp = bm_create( 16, 4, 4, data );
		}
		gr_set_bitmap( test_bmp );

		for (i=0; i<nverts; i++ )	{
			verts[i]->u = verts[i]->v = 0.5f;
		}
		*/
	}

	gr_texture_source texture_source = (gr_texture_source)-1;
	gr_alpha_blend alpha_blend = (gr_alpha_blend)-1;
	gr_zbuffer_type zbuffer_type = (gr_zbuffer_type)-1;


	if ( gr_zbuffering )	{
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

		if ( lpDevDesc->dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_ONE  )	{
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
		} else {

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
	} else {
		alpha_blend = ALPHA_BLEND_ALPHA_BLEND_ALPHA;
		alpha = 255;
	}

	if(flags & TMAP_FLAG_NO_FILTERING){
		tmap_type = TCACHE_TYPE_INTERFACE;
	}

	texture_source = TEXTURE_SOURCE_NONE;
 
	if ( flags & TMAP_FLAG_TEXTURED )	{
		if ( !gr_tcache_set(gr_screen.current_bitmap, tmap_type, &u_scale, &v_scale, 0))	{
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
	
	gr_d3d_set_state( texture_source, alpha_blend, zbuffer_type );
	

	D3DTLVERTEX d3d_verts[VERT_ARRAY_SIZE];
	D3DTLVERTEX *src_v = d3d_verts;

	int x1, y1, x2, y2;
	x1 = gr_screen.clip_left*16;
	x2 = gr_screen.clip_right*16+15;
	y1 = gr_screen.clip_top*16;
	y2 = gr_screen.clip_bottom*16+15;

	// turn on pixel fog if we're rendering against a fullneb background
	// if(flags & TMAP_FLAG_PIXEL_FOG){					
		// set fog
 	//	gr_fog_set(GR_FOGMODE_FOG, gr_screen.current_fog_color.red, gr_screen.current_fog_color.green, gr_screen.current_fog_color.blue);
	// }	

	assert(nverts < VERT_ARRAY_SIZE);
	for (i=0; i<nverts; i++ )	{
		vertex * va = verts[i];		
				
		// store in case we're doing vertex fog.		
		if ( gr_zbuffering || (flags & TMAP_FLAG_NEBULA) )	{
			src_v->sz = va->z / z_mult;	// For zbuffering and fogging
			if ( src_v->sz > 0.98f )	{
				src_v->sz = 0.98f;
			}		
		} else {
			src_v->sz = 0.99f;
		}			

		if ( flags & TMAP_FLAG_CORRECT )	{
			src_v->rhw = va->sw;				// For texture correction 						
		} else {
			src_v->rhw = 1.0f;				// For texture correction 
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
			r = Gr_gamma_lookup[verts[i]->b];
			g = Gr_gamma_lookup[verts[i]->b];
			b = Gr_gamma_lookup[verts[i]->b];
		} else if ( (flags & TMAP_FLAG_RGB)  && (flags & TMAP_FLAG_GOURAUD) )	{
			// Make 0.75 be 256.0f
			r = Gr_gamma_lookup[verts[i]->r];
			g = Gr_gamma_lookup[verts[i]->g];
			b = Gr_gamma_lookup[verts[i]->b];
		} else {
			// use constant RGB values...
		}

		src_v->color = RGBA_MAKE(r, g, b, a);

		// if we're fogging and we're doing vertex fog
		if((gr_screen.current_fog_mode != GR_FOGMODE_NONE) && (D3D_fog_mode == 1)){
			gr_d3d_stuff_fog_value(va->z, &src_v->specular);
		} else {
			src_v->specular = 0;
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
	{

		d3d_DrawPrimitive(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, (LPVOID)d3d_verts, nverts, NULL);
	}
	// turn off fog
	// if(flags & TMAP_FLAG_PIXEL_FOG){
		// gr_fog_set(GR_FOGMODE_NONE, 0, 0, 0);
	// }
}

void d3d_batch_render()
{
	if(batchVertexCount > 0)
	{
		d3d_DrawPrimitive(D3DPT_TRIANGLELIST, D3DVT_TLVERTEX, batchData.GetArray(), batchVertexCount, NULL);
		
		//char buffer[1024];
		//sprintf(buffer, "Rendered batch of %d\n", superBatchVertexCount );
		//::OutputDebugString(buffer);
		batchVertexCount = 0;
		batchCount= 0;
	}
}

void d3d_batch(D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, SafeArray<D3DTLVERTEX> &lpvVertices, DWORD dwVertexCount, DWORD dwFlags )
{
	//char buffer[1024];
	//sprintf(buffer, "Batch %d\n", dwVertexCount );
	//::OutputDebugString(buffer);

	if((batchVertexCount + dwVertexCount) >= kBatchLimit || (batchVertexCount + dwVertexCount) >= lpDevDesc->dwMaxVertexCount)
	{
		d3d_batch_render();
	}

	batchData.MemCpy(batchVertexCount, lpvVertices, dwVertexCount);
	batchVertexCount += dwVertexCount;

	if(batchVertexCount > largestVertexCount)
	{
		mprintf(("largestVertexCount = %d\n", largestVertexCount));
		largestVertexCount= batchVertexCount;
	}

	batchCount += 1;
}

void gr_d3d_tmapper( int nverts, vertex **verts, uint flags )	
{
	gr_d3d_tmapper_internal( nverts, verts, flags, 0 );
}

#define FIND_SCALED_NUM(x,x0,x1,y0,y1) (((((x)-(x0))*((y1)-(y0)))/((x1)-(x0)))+(y0))

void gr_d3d_scaler(vertex *va, vertex *vb )
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
		clipped_u0 = FIND_SCALED_NUM(xmin,x0,x1,u0,u1);
		clipped_x0 = xmin;
	}

	// Clip the right, moving u1 left as necessary
	if ( x1 > xmax )	{
		clipped_u1 = FIND_SCALED_NUM(xmax,x0,x1,u0,u1);
		clipped_x1 = xmax;
	}

	// Clip the top, moving v0 down as necessary
	if ( y0 < ymin ) 	{
		clipped_v0 = FIND_SCALED_NUM(ymin,y0,y1,v0,v1);
		clipped_y0 = ymin;
	}

	// Clip the bottom, moving v1 up as necessary
	if ( y1 > ymax ) 	{
		clipped_v1 = FIND_SCALED_NUM(ymax,y0,y1,v0,v1);
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

	gr_d3d_tmapper_internal( 4, vl, TMAP_FLAG_TEXTURED, 1 );
}

void gr_d3d_aascaler(vertex *va, vertex *vb )
{
}


void gr_d3d_pixel(int x, int y)
{
	gr_line(x,y,x,y);
}


void gr_d3d_clear()
{
	// Turn off zbuffering so this doesn't clear the zbuffer
	gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE );

	RECT dst;
	DDBLTFX ddbltfx;
	DDSURFACEDESC ddsd;	

	// Get the surface desc
	ddsd.dwSize = sizeof(ddsd);
	lpBackBuffer->GetSurfaceDesc(&ddsd);   

	memset(&ddbltfx, 0, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(DDBLTFX);

	ddbltfx.dwFillColor = RGB_MAKE(gr_screen.current_clear_color.red, gr_screen.current_clear_color.green, gr_screen.current_clear_color.blue);

	dst.left = gr_screen.clip_left+gr_screen.offset_x;
	dst.top = gr_screen.clip_top+gr_screen.offset_y;
	dst.right = gr_screen.clip_right+1+gr_screen.offset_x;
	dst.bottom = gr_screen.clip_bottom+1+gr_screen.offset_y;	

	if ( lpBackBuffer->Blt( &dst, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx ) != DD_OK )	{
		return;
	}
}


// sets the clipping region & offset
void gr_d3d_set_clip(int x,int y,int w,int h)
{
	d3d_batch_render();

	gr_screen.offset_x = x;
	gr_screen.offset_y = y;

	gr_screen.clip_left = 0;
	gr_screen.clip_right = w-1;

	gr_screen.clip_top = 0;
	gr_screen.clip_bottom = h-1;

	// check for sanity of parameters
	if ( gr_screen.clip_left+x < 0 ) {
		gr_screen.clip_left = -x;
	} else if ( gr_screen.clip_left+x > gr_screen.max_w-1 )	{
		gr_screen.clip_left = gr_screen.max_w-1-x;
	}
	if ( gr_screen.clip_right+x < 0 ) {
		gr_screen.clip_right = -x;
	} else if ( gr_screen.clip_right+x >= gr_screen.max_w-1 )	{
		gr_screen.clip_right = gr_screen.max_w-1-x;
	}

	if ( gr_screen.clip_top+y < 0 ) {
		gr_screen.clip_top = -y;
	} else if ( gr_screen.clip_top+y > gr_screen.max_h-1 )	{
		gr_screen.clip_top = gr_screen.max_h-1-y;
	}

	if ( gr_screen.clip_bottom+y < 0 ) {
		gr_screen.clip_bottom = -y;
	} else if ( gr_screen.clip_bottom+y > gr_screen.max_h-1 )	{
		gr_screen.clip_bottom = gr_screen.max_h-1-y;
	}

	gr_screen.clip_width = gr_screen.clip_right - gr_screen.clip_left + 1;
	gr_screen.clip_height = gr_screen.clip_bottom - gr_screen.clip_top + 1;

	// Setup the viewport for a reasonable viewing area
	D3DVIEWPORT viewdata;
	DWORD       largest_side;
	HRESULT		ddrval;

	// Compensate for aspect ratio
	if ( gr_screen.clip_width > gr_screen.clip_height )
		largest_side = gr_screen.clip_width;
	else
		largest_side = gr_screen.clip_height;

	viewdata.dwSize = sizeof( viewdata );
	viewdata.dwX = gr_screen.clip_left+x;
	viewdata.dwY = gr_screen.clip_top+y;
	viewdata.dwWidth = gr_screen.clip_width;
	viewdata.dwHeight = gr_screen.clip_height;
	viewdata.dvScaleX = largest_side / 2.0F;
	viewdata.dvScaleY = largest_side / 2.0F;
	viewdata.dvMaxX = ( float ) ( viewdata.dwWidth / ( 2.0F * viewdata.dvScaleX ) );
	viewdata.dvMaxY = ( float ) ( viewdata.dwHeight / ( 2.0F * viewdata.dvScaleY ) );
	viewdata.dvMinZ = 0.0F;
	viewdata.dvMaxZ = 0.0F; // choose something appropriate here!

	ddrval = lpViewport->SetViewport( &viewdata );
	if ( ddrval != DD_OK )	{
		mprintf(( "GR_D3D_SET_CLIP: SetViewport failed.\n" ));
	}

}


void gr_d3d_reset_clip()
{
	d3d_batch_render();

	gr_screen.offset_x = 0;
	gr_screen.offset_y = 0;
	gr_screen.clip_left = 0;
	gr_screen.clip_top = 0;
	gr_screen.clip_right = gr_screen.max_w - 1;
	gr_screen.clip_bottom = gr_screen.max_h - 1;
	gr_screen.clip_width = gr_screen.max_w;
	gr_screen.clip_height = gr_screen.max_h;

	// Setup the viewport for a reasonable viewing area
	D3DVIEWPORT viewdata;
	DWORD       largest_side;
	HRESULT		ddrval;

	// Compensate for aspect ratio
	if ( gr_screen.clip_width > gr_screen.clip_height )
		largest_side = gr_screen.clip_width;
	else
		largest_side = gr_screen.clip_height;

	viewdata.dwSize = sizeof( viewdata );
	viewdata.dwX = gr_screen.clip_left;
	viewdata.dwY = gr_screen.clip_top;
	viewdata.dwWidth = gr_screen.clip_width;
	viewdata.dwHeight = gr_screen.clip_height;
	viewdata.dvScaleX = largest_side / 2.0F;
	viewdata.dvScaleY = largest_side / 2.0F;
	viewdata.dvMaxX = ( float ) ( viewdata.dwWidth / ( 2.0F * viewdata.dvScaleX ) );
	viewdata.dvMaxY = ( float ) ( viewdata.dwHeight / ( 2.0F * viewdata.dvScaleY ) );
	viewdata.dvMinZ = 0.0F;
	viewdata.dvMaxZ = 0.0F; // choose something appropriate here!

	ddrval = lpViewport->SetViewport( &viewdata );
	if ( ddrval != DD_OK )	{
		mprintf(( "GR_D3D_SET_CLIP: SetViewport failed.\n" ));
	}

}

void gr_d3d_init_color(color *c, int r, int g, int b)
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

void gr_d3d_init_alphacolor( color *clr, int r, int g, int b, int alpha, int type )
{
	if ( r < 0 ) r = 0; else if ( r > 255 ) r = 255;
	if ( g < 0 ) g = 0; else if ( g > 255 ) g = 255;
	if ( b < 0 ) b = 0; else if ( b > 255 ) b = 255;
	if ( alpha < 0 ) alpha = 0; else if ( alpha > 255 ) alpha = 255;

	gr_d3d_init_color( clr, r, g, b );

	clr->alpha = unsigned char(alpha);
	clr->ac_type = (ubyte)type;
	clr->alphacolor = -1;
	clr->is_alphacolor = 1;
}

void gr_d3d_set_color( int r, int g, int b )
{
	Assert((r >= 0) && (r < 256));
	Assert((g >= 0) && (g < 256));
	Assert((b >= 0) && (b < 256));

	gr_d3d_init_color( &gr_screen.current_color, r, g, b );
}
void gr_d3d_set_color_fast(color *dst)
{
	if ( dst->screen_sig != gr_screen.signature )	{
		if ( dst->is_alphacolor )	{
			gr_d3d_init_alphacolor( dst, dst->red, dst->green, dst->blue, dst->alpha, dst->ac_type );
		} else {
			gr_d3d_init_color( dst, dst->red, dst->green, dst->blue );
		}
	}
	gr_screen.current_color = *dst;
}

void gr_d3d_set_bitmap( int bitmap_num, int alphablend_mode, int bitblt_mode, float alpha)
{
	gr_screen.current_alpha = alpha;
	gr_screen.current_alphablend_mode = alphablend_mode;
	gr_screen.current_bitblt_mode = bitblt_mode;
	gr_screen.current_bitmap = bitmap_num;
}

void gr_d3d_bitmap(int x, int y)
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

enum
{
	BITMAP_FONT,
	BITMAP_NORMAL,
};

// Renders text and HUD
void gr_d3d_aabitmap_ex_internal(int type, int x,int y,int w,int h,int sx,int sy)
{
	if ( w < 1 ) return;
	if ( h < 1 ) return;

	if ( !gr_screen.current_color.is_alphacolor )	return;

	float u_scale, v_scale;

	gr_d3d_set_state( TEXTURE_SOURCE_NO_FILTERING, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

	if ( !gr_tcache_set( gr_screen.current_bitmap, TCACHE_TYPE_AABITMAP, &u_scale, &v_scale ) )	{
		// Couldn't set texture
		//mprintf(( "GLIDE: Error setting aabitmap texture!\n" ));
		return;
	}

	LPD3DTLVERTEX src_v;
	D3DTLVERTEX d3d_verts[4];

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

	src_v = d3d_verts;

	uint color;

	if ( gr_screen.current_color.is_alphacolor )	{
		if ( lpDevDesc->dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATEALPHA )	{
			color = RGBA_MAKE(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue,gr_screen.current_color.alpha);
		} else {
			int r = (gr_screen.current_color.red*gr_screen.current_color.alpha)/255;
			int g = (gr_screen.current_color.green*gr_screen.current_color.alpha)/255;
			int b = (gr_screen.current_color.blue*gr_screen.current_color.alpha)/255;
		
			color = RGBA_MAKE(r,g,b, 255 );
		}
	} else {
		color = RGB_MAKE(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue);
	}

	src_v->sz = 0.99f;
	src_v->rhw = 1.0f;
	src_v->color = color;	 
	src_v->specular = 0;
	src_v->sx = x1;
	src_v->sy = y1;
	src_v->tu = u0;
	src_v->tv = v0;
	src_v++;

	src_v->sz = 0.99f;
	src_v->rhw = 1.0f;
	src_v->color = color;	 
	src_v->specular = 0;
	src_v->sx = x2;
	src_v->sy = y1;
	src_v->tu = u1;
	src_v->tv = v0;
	src_v++;

	src_v->sz = 0.99f;
	src_v->rhw = 1.0f;
	src_v->color = color;	 
	src_v->specular = 0;
	src_v->sx = x2;
	src_v->sy = y2;
	src_v->tu = u1;
	src_v->tv = v1;
	src_v++;

	src_v->sz = 0.99f;
	src_v->rhw = 1.0f;
	src_v->color = color;	 
	src_v->specular = 0;
	src_v->sx = x1;
	src_v->sy = y2;
	src_v->tu = u0;
	src_v->tv = v1;

	d3d_DrawPrimitive(D3DPT_TRIANGLEFAN,D3DVT_TLVERTEX,(LPVOID)d3d_verts,4,NULL);
}

void gr_d3d_aabitmap_ex(int x,int y,int w,int h,int sx,int sy)
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
	gr_d3d_aabitmap_ex_internal(BITMAP_NORMAL, dx1,dy1,dx2-dx1+1,dy2-dy1+1,sx,sy);
}

void gr_d3d_aabitmap(int x, int y)
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


void gr_d3d_string( int sx, int sy, const char *s )
{
	int width, spacing, letter;
	int x, y;

	if ( !Current_font )	{
		return;
	}

	gr_set_bitmap(Current_font->bitmap_id);

	x = sx;
	y = sy;

	if (sx==0x8000) {			//centered
		x = get_centered_x(s);
	} else {
		x = sx;
	}
	
	spacing = 0;

	while (*s)	{
		x += spacing;

		while (*s== '\n' )	{
			s++;
			y += Current_font->h;
			if (sx==0x8000) {			//centered
				x = get_centered_x(s);
			} else {
				x = sx;
			}
		}
		if (*s == 0 ) break;

		letter = get_char_width(s[0],s[1],&width,&spacing);
		s++;

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
		//if ( x < gr_screen.clip_left ) xd = gr_screen.clip_left - x;
		//if ( y < gr_screen.clip_top ) yd = gr_screen.clip_top - y;
		xc = x+xd;
		yc = y+yd;

		wc = width - xd; hc = Current_font->h - yd;
		//if ( xc + wc > gr_screen.clip_right ) wc = gr_screen.clip_right - xc;
		//if ( yc + hc > gr_screen.clip_bottom ) hc = gr_screen.clip_bottom - yc;

		if ( wc < 1 ) continue;
		if ( hc < 1 ) continue;

		font_char *ch;
	
		ch = &Current_font->char_data[letter];

		int u = Current_font->bm_u[letter];
		int v = Current_font->bm_v[letter];

		gr_d3d_aabitmap_ex_internal(BITMAP_FONT, xc, yc, wc, hc, u+xd, v+yd );
	}
}

void gr_d3d_flash(int r, int g, int b)
{
	CAP(r,0,255);
	CAP(g,0,255);
	CAP(b,0,255);

	if ( r || g || b )	{
		uint color;
		if ( lpDevDesc->dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_ONE  )	{
			gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_ADDITIVE, ZBUFFER_TYPE_NONE );
			color = RGBA_MAKE(r, g, b, 255);
		} else {
			gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );
	
			int a = (r+g+b)/3;
			color = RGBA_MAKE(r,g,b,a);
		}
	
		float x1, x2, y1, y2;
		x1 = i2fl(gr_screen.clip_left+gr_screen.offset_x);
		y1 = i2fl(gr_screen.clip_top+gr_screen.offset_y);
		x2 = i2fl(gr_screen.clip_right+gr_screen.offset_x);
		y2 = i2fl(gr_screen.clip_bottom+gr_screen.offset_y);
	
		LPD3DTLVERTEX src_v;
		D3DTLVERTEX d3d_verts[4];

		src_v = d3d_verts;

		src_v->sz = 0.99f;
		src_v->rhw = 1.0f;
		src_v->color = color;	 
		src_v->specular = 0;
		src_v->sx = x1;
		src_v->sy = y1;
		src_v++;

		src_v->sz = 0.99f;
		src_v->rhw = 1.0f;
		src_v->color = color;	 
		src_v->specular = 0;
		src_v->sx = x2;
		src_v->sy = y1;
		src_v++;

		src_v->sz = 0.99f;
		src_v->rhw = 1.0f;
		src_v->color = color;	 
		src_v->specular = 0;
		src_v->sx = x2;
		src_v->sy = y2;
		src_v++;

		src_v->sz = 0.99f;
		src_v->rhw = 1.0f;
		src_v->color = color;	 
		src_v->specular = 0;
		src_v->sx = x1;
		src_v->sy = y2;

		d3d_DrawPrimitive(D3DPT_TRIANGLEFAN,D3DVT_TLVERTEX,(LPVOID)d3d_verts,4,NULL);
	}
}



void gr_d3d_create_shader(shader * shade, float r, float g, float b, float c )
{
	shade->screen_sig = gr_screen.signature;
	shade->r = r;
	shade->g = g;
	shade->b = b;
	shade->c = c;
}

void gr_d3d_set_shader( shader * shade )
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

void gr_d3d_circle( int xc, int yc, int d )
{

	int p,x, y, r;

	r = d/2;
	p=3-d;
	x=0;
	y=r;

	// Big clip
	if ( (xc+r) < gr_screen.clip_left ) return;
	if ( (xc-r) > gr_screen.clip_right ) return;
	if ( (yc+r) < gr_screen.clip_top ) return;
	if ( (yc-r) > gr_screen.clip_bottom ) return;

	while(x<y)	{
		// Draw the first octant
		gr_d3d_line( xc-y, yc-x, xc+y, yc-x );
		gr_d3d_line( xc-y, yc+x, xc+y, yc+x );

		if (p<0) 
			p=p+(x<<2)+6;
		else	{
			// Draw the second octant
			gr_d3d_line( xc-x, yc-y, xc+x, yc-y );
			gr_d3d_line( xc-x, yc+y, xc+x, yc+y );
			p=p+((x-y)<<2)+10;
			y--;
		}
		x++;
	}
	if(x==y)	{
		gr_d3d_line( xc-x, yc-y, xc+x, yc-y );
		gr_d3d_line( xc-x, yc+y, xc+x, yc+y );
	}
	return;

}


void gr_d3d_line(int x1,int y1,int x2,int y2)
{
	int clipped = 0, swapped=0;
	DWORD color;

	// Set up Render State - flat shading - alpha blending
	if ( (lpDevDesc->dpcLineCaps.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA) && (lpDevDesc->dpcLineCaps.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA)  )	{
		gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );
		color = RGBA_MAKE(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha );
	} else {
		// Matrox MGA-G200 doesn't support alpha-blended lines.
		gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE );

		int r = (gr_screen.current_color.red*gr_screen.current_color.alpha)/255;
		int g = (gr_screen.current_color.green*gr_screen.current_color.alpha)/255;
		int b = (gr_screen.current_color.blue*gr_screen.current_color.alpha)/255;
		
		color = RGBA_MAKE(r,g,b, 255 );
	}

	INT_CLIPLINE(x1,y1,x2,y2,gr_screen.clip_left,gr_screen.clip_top,gr_screen.clip_right,gr_screen.clip_bottom,return,clipped=1,swapped=1);

	D3DTLVERTEX d3d_verts[2];
	D3DTLVERTEX *a = d3d_verts;
	D3DTLVERTEX *b = d3d_verts+1;

	d3d_make_rect(a,b,x1,y1,x2,y2);

	a->color = color;
	b->color = color;

	d3d_DrawPrimitive(D3DPT_LINELIST,D3DVT_TLVERTEX,(LPVOID)d3d_verts,2,NULL);
}

void gr_d3d_aaline(vertex *v1, vertex *v2)
{
	gr_d3d_line( fl2i(v1->sx), fl2i(v1->sy), fl2i(v2->sx), fl2i(v2->sy) );
}


void gr_d3d_gradient(int x1,int y1,int x2,int y2)
{
	int clipped = 0, swapped=0;

	if ( !gr_screen.current_color.is_alphacolor )	{
		gr_line( x1, y1, x2, y2 );
		return;
	}

	INT_CLIPLINE(x1,y1,x2,y2,gr_screen.clip_left,gr_screen.clip_top,gr_screen.clip_right,gr_screen.clip_bottom,return,clipped=1,swapped=1);

	uint color1, color2;

	// Set up Render State - flat shading - alpha blending
	if ( (lpDevDesc->dpcLineCaps.dwSrcBlendCaps & D3DPBLENDCAPS_SRCALPHA) && (lpDevDesc->dpcLineCaps.dwDestBlendCaps & D3DPBLENDCAPS_INVSRCALPHA)  )	{
		gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

		if (lpDevDesc->dpcLineCaps.dwShadeCaps & D3DPSHADECAPS_ALPHAGOURAUDBLEND )	{
			color1 = RGBA_MAKE(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha );
			color2 = RGBA_MAKE(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, 0 );
		} else if (lpDevDesc->dpcLineCaps.dwShadeCaps & D3DPSHADECAPS_COLORGOURAUDRGB )	{
			color1 = RGBA_MAKE(gr_screen.current_color.red,gr_screen.current_color.green,gr_screen.current_color.blue,gr_screen.current_color.alpha);
			color2 = RGBA_MAKE(0,0,0,gr_screen.current_color.alpha);
		} else {
			color1 = RGBA_MAKE(gr_screen.current_color.red,gr_screen.current_color.green,gr_screen.current_color.blue,gr_screen.current_color.alpha);
			color2 = RGBA_MAKE(gr_screen.current_color.red,gr_screen.current_color.green,gr_screen.current_color.blue,gr_screen.current_color.alpha);
		}
	} else {
		// Matrox MGA-G200 doesn't support alpha-blended lines.
		gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE );

		int r = (gr_screen.current_color.red*gr_screen.current_color.alpha)/255;
		int g = (gr_screen.current_color.green*gr_screen.current_color.alpha)/255;
		int b = (gr_screen.current_color.blue*gr_screen.current_color.alpha)/255;

		if (lpDevDesc->dpcLineCaps.dwShadeCaps & D3DPSHADECAPS_COLORGOURAUDRGB )	{
			color1 = RGBA_MAKE(r,g,b, 255 );
			color2 = RGBA_MAKE(0,0,0, 255 );
		} else {
			color1 = RGBA_MAKE(r,g,b, 255 );
			color2 = RGBA_MAKE(r,g,b, 255 );
		}
	}

//	gr_d3d_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_NONE, ZBUFFER_TYPE_NONE );
//	color1 = RGBA_MAKE(gr_screen.current_color.red,gr_screen.current_color.green,gr_screen.current_color.blue,255);
//	color2 = RGBA_MAKE(gr_screen.current_color.red,gr_screen.current_color.green,gr_screen.current_color.blue,255);

	D3DTLVERTEX d3d_verts[2];
	D3DTLVERTEX *a = d3d_verts;
	D3DTLVERTEX *b = d3d_verts+1;

	d3d_make_rect( a, b, x1, y1, x2, y2 );

	if ( swapped )	{
		b->color = color1;
		a->color = color2;
	} else {
		a->color = color1;
		b->color = color2;
	}
	d3d_DrawPrimitive(D3DPT_LINELIST,D3DVT_TLVERTEX,(LPVOID)d3d_verts,2,NULL);
}


void gr_d3d_set_palette(ubyte *new_palette, int restrict_alphacolor)
{
}

void gr_d3d_print_screen(char *filename)
{
}



