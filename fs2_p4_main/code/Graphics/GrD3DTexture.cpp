/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 


#ifndef UNITY_BUILD
#include "GrD3D.h"
#include "GrD3DInternal.h"
#include "2d.H"
#include "BmpMan.h"
#include "Key.h"
#include "SystemVars.h"
#include "OsRegistry.h"

#include "multi_log.h"
#endif

typedef struct tcache_slot_d3d {
	LPDIRECTDRAWSURFACE		vram_texture_surface;
	LPDIRECT3DTEXTURE2		vram_texture;
	D3DTEXTUREHANDLE			texture_handle;
	float							u_scale, v_scale;
	int							bitmap_id;
	int							size;
	char							used_this_frame;
	int							time_created;
	ushort						w, h;

	tcache_slot_d3d			*parent;
} tcache_slot_d3d;

tcache_slot_d3d *Textures = NULL;


int D3D_texture_ram = 0;
int D3D_frame_count = 0;
int D3D_min_texture_width = 0;
int D3D_max_texture_width = 0;
int D3D_min_texture_height = 0;
int D3D_max_texture_height = 0;
int D3D_pow2_textures = 1;
int D3D_textures_in = 0;
int D3D_textures_in_frame = 0;
int D3D_last_bitmap_id = -1;
int D3D_last_detail = -1;
int D3D_last_bitmap_type = -1;

int vram_full = 0;

int d3d_create_texture(int bitmap_handle, int bitmap_type, tcache_slot_d3d *tslot, int fail_on_full);

int d3d_free_texture( tcache_slot_d3d *t )
{
	// Bitmap changed!!	
	if ( t->bitmap_id > -1 )	{				
		// if I, or any of my children have been used this frame, bail		
		if(t->used_this_frame){			
			return 0;
		}		

		// ok, now we know its legal to free everything safely
		if ( t->vram_texture )	{
			t->vram_texture->Release();
			t->vram_texture = NULL;
		}

		if ( t->vram_texture_surface )	{
			t->vram_texture_surface->Release();
			t->vram_texture_surface = NULL;
		}

		t->texture_handle = NULL;

		if ( D3D_last_bitmap_id == t->bitmap_id )	{
			D3D_last_bitmap_id = -1;
		}		

		t->bitmap_id = -1;
		t->used_this_frame = 0;
		D3D_textures_in -= t->size;		
	}

	return 1;
}

// we must make sure we never free my parent or any of my siblings!!!!!
int d3d_older_test(tcache_slot_d3d *new_slot, tcache_slot_d3d *test, tcache_slot_d3d *oldest)
{
	if ( (test != new_slot) && (test != new_slot->parent) && (test->bitmap_id > -1) && (!test->used_this_frame))	{
		if ( (oldest == NULL) || (test->time_created < oldest->time_created))	{
			return 1;
		}
	}

	// not older
	return 0;
}

int d3d_free_some_texture_ram(tcache_slot_d3d *t, int size)
{	
	tcache_slot_d3d *oldest = NULL;
	
	// Go through all the textures... find the oldest one 
	// that was not used this frame yet.
	int i;

	int goal_size = D3D_textures_in - size*2;
	if ( goal_size < 0 )	{
		goal_size = 0;
	} else if ( goal_size > D3D_texture_ram*3/4 )	{
		goal_size = D3D_texture_ram*3/4;
	}

	while( D3D_textures_in > goal_size )	{
		oldest = NULL;
		for( i=0; i<MAX_BITMAPS; i++ )	{			
			// maybe pick this one
			if(d3d_older_test(t, &Textures[i], oldest)){
				oldest = &Textures[i];
			}							
		}

		if ( oldest == NULL )	{
			mprintf(( "Couldn't free enough VRAM this frame... you might see some ugliness!\n" ));
			return 0;
		}

		d3d_free_texture(oldest);
	}

	mprintf(( "Freed 1/4 of the VRAM\n" ));
	return 1;
}

#ifndef NDEBUG
int Show_uploads = 0;
DCF_BOOL( show_uploads, Show_uploads )
#endif

// is the given existing texture handle the same dimensions as the passed in bitmap?
int d3d_tcache_same_dimension(int bitmap_id, int bitmap_type, tcache_slot_d3d *t)
{
	int w, h, nframes, fps;
	ubyte flags;

	// get bitmap info
	bm_get_info(bitmap_id, &w, &h, &flags, &nframes, &fps);

	if((t->w != w) || (t->h != h)){
		return 0;
	}

	// all good
	return 1;
}

