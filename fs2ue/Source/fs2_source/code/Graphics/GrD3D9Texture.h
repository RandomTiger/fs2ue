#pragma once

struct IDirect3DTexture9;
struct vertex;

bool SetupTextureSystem();
void CleanupTextureSystem();
void ResetTextureSystem();

void d3d9_tcache_frame();
void gr_d3d9_tcache_flush();
int gr_d3d9_tcache_set(int bitmap_id, int bitmap_type, float *u_ratio, float *v_ratio, int fail_on_full, const bool justCacheNoSet);

HRESULT SetTexutre(int value, IDirect3DTexture9 *ptexture);