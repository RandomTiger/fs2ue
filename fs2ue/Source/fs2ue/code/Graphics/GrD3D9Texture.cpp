#ifndef FS2_UE
#ifndef UNITY_BUILD
#include "Graphics/GrD3D9Texture.h"

#include <windows.h>
#include <D3d9.h>
#include <d3d9types.h>

#include <vector>
#include <assert.h>

#include "Graphics/GrD3D9.h"
//#include "Graphics/GrD3D9Render.h"

#include "2d.h"
#include "GrInternal.h"
#include "bmpman.h"
#endif

int D3D9_last_bitmap_id   = -1;
int D3D9_last_bitmap_type = -1;

int D3D9_max_texture_width  = -1;
int D3D9_max_texture_height = -1;

int D3D3_textures_in = 0;

struct Texture 
{
	Texture() : 
		m_width(0),
		m_height(0),
		m_bitmapID(-1),
		m_size(0),

		m_handle(0), 
		m_uScale(0),
		m_vScale(0),
		
		m_usedThisFrameCount(false)
	{
	}

	int					m_width; 
	int					m_height;
	int					m_bitmapID;
	int					m_size;

	IDirect3DTexture9*	m_handle;
	float				m_uScale, m_vScale;
	bool				m_usedThisFrameCount;

};

std::vector<Texture> gTextures;

bool d3d_free_texture( Texture *t );

bool SetupTextureSystem()
{
	gTextures.reserve(MAX_BITMAPS);
	gTextures.resize(MAX_BITMAPS);

	D3DCAPS9 caps;
	GetDevice()->GetDeviceCaps(&caps);

	D3D9_max_texture_width  = caps.MaxTextureWidth;
	D3D9_max_texture_height = caps.MaxTextureHeight;

	ResetTextureSystem();
	D3D3_textures_in = 0;
	return true;
}

void CleanupTextureSystem()
{
	for(unsigned int i = 0; i < gTextures.size(); i++)
	{
		d3d_free_texture(&gTextures[i]);
	}
	gTextures.clear();
}

void ResetTextureSystem()
{
	D3D9_last_bitmap_id = -1;
	D3D9_last_bitmap_type = -1;
}

// get the final texture size (the one which will get allocated as a surface)
void d3d9_tcache_get_adjusted_texture_size(int w_in, int h_in, int *w_out, int *h_out)
{
	// starting size
	int tex_w = w_in;
	int tex_h = h_in;

	// bogus
	if((w_out == NULL) ||  (h_out == NULL)){
		return;
	}

	//if ( D3D_pow2_textures )	
	{
		int i;
		const int kCheckPower = 16;
		for (i=0; i<kCheckPower; i++ )	{
			const int checkSize = 1<<i;
			const int nextSize = 1<<(i+1);

			if ( tex_w > checkSize && tex_w <= nextSize ) {
				tex_w = nextSize;
				break;
			}
		}

		for (i=0; i<kCheckPower; i++ )	{
			const int checkSize = 1<<i;
			const int nextSize = 1<<(i+1);

			if ( tex_h > checkSize && tex_h <= nextSize ) {
				tex_h = nextSize;
				break;
			}
		}
	}

	int D3D_min_texture_width = 16;
	int D3D_min_texture_height = 16;

	if ( tex_w < D3D_min_texture_width ) {
		tex_w = D3D_min_texture_width;
	} 
	else if ( tex_w > D3D9_max_texture_width )	{
		tex_w = D3D9_max_texture_width;
	}

	if ( tex_h < D3D_min_texture_height ) {
		tex_h = D3D_min_texture_height;
	} 
	else if ( tex_h > D3D9_max_texture_height )	{
		tex_h = D3D9_max_texture_height;
	}

	// store the outgoing size
	*w_out = tex_w;
	*h_out = tex_h;
}