int d3d_getValidTextureWidth(int width)
{
	for(int i = D3D_min_texture_width; i <= D3D_max_texture_width; i = i*2)
	{
		if(width <= i)
		{
			return i;
		}
	}

	return D3D_max_texture_width;
}

int d3d_getValidTextureHeight(int height)
{
	for(int i = D3D_min_texture_height; i <= D3D_max_texture_height; i = i*2)
	{
		if(height <= i)
		{
			return i;
		}
	}

	return D3D_min_texture_height;
}

// get the final texture size (the one which will get allocated as a surface)
void d3d_tcache_get_adjusted_texture_size(int w_in, int h_in, int *w_out, int *h_out)
{
	int tex_w, tex_h;

	// bogus
	if((w_out == NULL) ||  (h_out == NULL)){
		return;
	}

	// starting size
	tex_w = w_in;
	tex_h = h_in;

	if ( D3D_pow2_textures )	{
		int i;
		for (i=0; i<16; i++ )	{
			if ( (tex_w > (1<<i)) && (tex_w <= (1<<(i+1))) )	{
				tex_w = 1 << (i+1);
				break;
			}
		}

		for (i=0; i<16; i++ )	{
			if ( (tex_h > (1<<i)) && (tex_h <= (1<<(i+1))) )	{
				tex_h = 1 << (i+1);
				break;
			}
		}
	}

	if ( tex_w < D3D_min_texture_width ) {
		tex_w = D3D_min_texture_width;
	} else if ( tex_w > D3D_max_texture_width )	{
		tex_w = D3D_max_texture_width;
	}

	if ( tex_h < D3D_min_texture_height ) {
		tex_h = D3D_min_texture_height;
	} else if ( tex_h > D3D_max_texture_height )	{
		tex_h = D3D_max_texture_height;
	}

	// store the outgoing size
	*w_out = tex_w;
	*h_out = tex_h;
}

// data == start of bitmap data
// sx == x offset into bitmap
// sy == y offset into bitmap
// src_w == absolute width of section on source bitmap
// src_h == absolute height of section on source bitmap
// bmap_w == width of source bitmap
// bmap_h == height of source bitmap
// tex_w == width of final texture
// tex_h == height of final texture
int d3d_create_texture_sub(int bitmap_type, int texture_handle, ushort *data, int bmap_w, int bmap_h, int tex_w, int tex_h, tcache_slot_d3d *t, int reload, int fail_on_full)
{
	LPDIRECTDRAWSURFACE		sys_texture_surface = NULL;
	LPDIRECT3DTEXTURE2		sys_texture = NULL;
	int ret_val = 1;

	#ifndef NDEBUG
		if ( Show_uploads )	{
			if ( reload )	{
				mprintf(( "Reloading '%s'\n", bm_get_filename(texture_handle) ));
			} else {
				mprintf(( "Uploading '%s'\n", bm_get_filename(texture_handle) ));
			}
		}
	#endif

	// bogus
	if(t == NULL){
		return 0;
	}
		
	if ( t->used_this_frame )	{
		mprintf(( "ARGHH!!! Texture already used this frame!  Cannot free it!\n" ));
		return 0;
	}	
	if ( !reload )	{
		// gah
		if(!d3d_free_texture(t)){
			return 0;
		}
	}

	DDSURFACEDESC ddsd;
	HRESULT ddrval;
	DWORD dwHeight, dwWidth;
	int i,j;	
	ushort *bmp_data;

	DDPIXELFORMAT *surface_desc;

	if(bm_get_bpp(texture_handle) == 32)
	{	
		surface_desc = &AlphaTextureFormat32;
	}
	else
	{
		switch( bitmap_type )	{
			case TCACHE_TYPE_AABITMAP:		
				surface_desc = &AlphaTextureFormat;
				break;

			case TCACHE_TYPE_XPARENT:
				Int3();

			default:
				surface_desc = &NonAlphaTextureFormat;
				break;
		}	
	}

	const bool alt = bitmap_type != TCACHE_TYPE_INTERFACE && tex_w < 256;

	if(bitmap_type == TCACHE_TYPE_AABITMAP || alt == false)
	{
		d3d_tcache_get_adjusted_texture_size(tex_w, tex_h, &tex_w, &tex_h);
	}

	if ( bitmap_type == TCACHE_TYPE_AABITMAP )	{
		t->u_scale = (float)bmap_w / (float)tex_w;
		t->v_scale = (float)bmap_h / (float)tex_h;
	} 
	else 
	{

		t->u_scale = 1.0f;
		t->v_scale = 1.0f;
	}
		

	dwWidth  = tex_w;
	dwHeight = tex_h;

	t->u_scale = ((float)bmap_w) / ((float)tex_w);
	t->v_scale = ((float)bmap_h) / ((float)tex_h);

	bmp_data = (ushort *)data;
	ubyte *bmp_data_byte = (ubyte*)data;

	// Create a surface in system memory and load texture into it.

	// Create a surface of the given format using the dimensions of the bitmap
	memset(&ddsd, 0, sizeof(DDSURFACEDESC));

	ddsd.dwSize = sizeof(DDSURFACEDESC);
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_SYSTEMMEMORY;
	ddsd.dwHeight = dwHeight;
	ddsd.dwWidth  = dwWidth;
	ddsd.ddpfPixelFormat = *surface_desc;

	sys_texture_surface = NULL;
	ddrval = lpDD->CreateSurface(&ddsd, &sys_texture_surface, NULL);
	if ( (ddrval != DD_OK) || (sys_texture_surface == NULL) ) {
		mprintf(("CreateSurface for texture failed (loadtex), w=%d, h=%d, %s\n", tex_w, tex_h, d3d_error_string(ddrval) ));
		mprintf(( "Texture RAM = %d KB\n", D3D_textures_in / 1024 ));
		// bm_unlock(bitmap_handle);
		return 0;
	}
	
	// Lock the surface so it can be filled with the bitmap data
	memset(&ddsd, 0, sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(DDSURFACEDESC);
	ddrval = sys_texture_surface->Lock(NULL, &ddsd, 0, NULL);
	if (ddrval != DD_OK) {
		sys_texture_surface->Release();
		mprintf(("Lock failed while loading surface (loadtex).\n" ));
		return 0;
	}

	
	if(bm_get_bpp(texture_handle) == 32 && bitmap_type != TCACHE_TYPE_AABITMAP && alt)
	{
	
		// Each RGB bit count requires different pointers
		uint *lpSP;	
		uint *bmp_dataUint = (uint *) bmp_data;
		/*
		uint xlat[256];
		int r, g, b, a;
		

		
		switch( bitmap_type )	{		
			case TCACHE_TYPE_AABITMAP:			
				// setup convenient translation table
				for (i=0; i<16; i++ )	{
					r = 255;
					g = 255;
					b = 255;
					a = Gr_gamma_lookup[(i*255)/15];
					r /= Gr_ta_red.scale;
					g /= Gr_ta_green.scale;
					b /= Gr_ta_blue.scale;
					a /= Gr_ta_alpha.scale;
					xlat[i] = unsigned short(((a<<Gr_ta_alpha.shift) | (r << Gr_ta_red.shift) | (g << Gr_ta_green.shift) | (b << Gr_ta_blue.shift)));
				}			
				
				xlat[15] = xlat[1];			
				for ( ; i<256; i++ )	{
					xlat[i] = xlat[0];						
				}			
				
				for (j = 0; j < tex_h; j++) {				
					lpSP = (unsigned short*)(((char*)ddsd.lpSurface) +	ddsd.lPitch * j);

					for (i = 0; i < tex_w; i++) {
						if ( (i < bmap_w) && (j<bmap_h) )	{						
							*lpSP++ = xlat[(ubyte)bmp_data_byte[j*bmap_w+i]];
						} else {
							*lpSP++ = 0;
						}
					}
				}
			break;

		default:	
		*/
		{	// normal:	
			{
				fix u, utmp, v, du, dv;

				u = v = 0;

				du = ( (bmap_w-1)*F1_0 ) / tex_w;
				dv = ( (bmap_h-1)*F1_0 ) / tex_h;
												
				for (j = 0; j < tex_h; j++) {
					lpSP = (uint *)(((char*)ddsd.lpSurface) +	ddsd.lPitch * j);

					utmp = u;				
					
					for (i = 0; i < tex_w; i++) {
						*lpSP++ = bmp_dataUint[f2i(v)*bmap_w+f2i(utmp)];
						utmp += du;
					}
					v += dv;
				}
			}
			//break;
		}
	}
	else if(bm_get_bpp(texture_handle) == 32 && true)
	{
		const int pixelStride = surface_desc->dwRGBBitCount / 8;
#ifdef _DEBUG
		memset((void * ) ddsd.lpSurface, 0xff, dwWidth * dwHeight * pixelStride);
#endif
		for(int j = 0; j < bmap_h; j++)
		{
			ubyte *dstCol = ((ubyte *) ddsd.lpSurface) + j * dwWidth * pixelStride;
			ubyte *srcCol = ((ubyte *) bmp_data_byte)  + j * bmap_w * pixelStride;
			memcpy(dstCol, srcCol, bmap_w * pixelStride);
		}
	}
	else if(surface_desc->dwRGBBitCount == 16 )
	{
		// Each RGB bit count requires different pointers
		ushort *lpSP;	
		ushort xlat[256];
		int r, g, b, a;
		switch( bitmap_type )	{		
			case TCACHE_TYPE_AABITMAP:			
				// setup convenient translation table
				for (i=0; i<16; i++ )	{
					r = 255;
					g = 255;
					b = 255;
					a = Gr_gamma_lookup[(i*255)/15];
					r /= Gr_ta_red.scale;
					g /= Gr_ta_green.scale;
					b /= Gr_ta_blue.scale;
					a /= Gr_ta_alpha.scale;
					xlat[i] = unsigned short(((a<<Gr_ta_alpha.shift) | (r << Gr_ta_red.shift) | (g << Gr_ta_green.shift) | (b << Gr_ta_blue.shift)));
				}			
				
				xlat[15] = xlat[1];			
				for ( ; i<256; i++ )	{
					xlat[i] = xlat[0];						
				}			
				
				for (j = 0; j < tex_h; j++) {				
					lpSP = (unsigned short*)(((char*)ddsd.lpSurface) +	ddsd.lPitch * j);

					for (i = 0; i < tex_w; i++) {
						if ( (i < bmap_w) && (j<bmap_h) )	{						
							*lpSP++ = xlat[(ubyte)bmp_data_byte[j*bmap_w+i]];
						} else {
							*lpSP++ = 0;
						}
					}
				}
			break;

		default:		{	// normal:		
				fix u, utmp, v, du, dv;

				u = v = 0;

				du = ( (bmap_w-1)*F1_0 ) / tex_w;
				dv = ( (bmap_h-1)*F1_0 ) / tex_h;
												
				for (j = 0; j < tex_h; j++) {
					lpSP = (unsigned short*)(((char*)ddsd.lpSurface) +	ddsd.lPitch * j);

					utmp = u;				
					
					for (i = 0; i < tex_w; i++) {
						*lpSP++ = bmp_data[f2i(v)*bmap_w+f2i(utmp)];
						utmp += du;
					}
					v += dv;
				}
			}
			break;
		}
	}
	

	// bm_unlock(bitmap_handle);

	// Unlock the texture 
	sys_texture_surface->Unlock(NULL);

	sys_texture = NULL;
#if !defined(_WIN64)
	ddrval = sys_texture_surface->QueryInterface(IID_IDirect3DTexture2, (LPVOID *)&sys_texture);
#endif
	if ( (ddrval != DD_OK) || (sys_texture == NULL) ) {
		mprintf(( "Getting sys surface's texture failed!\n" ));

		// bad return value
		ret_val = 0;

		goto FreeSurfacesAndExit;
	}

RetryLoad:

	if ( !reload )	{	
		// Create a surface of the given format using the dimensions of the bitmap
		memset(&ddsd, 0, sizeof(DDSURFACEDESC));

		ddsd.dwSize = sizeof(DDSURFACEDESC);
		ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_ALLOCONLOAD | DDSCAPS_VIDEOMEMORY;
		//| DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM | DDSCAPS_ALLOCONLOAD;
		ddsd.dwHeight = dwHeight;
		ddsd.dwWidth = dwWidth;
		ddsd.ddpfPixelFormat = *surface_desc;

		t->vram_texture_surface = NULL;
		ddrval = lpDD->CreateSurface(&ddsd, &t->vram_texture_surface, NULL);
		if ( (ddrval != DD_OK) || (t->vram_texture_surface == NULL) ) {
			t->vram_texture = NULL;
			t->vram_texture_surface = NULL;
			t->texture_handle = NULL;

			if ( ddrval==DDERR_OUTOFVIDEOMEMORY )	{
				mprintf(("Out of VRAM (w=%d, h=%d, used=%d KB)\n", tex_w, tex_h, D3D_textures_in / 1024 ));
				if ( fail_on_full )	{
					// bad return value
					ret_val = 0;

					goto FreeSurfacesAndExit;
				}
				if ( d3d_free_some_texture_ram(t,dwHeight*dwWidth*2))	{
					goto RetryLoad;
				}
			} else {
				mprintf(("CreateSurface for VRAM texture failed, w=%d, h=%d\n%s\n", tex_w, tex_h, d3d_error_string(ddrval) ));
				mprintf(( "Texture RAM = %d KB\n", D3D_textures_in / 1024 ));
			}
			vram_full = 1;

			// bad return value
			ret_val = 0;

			goto FreeSurfacesAndExit;
			//goto RetryLoad;
		}

		t->vram_texture = NULL;
#if !defined(_WIN64)
		ddrval = t->vram_texture_surface->QueryInterface(IID_IDirect3DTexture2, (LPVOID *)&t->vram_texture);
#endif
		if ( (ddrval != DD_OK) || (t->vram_texture == NULL) )	{
			mprintf(( "GR_D3D_INIT: TextureSurface->QueryInterface failed.\n" ));
			vram_full = 1;

			// bad return value
			ret_val = 0;

			goto FreeSurfacesAndExit;
		}

		//	char *name = bm_get_filename(bitmap_handle);
		//	mprintf(( "Uploading '%s'\n", name ));
		t->texture_handle = NULL;
		ddrval = t->vram_texture->GetHandle(lpD3DDevice, &t->texture_handle );
		if ( (ddrval != DD_OK) || (t->texture_handle == NULL) )	{
			mprintf(( "GR_D3D_INIT: Texture->GetHandle failed.\n" ));
			t->texture_handle = NULL;
			vram_full = 1;

			// bad return value
			ret_val = 0;

			goto FreeSurfacesAndExit;
		}
	}

	// argh. this texture appears to be bogus. lets free it
	if(t->vram_texture == NULL){
		d3d_free_texture(t);
			
		// bad
		ret_val = 0;
		
		goto FreeSurfacesAndExit;
	}

	ddrval = t->vram_texture->Load( sys_texture );
	if ( ddrval != DD_OK ) {
		mprintf(("VRAM Load failed, w=%d, h=%d, %s\n", tex_w, tex_h, d3d_error_string(ddrval) ));
		mprintf(( "Texture RAM = %d KB\n", D3D_textures_in / 1024 ));
		vram_full = 1;

		// bad return value
		ret_val = 0;

		goto FreeSurfacesAndExit;
	}

	t->bitmap_id = texture_handle;
	t->time_created = D3D_frame_count;
	t->used_this_frame = 0;	
	t->size = dwWidth * dwHeight * surface_desc->dwRGBBitCount / 8;	
	t->w = (ushort)dwWidth;
	t->h = (ushort)dwHeight;
	D3D_textures_in_frame += t->size;
	if ( !reload )	{	
		D3D_textures_in += t->size;
	}

FreeSurfacesAndExit:
	if ( sys_texture )	{
		sys_texture->Release();
		sys_texture = NULL;
	}

	if ( sys_texture_surface )	{
		sys_texture_surface->Release();
		sys_texture_surface = NULL;
	}

	// hopefully this is 1  :)
	return ret_val;
}

int d3d_create_texture(int bitmap_handle, int bitmap_type, tcache_slot_d3d *tslot, int fail_on_full)
{
	ubyte flags;
	bitmap *bmp;
	int final_w, final_h;
	int reload = 0;

	int bpp = kDefaultBpp;
	if(bm_get_type(bitmap_handle) == BM_TYPE_USER 
	// protect against bpp == 0!!!
		&& bm_get_bpp(bitmap_handle) != 0) 
	{
		bpp = bm_get_bpp(bitmap_handle);
	}

	// setup texture/bitmap flags
	flags = 0;
	switch(bitmap_type){
	case TCACHE_TYPE_AABITMAP:
		flags |= BMP_AABITMAP;
		bpp = 8;
		break;
	case TCACHE_TYPE_NORMAL:
		flags |= BMP_TEX_OTHER;
	case TCACHE_TYPE_INTERFACE:
	case TCACHE_TYPE_XPARENT:
		{
		flags |= BMP_TEX_XPARENT;	
		break;
		}
	}
	
	// lock the bitmap into the proper format
	bmp = bm_lock(bitmap_handle, bpp, flags);
	if ( bmp == NULL ) {
		mprintf(("Couldn't lock bitmap %d.\n", bitmap_handle ));    
		return 0;
	}

	int max_w = bmp->w;
	int max_h = bmp->h; 

	// get final texture size as it will be allocated as a DD surface
	d3d_tcache_get_adjusted_texture_size(max_w, max_h, &final_w, &final_h);

	// if this tcache slot has no bitmap
	if ( tslot->bitmap_id < 0) {			
		reload = 0;
	}
	// different bitmap altogether - determine if the new one can use the old one's slot
	else if (tslot->bitmap_id != bitmap_handle)	{
		if((final_w == tslot->w) && (final_h == tslot->h)){
			reload = 1;

			ml_printf("Reloading texture %d\n", bitmap_handle);
		} else {
			reload = 0;
		}
	}

	// call the helper
	int ret_val = d3d_create_texture_sub(bitmap_type, bitmap_handle, (ushort*)bmp->data, bmp->w, bmp->h, max_w, max_h, tslot, reload, fail_on_full);

	// unlock the bitmap
	bm_unlock(bitmap_handle);

	return ret_val;
}

int d3d_tcache_set(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full, const bool justCacheNoSet)
{
	int ret_val = 1;

	if ( bitmap_id < 0 )	{
		D3D_last_bitmap_id  = -1;
		return 0;
	}

	if ( D3D_last_detail != Detail.hardware_textures )	{
		D3D_last_detail = Detail.hardware_textures;
		d3d_tcache_flush();
	}

	//mprintf(( "Setting texture %d\n", bitmap_handle ));
	if ( vram_full ) {
		return 0;
	}

	int n = bm_get_cache_slot( bitmap_id, 1 );
	tcache_slot_d3d * t = &Textures[n];		
	
	if ( (D3D_last_bitmap_id == bitmap_id) && (D3D_last_bitmap_type==bitmap_type) && (t->bitmap_id == bitmap_id))	{
		t->used_this_frame++;

		int bmap_w = 0;
		bm_get_info(bitmap_id, &bmap_w);

		if(u_scale)
			*u_scale = t->u_scale;
		if(v_scale)
			*v_scale = t->v_scale;
		return 1;
	}	
	// all other "normal" textures
	//else 
	if((bitmap_id < 0) || (bitmap_id != t->bitmap_id)){		
		ret_val = d3d_create_texture( bitmap_id, bitmap_type, t, fail_on_full );		
	}			

	// everything went ok
	if(ret_val && (t->texture_handle != NULL) && !vram_full)
	{
		if(u_scale)
			*u_scale = t->u_scale;
		if(v_scale)
			*v_scale = t->v_scale;

		if(justCacheNoSet == false)
		{
			d3d_SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, t->texture_handle );
			
			D3D_last_bitmap_id = t->bitmap_id;
			D3D_last_bitmap_type = bitmap_type;

			t->used_this_frame++;	
		}

		return 1;
	}
	
	return 0;
}