bool d3d_free_texture( Texture *t )
{
	// Bitmap changed!!	
	if ( t->m_bitmapID > -1 && t->m_handle)	{				
		// if I, or any of my children have been used this frame, bail		
		if(t->m_usedThisFrameCount){			
			return false;
		}		

		t->m_handle->Release();
		t->m_handle = 0;

		if ( D3D9_last_bitmap_id == t->m_bitmapID )	{
			D3D9_last_bitmap_id = -1;
		}		

		t->m_bitmapID = -1;
		t->m_usedThisFrameCount = 0;
		D3D3_textures_in -= t->m_size;		
	}

	return true;
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
bool d3d_create_texture_sub(int bitmap_type, int texture_handle, ushort *data, int bmap_w, int bmap_h, int tex_w, int tex_h, Texture *t, int reload, int fail_on_full)
{
	#ifndef NDEBUG
	//	if ( Show_uploads )	
		{
			if ( reload )	{
				mprintf(( "Reloading '%s'\n", bm_get_filename(texture_handle) ));
			} else {
				mprintf(( "Uploading '%s'\n", bm_get_filename(texture_handle) ));
			}
		}
	#endif

	// bogus
	if(t == 0){
		return false;
	}
		
	if ( t->m_usedThisFrameCount )	{
		mprintf(( "ARGHH!!! Texture already used this frame!  Cannot free it!\n" ));
		return false;
	}	
	if ( !reload )	{
		// gah
		if(!d3d_free_texture(t)){
			return false;
		}
	}

	DWORD dwHeight, dwWidth;
	int i,j;	
	ushort *bmp_data;

	const int bpp = bm_get_bpp(texture_handle);
	bool alphaOn = true;
	if(bpp != 32 && bitmap_type == TCACHE_TYPE_XPARENT)
	{
		alphaOn = false;
	}

	const bool alt = bitmap_type != TCACHE_TYPE_INTERFACE && tex_w < 256;

	if(bitmap_type == TCACHE_TYPE_AABITMAP || alt == false)
	{
		d3d9_tcache_get_adjusted_texture_size(tex_w, tex_h, &tex_w, &tex_h);
	}

	if ( bitmap_type == TCACHE_TYPE_AABITMAP )	
	{
		t->m_uScale = (float)bmap_w / (float)tex_w;
		t->m_vScale = (float)bmap_h / (float)tex_h;
	} 
	else 
	{
		t->m_uScale = 1.0f;
		t->m_vScale = 1.0f;
	}

	dwWidth  = tex_w;
	dwHeight = tex_h;

	t->m_uScale = ((float)bmap_w) / ((float)tex_w);
	t->m_vScale = ((float)bmap_h) / ((float)tex_h);

	bmp_data = (ushort *)data;
	ubyte *bmp_data_byte = (ubyte*)data;

	HRESULT hr = S_OK;
	if ( !reload )	{
		assert(t->m_handle == 0);
		// Create a surface and load texture into it.
		const int kLevels = 1;
		mprintf(("Creating texutre: %d %d\n", dwWidth , dwHeight));
		hr = GetDevice()->CreateTexture(dwWidth , dwHeight, kLevels, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &(t->m_handle), 0);
		assert(SUCCEEDED(hr));
		assert(t->m_handle);
	}

	const int kLockLevel = 0;
	D3DLOCKED_RECT region;
	hr = t->m_handle->LockRect(kLockLevel, &region, 0, 0);
	assert(SUCCEEDED(hr));

	bool result = true;

	if(bm_get_bpp(texture_handle) == 32 && bitmap_type != TCACHE_TYPE_AABITMAP && alt)
	{
		// Each RGB bit count requires different pointers
		uint *lpSP;	
		uint *bmp_dataUint = (uint *) bmp_data;
		
		fix u, utmp, v, du, dv;

		u = v = 0;

		du = ( (bmap_w-1)*F1_0 ) / tex_w;
		dv = ( (bmap_h-1)*F1_0 ) / tex_h;
										
		for (j = 0; j < tex_h; j++) 
		{
			lpSP = (uint *)(((char*)region.pBits) +	region.Pitch * j);

			utmp = u;				
			
			for (i = 0; i < tex_w; i++) {
				*lpSP++ = bmp_dataUint[f2i(v)*bmap_w+f2i(utmp)];
				utmp += du;
			}
			v += dv;
		}
	}
	else if(bm_get_bpp(texture_handle) == 32)
	{
		const int pixelStride = bpp / 8;
#ifdef _DEBUG
		memset((void * ) region.pBits, 0xff, dwWidth * dwHeight * pixelStride);
#endif
		for(int j = 0; j < bmap_h; j++)
		{
			ubyte *dstCol = ((ubyte *) region.pBits) + j * dwWidth * pixelStride;
			ubyte *srcCol = ((ubyte *) bmp_data_byte)  + j * bmap_w * pixelStride;
			memcpy(dstCol, srcCol, bmap_w * pixelStride);
		}
	}
	else
	{
		// Each RGB bit count requires different pointers
		D3DCOLOR *lpSP;	
		D3DCOLOR xlat[256];

		switch( bitmap_type )	{		
			case TCACHE_TYPE_AABITMAP:			
				// setup convenient translation table
				for (i=0; i<16; i++ )	{
					const int a = (i*255)/15;
					xlat[i] = D3DCOLOR_RGBA(255,255,255, a);
				}			
				
				xlat[15] = xlat[1];			
				for ( ; i<256; i++ )	{
					xlat[i] = xlat[0];						
				}			
				
				for (j = 0; j < tex_h; j++, lpSP++) {				
					lpSP = (D3DCOLOR *)(((char*)region.pBits) + region.Pitch * j);

					for (i = 0; i < tex_w; i++) {
						if ( (i < bmap_w) && (j<bmap_h) )	{						
							*lpSP = xlat[(ubyte)bmp_data_byte[j*bmap_w+i]];
						} else {
							*lpSP = 0;
						}
						lpSP++;
					}
				}
			break;

		default:		{	// normal:		
				fix u, utmp, v, du, dv;

				u = v = 0;

				du = ( (bmap_w-1)*F1_0 ) / tex_w;
				dv = ( (bmap_h-1)*F1_0 ) / tex_h;
												
				for (j = 0; j < tex_h; j++) {
					lpSP = (D3DCOLOR *)(((char*)region.pBits) +	region.Pitch * j);

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

	hr = t->m_handle->UnlockRect(kLockLevel);
	assert(SUCCEEDED(hr));

	t->m_bitmapID = texture_handle;
//	t->time_created = D3D_frame_count;
	t->m_usedThisFrameCount = 0;	
	t->m_size = dwWidth * dwHeight * bpp / 8;	
	t->m_width  = dwWidth;
	t->m_height = dwHeight;
//	D3D_textures_in_frame += t->m_size;
	if ( !reload )	
	{	
		D3D3_textures_in += t->m_size;
	}

	return result;
}


bool d3d_create_texture(int bitmap_handle, int bitmap_type, Texture *tslot, int fail_on_full)
{
	int bpp = kDefaultBpp;
	if(bm_get_type(bitmap_handle) == BM_TYPE_USER 
	// protect against bpp == 0!!!
		&& bm_get_bpp(bitmap_handle) != 0) 
	{
		bpp = bm_get_bpp(bitmap_handle);
	}

	// setup texture/bitmap flags
	ubyte flags = 0;
	switch(bitmap_type){
	case TCACHE_TYPE_AABITMAP:
		flags |= BMP_AABITMAP;
		bpp = 8;
		break;
	case TCACHE_TYPE_NORMAL:
		flags |= BMP_TEX_OTHER;
	case TCACHE_TYPE_INTERFACE:
	case TCACHE_TYPE_XPARENT:
		flags |= BMP_TEX_XPARENT;	
		break;
	}
	
	// lock the bitmap into the proper format
	bitmap *bmp = bm_lock(bitmap_handle, bpp, flags);
	if ( bmp == NULL ) {
		mprintf(("Couldn't lock bitmap %d.\n", bitmap_handle ));    
		return false;
	}

	int max_w = bmp->w;
	int max_h = bmp->h; 

	int final_w, final_h;
	// get final texture size as it will be allocated as a DD surface
	d3d9_tcache_get_adjusted_texture_size(max_w, max_h, &final_w, &final_h);

	bool reload = false;
	// if this tcache slot has no bitmap
	if ( tslot->m_bitmapID < 0) {			
		reload = false;
	}
	// different bitmap altogether - determine if the new one can use the old one's slot
	else if (tslot->m_bitmapID != bitmap_handle)	
	{
		reload = (final_w == tslot->m_width) && (final_h == tslot->m_height);
		if(reload)
		{
			mprintf(("Reloading texture %d\n", bitmap_handle));
		}
	}

	// call the helper
	const bool result = 
		d3d_create_texture_sub(bitmap_type, bitmap_handle, 
			(ushort*)bmp->data, 
			bmp->w, bmp->h, 
			max_w, max_h, 
			tslot, reload, fail_on_full);

	// unlock the bitmap
	bm_unlock(bitmap_handle);

	return result;
}


int gr_d3d9_tcache_set(int bitmap_id, int bitmap_type, float *u_scale, float *v_scale, int fail_on_full, const bool justCacheNoSet)
{
	if ( bitmap_id < 0 )	{
		D3D9_last_bitmap_id  = -1;
		return false;
	}
/*
	if ( D3D_last_detail != Detail.hardware_textures )	{
		D3D_last_detail = Detail.hardware_textures;
		d3d_tcache_flush();
	}
*/
	const int n = bm_get_cache_slot( bitmap_id, 1 );
	Texture *t = &gTextures[n];		
	
	if ( (D3D9_last_bitmap_id == bitmap_id) && 
		(D3D9_last_bitmap_type==bitmap_type) && 
		(t->m_bitmapID == bitmap_id))	
	{
		t->m_usedThisFrameCount++;

		if(u_scale)
			*u_scale = t->m_uScale;
		if(v_scale)
			*v_scale = t->m_vScale;
		return 1;
	}	

	bool result = true;
	if((bitmap_id < 0) || (bitmap_id != t->m_bitmapID)){		
		result = d3d_create_texture( bitmap_id, bitmap_type, t, fail_on_full );		
	}			

	// everything went ok
	if(result && t->m_handle)
	{
		if(u_scale)
			*u_scale = t->m_uScale;
		if(v_scale)
			*v_scale = t->m_vScale;

		if(justCacheNoSet == false)
		{
			HRESULT hr = SetTexutre(0, t->m_handle);
			Assert(SUCCEEDED(hr));
			
			D3D9_last_bitmap_id   = t->m_bitmapID;
			D3D9_last_bitmap_type = bitmap_type;

			t->m_usedThisFrameCount++;	
		}

		return true;
	}
	
	return false;
}

HRESULT SetTexutre(int value, IDirect3DTexture9 *ptexture)
{
	return GetDevice()->SetTexture(0, ptexture);
}

void gr_d3d9_tcache_flush()
{
}

void d3d9_tcache_frame()
{
	D3D9_last_bitmap_id = -1;
//	D3D9_textures_in_frame = 0;

//	D3D9_frame_count++;

	int i;
	for( i=0; i<MAX_BITMAPS; i++ )	{
		gTextures[i].m_usedThisFrameCount = 0; 
	}

}
#endif