void d3d_tcache_init()
{
	int i;

	{
		DDSCAPS ddsCaps;
		DWORD dwFree, dwTotal;

		memset(&ddsCaps,0,sizeof(ddsCaps) );
		ddsCaps.dwCaps = DDSCAPS_TEXTURE;
		HRESULT ddrval = lpDD->GetAvailableVidMem(&ddsCaps, &dwTotal, &dwFree);
		if ( ddrval != DD_OK )	{
			mprintf(( "GR_D3D_INIT: GetAvailableVidMem failed.\n" ));
			dwFree = 0;
//			goto D3DError;
		}

		D3D_texture_ram = dwFree;


#ifndef NDEBUG
		int megs = dwFree / (1024*1024);		
		mprintf(( "TEXTURE RAM: %d bytes (%d MB) available\n", dwFree, megs ));
#endif
	}

	
  	D3D_min_texture_width = (int)lpDevDesc->dwMinTextureWidth;
	D3D_max_texture_width = (int)lpDevDesc->dwMaxTextureWidth;
	D3D_min_texture_height = (int)lpDevDesc->dwMinTextureHeight;
	D3D_max_texture_height = (int)lpDevDesc->dwMaxTextureHeight;

	// The following checks are needed because the PowerVR reports
	// it's texture caps as 0,0 up to 0,0.
	if ( D3D_min_texture_width < 16 )	{
		D3D_min_texture_width = 16;
	}
	if ( D3D_min_texture_height < 16 )	{
		D3D_min_texture_height = 16;
	}
	if ( D3D_max_texture_width < 16 )	{
		mprintf(( "Driver claims to have a max texture width of %d.  Bashing to 256.\n(Is this a PowerVR?)\n", D3D_max_texture_width ));
		D3D_max_texture_width = 256;					// Can we assume 256?
	}

	if ( D3D_max_texture_height < 16 )	{
		mprintf(( "Driver claims to have a max texture height of %d.  Bashing to 256.\n(Is this a PowerVR?)\n", D3D_max_texture_height ));
		D3D_max_texture_height = 256;					// Can we assume 256?
	}
	mprintf(( "Large textures enabled! %d %d\n", D3D_max_texture_width, D3D_max_texture_height ));
	
	Textures = (tcache_slot_d3d *)malloc(MAX_BITMAPS*sizeof(tcache_slot_d3d));
	if ( !Textures )	{
		exit(1);
	}

	// Init the texture structures
	for( i=0; i<MAX_BITMAPS; i++ )	{
		Textures[i].vram_texture = NULL;
		Textures[i].vram_texture_surface = NULL;
		Textures[i].texture_handle = NULL;

		Textures[i].bitmap_id = -1;
		Textures[i].size = 0;
		Textures[i].used_this_frame = 0; 

		Textures[i].parent = NULL;
	}

	D3D_last_detail = Detail.hardware_textures;
	D3D_last_bitmap_id = -1;
	D3D_last_bitmap_type = -1;

	D3D_textures_in = 0;
	D3D_textures_in_frame = 0;

}

void d3d_tcache_flush()
{
	int i; 


	for( i=0; i<MAX_BITMAPS; i++ )	{
		d3d_free_texture( &Textures[i] );		
	}
	if ( D3D_textures_in != 0 )	{
		mprintf(( "WARNING: VRAM is at %d instead of zero after flushing!\n", D3D_textures_in ));
		D3D_textures_in = 0;
	}

	D3D_last_bitmap_id = -1;
}

void d3d_tcache_cleanup()
{
	d3d_tcache_flush();
	
	D3D_textures_in = 0;
	D3D_textures_in_frame = 0;

	if ( Textures )	{
		free(Textures);
		Textures = NULL;
	}
}

void d3d_tcache_frame()
{
	D3D_last_bitmap_id = -1;
	D3D_textures_in_frame = 0;

	D3D_frame_count++;

	int i;
	for( i=0; i<MAX_BITMAPS; i++ )	{
		Textures[i].used_this_frame = 0; 
	}

	if ( vram_full )	{
		d3d_tcache_flush();
		vram_full = 0;
	}
}

// call this to safely fill in the texture shift and scale values for the specified texture type (Gr_t_*)
void gr_d3d_get_tex_format(int alpha)
{
	/*
	DDPIXELFORMAT *surface_desc;
	int s;	
	// RGB decoder
	unsigned long m;		

	// get the proper texture format
	if(alpha){	
		surface_desc = &AlphaTextureFormat;
	} else {	
		surface_desc = &NonAlphaTextureFormat;
	}

	// Determine the red, green and blue masks' shift and scale.
	for (s = 0, m = surface_desc->dwRBitMask; !(m & 1); s++, m >>= 1);
	Gr_t_red_shift = s;
	Gr_t_red_scale = 255 / (surface_desc->dwRBitMask >> s);
	for (s = 0, m = surface_desc->dwGBitMask; !(m & 1); s++, m >>= 1);
	Gr_t_green_shift = s;
	Gr_t_green_scale = 255 / (surface_desc->dwGBitMask >> s);
	for (s = 0, m = surface_desc->dwBBitMask; !(m & 1); s++, m >>= 1);
	Gr_t_blue_shift = s;
	Gr_t_blue_scale = 255 / (surface_desc->dwBBitMask >> s);

	if ( surface_desc->dwFlags & DDPF_ALPHAPIXELS ) {
		for (s = 0, m = surface_desc->dwRGBAlphaBitMask; !(m & 1); s++, m >>= 1);
		Gr_t_alpha_shift = s;
		Gr_t_alpha_scale = 255 / (surface_desc->dwRGBAlphaBitMask >> s);
	} else {
		Gr_t_alpha_shift = 0;
		Gr_t_alpha_scale = 256;
	}
	*/
}